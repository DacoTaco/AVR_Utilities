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

void _setPortMode(volatile uint8_t *port_ddr, volatile uint8_t *port, uint8_t mode, uint8_t mask)
{
	if(mode == INPUT)
	{
		//set as input, 0 bit == input
		//flip all bits in the mask ( 1 becomes 0, 0 becomes 1) and do an AND. this basically ensures 0 stays 0 and all 1's in mask become 0 in DDR
		*port_ddr &= ~(mask); //0b00000000;
		//enable pull up resistors, 1 bit = pull up enabled
		//add all 1's in mask to the port, enabling pull up
		*port |= mask;
	}
	else
	{
		//set as output, 1 bit = output
		*port_ddr |= (mask);//0b11111111;
		//set output as 0x00, 0 bit means normal output state ( 1 = tri-state)
		*port &= ~(mask);
	}

}