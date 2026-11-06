/* ============================================================
   03 — Receiver (HELLO)
   Node B. Listens continuously and reports RSSI and SNR.
   Pair this with sketch 02 on the other node.
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

// ---- these MUST match the transmitter ----
const int  SPREADING_FACTOR = 12;
const long BANDWIDTH        = 125000;
const int  CODING_RATE      = 5;

unsigned long received = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) { }   // wait for USB CDC
  delay(1000);
  Serial.println("=== Receiver (HELLO) ===");

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

  Serial.println("LoRa OK - listening");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize == 0) return;

  String message = "";
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  received++;

  Serial.print("Received: ");
  Serial.println(message);
  Serial.print("  RSSI: ");
  Serial.print(LoRa.packetRssi());
  Serial.println(" dBm");
  Serial.print("  SNR : ");
  Serial.print(LoRa.packetSnr(), 2);
  Serial.println(" dB");
  Serial.print("  total received: ");
  Serial.println(received);
  Serial.println();
}
