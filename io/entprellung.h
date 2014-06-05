/*
 *    Filename: entprellung.h
 *     Version: 0.0.2
 * Description: Software-Entprellung
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


// SOFTWARE-Entprellung

/******* Beispielbenutzung: *********
struct entprellung grosser_roter_knopf;
entprellung_init(grosser_roter_knopf,0); // Wenn der Input auf 0 ist, ist der Schalter aus = entprellter Status 0


while (1) {
... Hauptschleife ...
entprellung_scroll(grosser_roter_knopf,IN_NOTAUSSCHALTER);

}
*/

struct entprellung {
	int wert0;	// aktueller
	int wert1;	// letzter
	int wert2;	// vorletzter Wert
	int start;	// Wert für AUS
	int status;	// am Statuswert kann man stets einen zuverlässigen, software-entprellten Wert ablesen
};


#define entprellung_init(name,nullwert) \
	do { \
		name.start = nullwert; \
		name.wert0 = nullwert; \
		name.wert1 = nullwert; \
		name.wert2 = nullwert; \
		name.status = 0;\
	} while (false)

#define entprellung_wert(name) (name.status)

// neuen Wert der History hinzufügen, alte Werte nachschieben, ältester fällt raus, wenn sich Wert geändert hat, aber nicht rumzappelt (zB alt->mittelalt->neu: 1->0->1 zappelt; 0->1->1 hat sich geändert)
// val != 0, um Wert in Logik-Wert nach dem Prinzip "0 ist false, alles andere ist true" umzuwandeln
// zB 3 => 1; 0 => 0; -23 => 1;
// das do { ... } while (FALSE) verhält sich neutral und wird wegoptimiert
// es ist nur da, damit sich das Makro wie eine Funktion im Code verhält und keine komischen merkwürdigen Probleme verursacht
#define entprellung_scroll(arg,val) \
	do { \
		arg.wert2 = arg.wert1; \
		arg.wert1 = arg.wert0; \
		arg.wert0 = (((val)!=0) != arg.start); \
		if ((arg.wert2 == arg.wert1) && (arg.wert1 == arg.wert0)) { \
			arg.status = arg.wert0; \
		} \
	} while (false)


// #define entprellung_scroll(arg,val) \
// 	do { \
// 		arg.wert2 = arg.wert1; \
// 		arg.wert1 = arg.wert0; \
// 		arg.wert0 = (((val)!=0) != arg.start); \
// 		if((arg.status != arg.wert1) && (arg.wert2 == arg.wert1) && (arg.wert1 == arg.wert0)) { \
// 			arg.status = arg.wert0; \
// 		} \
// 	} while (false)
