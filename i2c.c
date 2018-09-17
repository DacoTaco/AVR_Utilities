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

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <util/twi.h>
#include "i2c.h"
//#include "serial.h"

//general defines
#define ADDR_MASK(x) (x & 0xFE)

//define CPU frequency here if its not defined in the Makefile
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

//the i2c clock speed in kHz
#ifndef SCL_CLOCK
#define SCL_CLOCK  400
#endif

//typedef
typedef struct _i2c_Param{
	volatile uint8_t ReadData[MAX_TWI_BUFFER_LENGHT];
	uint8_t ReadData_Size;
	volatile uint8_t WriteData[MAX_TWI_BUFFER_LENGHT];
	uint8_t WriteData_Size;	
	volatile int8_t ret;
}_i2c_Param;

//callbacks
void (*cb_read)(int8_t,uint8_t byte) = NULL;

//redo cb to remove the requirement for the Param. also split Param?
int8_t* (*cb_slave_read)(int8_t) = NULL;
void (*cb_slave_write)(int8_t,uint8_t byte) = NULL;

//general Variables
uint8_t _i2c_init = 0;
uint8_t dev_addr = 0x00;
volatile _i2c_Param i2c_data;
volatile uint8_t response_valid = 0;
volatile uint8_t TX_Index = 0;
volatile uint8_t RX_Index = 0;




//Override the Recv callback, or return it to default by giving NULL
void setI2cReadCallback(void* cb)
{
	cb_read = cb;
}
void setI2cSlaveReadCallback(int8_t* (*cb))
{
	cb_slave_read = cb;
}
void setI2cSlaveWriteCallback(void* cb)
{
	cb_slave_write = cb;
}

void _initI2c(uint8_t* addr, uint8_t inter_enable)
{
	if(_i2c_init)
		return;
	
#ifdef _FORCE_INTERRUPT_MODE_
	inter_enable = 1;
#endif
	
	I2CInfo.mode = ready;
	if(inter_enable > 0)
	{
		I2CInfo.InterruptEnabled = 1;
		//cb_read = NULL;
	}
	
	if(addr != NULL)
	{
		TWAR = ADDR_MASK(*addr);
	}
	
	// set i2c bitrate
	/*formula according to the ATMega8 documentation : 
		freq = F_CPU/ ( 16 + 2(TWBR)*(Prescaler) */
	#ifdef TWPS0
		// for processors with additional bitrate division (mega128)
		// SCL freq = F_CPU/(16+2*TWBR*4^TWPS)
		// set TWPS to zero
		TWSR &= ~(1<<TWPS0); 
		TWSR &= ~(1<<TWPS1); 
	#endif
	// calculate bitrate division	
	uint8_t bitrate_div = ((F_CPU/1000l)/SCL_CLOCK);
	if(bitrate_div >= 16)
		bitrate_div = (bitrate_div-16)/2;
	TWBR = bitrate_div;
	
	//setup the variables
	dev_addr = 0;
	
	//enable TWI
	TWCR = 	(1<<TWEN) | //enable TWI
			(1<<TWIE) | //enable interrupt
			(1<<TWEA); //respond with ACK to TWI calls (to this device's address or all calls, depending on if address was given in init). setting this bit to 0 causes the TWI to not listen anymore

	//enable interrupts
	sei();	
	
	_i2c_init = 1;
	return;
}

void i2c_Init(void)
{
	_initI2c(NULL,0);
}
void i2c_Init_addr(uint8_t addr,uint8_t inter_enable)
{
	_initI2c(&addr,inter_enable);
}


//FUCKING VOLTILE'S WONT WORK WITH MEMCPY/MEMSET FML!!
/*void CopyParamToResponse(_i2c_Param* param)
{
	if(param == NULL)
		return;
	
	for(uint8_t i=0;i<MAX_TWI_BUFFER_LENGHT;i++)
	{
		i2c_data.ReadData[i] = param->ReadData[i];
		i2c_data.WriteData[i] = param->WriteData[i];
	}
	i2c_data.ReadData_Size = param->ReadData_Size;
	i2c_data.WriteData_Size = param->WriteData_Size;
	i2c_data.ret = param->ret;
}

void ClearDataBuffer(void)
{
	for(uint8_t i=0;i<MAX_TWI_BUFFER_LENGHT;i++)
	{
		i2c_data.ReadData[i] = 0;
		i2c_data.WriteData[i] = 0;
	}
	i2c_data.ReadData_Size = 0;
	i2c_data.WriteData_Size = 0;
	i2c_data.ret = 0;
} */

void resetVariables(void)
{
	dev_addr = 0;
	response_valid = 0;
	
	for(uint8_t i=0;i<MAX_TWI_BUFFER_LENGHT;i++)
	{
		i2c_data.ReadData[i] = 0;
		i2c_data.WriteData[i] = 0;
	}
	
	i2c_data.ReadData_Size = 0;
	i2c_data.WriteData_Size = 0;
	i2c_data.ret = 0;
	
	TX_Index = 0;
	RX_Index = 0;
}

int8_t _i2c_stop(void)
{
	//send stop signal
	TWCR |= ((1<<TWEN) | (1<<TWSTO) | (1<<TWINT) | (1<<TWEA) );
	//stop start signal
    TWCR &= ~(1<<TWSTA); 
	
	//wait for bus to be released
	while(TWCR & (1<<TWSTO));
	
	//lets not reset the variables as it clears our response & therefor the output xD
	//resetVariables();
	I2CInfo.mode = ready;
	response_valid = 0;
	
	return 1;
}

int8_t _i2c_write(uint8_t data)
{
	TWDR = data;
	return 1;
}
uint8_t _i2c_read(void)
{
	return TWDR;
}

int8_t _i2c_start(void)
{
	//wait untill hardware is ready
	while(I2CInfo.mode != ready);
	
	//send start signal
	I2CInfo.mode = started;
	TWCR |= ((1<<TWSTA) | (1<<TWINT) | (1<<TWEA)); 
	
	return 1;
}
int8_t _i2c_write_next(void)
{
	if(response_valid == 0 || TX_Index >= i2c_data.WriteData_Size || TX_Index >= MAX_TWI_BUFFER_LENGHT)
		return -1;
	
	//cprintf("writing 0x%02X\n\r",i2c_data.WriteData[TX_Index]);
	
	_i2c_write(i2c_data.WriteData[TX_Index]);
	TX_Index++;
	
	return 1;
}

int8_t i2c_write(uint8_t addr,uint16_t write_data,uint8_t size)
{
	if(_i2c_init == 0)
	{
		return -1;
	}
	
	if(size > MAX_TWI_BUFFER_LENGHT || size == 0)
	{
		return -102;
	}
	
	//wait untill hardware is ready
	while(I2CInfo.mode != ready);
	
	//prepare the data for the device
	resetVariables();
	dev_addr = (ADDR_MASK(addr) | I2C_WRITE);
	
	i2c_data.WriteData_Size = size;
	i2c_data.WriteData[0] = (write_data & 0xFF00) >> 8;
	i2c_data.WriteData[1] = (write_data & 0x00FF);
	
	response_valid = 1;
	
	//send start. this will trigger all the interrupts!
	_i2c_start();
	
	if(I2CInfo.InterruptEnabled == 0)
	{
		//while untill all data is send
		while(response_valid != 0);
		//delay or otherwise we can be off before fully done xD
		return i2c_data.ret;
	}
	else
	{
		//if we are playing with interrupts, it'll fire when read is done or an error occured
		return 1;
	}
	return 1;
	
}

int8_t i2c_Write8(uint8_t addr,uint8_t write_data)
{
	uint16_t data = write_data << 8;
	int8_t ret;
	ret = i2c_write(addr,data,1);
	return ret;
}
int8_t i2c_Write16(uint8_t addr,uint16_t write_data)
{
	return i2c_write(addr,write_data,2);
}

int8_t i2c_read(uint8_t addr,uint16_t* read_data,uint8_t size)
{
	if(_i2c_init == 0)
	{
		return -1;
	}
	
	if(read_data == NULL || size > MAX_TWI_BUFFER_LENGHT || size == 0)
	{
		return -102;
	}
	
	//wait untill hardware is ready
	while(I2CInfo.mode != ready);
	
	//prepare the address of the device & ret
	resetVariables();
	dev_addr = (ADDR_MASK(addr) + I2C_READ);
	
	i2c_data.ReadData_Size = size;
	response_valid = 1;
	
	//send start. this will trigger all the interrupts!
	_i2c_start();
	
	if(I2CInfo.InterruptEnabled == 0)
	{
		//TWI is still processing our data...
		while(response_valid != 0);
		//delay or otherwise we can be off before data is read xD
		
		//copy over data from i2c_data
		*read_data = (i2c_data.ReadData[0] << 8) | (i2c_data.ReadData[1] & 0xFF);
				
		//clear signals, or send repeating start for more data
		//not needed as we stopped handled that in the ISR
		//_i2c_stop();
		
		return i2c_data.ret;
	}
	else
	{
		//if we are playing with interrupts, it'll fire when read is done or an error occured
		return 1;
	}
	return 1;
}
int8_t i2c_Read8(uint8_t addr,uint8_t* read_data)
{
	if(read_data == NULL)
		return -102;
	
	uint16_t data;
	int8_t ret;
	ret = i2c_read(addr,&data,1);
	*read_data = ((data & 0xFF00) >> 8);
	return ret;
}
int8_t i2c_Read16(uint8_t addr,uint16_t* read_data)
{	
	return i2c_read(addr,read_data,2);
}

//get the current i2c status
uint8_t i2c_GetStatus(void)
{
    return (TWSR & 0xF8);
}

//Interrupt handler of receiving or sending data. currently not enabled
ISR(TWI_vect)
{	
	uint8_t status = i2c_GetStatus();
	/*cprintf("status : ");
	cprintf_char(status);
	cprintf("\n\r");*/
	//cprintf("status : '0x%02X'\n\r",status);
	
	//OK, so. TWI.
	//for master mode, if a read bit was set we are running in MR mode, otherwise MT mode
	//for slave mode, if a read bit was set we are functioning in SR mode, otherwise in ST mode
	switch(status)
	{
		//--------------------------------------
		//connection lost and resetting stuff
		//--------------------------------------
		case TW_MR_SLA_NACK:   //0x48 SLA+R transmitted , NACK returned
		case TW_MT_SLA_NACK:   //0x20 SLA+W transmitted , NACK returned	
			//we probably addressed something not available. lets indicate it
			i2c_data.ret = -106;
			_i2c_stop();
			if(I2CInfo.InterruptEnabled == 1 && cb_read != NULL)
			{
				cb_read(-1,0);
			}
			break;
		case TW_BUS_ERROR:   //0x00
		case TW_MT_DATA_NACK:   //0x30 Data transmitted , NACK returned :(		
		//case TW_MT_ARB_LOST:   //0x38
		case TW_MR_ARB_LOST:   //0x38 - as master, we lost connection if i understand it correctly xD
			//basically, we lost connection and we either send a start signal for repeatingstart or close it off otherwise
			//cprintf("something went wrong :(\n\r",status);
			_i2c_stop();
			if(I2CInfo.InterruptEnabled == 1 && cb_read != NULL)
			{
				cb_read(-1,0);
			}
			break;
		
		
		//--------------------------------------
		//master cases - transmitting data
		//--------------------------------------		
		case TW_REP_START:   //0x10 - a repeated start has been transmitted
			//we have been successful in sending start and gaining control.
		case TW_START:   //0x08 - A start condition has been transmitted. send address of device we want to talk to
			//cprintf("start send, sending dev addr\n\r");
			_i2c_write(dev_addr);
			//clear the start signal
			TWCR &= ~((1<<TWSTA)); // clear TWSTA
			TWCR |= ((1<<TWINT) | (1<<TWEA));
			break;
		
		case TW_MT_SLA_ACK:   //0x18 - SLA+W transmitted, we received ACK. device ready for data		
		case TW_MT_DATA_ACK:   //0x28 - Data send and ACK received. if we have more data, send that shit!
			if(TX_Index < i2c_data.WriteData_Size && I2CInfo.mode != exiting)
			{
				//send data and ACK
				_i2c_write_next();
				i2c_data.ret = TX_Index;
				
			}
			else
			{
				I2CInfo.mode = exiting;
			}
			
			if(TX_Index > i2c_data.WriteData_Size || I2CInfo.mode == exiting)
			{
				//TODO : place repeating start here
				_i2c_stop();
			}
			else if(TX_Index < i2c_data.WriteData_Size)
			{
				//clear start and stop bits just in case, and send ACK!
				//TWCR &= ~((1<<TWSTA) | (1<<TWSTO)); 
				TWCR |= ((1<<TWINT) | (1<<TWEA));
			}
			else //if(TX_Index == i2c_data.WriteData_Size)
			{
				TWCR &= ~(1<<TWEA);
				TWCR |= (1<<TWINT);
			}
		
			break;
	
	
		//--------------------------------------
		//master cases - receiving data
		//--------------------------------------
		case TW_MR_SLA_ACK:   //0x40
			//we send the addr, and device is ready for reading!
			//clear any start and stop signals and lets read!
			TWCR &= ~((1<<TWSTA) | (1<<TWSTO)); 
			
			//set bit so we send ACK after reading
			TWCR |= ((1<<TWEA) | (1<<TWINT));
			break;
		case TW_MR_DATA_NACK:   //0x58 Data Received, NACK afterwards :( , probably signal to just kill
			I2CInfo.mode = exiting;
		case TW_MR_DATA_ACK:   //0x50 we received data from the slave, and an ACK!
			//read data from TWDR!
			if(RX_Index < i2c_data.ReadData_Size && I2CInfo.mode != exiting)
			{
				//uint8_t byte = _i2c_read();
				i2c_data.ReadData[RX_Index] = _i2c_read();
				//cprintf("read 0x%02X\n\r",i2c_data.ReadData[RX_Index]);
				RX_Index++;
			}
			else
			{
				I2CInfo.mode = exiting;
			}

			if(
				cb_read != NULL && 
				(RX_Index >= MAX_TWI_BUFFER_LENGHT || RX_Index >= i2c_data.ReadData_Size) &&
				I2CInfo.mode != exiting
			)
			{
				//TODO : only fire callback when all data is received
				//HOW DO WE EVEN KNOW WHEN ALL IS RECEIVED XD
				//move to the right else under this code
				cb_read(1,i2c_data.ReadData[RX_Index-1]);
			}
			
			if(RX_Index > i2c_data.ReadData_Size || I2CInfo.mode == exiting)
			{
				//TODO : implement repeating task here
				_i2c_stop();
			}
			else if(RX_Index < i2c_data.ReadData_Size)
			{
				//we are expecting more data. so time to send ACK and get more!
				TWCR |= ((1<<TWEA) | (1<<TWINT));
			} 
			else //if(RX_Index == i2c_data.ReadData_Size)
			{
				//we read everything, send NACK and in our next loop stop
				TWCR &= ~(1<<TWEA);
				TWCR |= (1<<TWINT);
			}
			break;		

			
		//--------------------------------------
		//Slave cases - Sending Data
		//--------------------------------------
#ifndef _MASTER_ONLY_
		case TW_ST_DATA_ACK:   //0xB8 - we have more data to send. send and if we have done all send NACK, else ACK
		case TW_ST_SLA_ACK:   //0xA8
			//we received our addr and send ack, time to send data!
			//retrieve data from cb, and get data. else, dont respond.	
			if(response_valid <= 0)
			{
				//response is null, lets see if we can get a response first...
				if(cb_slave_read != NULL)
				{	
					//TODO : redo the read cb
					/*int8_t data = cb_slave_read(status);
					
					if(ret != NULL && ret->WriteData_Size > 0 && ret->WriteData != NULL)
					{
						CopyParamToResponse(ret);
						response_valid = 1;
					}*/
					response_valid = 0;
				}
				
				if(response_valid <= 0)
				{
					//no data was prepped. lets bail
					resetVariables();
					_i2c_write(0xFF);
				}
			}
			
			if(response_valid > 0 && TX_Index <= MAX_TWI_BUFFER_LENGHT)
			{
				//we have some data to send. lets go!
				_i2c_write_next();	
			}
			
			
			//set up the response!
			if(response_valid <= 0 || TX_Index >= i2c_data.WriteData_Size)
			{
				resetVariables();
				TWCR &= ~((1<<TWSTO) | (1<<TWEA));
				TWCR |= (1<<TWINT);
				
			}
			else
			{
				//we have more data to send. response correctly!
				TWCR |= ((1<<TWEA) | (1<<TWINT));
			}	
			break;
		case TW_ST_DATA_NACK:   //0xC0
			break;
		case TW_ST_ARB_LOST_SLA_ACK:   //0xB0
			//arbitration as master lost, own slave addr received + ACK. datasheet said to act like just sending data
		case TW_ST_LAST_DATA:   //0xC8
			//all data send. lets re-enter slave mode 
			TWCR |= ((1<<TWEA) | (1<<TWINT));
			break;
		
		
		//--------------------------------------
		//Slave cases - Receiving Data
		//--------------------------------------
		case TW_SR_ARB_LOST_GCALL_ACK:   //0x78 - also status if master lost in Master mode
		case TW_SR_GCALL_ACK:   //0x70
		case TW_SR_ARB_LOST_SLA_ACK:   //0x68 - also status if master lost in Master mode
		case TW_SR_SLA_ACK:   //0x60 - //we received our addr and send ack, lets respond!
			//set bits so we send ACK after reading
			//TWCR &= ~(1<<TWSTO); 
			TWCR |= ((1<<TWEA) | (1<<TWINT));
			break;
		case TW_SR_GCALL_DATA_NACK:   //0x98
		case TW_SR_DATA_NACK:   //0x88 
		case TW_SR_GCALL_DATA_ACK:   //0x90
		case TW_SR_DATA_ACK:   //0x80 - data has been received, ACK has been send as response
		
			//TODO : only fire callback when all data is received
			if(cb_slave_write != NULL && RX_Index < MAX_TWI_BUFFER_LENGHT)
			{
				i2c_data.ReadData[RX_Index] = _i2c_read();
				cb_slave_write(status,i2c_data.ReadData[RX_Index]);
				RX_Index++;
			}
			TWCR &= ~(1<<TWSTO);
			//TWCR |= (1<<TWINT);
			//send ACK reply if we want more data
			TWCR |= ((1<<TWEA) | (1<<TWINT));
			break; 
		case TW_SR_STOP:   //0xA0 - we received a stop signal. release bus and listen again
			//go back to listening for a call
			TWCR |= ((1<<TWEA) | (1<<TWINT));
			resetVariables();
			//_i2c_stop();
			break;
#endif
		
		case TW_NO_INFO:   //0xF8
		default:
			break;
	}
	
	//set interrupt as handled. seems to make the interrupt keep firing...
	/*if(I2CInfo.mode != ready && I2CInfo.mode != started)
		TWCR |= (1<<TWINT);*/
}
