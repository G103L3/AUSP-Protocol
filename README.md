# AUSP - Acoustic Underground Sensor Protocol

A lightweight, layered acoustic communication protocol for master-slave sensor networks operating in underground environments such as tunnels, mines, ducts, and pipelines.

> **Paper under review** - *"Acoustic Communication Protocol for Master-Slave Networks in Underground Environments"*, submitted to IEEE Internet of Things Journal. First author: Gioele Giunta (Politecnico di Torino / University of Catania).

---

## Why acoustic communication underground?

Radio-frequency solutions (UHF, SHF) suffer from severe attenuation in soil and confined spaces. Typical coverage is limited to 10-33 meters even in optimal conditions, with additional losses of around 30 dB at 90-degree corners. The physics works against RF underground.

Sound behaves differently. In enclosed ducts and tunnels, air acts as a natural waveguide. Medium-frequency acoustic signals (1-9 kHz) maintain a predictable, measurable attenuation over tens of meters. The protocol leverages the **Schroeder cutoff frequency** to characterise each environment and operate above the modal regime, where the sound beam phenomenon dominates and propagation becomes more controllable.

For a representative mining tunnel (5 x 5 m cross-section, 100 m length, approximately 3 s reverberation time):

```
f_s = 2000 * sqrt(T60 / V) = 69 Hz
```

Operating at 1-9 kHz places the system well above the Schroeder frequency, in the statistical regime, enabling predictable coverage with low-cost hardware.

---

## Protocol Architecture

AUSP is organized into four independent layers:

```
+--------------------------------------------+
|            Application Layer               |  High-level commands (ASCII instruction set)
+--------------------------------------------+
|               Link Layer                   |  Packet framing, addressing, token management
+--------------------------------------------+
|               Bit Layer                    |  Run-length encoding, signal code mapping
+--------------------------------------------+
|             Physical Layer                 |  FFT-based tone detection, sine synthesis, I2S
+--------------------------------------------+
```

---

## Physical Layer

Audio is sampled at **48 kHz / 16-bit** and processed in blocks of 512 samples:

```
T_block = 512 / 48000 = 10.7 ms
```

The FFT produces 512 bins with a frequency resolution of **93.75 Hz/bin**. A dynamic threshold rejects noise: a peak is valid only if its amplitude exceeds the average noise floor by at least 8x, and it must be a local maximum relative to the six adjacent bins on each side. Sub-bin precision is recovered via **parabolic peak interpolation**.

Transmission uses pairs of overlapping tones synthesized digitally, with phase persistence between consecutive emissions to avoid spectral discontinuities. Each tone pair is emitted for **24 ms** (1152 samples), a duration chosen to ensure the FFT reception window never straddles a tone boundary.

### Frequency Table

| Frequency (Hz) | Role           | Signal Code |
|----------------|----------------|-------------|
| 1000           | Carrier        | (0)         |
| 1400 to 4600   | Data           | (1) to (9)  |
| 5000 to 8200   | Data           | (10) to (17)|
| 8600           | Config carrier | (none)      |
| 9000           | Slave carrier  | (none)      |

Three carrier frequencies (8200, 8600, 9000 Hz) identify the channel - **master**, **config**, or **slave** - allowing simultaneous multi-channel operation without address collision.

---

## Bit Layer

The Bit Layer translates between binary data and Signal Codes using **run-length encoding**. This compresses repetitive bit sequences and achieves a useful bitrate of up to **30 bps** (5 symbols/second in the best case).

### Encoding example

```
Input bits:    0 0 0 1 1 1 0 1
After RLE:     3   14   0  10
After packing: 3 3 3 S 14 14 14 S 0 0 0 S 10 10 10 S [EOP]
```

Each Signal Code maps to a run of identical bits. Code 8 (EOP - End of Packet) terminates a transmission. An 80 ms silence token separates groups of codes, making the protocol **synchronization-free**.

Signal code / frequency conversion:

```
signal_code = (frequency - 1000) / 400
frequency   = 1000 + (signal_code x 400)
```

---

## Link Layer

Packets follow a fixed structure:

```
[ SOP (14 bits, all 1) ][ dst_flag (1b) ][ Destination ID (8b) ]
[ src_flag (1b) ][ Source ID (8b) ][ Payload (n bits, ASCII) ]
[ EOP (21 bits, all 1) ]
```

- **SOP / EOP** are unambiguous framing sequences not reproducible by valid data.
- **IDs** are 8-bit, supporting up to 256 nodes (0x00 = hotspot, 0xFF = broadcast).
- **Flags** distinguish temporary (registration) IDs from permanent ones.
- **Overhead** is 53 bits. Efficiency exceeds 50% for payloads of 7 or more ASCII characters.

### Token-Based Access (TKN)

Non-hotspot nodes transmit only when the hotspot grants them a token:

1. Hotspot sends `TKN` on master frequencies.
2. Node gains transmission rights for up to **1 minute**.
3. Node sends `EXT` to request a 1-minute extension, or `OK` to confirm.
4. If no message is sent within the time window, the token expires and passes to another node.

This prevents collisions in multi-node networks without requiring clock synchronization.

### Addressing and Routing

- **Matching ID**: packet is processed locally; a reply is queued on the slave channel.
- **Non-matching ID**: node enters *parrot mode*, re-queues and retransmits the packet, waiting for an ACK.
- **Broadcast (0xFF)**: packet is processed by all nodes, no ACK required.

### Network Addresses

| Address        | Role                    |
|----------------|-------------------------|
| `0x00`         | Hotspot (master node)   |
| `0xFF`         | Broadcast               |
| `0x01` to `0xFE` | Individual sensor nodes |

---

## Application Layer

High-level commands use a simple ASCII syntax:

```
ID:destination_id{PAYLOAD}[k/l]{source_id}
```

| Command            | Description                             |
|--------------------|-----------------------------------------|
| `ID:0{REQ}l{xxx}`  | Registration request from a new node    |
| `ID:0{SET}`        | Hotspot assigns a permanent ID          |
| `ID:x{TKN}`        | Hotspot grants transmission token       |
| `ID:x{EXT}`        | Node requests token extension           |
| `ID:x{OK}`         | Generic acknowledgment                  |
| `ID:FF{ABORT}`     | Emergency broadcast, stop all activity  |

---

## Hardware Reference Implementation

| Component               | Description                          |
|-------------------------|--------------------------------------|
| ESP32-WROOM-32          | 32-bit dual-core MCU, 240 MHz        |
| Adafruit SPH0645LM4H-B  | Digital MEMS microphone, I2S output  |
| Adafruit MAX98357A      | Class-D amplifier, I2S input, 3 W    |
| Speaker                 | 4-8 ohm load, approx 3 W nominal     |

The ESP32 handles both FFT processing and tone synthesis in real time on a single core. The FFT computation takes approximately 0.21 ms per 512-sample block (about 3% of the available 10.7 ms window), leaving ample headroom without multi-threading.

The I2S bus connects both the microphone and the amplifier to the ESP32, forming a complete bidirectional acoustic communication node with less than $15 of hardware per unit.

---

## Measured Performance

Tests were conducted on two ESP32 nodes placed 20 cm apart, across 560 total packets in three scenarios:

| Scenario                       | Tests | Avg Signal Codes | Avg Time (ms) | Useful Bitrate | Retransmissions |
|--------------------------------|-------|------------------|---------------|----------------|-----------------|
| Configuration (REQ/SET/OK)     | 180   | 171              | 26 232        | 14.7 bit/s     | 0.9%            |
| Movement sensor ON             | 220   | 239              | 36 568        | 14.0 bit/s     | 0.6%            |
| Telemetry ASCII "1738450195"   | 160   | 155              | 23 720        | 14.5 bit/s     | 0.8%            |

**Overall average:** 14.4 bit/s, consistent with the theoretical expectations from the Bit Layer design.

The low retransmission rate reflects the continuous ACK mechanism: the receiving node must confirm reception within **100 ms** of decoding a frame. The single NACK recorded during testing triggered one retransmission, after which the exchange completed successfully.

---

## Sound Propagation Results

Measurements were carried out in an underground corridor 5 meters below ground level (1.2 m wide, 15 m long, with a side chamber branching at 10 m via a 90-degree turn). Source emission level: **70 dB SPL**.

| Position             | Sound Level  | Packet Loss |
|----------------------|--------------|-------------|
| Source (0 m)         | 70 dB SPL    | less than 1% |
| 5 m (straight)       | 65 dB SPL    | less than 2% |
| 10 m (straight)      | 62 dB SPL    | less than 3% |
| 15 m (end wall)      | 55 dB SPL    | less than 5% |
| 1.5 m after turn     | 59 dB SPL    | approx 8%   |
| 3 m after turn       | 52 dB SPL    | approx 18%  |

**Straight path:** overall loss of approximately 15 dB over 15 m, packet loss below 5% up to the end wall.

**After the 90-degree turn:** sharp drop to 52 dB SPL at 3 m, packet loss rising to approximately 18%.

**Background noise:** 35 dB SPL, providing a 16 dB margin even at the worst-case point after the corner, sufficient for reliable decoding.

Frequency components between 5 kHz and 7.4 kHz showed slightly stronger attenuation than lower frequencies, consistent with air absorption models.

### Projected range at mains power (220 V)

Current prototypes run on 3.3 V. Switching to a 220 V-powered amplifier increases the source level by:

```
20 x log10(220 / 3.3) = 36.5 dB
```

This brings the initial emission to approximately 106.5 dB SPL, extending reliable coverage well past 15 m on the straight path, and maintaining adequate signal levels (around 95 dB SPL) even 3 m beyond a 90-degree turn.

---

## Repository Structure

```
AUSP-Protocol/
├── README.md                   This document
├── firmware/                   ESP32 firmware (C, ESP-IDF / Arduino)
│   ├── audio_driver.c          I2S tone synthesis with phase persistence
│   ├── reader.c                Dual-buffer I2S acquisition
│   ├── fft.c                   Cooley-Tukey FFT + peak detection
│   ├── bit_freq_codec.c        Frequency to Signal Code translation
│   ├── bit_input_packer.c      FIFO accumulation and EOP flush
│   └── protocol.c              Link layer: addressing, tokens, routing
├── docs/
│   ├── frequency_table.md      Complete frequency / code / carrier mapping
│   └── packet_format.md        Bit-level packet structure diagrams
└── tests/
    └── loopback_test/          Simple loopback test for bench validation
```

> The firmware source is being prepared for open release alongside the journal publication.

---

## Roadmap

- Replace the ASCII opcode set with a compact binary instruction format: `opcode (4b) + funct (4b) + datalen (4b) + data (n bits)`, reducing average packet length from approximately 120 bits to 69 bits.
- Implement autonomous token generation on non-hotspot nodes to support multi-hop networks.
- Extend coverage measurements to longer tunnels and different cross-section geometries.
- Validate operation with multiple simultaneous nodes (current tests: 2 nodes).

---

## Citation

If you use this work in your research, please cite the forthcoming paper (citation will be updated upon acceptance):

```
G. Giunta, S. Monteleone, D. Patti,
"Acoustic Communication Protocol for Master-Slave Networks in Underground Environments,"
IEEE Internet of Things Journal, under review, 2026.
```

---

## Authors

**Gioele Giunta** - B.Sc. Computer Engineering, University of Catania. M.Sc. candidate, Embedded and Smart System Design, Politecnico di Torino. This protocol is his first peer-reviewed work.

**Salvatore Monteleone** - Associate Professor, Department of Engineering, Niccolo Cusano University, Rome.

**Davide Patti** - Associate Professor in Computer Engineering, University of Catania. Associate Editor, IEEE Access.

---

## License

Source code released under the MIT License. See [LICENSE](LICENSE) for details.
