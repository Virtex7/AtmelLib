/*
 *    Filename: lib/io/serial/uart.h
 *     Version: 0.0.1
 * Description: Beispieldatei
 *     License: GPLv2 or later
 *     Depends: io.h
 *
 *      Author: Copyright (C) Max Gaukler <development@maxgaukler.de>
 *        Date: 2007-12-12
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef _HAVE_LIB_IO_SERIAL_UART_ROM_H
#define _HAVE_LIB_IO_SERIAL_UART_ROM_H
#error noch nicht fertig
#include "./uart.h"

#include <avr/pgmspace.h>

void uart_tx_str_P(unsigned char* string) {
// 			_delay_us(104);
// 			RS232(1);
// 			RS232(0); // Startbit-
// 			// ASCII G=11100010
// 			RS232(1); RS232(1); RS232(1); RS232(0); RS232(0); RS232(0); RS232(1); RS232(0);
// 			RS232(1);
// 			RS232(1); // Stopbit
	uint16_t i=0;
	while (string[i] != 0x00) {
		uart_tx(string[i]);	
		i++;
	}
}

#endif
