#include <ArduinoBLE.h>
#include "BLEService.h"
#include "SpeedMapping.h"
#include "InclineMapping.h"

bool debug = false; //Set this to true if currently running it without a serial connection to the treadmill

const int stopCommand[3] = {1,0,182};

float currentSpeed[4];
float currentMPH;
float prevMPH = 13;
float currentIncline[4];
float currentPercent;
float prevPercent = 13;
int   currentStop[3];

float kmph;
float mphtokmph = 1.609344;
uint16_t inst_speed = 0; // Instantaneous speed in units of .01 km/h
uint16_t incl_percent = 20; // How feel going uphill or downhill. Signed, units of .1 percent

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
	
  sendBLE();

  while(debug == true) {
    Serial.print("DEBUG ");
    getSpeed();
  }
  
  if(Serial2.available() > 0) {
    char data1 = Serial2.read();
    if(data1 != 0) {
      return;
    }
    char data2 = Serial2.read();
    if(data2 != 255) {
      return;
    }
    char data3 = Serial2.read();
    switch (data3) {
      case 241: {
        getSpeed();
        break;
      }
      case 246: {
        getIncline();
        break;
      }
      case 249: {
        // Serial.println("Heartbeat Detected"); 
        break;
      }
      case 245: {
        getStop();
        break;
      }
      default: {
        Serial.print("Unknown Command detected: ");
        Serial.print("data3 = ");
        Serial.println(data3, DEC);
        Serial.print("{Command Received} ");
        int unknownCommand[4];
        for (int i = 0; i < 3; i++) {
          unknownCommand[i] = Serial2.read();
        }
        Serial.print("0, 255, ");
        Serial.print(data3, DEC);
        for (int i = 0; i < 4; i++) {
          Serial.print(", ");
          Serial.print(unknownCommand[i]);
        }
        Serial.println();
        break;
      } 
    }
  }
}

void getSpeed() {
  if (debug != true) {
    for(int i=0; i < 4; i++) {
      // Serial.print("Serial Data ");
      currentSpeed[i] = Serial2.read();
    }
  } else {
        Serial.print("DEBUG ");
        currentSpeed[0] = 2;
        currentSpeed[1] = 10;
        currentSpeed[2] = 170;
        currentSpeed[3] = 20;
  }

  if(currentSpeed[0] != 2) {return;}

  // Print the received serial command, useful for debugging
  // Serial.print("{Command Received} 0, 255, 241");
  // for (int i=0; i < 4; i++) {
  //   Serial.print(", ");
  //   Serial.print((int)currentSpeed[i]);
  // }
  // Serial.print(" = ");

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
    // Serial.print(currentMPH, 1);
    // Serial.println("mph ");
    if (currentMPH != prevMPH) {

        Serial.print(currentMPH, 1);
        Serial.println(" MPH");
        prevMPH = currentMPH;
      }
    }
  }
}




void getIncline() {
    if (debug != true) {
      for(int i=0; i < 4; i++) {
      // Serial.print("Serial Data ");
      currentIncline[i] = Serial2.read();
    }
  } else {
        Serial.print("DEBUG ");
        currentIncline[0] = 2;
        currentIncline[1] = 10;
        currentIncline[2] = 170;
        currentIncline[3] = 20;
  }

  if(currentIncline[0] != 2) {return;}

  // Print the received serial command, useful for debugging
  // Serial.print("{Command Received} 0, 255, 246");
  // for (int i=0; i < 4; i++) {
  //   Serial.print(", ");
  //   Serial.print((int)currentIncline[i]);
  // }
  // Serial.print(" = ");

  //Match up the received serial command to the inclineTable to find the incline in percent
  for (int i=0; i < 25; i++) {
  if(
    currentIncline[0] == inclineTable[i][0] &&
    currentIncline[1] == inclineTable[i][1] &&
    currentIncline[2] == inclineTable[i][2] &&
    currentIncline[3] == inclineTable[i][3]
  ) {
    //Print the current speed in MPH
    currentPercent = inclineTable[i][4];
    // Serial.print(currentPercent, 1);
    // Serial.println(" %");
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

  if (central) {
    // Serial.print("Connected to central: ");
    // Serial.println(central.address());
    digitalWrite(LED_BUILTIN,HIGH);
    if (central.connected()) {
    // kmph = 10.5;
    kmph = currentMPH * mphtokmph;
    inst_speed = kmph*100; // Sent in units of .01 km/h
    incl_percent = currentPercent;

   // 2 bytes of flags, 
   byte byteArray[6] = {
   4,0,
  (unsigned byte)inst_speed, (unsigned byte) (inst_speed >> 8),
  (unsigned byte)incl_percent, (unsigned byte) (incl_percent >> 8)
  // (unsigned byte)ramp_angle, (unsigned byte) (ramp_angle >> 8)
  };

  RSCMeasurementChar.writeValue(byteArray,8);
    }
  }
  digitalWrite(LED_BUILTIN,LOW);
  // Serial.print("Disconnected from central: ");
  // Serial.println(central.address());
}
