"""Validate independently encoded exports of the recovered command table."""
import csv
import json
import pathlib
import re

root = pathlib.Path(__file__).resolve().parents[1]
records = json.loads((root / 'data/commands.json').read_text())
assert len(records) == 17
commands = {int(r['command'], 16) for r in records}
assert len(commands) == 17
for record in records:
    command = int(record['command'], 16)
    assert record['protocol'] == 'NEC' and int(record['address'], 16) == 0
    raw = int(record['full_value_lsb'], 16)
    assert raw & 0xffff == 0xff00
    assert (raw >> 16) & 0xff == command
    assert (raw >> 24) == (command ^ 0xff)
    assert int(record['legacy_msb_value'], 16) == int(f'{raw:032b}'[::-1], 2)
    timings = record['raw_mark_space_us']
    assert len(timings) == 67 and timings[:2] == [9000, 4500] and timings[-1] == 560
    decoded = 0
    for bit in range(32):
        assert timings[2 + bit * 2] == 560
        space = timings[3 + bit * 2]
        assert space in (560, 1690)
        if space == 1690:
            decoded |= 1 << bit
    assert decoded == raw

def parse_flipper(path):
    text = path.read_text()
    assert text.startswith('Filetype: IR signals file\nVersion: 1\n')
    found = {}
    for block in re.split(r'^name: ', text, flags=re.M)[1:]:
        name, remainder = block.split('\n', 1)
        fields = dict(line.split(': ', 1) for line in remainder.splitlines()
                      if ': ' in line and not line.startswith('#'))
        assert fields['type'] == 'parsed' and fields['protocol'] == 'NEC'
        assert int.from_bytes(bytes.fromhex(fields['address']), 'little') == 0
        found[name] = int.from_bytes(bytes.fromhex(fields['command']), 'little')
    return found

normal = parse_flipper(root / 'flipper/MELPO_BLFL-LFBA.ir')
experimental = parse_flipper(root / 'experimental/MELPO_BLFL-LFBA_TIMER.ir')
assert len(normal) == 16 and set(normal.values()) == commands - {0x09}
assert experimental == {'Timer_candidate': 0x09}
assert normal['Power_on'] == 0x47 and normal['Power_off'] == 0x43
assert normal['Brightness_up'] == 0x45 and normal['Brightness_dn'] == 0x46
header = (root / 'firmware/CardputerIRTester/MelpoF2.h').read_text()
assert {int(x, 16) for x in re.findall(r'0x([0-9A-F]{2})\}', header)} == commands
with (root / 'data/commands.csv').open() as stream:
    csv_records = list(csv.DictReader(stream))
assert [(r['function'], r['command']) for r in csv_records] == [
    (r['function'], r['command']) for r in records]
print('Validated 17 mappings, NEC encodings, 16-signal Flipper export, separate Timer candidate, and firmware table.')
