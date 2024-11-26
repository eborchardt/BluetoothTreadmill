#include <bleFTMS.h>
#include <HardwareSerial.h>
#include "InclineMapping.h"
#include "SpeedMapping.h"


bool debug = false; //Set this to true if currently running it without a serial connection to the treadmill

const int stopCommand[3] = {1,0,182};

// Define command codes
const byte CMD_GET_SPEED = 241;
const byte CMD_GET_INCLINE = 246;
const byte CMD_HEARTBEAT = 249;
const byte CMD_GET_STOP = 245;

typedef struct {
  float kmph;
  float incline;
  float elevationGain;
  float totalDistance;
  int cadence;
} CurrentSettings;

CurrentSettings currentSettings = {
  .kmph = 0,
  .incline = 0.0,
  .elevationGain = 0.0,
  .totalDistance = 0.0,
  .cadence = 0
};

byte currentSpeed[4] = {0};
float currentMPH;
float prevMPH = 13;
float currentIncline[4];
float currentPercent;
float prevPercent = 13;
int   currentStop[3];

unsigned long lastSendTime = 0;
unsigned long speedMillisInterval=0;
unsigned long currentSpeedMillis=0;
unsigned long prevSpeedMillis=0;
unsigned long lastSpeedUpdateTime = 0;
float deltaSpeed = 0;
float totalDistance = 0;

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

bool previousLEDState = LOW;

void setup() {
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // Treadmill Connection
  Serial.begin(9600);  // USB Port for Debugging
  pinMode(LED_BUILTIN,OUTPUT);
  

  initBLE("Livestrong LS8.0T Development");

}

void loop() {
  if (bleClientConnected != previousLEDState) {
    previousLEDState = bleClientConnected;
    digitalWrite(LED_BUILTIN, bleClientConnected ? HIGH : LOW);
  }

  unsigned long currentTime = millis();

  // Check if a second (1000 milliseconds) has passed since the last send
  if (currentTime - lastSendTime >= 1000) {
    sendBLE();
    lastSendTime = currentTime;
  }

  while(debug == true) {
    getSpeed();
  }
  
  if(Serial2.available() > 2) {
    byte data1 = Serial2.read();
    if(data1 != 0) {
      return;
    }
    // Serial.println("zero detected!");

    byte data2 = Serial2.read();
    if(data2 != 255) {
      return;
    }
    // Serial.println("255 detected!");

    byte data3 = Serial2.read();

    // The next byte will determine the type of message received
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
        Serial.print(millis());
        Serial.println(" Heartbeat Detected?"); 
        break;
      }
      case CMD_GET_STOP: {
        getStop();
        break;
      }
      case 255: {
        printUnknownCommand(data3);
        break;
      }
      default: {
        printUnknownCommand(data3);
        break;
      } 
    }
  }
}

// This is not at all accurate yet, so don't rely on it
void updateDistance() {
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastSpeedUpdateTime) / 1000; // Use Seconds

  if (deltaTime >= 15) { // If 10+ seconds have passed
    float deltaDistance = currentMPH * deltaTime / 3600; // deltaDistance (miles) = deltaSpeed (mph) * deltaTime (seconds) / 3600;
    lastSpeedUpdateTime = currentTime;
  }
}

float getTotalDistance() {
  return totalDistance;
}

void printUnknownCommand(byte cmd) {
  Serial.print("Unknown Command detected: ");
  Serial.print("data3 = ");
  Serial.println(cmd, DEC);
  Serial.print("{Command Received} ");

  // I don't know how many more bytes to expect in unknown commands, arbitrary 4
  byte unknownCommand[4];
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

void getSpeed() {
    currentSpeedMillis = millis();
    speedMillisInterval = currentSpeedMillis - prevSpeedMillis;
  if (speedMillisInterval >= 1000) { // Only update the current speed once per second
      prevSpeedMillis = currentSpeedMillis;
    if (debug != true) {
      for (int i = 0; i < 4; i++) {
        while (!Serial2.available() >= 1) {} // Wait for data
        currentSpeed[i] = Serial2.read();
      }

    // Fake speed data for debugging
    } else {
          currentSpeed[0] = 2;
          currentSpeed[1] = 10;
          currentSpeed[2] = 170;
          currentSpeed[3] = 20;
    }

    //Match up the received serial command to the speedTable to find the speed in MPH
    for (int i=0; i < 117; i++) {
    if(
      currentSpeed[0] == speedTable[i][0] &&
      currentSpeed[1] == speedTable[i][1] &&
      currentSpeed[2] == speedTable[i][2] &&
      currentSpeed[3] == speedTable[i][3]
    ) {

      //Print the current speed in MPH
      currentMPH = speedTable[i][4];
      }
    }
  }
  Serial2.flush(); // I'm not positive this makes any difference
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
  if (!bleClientConnected) {
    Serial.println("Not connected to BLE Client");
    return;}

  // Turn off the built-in LED
  digitalWrite(LED_BUILTIN, LOW);

  // Prepare the data for BLE
  float kmph = currentMPH * mphtokmph;
  currentSettings.kmph = static_cast<uint16_t>(kmph * 100); // Instantaneous speed in units of .01 km/h
  currentSettings.incline = static_cast<uint16_t>(currentPercent * 10); // Incline in units of .1 percent
  currentSettings.elevationGain = 0;
  currentSettings.totalDistance = 0;
  currentSettings.cadence = 0;

  // Send data over BLE
  updateBLEdata(currentSettings.kmph, 
                currentSettings.incline, 
                currentSettings.elevationGain, 
                currentSettings.totalDistance, 
                currentSettings.cadence);

  delay(25);
  // Turn on the built-in LED
  digitalWrite(LED_BUILTIN, HIGH);
}
