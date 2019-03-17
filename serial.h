/*
serial - An AVR library to communicate with a computer over UART/RS232
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

//comment this out to reduce code size, but remove support for Variadic functions like in cprintf
#define _VA_SUPPORT

#ifdef SAVE_SPACE
#undef _VA_SUPPORT
#endif

#ifdef DEBUG
	#define cprintf_debug(x) cprintf(x)
	#define cprintf_debug_char(x) cprintf_char(x)
#else
	#define cprintf_debug(x) {}
	#define cprintf_debug_char(x) {}
#endif

void initConsole(void);
void setSerialRecvCallback(void* cb);
void DisableSerialInterrupt(void);
void EnableSerialInterrupt(void);

unsigned char Serial_ReadByte(void);
void cprintf_char( unsigned char text );
void cprintf_string(char* str);

#ifdef _VA_SUPPORT
void cprintf( char *text,... ); 
#else
#define cprintf cprintf_string
#endif 
