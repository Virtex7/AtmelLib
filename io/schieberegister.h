/*
 *    Filename: lib/io/schieberegister.h
 *     Version: 0.0.1
 * Description: Schieberegister-Ansteuerung
 *     License: GPLv2 or later
 *     Depends: (none)
 *
 *      Author: Copyright (C) Philipp Hörauf
 *        Date: 2011-11-01
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
#ifndef _HAVE_LIB_IO_SCHIEBEREGISTER_H
#define _HAVE_LIB_IO_SCHIEBEREGISTER_H

#include "./../global.h"

#ifndef SHIFT_PORT
#error SHIFT_PORT not defined!!!
#endif
#ifndef SHIFT_DDR
#error SHIFT_DDR not defined!!!
#endif

#ifndef SHIFT_SER
#error SHIFT_SER not defined!!!
#endif
#ifndef SHIFT_CLK
#error SHIFT_CLK not defined!!!
#endif
#ifndef SHIFT_LATCH
#error SHIFT_LATCH not defined!!!
#endif

void shift_init(void) { // Sorgt für "Zucht und Ordnung!!" bei den Ausgängen des µC
	cbi(SHIFT_PORT, SHIFT_SER);
	sbi(SHIFT_PORT, SHIFT_CLK);
	cbi(SHIFT_PORT, SHIFT_LATCH);

	sbi(SHIFT_DDR, SHIFT_SER);
	sbi(SHIFT_DDR, SHIFT_CLK);
	sbi(SHIFT_DDR, SHIFT_LATCH);
}

void shift_clock(void) { // clock Puls
	cbi(SHIFT_PORT, SHIFT_CLK);
	_delay_us(1);
	sbi(SHIFT_PORT, SHIFT_CLK);
// 	_delay_us(10);
}

void shift_latch(void) { // latche buffer auf Ausgänge
	sbi(SHIFT_PORT, SHIFT_LATCH);
	_delay_us(1);
	cbi(SHIFT_PORT, SHIFT_LATCH);
// 	_delay_us(10);
}

void shift_tx(uint8_t byte) {
	for(uint8_t i=0; i<8; i++) {
		if(byte & _BV(i)) { // sende 1
			sbi(SHIFT_PORT, SHIFT_SER);
		} else {
			cbi(SHIFT_PORT, SHIFT_SER); // sende 0
		}
		_delay_us(1);
		shift_clock();
	}
	_delay_us(10);
	shift_latch();
	delayus(10);
}

void shift_tx_16(uint16_t byte) {
	for(uint8_t i=0; i<16; i++) {
		if(byte & (1<<i)) { // sende 1
			sbi(SHIFT_PORT, SHIFT_SER);
		} else {
			cbi(SHIFT_PORT, SHIFT_SER); // sende 0
		}
		_delay_us(1);
		shift_clock();
	}
	_delay_us(10);
	shift_latch();
	delayus(10);
}


#endif
