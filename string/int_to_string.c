/*
 *    Filename: string/int_to_string.c
 *     Version: 0.0.1
 * Description: Beispieldatei
 *     License: GPLv2 or later
 *     Depends: math/32bit_zehnersystem.c
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
#ifndef _HAVE_LIB_STRING_INT_TO_STRING_C
#define _HAVE_LIB_STRING_INT_TO_STRING_C
#include "../math/32bit_zehnersystem.c"
uint8_t uint32_to_string_tausenderpunkte(uint32_t x,uint8_t stellen,char str[]) {
	unsigned int i=0,j=0,leer=1,ziffer;
	for (i=0;i<=stellen;i++) {

		ziffer=uint32_ziffer(x,stellen-i);
		// keine führenden Nullen ausgeben, aber eine einzelne letzte 0
		if (((leer==1) && (ziffer!=0)) || (i==stellen)) {
			leer=0;
		}
		if (leer) {
			str[j++]=' ';
		} else {
			str[j++]='0'+ziffer;
		}
		if (((stellen-i)%3==0) && (i!=stellen)) {
				// Tausender-Punkte, jedoch nicht nach dem letzten
			if (leer) {
				str[j++]=' ';
			} else {
				str[j++]='.';
			}
		}
	}
	return j;
}
// insert functions and defines here

#endif
