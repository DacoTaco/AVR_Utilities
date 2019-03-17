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

//includes
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h> 
#include <stdarg.h>
#include <stdio.h>
#include "serial.h"

// define some macros
#define _BUFFER_SIZE 64

//Connection info

//define CPU frequency here if its not defined in the Makefile
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

//default BAUD of 9600
#ifndef BAUD
#define BAUD 9600    
#endif

// define baudrate
#define BAUDRATE ((F_CPU)/(BAUD*8UL)-1)			// set baud rate value for UBRR

char _serial_init = 0;

//callbacks
void (*cb_recv)(char); 

//Override the Recv callback, or return it to default by giving NULL
void setSerialRecvCallback(void* cb)
{
	cb_recv = cb;
}

//Send a Char to the Other side
void usart_SendChar(char data) 
{
	if(_serial_init == 0)
		return;
	
    // Wait for empty transmit buffer
    while ( !(UCSRA & (_BV(UDRE))) );
    // Start transmission
    UDR = data;
}

//Send a string to the other side
void usart_SendString(char *s) {
	
	if(_serial_init == 0 || s == NULL)
	{
		return;
	}

    // loop through entire string
    while (*s) {
        usart_SendChar(*s);
        s++; 
    }
}

//Wait untill a single Char is received
//kinda useless as we have the interrupt xD
unsigned char usart_GetChar(void) 
{
	if(_serial_init == 0)
		return 0x00;
	
    // Wait for incoming data
    while ( !(UCSRA & (_BV(RXC))) );
    // Return the data
    return UDR;
}
void DisableSerialInterrupt(void)
{
	if(_serial_init == 0)
		return;
	
	UCSRB &= ~(1 << RXCIE);
}
void EnableSerialInterrupt(void)
{
	if(_serial_init == 0)
		return;
	
	UCSRB |= (1 << RXCIE);
}
//Inialise the Console			
void initConsole(void) {
    // Set baud rate
	UBRRH = (uint8_t)(BAUDRATE>>8);
    UBRRL = (uint8_t)BAUDRATE;

    // Enable receiver and transmitter
    UCSRB |= (1<<RXEN)|(1<<TXEN);
	UCSRA |= (1<<U2X);
	
    // Set frame format: 8data, 1stop bit
    UCSRC |= (1<<URSEL)|(3<<UCSZ0);	
	
	cb_recv = usart_SendChar;
	
	//enable interrupts in the chip, cli(); disables them again
	sei();
	
	_serial_init = 1;
		
	//enable the receive interrupt
	EnableSerialInterrupt();
	
	return;
}

//some top level functions are actually alias'

//Print a full string
void cprintf_string(char* str) __attribute__((alias("usart_SendString")));

//Wait and retrieve 1 byte
unsigned char Serial_ReadByte(void) __attribute__((alias("usart_GetChar")));

//Print a single character
void cprintf_char( unsigned char text ) __attribute__((alias("usart_SendChar")));


#ifdef _VA_SUPPORT
//Print a string
void cprintf( char *text,... )
{
	if(_serial_init == 0)
		return;
		
	char *output = text;
	char astr[_BUFFER_SIZE+1];
	memset(astr,0,_BUFFER_SIZE+1);
	
	va_list ap;
	va_start(ap,text);

	vsnprintf( astr, _BUFFER_SIZE,text,ap);
	va_end(ap);
	
	output = astr;
	usart_SendString(output);
}
#endif

//Interrupt handler of receiving characters
ISR(USART_RXC_vect)
{
	char ReceivedByte;
	//Put received char into the variable
	//we always need to do this, or the interrupt will keep triggering
	ReceivedByte = UDR;

	//Process the given character
	if(cb_recv != NULL)
	{
		cb_recv(ReceivedByte);
	}
}