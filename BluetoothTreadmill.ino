#include <ArduinoBLE.h>

BLEService fitness_machine_service("1826"); // Fitness Machine Service
BLECharacteristic RSCMeasurementChar("2ACD", BLENotify, 8); // Treadmill Data

bool debug = false; //Set this to true if currently running it without a serial connection to the treadmill

// Speed Mapping
// This consists of byte4 byte5 byte6 byte7 of the serial command, followed by the mapped speed in mph
const float speedTable[117][5] = {
  {2,0,0,221,0},
	{2,0,161,16,0.5},
	{2,0,195,201,0.6},
	{2,0,230,186,0.7},
	{2,1,8,144,0.8},
	{2,1,42,116,0.9},
	{2,1,76,105,1},
	{2,1,111,188,1.1},
	{2,1,145,33,1.2},
	{2,1,179,197,1.3},
	{2,1,213,216,1.4},
	{2,1,248,18,1.5},
	{2,2,26,156,1.6},
	{2,2,60,188,1.7},
	{2,2,94,101,1.8},
	{2,2,129,79,1.9},
	{2,2,163,171,2},
	{2,2,197,182,2.1},
	{2,2,231,82,2.2},
	{2,3,10,43,2.3},
	{2,3,44,11,2.4},
	{2,3,78,210,2.5},
	{2,3,112,8,2.6},
	{2,3,147,154,2.7},
	{2,3,181,186,2.8},
	{2,3,215,99,2.9},
	{2,3,249,250,3},
	{2,4,28,96,3.1},
	{2,4,62,132,3.2},
	{2,4,96,229,3.3},
	{2,4,130,70,3.4},
	{2,4,165,87,3.5},
	{2,4,199,142,3.6},
	{2,4,233,23,3.7},
	{2,5,11,64,3.8},
	{2,5,46,51,3.9},
	{2,5,80,212,4},
	{2,5,114,48,4.1},
	{2,5,148,87,4.2},
	{2,5,183,130,4.3},
	{2,5,217,38,4.4},
	{2,5,251,194,4.5},
	{2,6,29,136,4.6},
	{2,6,64,186,4.7},
	{2,6,98,94,4.8},
	{2,6,132,57,4.9},
	{2,6,166,221,5},
	{2,6,200,121,5.1},
	{2,6,235,172,5.2},
	{2,7,13,63,5.3},
	{2,7,47,219,5.4},
	{2,7,81,60,5.5},
	{2,7,116,79,5.6},
	{2,7,150,236,5.7},
	{2,7,184,117,5.8},
	{2,7,218,172,5.9},
	{2,7,253,189,6},
	{2,8,31,135,6.1},
	{2,8,65,230,6.2},
	{2,8,99,2,6.3},
	{2,8,134,54,6.4},
	{2,8,168,175,6.5},
	{2,8,202,118,6.6},
	{2,8,236,86,6.7},
	{2,9,15,48,6.8},
	{2,9,49,234,6.9},
	{2,9,83,51,7},
	{2,9,117,19,7.1},
	{2,9,152,158,7.2},
	{2,9,186,122,7.3},
	{2,9,220,103,7.4},
	{2,9,254,131,7.5},
	{2,10,33,132,7.6},
	{2,10,67,93,7.7},
	{2,10,101,125,7.8},
	{2,10,135,222,7.9},
	{2,10,170,20,8},
	{2,10,204,9,8.1},
	{2,10,238,237,8.2},
	{2,11,16,132,8.3},
	{2,11,51,81,8.4},
	{2,11,85,76,8.5},
	{2,11,119,168,8.6},
	{2,11,153,118,8.7},
	{2,11,188,5,8.8},
	{2,11,222,220,8.9},
	{2,12,0,105,9},
	{2,12,34,141,9.1},
	{2,12,69,161,9.2},
	{2,12,103,69,9.3},
	{2,12,137,155,9.4},
	{2,12,171,127,9.5},
	{2,12,206,49,9.6},
	{2,12,240,235,9.7},
	{2,13,18,188,9.8},
	{2,13,52,156,9.9},
	{2,13,87,116,10},
	{2,13,121,237,10.1},
	{2,13,155,78,10.2},
	{2,13,189,110,10.3},
	{2,13,224,92,10.4},
	{2,14,2,210,10.5},
	{2,14,36,242,10.6},
	{2,14,70,43,10.7},
	{2,14,105,131,10.8},
	{2,14,139,32,10.9},
	{2,14,173,0,11},
	{2,14,207,217,11.1},
	{2,14,242,80,11.2},
	{2,15,20,195,11.3},
	{2,15,54,39,11.4},
	{2,15,88,131,11.5},
	{2,15,123,86,11.6},
	{2,15,157,49,11.7},
	{2,15,191,213,11.8},
	{2,15,225,180,11.9},
	{2,16,4,119,12}
};

// Incline Mapping
// This consists of byte4 byte5 byte6 byte7 of the serial command, followed by the mapped incline in %
const float inclineTable[25][8] = {
	{2,0,40,112,0},
	{2,0,80,49,0.5},
	{2,0,120,14,1},
	{2,0,160,179,1.5},
	{2,0,200,177,2},
	{2,0,240,205,2.5},
	{2,1,24,65,3},
	{2,1,64,134,3.5},
	{2,1,104,185,4},
	{2,1,144,130,4.5},
	{2,1,184,189,5},
	{2,1,224,122,5.5},
	{2,2,8,47,6},
	{2,2,48,83,6.5},
	{2,2,88,81,7},
	{2,2,128,236,7.5},
	{2,2,168,211,8},
	{2,2,208,146,8.5},
	{2,2,248,173,9},
	{2,3,32,228,9.5},
	{2,3,72,230,10},
	{2,3,112,154,10.5},
	{2,3,152,226,11},
	{2,3,192,37,11.5},
	{2,3,232,26,12}
};

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
  // put your setup code here, to run once:
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
  // put your main code here, to run repeatedly:
  // delay(1000);

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