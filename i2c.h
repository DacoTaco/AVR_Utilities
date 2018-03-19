/*
i2c - An AVR library to communicate with other devices over the i2c/TWI protocol
Copyright (C) 2016-2017  DacoTaco
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

//bit used for marking the command read or write
#define I2C_READ    TW_READ
#define I2C_WRITE   TW_WRITE

#ifdef SAVE_SPACE
#define _FORCE_INTERRUPT_MODE_
#endif

#include <inttypes.h>

//variables
typedef enum {
	ready,
	started,
	masterTransmitter,
	masterReceiver,
	slaceTransmitter,
	slaveReciever
	} I2CMode;
	
 typedef struct _I2CInfo{
	I2CMode mode;
	uint8_t InterruptEnabled;
	char* error; //change into char array?
	}_I2CInfo;
_I2CInfo I2CInfo;

typedef struct _readData{
	int8_t reading;
	uint8_t data;
}_readData;

typedef struct _slaveReadData{
	int8_t ArraySize;
	uint8_t* data;
}_slaveReadData;

_readData ReadData;


//---------------------------------
//		- Base Functions -
//---------------------------------
void i2c_Init(void);
void i2c_Init_addr(uint8_t addr,uint8_t inter_enable);
void setI2cReadCallback(void* cb);
void setI2cWriteCallback(void* cb);
void setI2cSlaveReadCallback(_slaveReadData* (*cb));
void setI2cSlaveWriteCallback(void* cb);

uint8_t i2c_GetStatus(void);
uint8_t* GetErrorMsg(void);


//---------------------------------
//		- Master Functions -
//---------------------------------
uint8_t i2c_Read(uint8_t dev_addr,uint8_t repeating);
uint8_t i2c_Write(uint8_t addr,uint8_t data);

//---------------------------------
//		- Master Functions -
//---------------------------------
//slave is completely interrupt based. set the slave callback if you want to get in on the action :P
