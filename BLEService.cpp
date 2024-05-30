#include "BLEService.h"
BLEService fitness_machine_service("1826"); // Fitness Machine Service
BLECharacteristic RSCMeasurementChar("2ACD", BLENotify, 8); // Treadmill Data
