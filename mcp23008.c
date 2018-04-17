/*
mcp23008 - An AVR library to communicate with the MCP23008 chip. uses the i2c in this library
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

#include "i2c.h"
#include "serial.h"
#include "mcp23008.h"

#define REG_ADD(x,y) (uint16_t)((x << 8) + y)
#define CALC_ADDR(x) (BASE_23008_ADDR + (x & 0x0E))

int8_t _mcp23008_init[7] = {0};

void mcp23008_init(uint8_t dev_addr)
{
	if(_mcp23008_init[dev_addr & 0x0E])
		return;
	
	cprintf("lets init boys!\n\r");
	i2c_Init();
	
	//uint8_t addr = CALC_ADDR(dev_addr);
	//basically, all we do is set all the default value's. just to be sure the device is set as expected
	mcp23008_WriteReg(dev_addr,IODIR,0xFF);	
	mcp23008_WriteReg(dev_addr,GPINTEN,0x00);
	mcp23008_WriteReg(dev_addr,IOCON,0x00);
	mcp23008_WriteReg(dev_addr,IPOL,0x00);
	mcp23008_WriteReg(dev_addr,GPPU,0x00);
	mcp23008_WriteReg(dev_addr,GPIO,0x00);
	
	_mcp23008_init[dev_addr & 0x0E] = 1;
}

int8_t mcp23008_ReadReg(uint8_t dev_addr, uint8_t reg,uint8_t* read_data)
{
	if(reg > 0x0A || read_data == NULL)
	{
		return -103;
	}
	
	uint8_t addr = CALC_ADDR(dev_addr);
	i2c_Write8(addr,reg);
	return i2c_Read8(addr,read_data);
}

int8_t mcp23008_WriteReg(uint8_t dev_addr, uint8_t reg,uint8_t value)
{
	if(reg < 0 || reg > 0x0A)
	{
		return 0;
	}
	
	uint8_t addr = CALC_ADDR(dev_addr);
	return i2c_Write16(addr,REG_ADD(reg,value));
}