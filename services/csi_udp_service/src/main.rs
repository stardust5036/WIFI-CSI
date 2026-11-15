use std::collections::HashMap;
use std::env;
use std::io;
use std::net::{Ipv4Addr, SocketAddr, UdpSocket};

const MAGIC: &[u8; 4] = b"CSIU";
const VERSION: u8 = 1;
const TYPE_RAW: u8 = 1;
const TYPE_RESULT: u8 = 2;
const HEADER_LEN: usize = 42;
const MAX_PACKET: usize = 4096;
const BASELINE_FRAMES: usize = 50;

#[derive(Clone, Copy, Default)]
struct FirmwareResult {
    motion: bool,
    presence: bool,
    jitter: f32,
    wander: f32,
}

struct ServiceState {
    previous: Vec<f32>,
    baseline_energy: f32,
    baseline_frames: usize,
    activity: f32,
    motion: bool,
    presence: bool,
}

impl ServiceState {
    fn new() -> Self {
        Self {
            previous: Vec::new(),
            baseline_energy: 0.0,
            baseline_frames: 0,
            activity: 0.0,
            motion: false,
            presence: false,
        }
    }

    fn update(&mut self, amplitudes: &[f32]) -> Option<(f32, f32)> {
        if amplitudes.is_empty() {
            return None;
        }

        let energy = amplitudes.iter().sum::<f32>() / amplitudes.len() as f32;
        if self.previous.len() != amplitudes.len() {
            self.previous = amplitudes.to_vec();
            return None;
        }

        let mut squared_change = 0.0f32;
        for (current, previous) in amplitudes.iter().zip(self.previous.iter()) {
            let denominator = previous.max(1.0);
            let change = ((current - previous).abs() / denominator).min(1.0);
            squared_change += change * change;
        }
        let jitter = (squared_change / amplitudes.len() as f32).sqrt();
        self.previous.copy_from_slice(amplitudes);

        if self.baseline_frames < BASELINE_FRAMES {
            self.baseline_energy += energy;
            self.baseline_frames += 1;
            if self.baseline_frames == BASELINE_FRAMES {
                self.baseline_energy /= BASELINE_FRAMES as f32;
            }
            return Some((jitter, 0.0));
        }

        let energy_deviation =
            (energy - self.baseline_energy).abs() / self.baseline_energy.max(1.0);
        self.activity = self.activity * 0.85 + (jitter + energy_deviation) * 0.15;

        if self.activity > 0.08 {
            self.motion = true;
        } else if self.activity < 0.035 {
            self.motion = false;
        }

        let presence_score = self.activity;
        if presence_score > 0.045 {
            self.presence = true;
        } else if presence_score < 0.020 {
            self.presence = false;
        }

        Some((jitter, presence_score))
    }
}

fn read_u16(data: &[u8], offset: usize) -> Option<u16> {
    Some(u16::from_le_bytes([*data.get(offset)?, *data.get(offset + 1)?]))
}

fn read_u32(data: &[u8], offset: usize) -> Option<u32> {
    Some(u32::from_le_bytes([
        *data.get(offset)?,
        *data.get(offset + 1)?,
        *data.get(offset + 2)?,
        *data.get(offset + 3)?,
    ]))
}

fn read_f32(data: &[u8], offset: usize) -> Option<f32> {
    Some(f32::from_bits(read_u32(data, offset)?))
}

fn mac_string(mac: &[u8; 6]) -> String {
    format!(
        "{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    )
}

fn decode_amplitudes(data_type: u8, payload: &[u8]) -> Vec<f32> {
    match data_type {
        0 => payload
            .chunks_exact(2)
            .map(|pair| {
                let imag = pair[0] as i8 as f32;
                let real = pair[1] as i8 as f32;
                (real * real + imag * imag).sqrt()
            })
            .collect(),
        1 => payload
            .chunks_exact(4)
            .map(|sample| {
                let imag = i16::from_le_bytes([sample[0], sample[1]]) as f32;
                let real = i16::from_le_bytes([sample[2], sample[3]]) as f32;
                (real * real + imag * imag).sqrt()
            })
            .collect(),
        _ => Vec::new(),
    }
}

fn parse_args() -> (Ipv4Addr, u16) {
    let args: Vec<String> = env::args().collect();
    let mut bind = Ipv4Addr::UNSPECIFIED;
    let mut port = 5005;
    let mut index = 1;
    while index < args.len() {
        match args[index].as_str() {
            "--bind" if index + 1 < args.len() => {
                bind = args[index + 1].parse().expect("invalid --bind address");
                index += 2;
            }
            "--port" if index + 1 < args.len() => {
                port = args[index + 1].parse().expect("invalid --port");
                index += 2;
            }
            _ => {
                eprintln!("usage: csi_udp_service [--bind 0.0.0.0] [--port 5005]");
                std::process::exit(2);
            }
        }
    }
    (bind, port)
}

fn main() -> io::Result<()> {
    let (bind, port) = parse_args();
    let socket = UdpSocket::bind(SocketAddr::from((bind, port)))?;
    println!("listening on {}:{}", bind, port);

    let mut packet = [0u8; MAX_PACKET];
    let mut states: HashMap<[u8; 6], ServiceState> = HashMap::new();
    let mut firmware: HashMap<[u8; 6], FirmwareResult> = HashMap::new();

    loop {
        let (length, sender) = socket.recv_from(&mut packet)?;
        if length < HEADER_LEN || &packet[0..4] != MAGIC || packet[4] != VERSION {
            continue;
        }

        let packet_type = packet[5];
        let payload_len = usize::from(read_u16(&packet, 8).unwrap_or(0));
        let sequence = read_u32(&packet, 12).unwrap_or(0);
        let data_type = packet[26];
        let mac: [u8; 6] = packet[20..26].try_into().unwrap();
        let jitter = read_f32(&packet, 30).unwrap_or(0.0);
        let wander = read_f32(&packet, 34).unwrap_or(0.0);
        let payload_end = HEADER_LEN.saturating_add(payload_len);
        if payload_end > length {
            eprintln!("dropping truncated packet from {}", sender);
            continue;
        }

        if packet_type == TYPE_RESULT {
            let result = FirmwareResult {
                motion: packet[27] != 0,
                presence: packet[28] != 0,
                jitter,
                wander,
            };
            firmware.insert(mac, result);
            continue;
        }

        if packet_type != TYPE_RAW {
            continue;
        }

        let amplitudes = decode_amplitudes(data_type, &packet[HEADER_LEN..payload_end]);
        let state = states.entry(mac).or_insert_with(ServiceState::new);
        let Some((service_jitter, presence_score)) = state.update(&amplitudes) else {
            continue;
        };
        let fw = firmware.get(&mac).copied().unwrap_or_default();

        println!(
            "peer={} seq={} fw_motion={} fw_presence={} fw_jitter={:.5} fw_wander={:.5} \
             service_motion={} service_presence={} service_jitter={:.5} presence_score={:.5}",
            mac_string(&mac),
            sequence,
            fw.motion,
            fw.presence,
            fw.jitter,
            fw.wander,
            state.motion,
            state.presence,
            service_jitter,
            presence_score
        );
    }
}
