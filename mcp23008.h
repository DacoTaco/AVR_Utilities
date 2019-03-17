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

#define BASE_23008_ADDR 0x40

//IODIR - REG that controls wether the pins are set as INPUT(1) or OUTPUT(0)
//default : 0xFF
#define IODIR 0x00

//IPOL - REG that controls the logic polarity of the pins. 0 meaning normal logic(1 = high, 0 = low) and 1 inverted logic (0=high , 1=low)
//default : 0x00
#define IPOL 0x01

//GPINTEN - REG that controls the interrupt on change of each pin. 1 = fire interrupt on pin, 0 = disabled
//default : 0x00
#define GPINTEN 0x02

//DEFVAL - REG that controls the default compare value. if the pin is the opposite value of the DEFVAL pin value, interrupt fires.
//default : 0x00
#define DEFVAL 0x03

//INTCON - REG that controls when the interrupt should fire. 1 = compare to DEFVAL value, 0 = compare to previous value
//default : 0x00
#define INTCON 0x04

//IOCON - REG that controls several settings of the MCP23008. 
//only the following bits are used : 0b00xx0xx0
//bit 7 & 6 : unused
//bit 5 : SEQOP , Sequential Operation.
//bit 4 : DISSLW, Slew rate control bit
//bit 3 :  unused
//bit 2 : ODR , set interrupt pin as open drain output. 1 = open drain output, 0 = active driver output
//bit 1 : INTPOL, set the interrupt polarity. 1 = active high, 0 = active low
//bit 0 : unused
//default : 0x00
#define IOCON 0x05

//GPPU - REG that controls the pull up resistors for the pins. if pin is set as input and set, pulled up with 100kOhms resistor
//default : 0x00
#define GPPU 0x06

//INTF - READ ONLY - REG that reflects the Interupt condition of any interrupt enabled pin. 1 = pin is interrupted. 
//default : 0x00
#define INTF 0x07

//INTCAP - REG that reflects the GPIO when interrupt fired. updated when interrupt fired
//default : 0x00
#define INTCAP 0x08

//GPIO- REG that controls the GPIO pins. heart of the chip. read this reg to read pin status or write to set an output pin. Writing will also write OLAT
//default : 0x00
#define GPIO 0x09

//OLAT - REG that controls the output latches. setting these will set an output pin low or high 
//default : 0x00
#define OLAT 0x0A


void mcp23008_init(uint8_t dev_addr);
int8_t mcp23008_ReadReg(uint8_t dev_addr, uint8_t reg,uint8_t* read_data);
void mcp23008_WriteReg(uint8_t dev_addr, uint8_t reg,uint8_t value);