/*
 *    Filename: lib/io/serial/uart.h
 *     Version: 0.0.4
 * Description: UART-lib für Atmel-8bit-Mikrocontroller.
 *     License: GPLv2 or later
 *     Depends: io.h
 *
 *      Author: Copyright (C) Max Gaukler <development@maxgaukler.de> and 
 * 		 Philipp Hörauf (Unterstützung für neuere Atmels...)
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
#ifndef _HAVE_LIB_IO_SERIAL_UART_H
#define _HAVE_LIB_IO_SERIAL_UART_H

#define BAUD 57600
#include <util/setbaud.h>

// Abwärtskompatibilität...
#define uart_tx uartTx
#define uart_rx uartRx
#define uart_rx_ready uartRxReady
#define uart_tx_dec uartTxDec
#define uart_tx_bin uartTxBin
#define uart_tx_bin8 uartTxBin8
#define uart_tx_str uartTxStr
#define uart_tx_strln uartTxStrln
#define uart_tx_newline uartTxNewline


#ifdef tiny2313
#define URSEL UMSEL
#define PE UPE
#endif

#if defined mega48 || defined mega88 || defined mega168 || defined mega328
#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UCSRC UCSR0C
#define RXEN RXEN0
#define TXEN TXEN0
#define UCSZ0 UCSZ00
#define RXC RXC0
#define TXC TXC0
#define UDRE UDRE0
#define UDR UDR0
#define UBRRH UBRR0H
#define UBRRL UBRR0L

#endif

#include "../io.h"

// Formel zur Baudrate->UBRR-Umrechung: UBRR=((F_CPU)/(baud*16L)*2 + 1) /2

void uartInit(void) {
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	
	// RX/TX aktivieren
	UCSRB = (1 << RXEN) | (1 << TXEN);
	
	// 8 Bit Daten; 1 Stopbit; Asynchronous Operation, no Parity
	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0); 
	
	// 2 Stoppbits wenn N_STOPPBITS = 2 definiert ist.
	#if N_STOPPBITS == 2
	UCSRC |= (1 << USBS);
	#endif
}

uint8_t uartRxReady(void) {
	if(UCSRA & (1<<RXC)) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t uartRx(void) {
//	while(uart_rx_ready() == 0);
	return UDR;
}

uint8_t uartTxReady(void) {
	if(UCSRA & (1<<UDRE)) {
		return 1;
	} else {
		return 0;
	}
}

void uartTx(uint8_t zeichen) {
	while(uartTxReady() == 0);
	UDR = zeichen;
}

void uartTxNewline(void) {
	uartTx('\n');
	uartTx('\r');
}

void uartTxStr(char text[]) { // nur chars mit weniger als 65536 Zeichen!
uint16_t i = 0;
while(text[i]) {
	uartTx((uint8_t)text[i]);
	i++;
}
}

void uartTxStrln(char text[]) {
	uartTxStr(text);
	uartTxNewline();
}

void uartTxDec(uint16_t zahl) {
	uartTx(0x30+(zahl/10000));		// 10k   Stelle
	uartTx(0x30+(zahl%10000/1000));		//  1k   Stelle
	uartTx(0x30+(zahl%1000/100));		// 100er Stelle
	uartTx(0x30+(zahl%100/10));		// 10er  Stelle
	uartTx(0x30+(zahl%10));			// 1er   Stelle
}

void uartTxDec2(uint16_t zahl) {
	uartTx(0x30+(zahl%100/10));		// 10er  Stelle
	uartTx(0x30+(zahl%10));			// 1er   Stelle
}

void uartTxBin(uint32_t value) { // maximal 32 Bit auf einmal
for(uint8_t i=0; i<32; i++) {
	if(value & ((uint32_t)1<<(31-i))) {
		uartTx(0x31);
	} else {
		uartTx(0x30);
	}
}
}

void uartTxBin8(uint8_t value) { // effizientere Version für 8 Bit
for(uint8_t i=0; i<8; i++) {
	if(value & (1<<(7-i))) {
		uartTx(0x31);
	} else {
		uartTx(0x30);
	}
}
}
#endif
