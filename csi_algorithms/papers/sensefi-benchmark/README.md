# SenseFi: A Library and Benchmark for Deep Learning in Wi-Fi Sensing

- **Type:** benchmark and software paper
- **Source:** [paper](https://arxiv.org/abs/2207.07859), [code](https://github.com/xyanchen/WiFi-CSI-Sensing-Benchmark)
- **Use here:** model comparison and reproducible evaluation

## Algorithm findings

SenseFi provides a common pipeline for comparing deep models on Wi-Fi sensing
datasets. This is useful for benchmarking CNN/LSTM-style alternatives after a
dataset has been collected and labeled.

## Relevance and limitations

It is a research-PC benchmark, not an ESP32 firmware algorithm. Its reported
accuracy does not establish that the same model works with the ESP32 raw CSI
format or the current UDP stream.
