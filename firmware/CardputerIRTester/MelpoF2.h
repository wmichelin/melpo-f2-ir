#pragma once
#include <stdint.h>

// Recovered on the user's MELPO B08T5XR951 / BLFL-LFBA by physical tests.
// NEC, address 0x0000. See ../../README.md for evidence and limits.
// Timer duration and DIY programming/hold behavior remain unverified.
struct MelpoControl { const char *label; uint8_t command; };
static const MelpoControl melpoControls[] = {
  {"On",             0x47},
  {"Off",            0x43},
  {"Daily white",    0x1C},
  {"Brightness up",  0x45},
  {"Brightness down",0x46},
  {"Store 1 recall", 0x07},
  {"Store 2 recall", 0x15},
  {"Wheel: red",     0x0C},
  {"Wheel: yellow",  0x5E},
  {"Wheel: green",   0x4A},
  {"Wheel: blue",    0x42},
  {"DIY-FLASH",      0x44},
  {"Smooth",         0x40},
  {"Cozy",           0x16},
  {"Fresh",          0x19},
  {"Romantic",       0x0D},
  {"Timer (candidate)",0x09},
};
static constexpr uint8_t melpoControlCount = sizeof(melpoControls)/sizeof(melpoControls[0]);
