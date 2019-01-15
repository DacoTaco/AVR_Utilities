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

#define SPI_READ 1
#define SPI_WRITE 0

void spi_init(void);
void spi_init_as(char _slave);
uint8_t spi_tranceiver_8(uint8_t data);
uint8_t spi_tranceiver(uint16_t data);
