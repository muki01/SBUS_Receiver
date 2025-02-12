#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
RF24 radio(6, 7);
const uint64_t pipeIn = 0xE8E8F0F0E1LL;

// #define SerialOUTPUT
#define SBus Serial

#define RC_CHANNEL_MIN 988
#define RC_CHANNEL_MAX 2011

#define SBUS_MIN_OFFSET 173
#define SBUS_MID_OFFSET 992
#define SBUS_MAX_OFFSET 1811

#define SBUS_CHANNEL_NUMBER 8
#define SBUS_PACKET_LENGTH 25

#define SBUS_FRAME_HEADER 0x0f
#define SBUS_FRAME_FOOTER 0x00
#define SBUS_FRAME_FOOTER_V2 0x04
#define SBUS_STATE_FAILSAFE 0x08
#define SBUS_STATE_SIGNALLOSS 0x04
#define SBUS_UPDATE_RATE 15  //ms

#define SIGNAL_TIMEOUT 1000  // This is signal timeout in milli seconds.
unsigned long lastRecvTime = 0;

struct PacketData {
  byte ch1;
  byte ch2;
  byte ch3;
  byte ch4;
  byte ch5;
  byte ch6;
  byte ch7;
  byte ch8;
};
PacketData receiverData;

void sbusPreparePacket(uint8_t packet[], int channels[], bool isSignalLoss, bool isFailsafe) {
  static int output[SBUS_CHANNEL_NUMBER] = { 0 };
  /*
     * Map 1000-2000 with middle at 1500 chanel values to
     * 173-1811 with middle at 992 S.BUS protocol requires
     */
  for (uint8_t i = 0; i < SBUS_CHANNEL_NUMBER; i++) {
    output[i] = map(channels[i], RC_CHANNEL_MIN, RC_CHANNEL_MAX, SBUS_MIN_OFFSET, SBUS_MAX_OFFSET);
  }

  uint8_t stateByte = 0x00;
  if (isSignalLoss) {
    stateByte |= SBUS_STATE_SIGNALLOSS;
  }
  if (isFailsafe) {
    stateByte |= SBUS_STATE_FAILSAFE;
  }
  packet[0] = SBUS_FRAME_HEADER;  //Header

  packet[1] = (uint8_t)(output[0] & 0x07FF);
  packet[2] = (uint8_t)((output[0] & 0x07FF) >> 8 | (output[1] & 0x07FF) << 3);
  packet[3] = (uint8_t)((output[1] & 0x07FF) >> 5 | (output[2] & 0x07FF) << 6);
  packet[4] = (uint8_t)((output[2] & 0x07FF) >> 2);
  packet[5] = (uint8_t)((output[2] & 0x07FF) >> 10 | (output[3] & 0x07FF) << 1);
  packet[6] = (uint8_t)((output[3] & 0x07FF) >> 7 | (output[4] & 0x07FF) << 4);
  packet[7] = (uint8_t)((output[4] & 0x07FF) >> 4 | (output[5] & 0x07FF) << 7);
  packet[8] = (uint8_t)((output[5] & 0x07FF) >> 1);
  packet[9] = (uint8_t)((output[5] & 0x07FF) >> 9 | (output[6] & 0x07FF) << 2);
  packet[10] = (uint8_t)((output[6] & 0x07FF) >> 6 | (output[7] & 0x07FF) << 5);
  packet[11] = (uint8_t)((output[7] & 0x07FF) >> 3);
  packet[12] = (uint8_t)((output[8] & 0x07FF));
  packet[13] = (uint8_t)((output[8] & 0x07FF) >> 8 | (output[9] & 0x07FF) << 3);
  packet[14] = (uint8_t)((output[9] & 0x07FF) >> 5 | (output[10] & 0x07FF) << 6);
  packet[15] = (uint8_t)((output[10] & 0x07FF) >> 2);
  packet[16] = (uint8_t)((output[10] & 0x07FF) >> 10 | (output[11] & 0x07FF) << 1);
  packet[17] = (uint8_t)((output[11] & 0x07FF) >> 7 | (output[12] & 0x07FF) << 4);
  packet[18] = (uint8_t)((output[12] & 0x07FF) >> 4 | (output[13] & 0x07FF) << 7);
  packet[19] = (uint8_t)((output[13] & 0x07FF) >> 1);
  packet[20] = (uint8_t)((output[13] & 0x07FF) >> 9 | (output[14] & 0x07FF) << 2);
  packet[21] = (uint8_t)((output[14] & 0x07FF) >> 6 | (output[15] & 0x07FF) << 5);
  packet[22] = (uint8_t)((output[15] & 0x07FF) >> 3);

  packet[23] = stateByte;          //Flags byte
  packet[24] = SBUS_FRAME_FOOTER;  //Footer
}

uint8_t sbusPacket[SBUS_PACKET_LENGTH];
int channelValue[SBUS_CHANNEL_NUMBER] = { 1000, 1500, 1500, 1500, 1000, 1000, 1000, 1000 };
uint32_t sbusTime = 0;

void reset_received_Data() {
  receiverData.ch1 = 0;  //Throttle
  receiverData.ch2 = 127;
  receiverData.ch3 = 127;
  receiverData.ch4 = 127;
  receiverData.ch5 = 0;
  receiverData.ch6 = 0;
  receiverData.ch7 = 0;
  receiverData.ch8 = 0;

  channelValue[0] = 1000;
  channelValue[1] = 1500;
  channelValue[2] = 1500;
  channelValue[3] = 1500;
  channelValue[4] = 1000;
  channelValue[5] = 1000;
  channelValue[6] = 1000;
  channelValue[7] = 1000;
}

void setup() {
  SPI.begin(8, 5, 10, 7);  // SCK=8, MISO=5, MOSI=10, CSN=7
  if (!radio.begin()) {
#ifdef SerialOUTPUT
    Serial.println("NRF24L01 Error");
#endif
    while (1)
      ;
  }

  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipeIn);
  radio.startListening();

  reset_received_Data();

#ifdef SerialOUTPUT
  Serial.begin(115200);
#else
  SBus.begin(100000, SERIAL_8E2);
#endif
}

void loop() {
  if (radio.available()) {
    radio.read(&receiverData, sizeof(PacketData));
    channelValue[0] = map(receiverData.ch1, 0, 255, 1000, 2000);
    channelValue[1] = map(receiverData.ch2, 0, 255, 1000, 2000);
    channelValue[2] = map(receiverData.ch3, 0, 255, 1000, 2000);
    channelValue[3] = map(receiverData.ch4, 0, 255, 1000, 2000);
    channelValue[4] = map(receiverData.ch5, 0, 1, 1000, 2000);
    channelValue[5] = map(receiverData.ch6, 0, 1, 1000, 2000);
    channelValue[6] = map(receiverData.ch7, 0, 1, 1000, 2000);
    channelValue[7] = map(receiverData.ch8, 0, 1, 1000, 2000);

    lastRecvTime = millis();
  }

  unsigned long now = millis();
  if (now - lastRecvTime > SIGNAL_TIMEOUT) {
    reset_received_Data();
#ifdef SerialOUTPUT
    Serial.println("Disconnected");
#endif
  }

  uint32_t currentMillis = millis();
  if (currentMillis > sbusTime) {
    sbusPreparePacket(sbusPacket, channelValue, false, false);
    SBus.write(sbusPacket, SBUS_PACKET_LENGTH);
    sbusTime = currentMillis + SBUS_UPDATE_RATE;
  }
}