/*
shift - An AVR library to shift out data to IC's like SN74HC595N
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
#include "shift.h"

//general variables
uint8_t _shift_init == 0;
	
volatile uint8_t *CTRL_PORT = NULL;
volatile uint8_t *CTRL_PIN = NULL;
volatile uint8_t *CTRL_DDR = NULL;

volatile uint8_t *CTRL_LATCH = NULL;
volatile uint8_t *CTRL_DATA = NULL;
volatile uint8_t *CTRL_CLK = NULL;


#define SetPin(x,y) _setPin(x,(1<<y))
#define ClearPin(x,y) _clearPin(x,(1<<y))

//the Barebone functions
//--------------------------------
void _setPin(volatile uint8_t *port,uint8_t mask)
{
	*port |= mask;
}
void _clearPin(volatile uint8_t *port,uint8_t mask)
{
	*port &= ~mask;
}

int8_t Shift_Init(volatile uint8_t *Shift_DDR,volatile uint8_t *Shift_Port,volatile uint8_t *Shift_Pin,
				volatile uint8_t *Latch_Pin,volatile uint8_t *Data_Pin,volatile uint8_t *Clock_Pin)
{
	if(
		Shift_DDR == NULL || Shift_Port == NULL || Shift_Pin == NULL ||
		Latch_Pin == NULL || Data_Pin == NULL || Clock_Pin == NULL
		)
	{
		return -1;	
	}
	
	CTRL_DDR = Shift_DDR;
	CTRL_PORT = Shift_Port;
	CTRL_PIN = Shift_Pin;
	
	CTRL_LATCH = Latch_Pin;
	CTRL_DATA = Data_Pin;
	CTRL_CLK = Clock_Pin;
	
	//set the pins correctly, as outputs
	CTRL_DDR |= ( (1 << CTRL_DATA) | (1 << CTRL_CLK) | (1 << CTRL_LATCH) );
	
	//we are init
	_shift_init = 1;
	return 1;
	
}
int8_t ShiftData(uint8_t data)
{
	if(_shift_init == 0)
		return -1;
	
	//write the 16 bits to the shift register
	ClearPin(CTRL_PORT,CTRL_LATCH);
	for(uint8_t i=0;i<16;i++)
	{
		if(data & 0b1000000000000000)
		{
			//set the data bit as high
			SetPin(CTRL_PORT,CTRL_DATA);
		}
		else
		{
			//set the data bit as low
			ClearPin(CTRL_PORT,CTRL_DATA);
		}
		
		//pulse the shifting register to set the bit
		SetPin(CTRL_PORT,CTRL_CLK);
		_delay_us(1);
		ClearPin(CTRL_PORT,CTRL_CLK);
		
		//shift with 1 bit, so we can take on the next bit
		data = data << 1;
	}	
	//all bits transfered. time to let the shifting register latch the data and set the pins accordingly
	SetPin(CTRL_PORT,CTRL_LATCH);	
}