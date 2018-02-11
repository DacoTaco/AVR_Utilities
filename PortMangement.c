/*
Port Mangement - An AVR library to handle ports/pins more easily

Copyright (C) 2016-2018  DacoTaco
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
#include "PortMangement.h"

void _setPin(volatile uint8_t *port, uint8_t mask)
{
	*port |= mask;
}
void _clearPin(volatile uint8_t *port, uint8_t mask)
{
	*port &= ~mask;
}

int8_t _checkPin(volatile uint8_t *port , uint8_t Pin)
{
	if((*port & (1<< Pin))==0)
		return LOW;
	else
		return HIGH;
}

void _setPortMode(volatile uint8_t *port_ddr, volatile uint8_t *port, uint8_t mode)
{
	if(mode == 0)
	{
		//set as input;
		*port_ddr &= ~(0xFF); //0b00000000;
		//enable pull up resistors
		*port = 0xFF;
	}
	else
	{
		//set as output
		*port_ddr |= (0xFF);//0b11111111;
		//set output as 0x00
		*port = 0x00;
	}

}
