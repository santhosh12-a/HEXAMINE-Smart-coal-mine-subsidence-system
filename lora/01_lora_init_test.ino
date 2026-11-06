/* ============================================================
   01 — LoRa Init Test
   Proves the ESP32 can talk to the SX1278 over SPI.
   Nothing is transmitted. Upload this to BOTH nodes first.
   Library: "LoRa" by Sandeep Mistry
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

// ---- your pair number: ASK THE INSTRUCTOR, then set it here ----
#define PAIR_ID 1

// each pair gets its own channel so the room does not talk over itself
const long FREQUENCY = 433100000L + ((PAIR_ID - 1) * 300000L);
const uint8_t SYNC_WORD = 0x10 + PAIR_ID;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) { }   // wait for USB CDC
  delay(1000);

  Serial.println();
  Serial.println("=== LoRa init test ===");
  Serial.print("Pair ID   : "); Serial.println(PAIR_ID);
  Serial.print("Frequency : "); Serial.print(FREQUENCY / 1000000.0, 3); Serial.println(" MHz");

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);   // S3 needs this
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("LoRa init FAILED");
    Serial.println("-> This is a WIRING problem, not a code problem.");
    Serial.println("-> Check NSS, RST, DIO0, GND, and 3V3 (never 5V).");
    while (true) { delay(1000); }
  }

  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125000);
  LoRa.setCodingRate4(5);
  LoRa.setTxPower(17);

  Serial.println("LoRa OK");
  Serial.println("Radio is alive. You may move on to sketch 02 / 03.");
}

void loop() {
  delay(1000);
}
