#include <Wire.h>
#include <math.h>

#include <FXAS21002C.h>

// Public Methods //////////////////////////////////////////////////////////////


FXAS21002C::FXAS21002C(byte addr)
{
	address = addr;
	gyroODR = GODR_200HZ; // In hybrid mode, accel/mag data sample rates are half of this value
	gyroFSR = GFS_500DPS;
}

void FXAS21002C::writeReg(byte reg, byte value)
{
	Wire.beginTransmission(address);
	Wire.write(reg);
	Wire.write(value);
	Wire.endTransmission();
}

// Reads a register
byte FXAS21002C::readReg(byte reg)
{
	byte value;

	Wire.beginTransmission(address);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(address, (uint8_t)1);
	value = Wire.read();

	return value;
}

void FXAS21002C::readRegs(byte reg, uint8_t count, byte dest[])
{
	uint8_t i = 0;

	Wire.beginTransmission(address);   // Initialize the Tx buffer
	Wire.write(reg);            	   // Put slave register address in Tx buffer
	Wire.endTransmission(false);       // Send the Tx buffer, but send a restart to keep connection alive
	Wire.requestFrom(address, count);  // Read bytes from slave register address 

	while (Wire.available()) {
		dest[i++] = Wire.read();   // Put read results in the Rx buffer
	}
}

// Read the temperature data
void FXAS21002C::readTempData()
{
	tempData = readReg(FXAS21002C_H_TEMP);
}

// Put the FXAS21002C into standby mode.
// It must be in standby for modifying most registers
void FXAS21002C::standby()
{
	byte c = readReg(FXAS21002C_H_CTRL_REG1);
	writeReg(FXAS21002C_H_CTRL_REG1, c & ~(0x03));// Clear bits 0 and 1; standby mode
}
// Sets the FXAS21000 to active mode.
// Needs to be in this mode to output data
void FXAS21002C::ready()
{
  byte c = readReg(FXAS21002C_H_CTRL_REG1);
  writeReg(FXAS21002C_H_CTRL_REG1, c & ~(0x03));  // Clear bits 0 and 1; standby mode
  writeReg(FXAS21002C_H_CTRL_REG1, c |   0x01);   // Set bit 0 to 1, ready mode; no data acquisition yet
}

// Put the FXAS21002C into active mode.
// Needs to be in this mode to output data.
void FXAS21002C::active()
{
	byte c = readReg(FXAS21002C_H_CTRL_REG1);
	writeReg(FXAS21002C_H_CTRL_REG1, c & ~(0x03));  // Clear bits 0 and 1; standby mode
  	writeReg(FXAS21002C_H_CTRL_REG1, c |   0x02);   // Set bit 1 to 1, active mode; data acquisition enabled
}

void FXAS21002C::init()
{
	standby();  // Must be in standby to change registers

	// Set up the full scale range to 250, 500, 1000, or 2000 deg/s.
	gyroFSR = GFS_500DPS;
	writeReg(FXAS21002C_H_CTRL_REG0, GFS_500DPS);
	 // Setup the 3 data rate bits, 4:2
	if (gyroODR < 8) 
		writeReg(FXAS21002C_H_CTRL_REG1, gyroODR << 2);   

	// Disable FIFO, route FIFO and rate threshold interrupts to INT2, enable data ready interrupt, route to INT1
  	// Active HIGH, push-pull output driver on interrupts
  	writeReg(FXAS21002C_H_CTRL_REG2, 0x0E);

  	 // Set up rate threshold detection; at max rate threshold = FSR; rate threshold = THS*FSR/128
  	writeReg(FXAS21002C_H_RT_CFG, 0x07);         // enable rate threshold detection on all axes
  	writeReg(FXAS21002C_H_RT_THS, 0x00 | 0x0D);  // unsigned 7-bit THS, set to one-tenth FSR; set clearing debounce counter
  	writeReg(FXAS21002C_H_RT_COUNT, 0x04);       // set to 4 (can set up to 255)         
	// Configure interrupts 1 and 2
	writeReg(FXAS21002C_H_CTRL_REG3, 0x00); // clear bits 0, 1 
	//writeReg(CTRL_REG3, readReg(CTRL_REG3) |  (0x02)); // select ACTIVE HIGH, push-pull interrupts    
	//writeReg(CTRL_REG4, readReg(CTRL_REG4) & ~(0x1D)); // clear bits 0, 3, and 4
	//writeReg(CTRL_REG4, readReg(CTRL_REG4) |  (0x1D)); // DRDY, Freefall/Motion, P/L and tap ints enabled  
	//writeReg(CTRL_REG5, 0x01);  // DRDY on INT1, P/L and taps on INT2

	active();  // Set to active to start reading
}

// Read the gyroscope data
void FXAS21002C::readGyroData()
{
	uint8_t rawData[6];  // x/y/z gyro register data stored here
	readRegs(FXAS21002C_H_OUT_X_MSB, 6, &rawData[0]);  // Read the six raw data registers into data array
	gyroData.x = ((int16_t)(((int16_t)rawData[0]) << 8 | ((int16_t) rawData[1])));
	gyroData.y = ((int16_t)(((int16_t)rawData[2]) << 8 | ((int16_t) rawData[3])));
	gyroData.z = ((int16_t)(((int16_t)rawData[4]) << 8 | ((int16_t) rawData[5])));
}

// Get accelerometer resolution
float FXAS21002C::getGres(void)
{
	switch (gyroFSR)
	{
		// Possible gyro scales (and their register bit settings) are:
  // 250 DPS (11), 500 DPS (10), 1000 DPS (01), and 2000 DPS  (00). 
    case GFS_2000DPS:
          return 2000.0/16384.0;
    case GFS_1000DPS:
          return 1000.0/16384.0;
    case GFS_500DPS:
          return 500.0/16384.0;       
    case GFS_250DPS:
          return 250.0/16384.0;
	}
}

void FXAS21002C::calibrate(float * gBias)
{
  int32_t gyro_bias[3] = {0, 0, 0};
  uint16_t ii, fcount;
  int16_t temp[3];
  
  // Clear all interrupts by reading the data output and STATUS registers
  //readGyroData(temp);
  readReg(FXAS21002C_H_STATUS);
  
  standby();  // Must be in standby to change registers

  writeReg(FXAS21002C_H_CTRL_REG1, 0x08);   // select 50 Hz ODR
  fcount = 50;                                     // sample for 1 second
  writeReg(FXAS21002C_H_CTRL_REG0, 0x03);   // select 200 deg/s full scale
  float gyrosensitivity = 32000.0/250.0; //GFS_250DPS;

  active();  // Set to active to start collecting data
   
  uint8_t rawData[6];  // x/y/z FIFO accel data stored here
  for(ii = 0; ii < fcount; ii++)   // construct count sums for each axis
  {
  readRegs(FXAS21002C_H_OUT_X_MSB, 6, &rawData[0]);  // Read the FIFO data registers into data array
  temp[0] = ((int16_t)( ((int16_t) rawData[0]) << 8 | ((int16_t) rawData[1])));
  temp[1] = ((int16_t)( ((int16_t) rawData[2]) << 8 | ((int16_t) rawData[3])));
  temp[2] = ((int16_t)( ((int16_t) rawData[4]) << 8 | ((int16_t) rawData[5])));
  
  gyro_bias[0] += (int32_t) temp[0];
  gyro_bias[1] += (int32_t) temp[1];
  gyro_bias[2] += (int32_t) temp[2];
  
  delay(25); // wait for next data sample at 50 Hz rate
  }
 
  gyro_bias[0] /= (int32_t) fcount; // get average values
  gyro_bias[1] /= (int32_t) fcount;
  gyro_bias[2] /= (int32_t) fcount;
  
  gBias[0] = (float)gyro_bias[0]/(float) gyrosensitivity; // get average values
  gBias[1] = (float)gyro_bias[1]/(float) gyrosensitivity; // get average values
  gBias[2] = (float)gyro_bias[2]/(float) gyrosensitivity; // get average values

  ready();  // Set to ready
}

void FXAS21002C::reset() 
{
	writeReg(FXAS21002C_H_CTRL_REG1, 0x20); // set reset bit to 1 to assert software reset to zero at end of boot process
	delay(100);
while(!(readReg(FXAS21002C_H_INT_SRC_FLAG) & 0x08))  { // wait for boot end flag to be set
}

}
// Private Methods //////////////////////////////////////////////////////////////
