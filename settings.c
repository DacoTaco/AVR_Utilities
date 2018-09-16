/*
settings - An AVR library to save settings to EEPROM
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
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "settings.h"

void _saveSettings(uint16_t address ,void* data, uint16_t size)
{	
	//GTFO if no valid data is given
	if(data == NULL || size == 0)
	{
		return;
	}
	
	uint16_t addr = address;
	
	while(eeprom_is_ready() == 0);
	
	//disable all interrupts so nothing weird happens in between
	cli();
	
	eeprom_write_block(data, (void*)addr, size);
	
	//reactivate all interrupts
	sei();
}

int8_t _readSettings(uint16_t address ,void* data, uint16_t size)
{
	//GTFO if no valid data is given
	if(data == NULL || size == 0)
	{
		return -1;
	}
	
	while(eeprom_is_ready() == 0);
	
	eeprom_read_block(data, (const void*)address, size);
	
	return 1;
}