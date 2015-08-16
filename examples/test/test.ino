#include <Wire.h>
#include <FXAS21002C.h>

FXAS21002C sensor = FXAS21002C(0x20); // SA0=1 0x21

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize the FXOS8700CQ
  sensor.init();
}

void loop() {
  // Query the sensor
  sensor.readGyroData();
  
  // Print out the data
  // Gyroscope
  Serial.print("Gyroscope ");
  Serial.print("X: ");
  Serial.print((int)sensor.gyroData.x);
  Serial.print(" Y: ");
  Serial.print((int)sensor.gyroData.y);
  Serial.print(" Z: ");
  Serial.println((int)sensor.gyroData.z);

  delay(100);
}
