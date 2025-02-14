# SBUS_Receiver

SBUS_Receiver is a project that receives wireless data via the NRF24L01 module and outputs it as an SBus signal. It is compatible with microcontrollers such as Arduino, ESP8266, and ESP32. It is designed for applications requiring SBus output, such as remote control systems or robotic control.

## ✨ Features
- Wireless data reception via NRF24L01
- SBus (Serial Bus) signal output
- Easy to use and integrate

## 🛠 Required Components
- Arduino, ESP8266, or ESP32
- NRF24L01 wireless module
- Jumper wires for connections

## 🔌 Wiring
1. Connect the NRF24L01 module to the microcontroller:
   - VCC -> 3.3V (Do not use 5V!)
   - GND -> GND
   - CE -> Pin 9
   - CSN -> Pin 10
   - SCK -> Pin 13 (May vary for ESP boards)
   - MOSI -> Pin 11
   - MISO -> Pin 12
2. Connect the SBus output pin to your desired device (e.g., servos, control boards, etc.).

## 🚀 Installation
1. Install the [RF24 Arduino library](https://github.com/nRF24/RF24) for the NRF24L01 module.
2. Upload the `SBusReceiver.ino` code to your Arduino, ESP8266, or ESP32 board.
3. Use the Serial Monitor to check the received data.

## 📖 Usage
- Once uploaded, the receiver will start receiving wireless data and output it as an SBus signal.
- Ensure that the transmitter is correctly configured and paired with the receiver.

## 📸 Photos
Here are some images of the SBUS Receiver setup:

