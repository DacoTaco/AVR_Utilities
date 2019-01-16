/*
spi - An AVR library to communicate with other devices over the spi protocol/bus
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

#include "spi.h"

int8_t _spi_mode = 0;
int8_t _control_cs = 0;
volatile uint8_t *_spi_ddr = &DDRB;
volatile uint8_t *_spi_port = &PORTB;

#ifdef __AVR_ATmega8__
	uint8_t CS_PIN = PB1;
	uint8_t SS_PIN = PB2;
	uint8_t MOSI_PIN = PB3;
	uint8_t MISO_PIN = PB4;
	uint8_t SCK_PIN = PB5;	
#endif

//Initialize SPI Master Device
void spi_init()
{
	return spi_init_as(1,1);				
}

void spi_init_as(int8_t master, int8_t control_cs)
{
	if(_spi_mode > 0)
		return;
	
	uint8_t _ddr = *_spi_ddr;
	uint8_t _port = *_spi_port;
	uint8_t _spcr = 0;
	_control_cs = control_cs;
	
	if(master != 0)
	{	
		//Set SS,MOSI & SCK as Output
		_ddr |= ((1<<SS_PIN)|(1<<MOSI_PIN)|(1<<SCK_PIN));  

		//Set MISO as input
		_ddr &= ~(1<<MISO_PIN);
		
		// set MOSI as output LOW & MISO as tristate input
		_port &= ~((1<<MOSI_PIN)|(1<<MISO_PIN)); 
		
		// set CS_PIN , SS HIGH + set pull up of SCK
		_port |= ((1<<SS_PIN)|(1<<SCK_PIN)); 
		
		if(_control_cs > 0)
		{
			//Set CS_PIN as output and set high
			_ddr |= (1<<CS_PIN);
			_port |= (1<<CS_PIN);
		}
		
		//Set as Master
		_spcr = (1<<SPE)|(1<<MSTR);
	}
	else
	{
		_ddr |= (1<<MISO_PIN);     //MISO as OUTPUT
		_ddr &= ~((1<<SCK_PIN)|(1<<SS_PIN)|(1<<MOSI_PIN));   //SCK,SS,MOSI as INPUT
		
		_port |= (1<<MISO_PIN);	//set MISO as HIGH
		_spcr = (1<<SPE);    //Enable SPI
	}
	
	*_spi_ddr = _ddr;
	*_spi_port = _port;
	SPCR = _spcr;
	
	_spi_mode = 1;
	if(master == 0)
		_spi_mode++;
	
	return;
}

//Function to send and receive data
uint8_t spi_tranceiver_8(uint8_t data)
{
	if(_spi_mode < 1)
		return 0x00;
	
	uint8_t d;
	if(_spi_mode == 1 && _control_cs == 1)
	{
		//in master mode we need to set the CS pin low to signal the slave we are going to transmit data
		*_spi_port &= ~(1<<CS_PIN); //set CS LOW
		
		//give the slave some time to set its data
		asm("nop");
		asm("nop");	              
	}
	
	SPDR = data; // Load data into the buffer
	while(!(SPSR & (1<<SPIF) )); //Wait until transmission complete
	d = SPDR; //get data
	
	if(_spi_mode == 1 && _control_cs == 1)
	{
		*_spi_port |= (1<<CS_PIN); //set CS high	
	}
	
	//Return received data
	return d; 
}

uint16_t spi_tranceiver(uint16_t data)
{
	uint8_t d = data >> 8;
	uint16_t r = 0x00;
	
	r = spi_tranceiver_8(d) << 8;
	r = r | spi_tranceiver_8(data & 0xFF);
    return(r);
}