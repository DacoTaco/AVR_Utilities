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
#include <compat/twi.h>
#include "i2c.h"


//define CPU frequency here if its not defined in the Makefile
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

//the i2c clock speed in Hz
//the PCF8575 runs at 400khz... :/
/*formula according to the ATMega8 documentation : 
	freq = F_CPU/ ( 16 + 2(TWBR)*(Prescaler) */
#define SCL_CLOCK  200000L

void i2c_Init(void)
{
    //set SCL to SCL_CLOCK
    TWSR = 0x00;
    TWBR = 0x0C;//((F_CPU/SCL_CLOCK)-16)/2;
    //enable TWI
    TWCR = (1<<TWEN);
}
//send start signal, this is send before a data signal
int8_t i2c_Start(uint8_t device_addr)
{
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
	
	_i2c_write(device_addr);
	uint8_t status = i2c_GetStatus();
	
	if(status != TW_MT_SLA_ACK && status != TW_MR_SLA_ACK)
		return 0;
	
	return 1;
	
}
//send stop signal, send after a data signal
void i2c_Stop(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}



//send data
void i2c_Write(uint8_t addr, uint8_t data)
{
	i2c_Start(addr);
	_i2c_write(data);
	i2c_Stop();
}
void _i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}




//receive data
uint8_t i2c_Read(uint8_t addr)
{
	i2c_Start(addr);
	uint8_t data = _i2c_readACK();
	i2c_Stop();
	return data;
}
uint8_t _i2c_readACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}
//receive byte & send NACK
uint8_t _i2c_readNACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

//get the current i2c status
uint8_t i2c_GetStatus(void)
{
    return (TWSR & 0xF8);
}

