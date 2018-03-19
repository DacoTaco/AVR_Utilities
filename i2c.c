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
#include <stdio.h>
#include <compat/twi.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "serial.h"

//general defines
#define ADDR_MASK(x) (x & 0xFE)
#define MAX_TWI_BUFFER_LENGHT 11

//define CPU frequency here if its not defined in the Makefile
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

//the i2c clock speed in Hz
//the PCF8575 runs at 400khz... :/
/*formula according to the ATMega8 documentation : 
	freq = F_CPU/ ( 16 + 2(TWBR)*(Prescaler) */
#ifndef SCL_CLOCK
#define SCL_CLOCK  400000L
#endif

//callbacks
void (*cb_read)(int8_t,uint8_t) = NULL;
void (*cb_write)(int8_t) = NULL;
_slaveReadData* (*cb_slave_read)(int8_t) = NULL;
void (*cb_slave_write)(int8_t,uint8_t) = NULL;

//general Variables
uint8_t dev_addr = 0x00;
uint8_t TXData[MAX_TWI_BUFFER_LENGHT] = {0};
uint8_t TX_Size = 0;
uint8_t TX_Index = 0;


//Override the Recv callback, or return it to default by giving NULL
void setI2cReadCallback(void* cb)
{
	cb_read = cb;
}
void setI2cWriteCallback(void* cb)
{
	cb_write = cb;
}
void setI2cSlaveReadCallback(_slaveReadData* (*cb))
{
	cb_slave_read = cb;
}
void setI2cSlaveWriteCallback(void* cb)
{
	cb_slave_write = cb;
}

void _initI2c(uint8_t* addr, uint8_t inter_enable)
{
	I2CInfo.mode = ready;
	I2CInfo.error = NULL;
	if(inter_enable > 0)
	{
		I2CInfo.InterruptEnabled = 1;
	}
	
	if(addr != NULL)
	{
		TWAR = ADDR_MASK(*addr);
	}
	
	//set callbacks to null
	/*setI2cReadCallback(NULL);
	setI2cWriteCallback(NULL);
	setI2cSlaveCallback(NULL);*/
	
	//disabled untill we make that part
    //set SCL to SCL_CLOCK
    TWSR &= ~((1<<TWPS0) | (1<<TWPS1)); //0x00;
    TWBR = ((F_CPU/SCL_CLOCK)-16)/2;
	
	//TWCR |= (1<<TWEA) | (1<<TWEN) | (1<<TWIE); 
	uint8_t mask = 	(1<<TWEN) | //enable TWI
					(1<<TWIE) | //enable interrupt
					(1<<TWEA); //respond with ACK to TWI calls (to this device's address or all calls, depending on if address was given in init). setting this bit to 0 causes the TWI to not listen anymore
					
	//setup the variables
	dev_addr = 0;
	
    //enable TWI
    TWCR = mask;

	//enable interrupts
	sei();	
}

void i2c_Init(void)
{
	_initI2c(NULL,0);
}
void i2c_Init_addr(uint8_t addr,uint8_t inter_enable)
{
	_initI2c(&addr,inter_enable);
}

int8_t _i2c_stop(void)
{
	//send stop signal
	TWCR |= ((1<<TWSTO) | (1<<TWINT) | (1<<TWEA));
	//stop start signal
    TWCR &= ~(1<<TWSTA); 
	
	//wait for bus to be released
	while(TWCR & (1<<TWSTO));
	
	I2CInfo.mode = ready;
	dev_addr = 0;
	ReadData.reading = 0;
	TXData[0] = 0x00;
	TX_Size = 0;
	TX_Index = 0;
	
	return 1;
}

uint8_t _i2c_write(uint8_t data)
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
	TWCR |= (1<<TWSTA); 
	
	return 1;
}


uint8_t i2c_Write(uint8_t addr,uint8_t data)
{
	//wait untill hardware is ready
	while(I2CInfo.mode != ready);
	
	//prepare the data for the device
	dev_addr = (ADDR_MASK(addr) + I2C_WRITE);
	TXData[0] = data;
	TX_Size = 1;
	TX_Index = 0;
	
	
	//send start. this will trigger all the interrupts!
	_i2c_start();
	
	if(I2CInfo.InterruptEnabled == 0)
	{
		//while untill all data is send
		//TODO : fix this while, its stuck in loop
		while(TX_Size != 0);
		//delay or otherwise we can be off before fully done xD
		_delay_ms(10);
		
		return 1;
	}
	else
	{
		//if we are playing with interrupts, it'll fire when read is done or an error occured
		return 1;
	}
	return 1;
}

uint8_t i2c_Read(uint8_t addr,uint8_t repeating)
{	
	//wait untill hardware is ready
	while(I2CInfo.mode != ready);
	
	//prepare the address of the device
	dev_addr = (ADDR_MASK(addr) + I2C_READ);
	
	//send start. this will trigger all the interrupts!
	_i2c_start();
	
	if(I2CInfo.InterruptEnabled == 0)
	{
		//TWI is still processing our data...
		while(ReadData.reading == 1);
		//delay or otherwise we can be off before data is read xD
		_delay_ms(10);
		
		//clear signals, or send repeating start for more data
		//not needed as we stopped handled that in the ISR
		//_i2c_stop();
		
		return ReadData.data;
	}
	else
	{
		//if we are playing with interrupts, it'll fire when read is done or an error occured
		return 1;
	}
	return 1;
}




void _sendACK_NACK(uint8_t ACK)
{
	uint8_t mask = (1<<TWINT)|(1<<TWEN);
	if(ACK)
	{
		mask |= (1<<TWEA);
	}
	if(I2CInfo.InterruptEnabled)
	{
		mask |= (1<<TWIE);
	}
	TWCR = mask;
	return;
}

void SendACK(void)
{
	_sendACK_NACK(1);
}
void SendNACK(void)
{
	_sendACK_NACK(0);
}

//get the current i2c status
uint8_t i2c_GetStatus(void)
{
    return (TWSR & 0xF8);
}
uint8_t* GetErrorMsg(void)
{
	uint8_t* str = (uint8_t*)I2CInfo.error;
	I2CInfo.error = NULL;
	if(str == NULL)
		str = (uint8_t*)"\0";
	return str;
}

//Interrupt handler of receiving or sending data. currently not enabled
ISR(TWI_vect)
{	
	uint8_t status = i2c_GetStatus();
	//cprintf("status : '0x%02X'\n\r",status);
	
	//OK, so. TWI.
	//for master mode, if a read bit was set we are running in MR mode, otherwise MT mode
	//for slave mode, if a read bit was set we are functioning in SR mode, otherwise in ST mode
	switch(status)
	{
		//--------------------------------------
		//connection lost and resetting stuff
		//--------------------------------------
		case TW_BUS_ERROR:   //0x00
		case TW_MR_SLA_NACK:   //0x48 SLA+R transmitted , NACK returned
		case TW_MT_SLA_NACK:   //0x20 SLA+W transmitted , NACK returned		
		case TW_MT_DATA_NACK:   //0x30 Data transmitted , NACK returned :(		
		//case TW_MT_ARB_LOST:   //0x38
		case TW_MR_ARB_LOST:   //0x38 - as master, we lost connection if i understand it correctly xD
			//basically, we lost connection and we either send a start signal for repeatingstart or close it off otherwise
			//cprintf("something went wrong :(\n\r",status);
			I2CInfo.error = "NACK or connection lost";
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
			TWCR |= (1<<TWINT);
			break;
		case TW_MT_SLA_ACK:   //0x18 - SLA+W transmitted, we received ACK. device ready for data		
		case TW_MT_DATA_ACK:   //0x28 - Data send and ACK received. if we have more data, send that shit!
			//if we have MORE data to write, go to next case, else : break!
			if(TX_Index >= TX_Size)
			{
				TX_Index = 0;
				TX_Size = 0;		
				_i2c_stop();
			}
			else
			{
				//write data
				//cprintf("writing 0x%02X\n\r",TXData[TX_Index]);
				_i2c_write(TXData[TX_Index]);
				//clear any start and stop and lets write!
				TWCR &= ~((1<<TWSTA) | (1<<TWSTO) | (1<<TWEA)); 
				TWCR |= (1<<TWINT);
				TX_Index++;
			}			
			break;
	
	
		//--------------------------------------
		//master cases - receiving data
		//--------------------------------------
		case TW_MR_SLA_ACK:   //0x40
			//we send the addr, and device is ready for reading!
			//clear any start and stop signals and lets read!
			if(I2CInfo.InterruptEnabled == 0)
				ReadData.reading = 1;
			TWCR &= ~((1<<TWSTA) | (1<<TWSTO)); 
			
			//set bit so we send ACK after reading
			TWCR |= ((1<<TWEA) | (1<<TWINT));
			break;
		case TW_MR_DATA_NACK:   //0x58 Data Received, NACK afterwards :(
		case TW_MR_DATA_ACK:   //0x50
			//we received data from the slave, and an ACK!
			//read data from TWDR!
			//cprintf("data read!\n\r");	
			ReadData.data = _i2c_read();
			if(I2CInfo.InterruptEnabled == 0)
			{
				ReadData.reading = 0;
			}
			else if(cb_read != NULL)
			{
				cb_read(1,ReadData.data);
			}
			
			//TODO : if we want more(repeating start), implement this here
			_i2c_stop();
			break;		

			
		//--------------------------------------
		//Slave cases - Sending Data
		//--------------------------------------
		case TW_ST_SLA_ACK:   //0xA8
			//we received our addr and send ack, time to send data!
			//retrieve data from cb, and get data. else, dont respond.
			if(cb_slave_read != NULL)
			{
				_slaveReadData* data = cb_slave_read(status);
				
				if(data != NULL && data->ArraySize > 0 && data->data != NULL)
				{
					//send response!
					_i2c_write(data->data[0]);
					
					//clear stop signal and send!
					TWCR &= ~((1<<TWSTO) | (1<<TWEA));
					TWCR |= (1<<TWINT);
				}
			}
			break;
		case TW_ST_ARB_LOST_SLA_ACK:   //0xB0
			//arbitration as master lost, own slave addr received + ACK. datasheet said to act like just sending data
		case TW_ST_DATA_ACK:   //0xB8
			//_i2c_write(data)
			TWCR |= ((1<<TWEA) | (1<<TWINT));
			break;
		case TW_ST_DATA_NACK:   //0xC0
			break;
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
			if(cb_slave_write != NULL)
			{
				uint8_t data = _i2c_read();
				cb_slave_write(status,data);
			}
			TWCR &= ~(1<<TWSTO);
			TWCR |= (1<<TWINT);
			//send ACK reply if we want more data
			//TWCR |= ((1<<TWEA) | (1<<TWINT));
			break; 
		case TW_SR_STOP:   //0xA0 - we received a stop signal. release bus and listen again
			//go back to listening for a call
			TWCR |= ((1<<TWEA) | (1<<TWINT));
			//_i2c_stop();
			break;
		
		case TW_NO_INFO:   //0xF8
		default:
			break;
	}
	
	//set interrupt as handled. seems to make the interrupt keep firing...
	/*if(I2CInfo.mode != ready && I2CInfo.mode != started)
		TWCR |= (1<<TWINT);*/
}
