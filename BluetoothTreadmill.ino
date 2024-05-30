#include <ArduinoBLE.h>
#include "BLEService.h"
#include "SpeedMapping.h"
#include "InclineMapping.h"

bool debug = false; //Set this to true if currently running it without a serial connection to the treadmill

const int stopCommand[3] = {1,0,182};

// Define command codes
const byte CMD_GET_SPEED = 241;
const byte CMD_GET_INCLINE = 246;
const byte CMD_HEARTBEAT = 249;
const byte CMD_GET_STOP = 245;

float currentSpeed[4];
float currentMPH;
float prevMPH = 13;
float currentIncline[4];
float currentPercent;
float prevPercent = 13;
int   currentStop[3];
unsigned long lastSendTime = 0;

float kmph;
float mphtokmph = 1.609344;
uint16_t inst_speed = 0; // Instantaneous speed in units of .01 km/h
uint16_t incl_percent = 20; // How feel going uphill or downhill. Signed, units of .1 percent

// Define constants
const int SPEED_CMD_SIZE = 4;
const int SPEED_TABLE_SIZE = 117;
const int SPEED_CMD_ID = 2;
const int INCLINE_CMD_SIZE = 4;
const int INCLINE_TABLE_SIZE = 25;
const int INCLINE_CMD_ID = 2;

void setup() {
  Serial2.begin(9600);
  Serial.begin(9600);
  pinMode(LED_BUILTIN,OUTPUT);

  if (!BLE.begin()) {
    Serial.println("starting BLE failed");
    while(1);
  }

  BLE.setLocalName("Livestrong LS8.0T");
  BLE.setAdvertisedService(fitness_machine_service);
  fitness_machine_service.addCharacteristic(RSCMeasurementChar);
  BLE.addService(fitness_machine_service);
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connection…");

}

void loop() {
  unsigned long currentTime = millis();

  // Check if a second (1000 milliseconds) has passed since the last send
  if (currentTime - lastSendTime >= 1000) {
    // If so, call sendBLE() and update the last send time
    sendBLE();
    lastSendTime = currentTime;
  }
  sendBLE();

  while(debug == true) {
    Serial.print("DEBUG ");
    getSpeed();
  }
  
  if(Serial2.available() > 0) {
    char data1 = Serial2.read();
    if(data1 != 0) {
      continue;
    }
    char data2 = Serial2.read();
    if(data2 != 255) {
      continue;
    }
    char data3 = Serial2.read();
    switch (data3) {
      case CMD_GET_SPEED: {
        getSpeed();
        break;
      }
      case CMD_GET_INCLINE: {
        getIncline();
        break;
      }
      case CMD_HEARTBEAT: {
        // Serial.println("Heartbeat Detected"); 
        break;
      }
      case CMD_GET_STOP: {
        getStop();
        break;
      }
      default: {
        printUnknownCommand(data3);
        break;
      } 
    }
  }
}

void printUnknownCommand(char cmd) {
  Serial.print("Unknown Command detected: ");
  Serial.print("data3 = ");
  Serial.println(cmd, DEC);
  Serial.print("{Command Received} ");
  int unknownCommand[4];
  for (int i = 0; i < 3; i++) {
    unknownCommand[i] = Serial2.read();
  }
  Serial.print("0, 255, ");
  Serial.print(cmd, DEC);
  for (int i = 0; i < 4; i++) {
    Serial.print(", ");
    Serial.print(unknownCommand[i]);
  }
  Serial.println();
}

void readSpeedCommand() {
  for(int i = 0; i < SPEED_CMD_SIZE; i++) {
    currentSpeed[i] = Serial2.read();
  }
}

void getSpeed() {
  if (debug) {
    Serial.print("DEBUG ");
    currentSpeed[0] = 2;
    currentSpeed[1] = 10;
    currentSpeed[2] = 170;
    currentSpeed[3] = 20;
  } else {
    readSpeedCommand();
  }

  if(currentSpeed[0] != SPEED_CMD_ID) {
    return;
  }

  //Match up the received serial command to the speedTable to find the speed in MPH
  for (int i = 0; i < SPEED_TABLE_SIZE; i++) {
    if(currentSpeed[0] == speedTable[i][0] &&
       currentSpeed[1] == speedTable[i][1] &&
       currentSpeed[2] == speedTable[i][2] &&
       currentSpeed[3] == speedTable[i][3]) {
      //Print the current speed in MPH
      currentMPH = speedTable[i][4];
      if (currentMPH != prevMPH) {
        Serial.print(currentMPH, 1);
        Serial.println(" MPH");
        prevMPH = currentMPH;
      }
    }
  }
}

void readInclineCommand() {
  for(int i = 0; i < INCLINE_CMD_SIZE; i++) {
    currentIncline[i] = Serial2.read();
  }
}

void getIncline() {
  if (debug) {
    Serial.print("DEBUG ");
    currentIncline[0] = 2;
    currentIncline[1] = 10;
    currentIncline[2] = 170;
    currentIncline[3] = 20;
  } else {
    readInclineCommand();
  }

  if(currentIncline[0] != INCLINE_CMD_ID) {
    return;
  }

  //Match up the received serial command to the inclineTable to find the incline in percent
  for (int i = 0; i < INCLINE_TABLE_SIZE; i++) {
    if(currentIncline[0] == inclineTable[i][0] &&
       currentIncline[1] == inclineTable[i][1] &&
       currentIncline[2] == inclineTable[i][2] &&
       currentIncline[3] == inclineTable[i][3]) {
      //Print the current incline in percent
      currentPercent = inclineTable[i][4];
      if (currentPercent != prevPercent) {
        Serial.print(currentPercent, 1);
        Serial.println(" %");
        prevPercent = currentPercent;
      }
    }
  }
}

void getStop() {
  if (debug != true) {
      for(int i=0; i < 3; i++) {
      // Serial.print("Serial Data ");
      currentStop[i] = Serial2.read();
    }
  } 
  // Print the received serial command, useful for debugging
  Serial.print("{Command Received} 0, 255, 245");
  for (int i=0; i < 3; i++) {
    Serial.print(", ");
    Serial.print((int)currentStop[i]);
  }
  Serial.println();
  if ( 
    currentStop[0] == stopCommand[0] &&
    currentStop[1] == stopCommand[1] &&
    currentStop[2] == stopCommand[2]
  ) {
    Serial.println("Stop Command Detected");
  }
}

void sendBLE() {
  BLEDevice central = BLE.central();

  // Check if a central device is connected
  if (!central || !central.connected()) {
    digitalWrite(LED_BUILTIN, LOW); // Turn off the built-in LED
    return;
  }

  // Turn on the built-in LED
  digitalWrite(LED_BUILTIN, HIGH);

  // Calculate speed and incline
  float kmph = currentMPH * mphtokmph;
  uint16_t inst_speed = static_cast<uint16_t>(kmph * 100); // Instantaneous speed in units of .01 km/h
  uint16_t incl_percent = static_cast<uint16_t>(currentPercent * 10); // Incline in units of .1 percent

  // Prepare the data to be sent
  byte byteArray[6] = {
    4, 0, // Flags for the FTMS protocol
    static_cast<byte>(inst_speed), static_cast<byte>(inst_speed >> 8), // Speed
    static_cast<byte>(incl_percent), static_cast<byte>(incl_percent >> 8) // Incline
  };

  // Send the data
  RSCMeasurementChar.writeValue(byteArray, sizeof(byteArray));
  RSCMeasurementChar.notify(); // Manually trigger the notification

  // Turn off the built-in LED
  digitalWrite(LED_BUILTIN, LOW);
}
