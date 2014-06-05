/*
 *    Filename: lib/io/acomp.h
 *     Version: 0.0.1
 * Description: Analog-Komparator
 *     License: GPLv2 or later
 *     Depends: (none)
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
#ifndef _HAVE_LIB_IO_ACOMP_H
#define _HAVE_LIB_IO_ACOMP_H
void acomp_start (uint8_t pin) {
#if defined (__AVR_ATmega8__)
	ACSR=_BV(ACI); // Analog Comparator Enable, Interrupt disabled, Interrupt flag zurückgesetzt, Input Capture Disable
	cbi(ADCSRA,ADEN);
	sbi(SFIOR,ACME); // Analog Comparator Multiplexer Enable
	ADMUX &= 0xf0; // MUX0...3 leeren
	ADMUX|=pin;
#elif defined (__AVR_ATtiny26__)
	ACSR=_BV(ACI)|_BV(ACME); // Analog Comparator Enable, Interrupt disabled, Interrupt flag zurückgesetzt, Multiplexer an
	cbi(ADCSR,ADEN);
	ADMUX &= 0xf0; // MUX0...3 leeren
	ADMUX|=pin;

#else
#warning "device type not added in acomp.h"
#endif

}

inline uint8_t acomp_get() {
	return (ACSR & _BV(ACO));
}
// insert functions and defines here

#endif
