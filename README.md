# X2D / X3D Tools & Firmware

Repository combining host-side Python tooling and embedded firmware for working with Delta Dore X2D and X3D
wireless protocols. It provides parsers/formatters, modulation/packetization processors, example runners, a Bluepill
(STM32) firmware test implementation, and esphome integration for Home Assistant.

## Key Python files

- `X2D.py` — Construct-based protocol definitions and structures for X2D messages.
- `X3D.py` — Protocol logic for X3D (message ID encryption, CRCs) and Construct structures.
- `encoding.py` — Processing pipeline (Packetizer, Bitstream, OOK/Biphase/Manchester, whitening, X2D/X3D
  processors and helpers).
- `main.py` — Example runner and helpers for X2D decode/encode flows.
- `main_x3d.py` — Example runner and round-trip checks for X3D.
- `server.py` — TCP server acting as a GNU Radio sink (accepts TCP connections and forwards radio sample streams).

## Quick start

### Python

Install dependencies and run example scripts:

```bash
python -m pip install -r requirements.txt
python main_x3d.py
python main.py
```

### Firmware (Bluepill)

Build and (optionally) upload with PlatformIO from the `bluepill/` folder:

```bash
cd bluepill
pio run -e bluepill_f103c8
pio run -e bluepill_f103c8 -t upload
```

### ESPHome

The `esphome/` directory contains low-level C/C++ helpers and two example esphome node configurations
(`espx2d.yaml`, `espx3d.yaml`) that demonstrate integrating CC1101-based X2D/X3D support into an esphome firmware
image for Home Assistant. To build and run the example firmwares (requires `esphome` installed):

```bash
cd esphome
esphome run espx2d.yaml
esphome run espx3d.yaml
```

The YAML files include the C/C++ sources via `includes:` so the custom components compile into the firmware.

## Notes

- The repository mixes host-side Python tooling and embedded code; they are complementary and can be used together for
  development and testing.
