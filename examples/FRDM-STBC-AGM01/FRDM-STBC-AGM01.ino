/*
 * Example for Shield Arduino FRDM STBC AGM01 of Freescale
 * Author: Andres Sabas 
 * 17 Agust 2015
 * 
 * 
 */
#include <Wire.h>
#include <FXAS21002C.h>
#include <FXOS8700CQ.h>
FXOS8700CQ sensor = FXOS8700CQ(0x1E); //SA0=1 0x1F
FXAS21002C sensor2 = FXAS21002C(0x20); // SA0=1 0x21
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize the FXOS8700CQ
  sensor.init();
  
  // Initialize the FXAS21002C
  sensor2.init();
}
void loop() {
  // put your main code here, to run repeatedly:
  sensor2.getGres();
  // Query the sensor
  sensor.readAccelData();
  sensor.readMagData();
  sensor.readTempData();
  
  // Query the sensor
  sensor2.readGyroData();
  sensor2.readTempData();
  // Print out the data
  // Gyroscope
  Serial.print("Gyroscope ");
  Serial.print("X: ");
  Serial.print((int)sensor2.gyroData.x);
  Serial.print(" Y: ");
  Serial.print((int)sensor2.gyroData.y);
  Serial.print(" Z: ");
  Serial.println((int)sensor2.gyroData.z);
  // Temperatura Gyroscope
  Serial.print(" T Gyroscope: ");
  Serial.println((int)sensor2.tempData);
  
  // Print out the data
  // Accelerometer
  Serial.print("Accel ");
  Serial.print("X: ");
  Serial.print((int)sensor.accelData.x);
  Serial.print(" Y: ");
  Serial.print((int)sensor.accelData.y);
  Serial.print(" Z: ");
  Serial.println((int)sensor.accelData.z);
  
  // Magnometer
  Serial.print("Mag ");
  Serial.print("X: ");
  Serial.print((int)sensor.magData.x);
  Serial.print(" Y: ");
  Serial.print((int)sensor.magData.y);
  Serial.print(" Z: ");
  Serial.println((int)sensor.magData.z);
  // Temperatura Magnometer
  Serial.print(" T Gyroscope: ");
  Serial.println((int)sensor.tempData);
  delay(100);
  
}
