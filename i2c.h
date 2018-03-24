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

//general defines
#define MAX_TWI_BUFFER_LENGHT 0x04

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
	
//typedef!
typedef struct _i2c_Param{
	volatile uint8_t ReadData[MAX_TWI_BUFFER_LENGHT];
	uint8_t ReadData_Size;
	volatile uint8_t WriteData[MAX_TWI_BUFFER_LENGHT];
	uint8_t WriteData_Size;	
	uint8_t ret;
}_i2c_Param;
	
 typedef struct _I2CInfo{
	I2CMode mode;
	uint8_t InterruptEnabled;
	char* error; //change into char array?
	}_I2CInfo;
_I2CInfo I2CInfo;


//---------------------------------
//		- Base Functions -
//---------------------------------

//setup i2c, dont set an address of our device and setup functions as SYNC
void i2c_Init(void);
//setup i2c, set the device as the given address and setup functions as ASYNC
void i2c_Init_addr(uint8_t addr,uint8_t inter_enable);

//set the callbacks for the ASYNC i2c functions. these will be called back with the result
void setI2cReadCallback(void* cb);
void setI2cWriteCallback(void* cb);
void setI2cSlaveReadCallback(_i2c_Param* (*cb));
void setI2cSlaveWriteCallback(void* cb);

uint8_t i2c_GetStatus(void);
uint8_t* GetErrorMsg(void);


//---------------------------------
//		- Master Functions -
//---------------------------------
int8_t i2c_Read8(uint8_t addr,uint8_t* read_data);
int8_t i2c_Read16(uint8_t addr,uint16_t* read_data);
int8_t i2c_Write8(uint8_t addr,uint8_t write_data);
int8_t i2c_Write16(uint8_t addr,uint16_t write_data);

//---------------------------------
//		- Master Functions -
//---------------------------------
//slave is completely interrupt based. set the slave callback if you want to get in on the action :P
