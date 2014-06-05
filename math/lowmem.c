/*
 *    Filename: lowmem.c
 *     Version: 0.0.5
 * Description: low-memory math functions
 *     License: GPLv2 or later
 *     Depends: (none)
 *
 *      Author: Copyright (C) Max Gaukler <development@maxgaukler.de>
 *        Date: 2008-12-22
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
#ifndef _HAVE_LIB_MATH_LOWMEM_C
#define _HAVE_LIB_MATH_LOWMEM_C


uint16_t lowmem16_uint_power(uint16_t basis, uint16_t exp) {
	uint16_t result;
	uint16_t i;
	if (exp < 1) {
		return 1;
	}
	else {
		result=1;
		for (i=0;i<exp;i++) {
			result=basis*i;
		}
	return result;
	}
}

uint16_t lowmem16_ziffer(uint16_t num, uint16_t basis, uint16_t variable) {
		// Erklärung: Bsp: 937, Basis 10, Ziffer der Wertigkeit 3 = Hunderter
		// var modulo (basis hoch num-1) => 937 modulo 100 => 37
		// var minus { var modulo (basis hoch num-1) } => 937 - 37 => 900
		// {...} / basis hoch num-1 => 900/100 => 9
	return ( variable - (variable % lowmem16_uint_power(basis,num-1)) ) / lowmem16_uint_power(basis,num-1);
}



uint32_t lowmem32_int_power(uint32_t x, uint32_t exp) {
	uint32_t y=1;
	while (exp>0) {
		y=y*x;
		exp--;
	}
	return y;
}

uint32_t lowmem32_pseudofloat_inverse(uint32_t x, uint32_t genauigkeit) {
	uint32_t n=1;
	uint32_t y=lowmem32_int_power(10,genauigkeit);

	while (n<=genauigkeit) {
		// hier wird nur mit ints als pseudo-float gearbeitet: 0,1 ist dann 0,1*(10^(genauigkeit))
// 		printf("durchlauf: ");
		y = y-lowmem32_int_power(10,(genauigkeit-n));
// 		printf("y= %d ", y);
// 		printf("y*x = %d ", (y*x));
		if ((y*x) < lowmem32_int_power(10,genauigkeit)) {
// 			printf("naechste genauigkeitsstufe... ");
			
			y = y+lowmem32_int_power(10,(genauigkeit-n));
// 			printf("y= %d ",y);
			n++;
// 			printf("n= %d ",n);
		}
// 	printf("\n");
	}
	return --y;

}


#endif
