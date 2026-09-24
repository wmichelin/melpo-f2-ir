#define DISABLE_CODE_FOR_RECEIVER
#define SEND_PWM_BY_TIMER
#define IR_TX_PIN 44
#include <M5Cardputer.h>
#include <IRremote.hpp>
#include <Preferences.h>
#include "Candidates.h"
#include "MelpoF2.h"

Preferences prefs;
bool powerMode = false;
bool mappedMode = true;
uint8_t mappedIndex = 0;
uint16_t candidateIndex = 0;
uint16_t address = 0;
uint8_t command = 0;
bool scanning = false;
bool advanceNext = false;
uint32_t nextAt = 0;
const uint16_t intervals[] = {250, 500, 1500};
uint8_t speedIndex = 0;
String notice = "Ready - no signals sent";
String serialLine;

void screen() {
  auto &d = M5Cardputer.Display;
  d.fillScreen(BLACK);
  d.setCursor(4, 4);
  d.setTextSize(2);
  d.setTextColor(scanning ? GREEN : WHITE);
  if (mappedMode) {
    d.println("MELPO REMOTE");
    d.setTextSize(1);
    d.printf("\n%u/%u  %s\n", mappedIndex + 1, melpoControlCount, melpoControls[mappedIndex].label);
    d.printf("NEC 0000:%02X\n", melpoControls[mappedIndex].command);
    d.println("\nN/P select   ENTER or SPACE send");
    d.println("O on  K off  W white  +/- dim");
    d.println("S save  L load  M scanner  X stop");
    d.println("\n" + notice);
    return;
  }
  d.println(scanning ? "IR TEST: RUNNING" : "IR TEST: PAUSED");
  if (powerMode) d.printf("POWER %u / %u\n", candidateIndex + 1, candidateCount);
  else d.printf("NEC %04X : %02X\n", address, command);
  d.setTextSize(1);
  d.println("\nSPACE scan/pause  ENTER send");
  d.println(powerMode ? "N/P candidate +/-  M manual mode" : "N/P cmd  A/D addr  M power mode");
  d.println("S save   L load   X stop");
  d.println("R MELPO remote");
  d.printf("F speed: %u ms per signal\n", intervals[speedIndex]);
  d.println("\n" + notice);
}

void transmit() {
  if (mappedMode) {
    address = 0;
    command = melpoControls[mappedIndex].command;
  }
  if (powerMode) {
    const auto &c = candidates[candidateIndex];
    IrSender.sendRaw(c.wave, c.length, c.khz);
    notice = String("Sent: ") + c.label;
    Serial.printf("SENT POWER %u %s\n", candidateIndex + 1, c.label);
    screen();
    return;
  }
  IrSender.sendNEC(address, command, 0);
  notice = "Sent " + String(address, HEX) + ":" + String(command, HEX);
  Serial.printf("SENT NEC %04X %02X\n", address, command);
  screen();
}

void pauseScan() { scanning = false; }

void key(char k) {
  if (k == 'r') {
    pauseScan(); mappedMode = true; powerMode = false; advanceNext = false;
    notice = "Select a control"; screen(); return;
  }
  if (mappedMode) {
    if (k == 'n' || k == 'p') {
      mappedIndex = (mappedIndex + melpoControlCount + (k == 'n' ? 1 : -1)) % melpoControlCount;
      notice = "Selected - ENTER to send"; screen(); return;
    }
    if (k == ' ' || k == '\r' || k == '\n') { pauseScan(); transmit(); return; }
    if (k == 'o' || k == 'k' || k == 'w' || k == '+' || k == '=' || k == '-') {
      mappedIndex = k == 'o' ? 0 : k == 'k' ? 1 : k == 'w' ? 2 : k == '-' ? 4 : 3;
      pauseScan(); transmit(); return;
    }
    if (k == 'm') {
      mappedMode = false; powerMode = false; pauseScan(); advanceNext = false;
      notice = "Manual scan; R returns to remote"; screen(); return;
    }
    if (k != 's' && k != 'l' && k != 'x') return;
  }
  if (k == ' ') {
    scanning = !scanning;
    nextAt = millis();
    notice = scanning ? "Scanning; SPACE to pause" : "Paused on last signal";
  } else if (k == '\r' || k == '\n') {
    pauseScan();
    transmit();
  } else if (k == 'n' || k == 'p') {
    pauseScan();
    if (powerMode) candidateIndex = (candidateIndex + candidateCount + (k == 'n' ? 1 : -1)) % candidateCount;
    else command += (k == 'n' ? 1 : -1);
    advanceNext = false;
    notice = "Selected - ENTER to send";
  } else if (k == 'm') {
    pauseScan();
    powerMode = !powerMode;
    advanceNext = false;
    notice = powerMode ? "LED power candidates" : "Manual NEC scan";
  } else if (!powerMode && (k == 'a' || k == 'd')) {
    pauseScan();
    address += (k == 'd' ? 1 : -1);
    command = 0;
    advanceNext = false;
    notice = "Selected - ENTER to send";
  } else if (k == 's') {
    pauseScan();
    if (mappedMode) { address = 0; command = melpoControls[mappedIndex].command; }
    uint32_t saved = powerMode ? 0x80000000UL | candidateIndex : (uint32_t(address) << 8) | command;
    bool ok = prefs.putUInt("code", saved) == 4;
    notice = ok ? "Saved to flash" : "Save failed";
    Serial.printf("%s mode=%s index=%u NEC=%04X:%02X\n", ok ? "SAVED" : "SAVE FAILED", powerMode ? "POWER" : "NEC", candidateIndex + 1, address, command);
  } else if (k == 'l') {
    pauseScan();
    if (prefs.isKey("code")) {
      uint32_t code = prefs.getUInt("code");
      mappedMode = false;
      powerMode = (code & 0x80000000UL) != 0;
      if (powerMode) candidateIndex = (code & 0x7FFFFFFFUL) % candidateCount;
      else { address = code >> 8; command = code & 255; }
      advanceNext = false;
      notice = "Loaded - ENTER to send";
    } else notice = "No saved signal";
  } else if (k == 'f') {
    speedIndex = (speedIndex + 1) % 3;
    nextAt = millis() + intervals[speedIndex];
    notice = "Scan speed changed";
  } else if (k == 'x') {
    pauseScan();
    notice = "Stopped";
  }
  screen();
}

void handleSerial(String line) {
  line.trim();
  unsigned int a, c;
  char extra;
  if (sscanf(line.c_str(), "send %x %x %c", &a, &c, &extra) == 2 && a <= 65535 && c <= 255) {
    pauseScan();
    mappedMode = false;
    powerMode = false;
    address = a;
    command = c;
    advanceNext = false;
    transmit();
  } else if (line == "status") {
    Serial.printf("MODE %s candidate=%u/%u\n", mappedMode ? "MELPO" : powerMode ? "POWER" : "NEC", candidateIndex + 1, candidateCount);
    Serial.printf("STATUS NEC %04X %02X scan=%d interval=%u\n", address, command, scanning, intervals[speedIndex]);
  } else if (line == "scan") key(' ');
  else if (line == "stop") key('x');
  else if (line == "save") key('s');
  else if (line == "load") key('l');
  else if (line == "speed") key('f');
  else if (line == "mode") key('m');
  else if (line == "remote") key('r');
  else if (line == "on" || line == "off" || line == "white" || line == "brighter" || line == "dimmer") {
    key('r');
    key(line == "on" ? 'o' : line == "off" ? 'k' : line == "white" ? 'w' : line == "brighter" ? '+' : '-');
  }
  else Serial.println("Commands: send HEXADDR HEXCMD | on | off | white | brighter | dimmer | status | scan | stop | save | load | speed | mode | remote");
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  Serial.begin(115200);
  IrSender.begin(DISABLE_LED_FEEDBACK);
  IrSender.setSendPin(IR_TX_PIN);
  prefs.begin("ir-tester", false);
  screen();
}

void loop() {
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto state = M5Cardputer.Keyboard.keysState();
    for (char c : state.word) key(c);
    if (state.enter) key('\r');
  }
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') { handleSerial(serialLine); serialLine = ""; }
    else if (c != '\r') {
      if (serialLine.length() < 80) serialLine += c;
      else serialLine = "";
    }
  }
  if (scanning && int32_t(millis() - nextAt) >= 0) {
    if (advanceNext) {
      if ((powerMode && candidateIndex + 1 >= candidateCount) || (!powerMode && command == 255)) {
        scanning = false;
        advanceNext = false;
        notice = powerMode ? "Power scan complete" : "Address complete; choose next";
        screen();
        return;
      }
      if (powerMode) ++candidateIndex;
      else ++command;
    }
    nextAt = millis() + intervals[speedIndex];
    transmit();
    advanceNext = true;
  }
  delay(5);
}
