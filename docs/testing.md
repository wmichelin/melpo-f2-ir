# How the map was recovered

## Setup and scope

- Target: MELPO BLFL-LFBA, ASIN B08T5XR951, 50 W RGBW IR floodlight.
- Transmitter: Cardputer ADV, GPIO 44 IR LED, Arduino-IRremote 4.7.1.
- The original remote was unavailable. No original-remote waveform capture was made.
- An initially dark unit gave no visible feedback. An identical unit that powered up lit made color, brightness, and off commands observable. The systematic map was verified on that lit unit; verification on multiple independently identified hardware revisions remains useful.

## Method

All 256 NEC command bytes at address 0000 were screened. Each useful response was investigated from controlled starting states. Once On was identified, subsequent tests explicitly restored power before sending the baseline setting. Candidate timing values in the JSON are generated standard NEC waveforms, not measured carrier/timing captures.

The camera compared frames before and after a transmission. Color differences helped locate responses, but actual frames were reviewed because automatic exposure can make a dimmed lamp's surroundings appear brighter. A camera obstruction during two trials was identified and those commands were retested.

| Test | Observation |
|---|---|
| `43, 43, 47, 47, 43, 47` | Off stays off; On stays on. These are separate functions. |
| White `1C`, twelve `46`, twelve `45` | White output visibly dimmed, then brightened again. |
| Select a Store slot, adjust the wheel, switch away and recall | Changed color persisted in that slot. Verified for both slots. |
| Repeated `0C`, `42`, `4A`, `5E` | Clear fine-color changes toward red, blue, green, and a pale warm/yellow appearance. Exact hue was not measured. |
| `44` over 18 seconds | Stepped red/green/blue/white sequence. DIY programming was not exercised. |
| `40` over 60 seconds | Broad gradual color transition, consistent with Smooth. |
| `16` over 60 seconds | Warm changing red/orange/yellow effect, consistent with Cozy. |
| `19` and `0D` over 18 seconds each | Green/cyan and purple/magenta effects, consistent with Fresh and Romantic. |
| `09` | A brief lamp-off acknowledgement. Timer duration, recurrence, and cancellation remain unverified. |

The named Cardputer firmware compiled, uploaded, and passed flash verification. After installation, named On, Off, White, Dimmer, and Brighter commands were exercised again and checked against camera frames. The GPIO, carrier generation, and serial `send` path were unchanged.

## What this does not establish

A no-change result does not prove a code is unused: state, brightness limits, small hue steps, timers, stored settings, or held-button semantics may matter. Scene labels are matched to the F2 layout and observed behavior, not read from an original remote capture. No F2-S or HYDONG hardware was tested. The Timer candidate is separated from the normal Flipper file.

All camera images, original logs, device identifiers, and local file paths are omitted from this public repository. This document is the qualitative test record, not independently reviewable raw camera evidence. Independent device confirmations are welcome.

## Encoding conventions

For address 0000 and command `c`, Arduino IRremote's 32-bit LSB representation is `0xFF00 | (c << 16) | ((c ^ 0xFF) << 24)`. The legacy MSB value is the reversal of all 32 bits. Flipper parsed NEC fields use little-endian byte lists: address `00 00 00 00`, command `cc 00 00 00`.

The JSON's raw mark/space list encodes a nominal NEC frame with a 9000/4500 microsecond lead-in, 560 microsecond marks, and 560/1690 microsecond data spaces. These generated values were validated against the address/command pair; the raw export itself has not been separately exercised on every platform.
