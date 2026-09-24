# Cardputer ADV firmware

This is the device-tested named MELPO remote plus the retained original scanner. It sends no IR at boot. Timer is labeled candidate. The repository distributes source, not local prebuilt binaries.

## Keys

- O: On; K: Off; W: white.
- + or =: brighter; -: dimmer.
- N/P: select a control; Enter or Space: send once.
- M: original scanner; R: named remote.
- X: stop; S/L: save/load one command.

## USB

115200 baud, newline terminated: `on`, `off`, `white`, `brighter`, `dimmer`, `remote`, or `send 0000 47`.

Existing `status`, `scan`, `stop`, `save`, `load`, `speed`, and `mode` commands remain available. In named remote mode, Space/scan sends a single selected command; select scanner mode first for a sweep.

## Reproduce the tested build

Install Arduino CLI, then:

```sh
arduino-cli core update-index --additional-urls https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
arduino-cli core install m5stack:esp32@3.3.9 --additional-urls https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
arduino-cli lib install 'M5Cardputer@1.1.1' 'M5Unified@0.2.23' 'M5GFX@0.2.30' 'IRremote@4.7.1'
arduino-cli compile --fqbn m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=default_8MB firmware/CardputerIRTester
```

Run from the repository root. To upload, use `arduino-cli board list` to identify your own serial port, then use `arduino-cli upload` with the same FQBN, sketch path, and your port. No local USB identifier is hard-coded in this source.

`MelpoF2.h` is the recovered map. `Candidates.h` and `candidate-sources.json` are legacy generic LED scanner data from Flipper-IRDB, with `IRDB-LICENSE` retained. Generic candidates are not F2 code evidence. Imported libraries retain their own licenses and are installed separately.
