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

//bit used for marking the command read or write
#define I2C_READ    1
#define I2C_WRITE   0

#include <inttypes.h>

void i2c_Init(void);
int8_t i2c_Start(uint8_t device_addr);
void i2c_Stop(void);

void i2c_Write(uint8_t addr, uint8_t data);
uint8_t i2c_Read(uint8_t addr);
uint8_t i2c_GetStatus(void);


void _i2c_write(uint8_t data);
uint8_t _i2c_readACK(void);
uint8_t _i2c_readNACK(void);