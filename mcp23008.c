/*
mcp23008 - An AVR library to communicate with the MCP23X08 chip family. uses the i2c or SPI in this library
Copyright (C) 2018-201x  DacoTaco
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
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>
#include <string.h>

#ifdef _SPI_MODE
	#include "spi.h"
	//only accept bit 6 & 7 as addr bits -> 0000 0xx0
	#define _DEV_ADDR 0x06
#else
	#include "i2c.h"
	//only accept bit 5,6 & 7 as addr bits -> 0000 xxx0
	#define _DEV_ADDR 0x0E
#endif


#include "serial.h"
#include "mcp23008.h"

#define REG_ADD(x,y) (uint16_t)((x << 8) | y)
#define CALC_ADDR(x) (BASE_23008_ADDR | (x & _DEV_ADDR))

int8_t _mcp23008_init[0x0F] = {0};

void mcp23008_init(uint8_t dev_addr)
{
	cprintf_debug("init mcp23008\n\r");
	cprintf_debug_char(dev_addr);
	cprintf_debug("\n\r");
	cprintf_debug_char(_mcp23008_init[dev_addr & _DEV_ADDR]);
	cprintf_debug("\n\r");
	if(_mcp23008_init[dev_addr & _DEV_ADDR] != 0)
		return;
	
	cprintf_debug("init dev\n\r");

#ifdef _SPI_MODE
	spi_init_as(1,0);
	
	//set the CS pin, we will use PB1
	DDRB |= (1<<PB1);
	PORTB |= (1<<PB1);
	
	cprintf_debug("SPI done\n\r");
#else
	i2c_Init();
	cprintf_debug("i2c done\n\r");
#endif
	
	//uint8_t addr = CALC_ADDR(dev_addr);
	//dev_addr = CALC_ADDR(dev_addr);
	//basically, all we do is set all the default value's. just to be sure the device is set as expected
	mcp23008_WriteReg(dev_addr,IODIR,0xFF);	
	cprintf_debug("IODIR\n\r");	
	
	mcp23008_WriteReg(dev_addr,GPINTEN,0x00);
	cprintf_debug("GPINTEN\n\r");
	mcp23008_WriteReg(dev_addr,IOCON,0x00);
	cprintf_debug("IOCON\n\r");
	mcp23008_WriteReg(dev_addr,IPOL,0x00);
	cprintf_debug("IPOL\n\r");
	mcp23008_WriteReg(dev_addr,GPPU,0x00);
	cprintf_debug("GGPU\n\r");
	mcp23008_WriteReg(dev_addr,GPIO,0x00);
	cprintf_debug("GPIO\n\r");
	
	_mcp23008_init[dev_addr & _DEV_ADDR] = 1;
	cprintf_debug("init!\n\r");
	
}

int8_t mcp23008_ReadReg(uint8_t dev_addr, uint8_t reg,uint8_t* read_data)
{
	if(reg > 0x0A || read_data == NULL)
	{
		return -103;
	}
	
	uint8_t addr = CALC_ADDR(dev_addr);
#ifdef _SPI_MODE
	addr |= SPI_READ;
	PORTB &= ~(1<<PB1);
	spi_tranceiver(REG_ADD(addr,reg));
	*read_data = spi_tranceiver_8(0x00);
	PORTB |= (1<<PB1);
	return 1;
#else
	i2c_Write8(addr,reg);
	return i2c_Read8(addr,read_data);
#endif
}

int8_t mcp23008_WriteReg(uint8_t dev_addr, uint8_t reg,uint8_t value)
{
	if(reg < 0 || reg > 0x0A)
	{
		return 0;
	}
	
	uint8_t addr = CALC_ADDR(dev_addr);
#ifdef _SPI_MODE
	//this is a moot operation, cause write is 0. just here for completeness
	//addr |= SPI_WRITE; 
	
	PORTB &= ~(1<<PB1);
	spi_tranceiver(REG_ADD(addr,reg));
	int8_t d = spi_tranceiver_8(value);
	PORTB |= (1<<PB1);
	return d;
#else
	return i2c_Write16(addr,REG_ADD(reg,value));
#endif
}