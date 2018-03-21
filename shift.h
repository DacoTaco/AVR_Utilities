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

int8_t Shift_Init(volatile uint8_t *Shift_DDR,volatile uint8_t *Shift_Port,volatile uint8_t *Shift_Pin,
				volatile uint8_t *Latch_Pin,volatile uint8_t *Data_Pin,volatile uint8_t *Clock_Pin);
int8_t ShiftData(uint8_t data);