/*
 *    Filename: lib/ordner/beispiel.h
 *     Version: 0.0.1
 * Description: Beispieldatei
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
#ifndef _HAVE_LIB_IO_ADC_H
#define _HAVE_LIB_IO_ADC_H
void adc_start (uint8_t channel) {
	// Referenzspannung: AVcc mit externem Kondensator
	ADMUX = (1<<REFS0)|(0<<REFS1);
	// Stelle Kanal ein:
	ADMUX |= channel;
	// Stelle Prescaler auf 64 und Starte ADC-auslesung
	ADCSRA = (1<<ADEN)|(1<<ADSC)|(0<<ADFR)|(0<<ADIF)|(0<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);
}
void adc_init(uint8_t channel, uint8_t prescaler) {
	// Referenzspannung: AVcc mit externem Kondensator
	ADMUX = (1<<REFS0)|(0<<REFS1);
	// Stelle Kanal ein:
	ADMUX |= channel;
	// Stelle Prescaler auf 64 und Starte ADC-auslesung
	ADCSRA = (1<<ADEN)|(1<<ADSC)|(0<<ADFR)|(0<<ADIF)|(0<<ADIE)|(prescaler<<ADPS0);
}
inline uint8_t adc_ready() {
	return !(ADCSRA & _BV(ADSC));
}
// insert functions and defines here

#endif
