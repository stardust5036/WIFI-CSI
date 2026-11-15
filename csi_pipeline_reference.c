/*
 * CSI raw data -> motion result reference pipeline.
 *
 * This is a standalone, readable reference implementation. The
 * esp_radar_motion_dec component used by Espressif is not reproduced here;
 * motion_feature() is a compatible replacement for experiments, not the
 * exact Espressif implementation.
 *
 * Input format:
 *   one CSI frame containing interleaved signed 8-bit IQ samples:
 *   [imag0, real0, imag1, real1, ...]
 *
 * Result:
 *   CSI -> amplitude -> per-subcarrier change -> jitter -> ACTIVE/INACTIVE
 */

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CSI_MAX_SUBCARRIERS 128
#define BASELINE_PACKETS 50
#define BASELINE_WINDOW_MS 2000
#define CONFIRM_COUNT 2
#define SCALE_FACTOR 1000.0f

typedef enum {
    MOTION_INACTIVE = 0,
    MOTION_ACTIVE = 1,
} motion_result_t;

typedef struct {
    uint32_t timestamp_ms;
    const int8_t *iq;
    size_t iq_bytes;
} csi_frame_t;

typedef struct {
    size_t subcarriers;
    float amplitude[CSI_MAX_SUBCARRIERS];
    float previous_amplitude[CSI_MAX_SUBCARRIERS];
    float feature[CSI_MAX_SUBCARRIERS];
    bool have_previous;
} csi_decoder_t;

typedef struct {
    bool initialized;
    uint32_t first_timestamp_ms;
    uint32_t sample_count;
    float baseline_sum;
    float baseline;
    float smooth;
    float noise;
    int active_hits;
    int inactive_hits;
    motion_result_t state;
} motion_fsm_t;

typedef struct {
    float jitter;
    float smooth;
    float baseline;
    float enter_threshold;
    float exit_threshold;
    motion_result_t state;
    bool event;
} motion_output_t;

static float clamp_float(float value, float low, float high)
{
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

static float lerp(float a, float b, float t)
{
    return a + (b - a) * clamp_float(t, 0.0f, 1.0f);
}

/*
 * Step 1: decode signed IQ and calculate CSI amplitude.
 *
 * Espressif's esp-radar performs the same physical conversion before its
 * motion decoder. The exact subcarrier selection depends on the Wi-Fi
 * signal mode and bandwidth, so this reference assumes already-filtered IQ.
 */
static bool decode_amplitude(csi_decoder_t *decoder, const csi_frame_t *frame)
{
    if (!decoder || !frame || !frame->iq || decoder->subcarriers == 0 ||
            decoder->subcarriers > CSI_MAX_SUBCARRIERS ||
            frame->iq_bytes < decoder->subcarriers * 2) {
        return false;
    }

    for (size_t i = 0; i < decoder->subcarriers; ++i) {
        int real = frame->iq[i * 2];
        int imag = frame->iq[i * 2 + 1];
        decoder->amplitude[i] = sqrtf((float)(real * real + imag * imag));
    }
    return true;
}

/*
 * Step 2: convert one CSI amplitude frame to a motion waveform.
 *
 * This replacement compares adjacent frames per subcarrier, normalizes by
 * the previous amplitude, and combines subcarriers using RMS. It suppresses
 * isolated spikes and returns the same useful semantic range as radar
 * jitter: approximately [0, 1], where larger means more change.
 */
static float motion_feature(csi_decoder_t *decoder)
{
    if (!decoder) {
        return 0.0f;
    }

    if (!decoder->have_previous) {
        memcpy(decoder->previous_amplitude, decoder->amplitude,
               decoder->subcarriers * sizeof(float));
        decoder->have_previous = true;
        return 0.0f;
    }

    float sum_squared = 0.0f;
    for (size_t i = 0; i < decoder->subcarriers; ++i) {
        float current = decoder->amplitude[i];
        float previous = decoder->previous_amplitude[i];
        float denominator = fmaxf(previous, 1.0f);
        float relative_change = fabsf(current - previous) / denominator;

        decoder->feature[i] = clamp_float(relative_change, 0.0f, 1.0f);
        sum_squared += decoder->feature[i] * decoder->feature[i];
        decoder->previous_amplitude[i] = current;
    }

    return clamp_float(sqrtf(sum_squared / (float)decoder->subcarriers),
                       0.0f, 1.0f);
}

/*
 * Step 3: reproduce the public esp_wifi_sensing FSM behavior.
 *
 * The production component first encodes jitter as:
 *   log(1 + jitter * 1000) * 1000
 * and then smooths it against a dynamic baseline.
 */
static bool process_csi_frame(csi_decoder_t *decoder,
                               motion_fsm_t *fsm,
                               const csi_frame_t *frame,
                               float sensitivity,
                               float active_jitter_min,
                               motion_output_t *output)
{
    if (!decoder || !fsm || !frame || !output || !decode_amplitude(decoder, frame)) {
        return false;
    }

    memset(output, 0, sizeof(*output));
    float jitter = motion_feature(decoder);
    output->jitter = jitter;

    /*
     * The first two seconds and at least 50 packets establish the background
     * center, matching the production component's initial calibration policy.
     */
    if (!fsm->initialized) {
        if (fsm->sample_count == 0) {
            fsm->first_timestamp_ms = frame->timestamp_ms;
        }
        fsm->baseline_sum += log1pf(jitter * SCALE_FACTOR) * SCALE_FACTOR;
        fsm->sample_count++;

        uint32_t elapsed = frame->timestamp_ms - fsm->first_timestamp_ms;
        if (elapsed >= BASELINE_WINDOW_MS &&
                fsm->sample_count >= BASELINE_PACKETS) {
            fsm->baseline = fsm->baseline_sum / (float)fsm->sample_count;
            fsm->smooth = fsm->baseline;
            fsm->initialized = true;
        }
        output->smooth = fsm->smooth;
        output->baseline = fsm->baseline;
        output->state = fsm->state;
        return true;
    }

    float encoded = log1pf(jitter * SCALE_FACTOR) * SCALE_FACTOR;
    float smooth_coef = lerp(0.06f, 0.38f, sensitivity);
    if (encoded < fsm->smooth) {
        smooth_coef = fminf(smooth_coef * 2.0f, 1.0f);
    }
    fsm->smooth += (encoded - fsm->smooth) * smooth_coef;

    float diff = fsm->smooth - fsm->baseline;
    float residual = fabsf(diff);
    float noise_coef = residual >= fsm->noise ? 0.18f : 0.18f * 0.35f;
    fsm->noise += (residual - fsm->noise) * noise_coef;

    bool background_like = residual <= fmaxf(fsm->baseline * 0.30f,
                                             fsm->noise * 0.18f);
    float baseline_coef = background_like ? 0.020f : 0.020f * 0.25f;
    fsm->baseline += clamp_float(diff,
                                 -fmaxf(fsm->baseline * 0.105f, 1000.0f),
                                  fmaxf(fsm->baseline * 0.30f, 1000.0f))
                     * baseline_coef;

    float threshold_ratio = lerp(0.90f, 0.20f, sensitivity);
    float reference = fmaxf(fmaxf(fsm->baseline, fsm->smooth), 1000.0f);
    float enter_threshold = threshold_ratio * reference * (1.0f + 0.28f * 0.65f)
                            + fsm->noise * 0.18f;
    float exit_threshold = threshold_ratio * reference * (1.0f - 0.28f)
                           + fsm->noise * (0.18f * 0.75f);

    output->smooth = fsm->smooth;
    output->baseline = fsm->baseline;
    output->enter_threshold = enter_threshold;
    output->exit_threshold = exit_threshold;
    output->state = fsm->state;

    if (active_jitter_min > 0.0f && jitter < active_jitter_min) {
        fsm->active_hits = 0;
        fsm->inactive_hits = 0;
        if (fsm->state == MOTION_ACTIVE) {
            fsm->state = MOTION_INACTIVE;
            output->event = true;
        }
        output->state = fsm->state;
        return true;
    }

    if (fsm->state == MOTION_INACTIVE) {
        if (diff > enter_threshold) {
            fsm->active_hits++;
            if (fsm->active_hits >= CONFIRM_COUNT) {
                fsm->state = MOTION_ACTIVE;
                fsm->active_hits = 0;
                fsm->inactive_hits = 0;
                output->event = true;
            }
        } else {
            fsm->active_hits = 0;
        }
    } else {
        if (diff < exit_threshold) {
            fsm->inactive_hits++;
            if (fsm->inactive_hits >= CONFIRM_COUNT) {
                fsm->state = MOTION_INACTIVE;
                fsm->inactive_hits = 0;
                fsm->active_hits = 0;
                output->event = true;
            }
        } else {
            fsm->inactive_hits = 0;
        }
    }

    output->state = fsm->state;
    return true;
}

static const char *motion_state_name(motion_result_t state)
{
    return state == MOTION_ACTIVE ? "ACTIVE" : "INACTIVE";
}

/*
 * Replace this loop with esp_wifi_set_csi_rx_cb() on ESP-IDF, or call it
 * while reading saved CSI frames from a file on a PC.
 */
int main(void)
{
    static const int8_t example_iq[] = {
        3, 4,  4, 3,  3, 4,  4, 3,
        3, 4,  4, 3,  3, 4,  4, 3,
    };

    csi_decoder_t decoder = {
        .subcarriers = sizeof(example_iq) / (2 * sizeof(example_iq[0])),
    };
    motion_fsm_t fsm = {0};

    csi_frame_t frame = {
        .timestamp_ms = 0,
        .iq = example_iq,
        .iq_bytes = sizeof(example_iq),
    };
    motion_output_t output;

    /*
     * A real application calls process_csi_frame() once per CSI callback.
     * This small example only demonstrates the API and data flow.
     */
    if (!process_csi_frame(&decoder, &fsm, &frame, 0.50f, 0.0f, &output)) {
        fprintf(stderr, "invalid CSI frame\n");
        return 1;
    }

    printf("jitter=%.6f state=%s\n",
           output.jitter, motion_state_name(output.state));
    return 0;
}
