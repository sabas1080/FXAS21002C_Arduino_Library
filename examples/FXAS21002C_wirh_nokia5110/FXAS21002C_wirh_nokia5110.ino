/* FXAS21002C Example Code
 Original sketch by: Kris Winer
 May 30, 2014

 Modified from FXAS21002C  by Sabas
 
 license: Beerware - Use this code however you'd like. If you 
 find it useful you can buy me a beer some time.
 
 FXAS21002C is a small, low-power, 3-axis yaw, pitch, and roll
 angular rate gyroscope. The full-scale range is adjustable from
 ±250°/s to ±2000°/s. It features both I2C and SPI interfaces. We parameterize the registers, 
 calibrate the gyro, get properly scaled angular rates, and display all on an on-breadboard 
 84 x 68 Nokia 5110 LCD display.
 
 We also set up the angular rate threshold detection so rates over some fraction (here 10%) of the full scale are
 displayed with axis over threshold and the polarity.
 
 Hardware setup:
 FXAS21002D Breakout ------- Arduino
 3.3V --------------------- 3.3V
 SDA ----------------------- A4
 SCL ----------------------- A5
 GND ---------------------- GND
 
 SDA and SCL should have external pull-up resistors (to 3.3V).
 10k resistors are on the FRDM-FXS-MULTI breakout board.
 
 Note: The FXAS21002C is an I2C sensor; here we make use of the Arduino Wire library.
 Because the sensor is not 5V tolerant, we are using a 3.3 V 8 MHz Pro Mini or a 3.3 V Teensy 3.1.
 We have disabled the internal pull-ups used by the Wire library in the Wire.h/twi.c utility file.
 We are also using the 400 kHz fast I2C mode by setting the TWI_FREQ  to 400000L /twi.h utility file.
 */
 
#include "Wire.h"
#include <SPI.h>  
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <FXAS21002C.h>

FXAS21002C sensor = FXAS21002C(0x20); // SA0=1 0x21

// Using NOKIA 5110 monochrome 84 x 48 pixel display
// pin 9 - Serial clock out (SCLK)
// pin 8 - Serial data out (DIN)
// pin 7 - Data/Command select (D/C)
// pin 5 - LCD chip select (CS)
// pin 6 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(9, 8, 7, 5, 6);

// Pin definitions
int int1Pin = 12;  // These can be changed, 2 and 3 are the Arduinos ext int pins
int ledPin  = 13;  // Pro Mini led

int8_t tempCount;
float temperature;
uint32_t count = 0;
boolean sleepMode = false;

void setup()
{
  Serial.begin(38400);

//  lcd.begin(16, 2);// Initialize the LCD with 16 characters and 2 lines
 
  // Set up the interrupt pins, they're set as active high, push-pull
  pinMode(int1Pin, INPUT);
  digitalWrite(int1Pin, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  display.begin(); // Initialize the display
  display.setContrast(58); // Set the contrast
  display.setRotation(2); //  0 or 2) width = width, 1 or 3) width = height, swapped etc.

  
// Start device display with ID of sensor
  display.clearDisplay();
  display.setCursor(0,0); display.print("FXAS21000");
  display.setCursor(0, 20); display.print("3-axis 14-bit");
  display.setCursor(0, 30); display.print("gyroscope");
  display.setCursor(0, 40); display.print("25 mdeg/s LSB");
  display.display();
  delay(1000);

// Set up for data display
  display.setTextSize(1); // Set text size to normal, 2 is twice normal etc.
  display.setTextColor(BLACK); // Set pixel color; 1 on the monochrome screen
  display.clearDisplay();   // clears the screen and buffer
  
  // Read the WHO_AM_I register, this is a good test of communication
  /*byte c = readReg(FXAS21002C_H_WHO_AM_I);  // Read WHO_AM_I register
  display.clearDisplay();
  display.setCursor(0,0); display.print("FXAS21002C");  
  display.setCursor(0,10); display.print("I Am");
  display.setCursor(0, 20); display.print("Ox");display.print(c, HEX);  
  display.setCursor(0, 30); display.print("I Should be"); 
  display.setCursor(0, 40); display.print("Ox");display.print(0xD1, HEX);  
  display.display();
  delay(1000);
  
  if (c == 0xD1) // WHO_AM_I should always be 0x2A
  {  
    //FXAS21002CReset(); // Start by resetting sensor device to default settings
    sensor.calibrate(gBias);
    sensor.init();  // init the accelerometer if communication is OK
    Serial.println("FXAS21002C is online...");

  display.clearDisplay();
  display.setCursor(0,0); display.print("FXAS21002C");  
  display.setCursor(0,10); display.print("gyro bias");
  display.setCursor(0, 20); display.print("x "); display.print(gBias[0], 2); display.print(" o/s");
  display.setCursor(0, 30); display.print("y "); display.print(gBias[1], 2); display.print(" o/s");
  display.setCursor(0, 40); display.print("z "); display.print(gBias[2], 2); display.print(" o/s");
  display.display();
  delay(1000);
  //}
  /*else
  {
    Serial.print("Could not connect to FXAS21002C: 0x");
    Serial.println(c, HEX);
    while(1) ; // Loop forever if communication doesn't happen
  }*/
}

void loop()
{  
  // One can use the interrupt pins to detect a data ready condition; here we just check the STATUS register for a data ready bit
  /*
  if(readReg(FXAS21002C_H_DR_STATUS) & 0x08)  // When this bit set, all axes have new data
  {
    sensor.readGyroData(gyroCount);  // Read the x/y/z adc values
    sensor.getGres();
    
    // Calculate the gyro value into actual degrees per second
    gx = (float)gyroCount[0]*gRes - gBias[0];  // get actual gyro value, this depends on scale being set
    gy = (float)gyroCount[1]*gRes - gBias[1];  
    gz = (float)gyroCount[2]*gRes - gBias[2];   
    
    // Print out values
    Serial.print("x-rate = "); Serial.print(gx); Serial.print(" deg/s");   
    Serial.print("y-rate = "); Serial.print(gy); Serial.print(" deg/s");   
    Serial.print("z-rate = "); Serial.print(gz); Serial.print(" deg/s");  
      
    tempCount = sensor.readTempData();  // Read the x/y/z adc values
    temperature = (float) tempCount; // Temperature in degrees Centigrade
  }
  */
    uint32_t deltat = millis() - count;
    if (deltat > 500) { // update LCD once per half-second independent of read rate

    display.clearDisplay();    
    display.setCursor(0,0);   display.print("FXAS21002C gyro");
    display.setCursor(0,8);   display.print("x ");  display.print((int)sensor.gyroData.x); 
    display.setCursor(43,8);  display.print(" deg/s");
    display.setCursor(0,16);  display.print("y "); display.print((int)sensor.gyroData.y); 
    display.setCursor(43,16); display.print(" deg/s");
    display.setCursor(0,24);  display.print("z "); display.print((int)sensor.gyroData.z); 
    display.setCursor(43,24); display.print(" deg/s");
    //display.setCursor(0, 32); display.print("T "); display.print(grados); 
    //display.setCursor(43,32); display.print(" C");
   display.display(); 

    count = millis();
    digitalWrite(ledPin, !digitalRead(ledPin));
    }

  // One can use the interrupt pins to detect a motion/tap condition; 
  // here we just check the RT_SOURCE register to interpret the rate interrupt condition
  /*byte source = readReg(FXAS21002C_H_RT_SRC);  // Read the interrupt source register
  if(source & 0x40) {  // Check if event flag has been set
 
   // Check source of event
   if(source & 0x20) {    // Z-axis rate event
    if(source & 0x10) {
    Serial.println("-z-axis RT exceeded");
    display.setCursor(48, 40); display.print("-z "); 
    }
    else {
    Serial.println("+z-axis rate exceeded");
    display.setCursor(48, 40); display.print("+z ");
    }
   }
 
   // Check source of event
   if(source & 0x08) {    // Y-axis rate event
    if(source & 0x04) {
    Serial.println("-y-axis RT exceeded");
    display.setCursor(24, 40); display.print("-y ");
    }
    else {
    Serial.println("+y-axis rate exceeded");
    display.setCursor(24, 40); display.print("+y ");
    }
   }
   
      // Check source of event
   if(source & 0x02) {    // X-axis rate event
    if(source & 0x01) {
    Serial.println("-x-axis RT exceeded");
    display.setCursor(0, 40); display.print("-x ");
    }
    else {
    Serial.println("+x-axis rate exceeded");
    display.setCursor(0, 40); display.print("+x ");
    }
   }
    display.setCursor(64, 40); display.print("RT!");
    display.display();
  }

}
*/
display.clearDisplay();
display.setCursor(0,0); display.print("FXAS21002C");
display.setCursor(0,8); display.print("boot end");
display.setCursor(0,16); display.print("flag");  
display.setCursor(0,24); display.print("detected");  
display.display();
}
