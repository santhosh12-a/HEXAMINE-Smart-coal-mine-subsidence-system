/* ============================================================
   02 — Transmitter (HELLO)
   Node A. Sends one plain-text packet every 2 seconds.
   Pair this with sketch 03 on the other node.
   BOARD: ESP32-S3   (Tools > USB CDC On Boot: Enabled)
   ============================================================ */

#include <SPI.h>
#include <LoRa.h>

// ---- ESP32-S3 pin map ----
#define LORA_SCK  12
#define LORA_MISO 13
#define LORA_MOSI 11
#define LORA_SS   10
#define LORA_RST   5
#define LORA_DIO0  4

#define PAIR_ID 1                      // <-- same on both nodes

const long FREQUENCY = 433100000L + ((PAIR_ID - 1) * 300000L);
const uint8_t SYNC_WORD = 0x10 + PAIR_ID;

// ---- radio settings: these MUST match the receiver ----
const int  SPREADING_FACTOR = 7;       // 7 .. 12
const long BANDWIDTH        = 125000;  // 125000 / 250000 / 500000
const int  CODING_RATE      = 5;       // 5 .. 8  (means 4/5 .. 4/8)
const int  TX_POWER         = 17;      // dBm, 2 .. 20

unsigned long counter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) { }   // wait for USB CDC
  delay(1000);
  Serial.println("=== Transmitter (HELLO) ===");

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);   // S3 needs this
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("LoRa init FAILED - check wiring");
    while (true) { delay(1000); }
  }

  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE);
  LoRa.setTxPower(TX_POWER);

  Serial.println("LoRa OK - transmitting");
}

void loop() {
  counter++;

  unsigned long t0 = millis();
  LoRa.beginPacket();
  LoRa.print("HELLO FROM NODE 1");
  LoRa.endPacket();                    // blocks until the packet is on air
  unsigned long airtime = millis() - t0;

  Serial.print("Sent #");
  Serial.print(counter);
  Serial.print("   airtime approx ");
  Serial.print(airtime);
  Serial.println(" ms");

  delay(2000);
}
