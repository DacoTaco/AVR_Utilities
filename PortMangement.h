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


//setpin and clearpin are often defined by some persons' code. so undefine and redefine them
#ifdef SetPin
#undef SetPin
#endif 

#ifdef ClearPin
#undef ClearPin
#endif

//these defines is basically what your code should be using.
#define SetPin(x,y) _setPin(&x,(1<<y))
#define ClearPin(x,y) _clearPin(&x,(1<<y))
#define CheckPin(x,y) _checkPin(&x,y)
#define SetPortMode(x,y,z) _setPortMode(&x,&y,z,0xFF)
#define SetPortModeWithMask(x,y,z,m) _setPortMode(&x,&y,z,m)

#define HIGH 1
#define LOW 0

#define INPUT 0
#define OUTPUT 1

void _setPin(volatile uint8_t *port, uint8_t mask);
void _clearPin(volatile uint8_t *port, uint8_t mask);
int8_t _checkPin(volatile uint8_t *port , uint8_t Pin);
void _setPortMode(volatile uint8_t *port_ddr, volatile uint8_t *port, uint8_t mode, uint8_t mask);