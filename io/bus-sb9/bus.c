/*
 *    Filename: bus.c
 *     Version: 0.2.1 - last change: added support for mega644p - first UART-module
 * Description: 9bit serial Bus ("SB9") Library for ATMEL AVR
 *     License: Public Domain
 *
 *      Author: Copyright (C) Max Gaukler <development@maxgaukler.de>, Philipp Hörauf
 *        Date: 2010
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#include <avr/eeprom.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
EEMEM uint8_t bus_rom_address=255;
uint8_t bus_address=255;
uint8_t bus_wait_before=255;



// DEFINITIONEN
#define BUS_OK 0x00
#define BUS_ERR_INTERNAL 0xFC
#define BUS_ERR_PARAM 0xFD
#define BUS_ERR_LEN 0xFE
#define BUS_ERR_CMD 0xFF

#define BUS_CMD_LCD_DATA 0xE2
#define BUS_CMD_LCD_COMMAND 0xE1
#define BUS_CMD_LCD_PRINT 0xE3
#define BUS_CMD_LCD_PUTSTR 0xE4

#define BUS_CMD_OUTPUT_ON 0xF0
#define BUS_CMD_OUTPUT_OFF 0xF1
#define BUS_CMD_OUTPUT_SET 0xF2
#define BUS_CMD_OUTPUT_TOGGLE 0xF3
#define BUS_CMD_OUTPUT_GET 0xF4
#define BUS_CMD_INPUT_GET 0xF5
// 0xF6 reserviert
#define BUS_CMD_SHUTDOWN 0xF7

#define BUS_CMD_TEST 0xFA
#define BUS_CMD_FAULT_MEMORY 0xFB
#define BUS_CMD_IDENT 0xFC
#define BUS_CMD_LOOPBACK 0xFD
#define BUS_CMD_RESET 0xFE
#define BUS_CMD_SET_ADDR 0xFF

// TODO: add full example for classic mode

/**
 Bus modes:                         *
 BUS_MODE_CLASSIC: saves some RAM, but cannot receive new packets while trying to send (this is by design and cannot be further improved).
 The main program looks like this:
 \code
 int main(void) {
 uint8_t sender, command, len, repeated;
 uint8_t data[20], answer_data[20];
 uint8_t answer_len;
 uint8_t status;
 
 // own init stuff
 bus_init();
 // other own init stuff
 while(1) {

	 if (bus_rx_packet(&sender, &command, &len, 20, data, &repeated)!=0) {
		 status=BUS_ERR_INTERNAL;
		 answer_len=0;
		 // process command and data, do something
		 if (command==BUS_CMD_OUTPUT_SET) {
			 if (data[0]==0) {
				 if (len==2) {
					 status=BUS_OK;
					 PORTC=data[1];
					 answer_len=1;
					 answer_data[0]=PORTC;
				 } else {
					 status=BUS_ERR_LEN;
				 }
			 } else {
				 status=BUS_ERR_PARAM;
			 }
		 } else {
			 status=BUS_ERR_CMD;
		 }
		 bus_rx_send_answer(status, answer_data, answer_len);
	 }
	 //... do something else (maximum 3ms) or simply _delay_ms(3);
	 //... you can send a packet by calling bus_tx_packet(), it returns after sending has finished
 }
}
\endcode

BUS_MODE_EVENT: possibility for interrupt-driven receive events
This mode uses the UART_RXC interrupt. If a new packet is received, your custom function bus_rx_event() is called:
void bus_rx_event(uint8_t sender, uint8_t command, uint8_t len, uint8_t data[], uint8_t repeated)
rx_sender: originating address
rx_command: command
rx_data: data
rx_len: data length
answer_status: set it to one of the status constants (BUS_OK, BUS_ERR_...) if you processed a command. If the command was unknown (or not implemented), set it to BUS_ERR_CMD. If the status is set to BUS_ERR_CMD, some commands (BUS_CMD_SET_ADDR and some others) are processed by default implementations.

for more see the german file "Protokolldefinitionen.txt"


The program looks like this:
\code
static inline void bus_rx_event(uint8_t rx_sender, uint8_t rx_command, uint8_t rx_data[], uint8_t rx_len, uint8_t *answer_status, uint8_t *answer_data, uint8_t *answer_len, uint8_t answer_max_len) {
if (command==BUS_CMD_OUTPUT_TOGGLE) {
	if (data[0]==0) {
		if (len==2) {
			status=BUS_OK;
			PORTC^=data[1];
			*answer_len=1;
			answer_data[0]=PORTC;
		} else {
			status=BUS_ERR_LEN;
		} else {
			status=BUS_ERR_PARAM;
		}
	} else {
		status=BUS_ERR_CMD;
	}
}

main() {
	while (1) {
		bus_poll();
		//do something else (maximum 3ms)
		// you can send a packet by calling bus_tx_packet();
	}
}

\endcode
*/

#if defined BUS_MODE_EVENT
#if !defined BUS_RX_BUF_LENGTH
#error you need to define BUS_RX_BUF_LENGTH to the maximum length of a received packet, e.g. 20
#endif
#if !defined BUS_ANSWER_BUF_LENGTH
#error you need to define BUS_ANSWER_BUF_LENGTH to the maximum length of the answer to a received packet, e.g. 20
#endif
#if !defined BUS_IDENT_STRING
#error you need to define BUS_IDENT_STRING to a string enclosed in double quotes (string length <= BUS_ANSWER_BUF_LENGTH) that identifies your device type, e.g. "Coffee Machine v2"
#endif
// you need to implement the following function: (see docs)
static inline void bus_rx_event(uint8_t rx_sender, uint8_t rx_command, uint8_t rx_data[], uint8_t rx_len, uint8_t *answer_status, uint8_t *answer_data, uint8_t *answer_len, uint8_t answer_max_len);
#elif defined BUS_MODE_CLASSIC
#else
#error you need to define BUS_MODE_CLASSIC or BUS_MODE_EVENT
#endif

// Funktionsprototypen, erzeugt mit:
// cat bus.c | sed -n "s/^\([a-z].*(.*)\)[ \\t]*[\{][^;]*\$/\1;/p"
// (einige Zeilen der Ausgabe bitte ignorieren wegen Dokumentation)
uint8_t bus_checksum(uint8_t x, uint8_t prev);
static inline uint8_t bus_uart_rx_ready(void);
uint8_t bus_uart_rx_ready_timeout(uint8_t time);
void bus_uart_rx_flush(void);
void bus_uart_tx_byte_9(uint16_t data);
uint8_t bus_tx(uint16_t data);
uint16_t bus_uart_rx9(void);
uint16_t bus_rx(void);
void bus_set_address(uint8_t addr);
void bus_read_eeprom_address(void);
uint8_t bus_get_address(void);
void bus_init(void);
void bus_set_mpcm(uint8_t on);
// inline uint8_t bus_tx_ready(void);
uint8_t bus_tx_packet(uint8_t destination, uint8_t command, uint8_t len, uint8_t data[], uint8_t rx_data[], uint8_t rx_max_len, uint8_t *rx_len, uint8_t *rx_status);
static inline uint8_t bus_tx_packet_(uint8_t destination, uint8_t command, uint8_t len, uint8_t data[], uint8_t rx_data[], uint8_t rx_max_len, uint8_t *rx_len, uint8_t *rx_status);
uint8_t bus_rx_packet(uint8_t *sender, uint8_t *command, uint8_t *len, uint8_t max_len, uint8_t data[], uint8_t *repeated);
static inline uint8_t bus_rx_send_answer(uint8_t status, uint8_t answer_data[], uint8_t len);
void bus_poll(void);

uint8_t bus_checksum(uint8_t x, uint8_t prev) {
	// checksum: x is the next byte, prev is the last checksum value
	// example: uint8_t chk; for (i=0;i<len;i++) { chk=bus_checksum(data[i],chk); }
	x ^= prev; // XOR old checksum and new byte
	x=(x>>1)|((x&1)<<7); // rotate-right 1 bit
	return x;
}


// processor-specific-definitions:

#if defined(UART_MPCM_REGISTER) && defined(UART_MPCM_BIT) && defined(UART_RX_PIN) && defined(UART_RX_BIT)
// registers already defined for custom setting - if you want a custom setting, define ALL FOUR of them!
#elif defined(__AVR_ATmega8__)
#define UART_MPCM_REGISTER UCSRA
#define UART_MPCM_BIT MPCM
#define UART_RX_PIN PIND
#define UART_RX_BIT PD0
#elif defined(__AVR_ATtiny2313__)
#define UART_MPCM_REGISTER UCSRA
#define UART_MPCM_BIT MPCM
#define UART_RX_PIN PIND
#define UART_RX_BIT PD0
#define PE UPE
#elif defined(__AVR_ATmega644P__) // zwei UART-Module!! -> hier UART0 verwenden
#define UART_MPCM_REGISTER UCSR0A
#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UCSRC UCSR0C
#define UART_MPCM_BIT MPCM0
#define UART_RX_PIN PIND
#define UART_RX_BIT PD0
#define UDR UDR0
#define FE FE0
#define DOR DOR0
#define PE UPE0
#define RXB8 RXB80
#define TXB8 TXB80
#define RXC RXC0
#define UDRE UDRE0
#else
#error unsupported processor type, please define UART_MPCM_REGISTER (e.g. UCSRA), UART_MPCM_BIT (e.g. MPCM), UART_RX_PIN (e.g. PIND), UART_RX_BIT (e.g. PD0)
#endif


// LOW-LEVEL RX/TX FUNCTIONS

static inline uint8_t bus_uart_rx_ready(void) {
	return UCSRA&(1<<RXC);
}

uint8_t bus_uart_rx_ready_timeout(uint8_t time) {
	// time in ms, max. 25ms
	time*=10;
	uint8_t i;
	for (i=0;i<time;i++) {
		_delay_us(100);
		if (bus_uart_rx_ready()) {
			return 1;
		}
	}
	return 0;
}

void bus_uart_rx_flush(void) {
	while (bus_uart_rx_ready()) {
		volatile uint8_t dummy=UDR;
		dummy++; // do something so that dummy variable is not unused
	}
}

void bus_uart_tx_byte_9(uint16_t data) {
	// ANNAHME: uart rx buf. leer
	// hier bewusst kein rx_buf_flush, um zu erkennen wenn unerwartet Pakete empfangen wurden
	while (!(UCSRA&(1<<UDRE))) {}
	if (data&(1<<8)) {
		UCSRB |= 1<<TXB8;
	} else {
		UCSRB &= ~(1<<TXB8);
	}
	UDR=data&0xFF;
}

uint8_t bus_tx(uint16_t data) {
	// return 1 = OK
	// return 0 = Error
	bus_uart_tx_byte_9(data);
	
	// Kollisionserkennung: RX==TX???
	if (!bus_uart_rx_ready_timeout(1)) {
		// Paket ist auf der Strecke verloren gegangen
		return 0;
	}
	if (bus_uart_rx9()==data) {
		return 1;
	} else {
		return 0;
	}
	
}

// MIDDLE-LEVEL RX/TX FUNCTIONS

#define UART_RX_ERR 0xFFFF
uint16_t bus_uart_rx9(void) {
	uint8_t ucsra_cached=UCSRA;
	uint8_t ucsrb_cached=UCSRB;
	uint8_t udr_cached=UDR;
	if (ucsra_cached & ((1<<FE) | (1<<DOR) | (1<<PE))) {
		// Fehler
		bus_uart_rx_flush();
		return UART_RX_ERR;
	} else {
		// Daten + 9. bit
		return ((ucsrb_cached & (1<<RXB8))<<(8-RXB8)) | udr_cached;
	}
}

#define BUS_RX_TIMEOUT (1<<15)
#define BUS_RX_UART_ERR (1<<14)
#define BUS_RX_ERR (BUS_RX_TIMEOUT|BUS_RX_UART_ERR)

uint16_t bus_rx(void) {
	if (!bus_uart_rx_ready_timeout(5)) {
		return BUS_RX_TIMEOUT;
	}
	uint16_t rx;
	rx=bus_uart_rx9();
	if (rx==UART_RX_ERR) {
		return BUS_RX_UART_ERR;
	}
	return rx;
}



// ADDRESS SET/GET FUNCTIONS

void bus_set_address(uint8_t addr) {
	bus_address=addr;
	bus_wait_before=bus_address;
	eeprom_write_byte(&bus_rom_address,addr);
	// 	eeprom_busy_wait();
	// wait needs too long, blocks bus_poll
}

void bus_read_eeprom_address(void) {
	bus_address=eeprom_read_byte(&bus_rom_address);
}

uint8_t bus_get_address(void) {
	return bus_address;
}
#ifdef BUS_MODE_EVENT
uint8_t bus_rx_interrupt_status(void);
void bus_enable_rx_interrupt(uint8_t enable);
#endif
// BUS INIT

void bus_init(void) {
	// async. UART: 9bit, even parity, 2 stopbits, 19,2 kBaud
	#if defined(__AVR_ATmega8__)
	UCSRA=0;
	UCSRB=(1<<RXEN)|(1<<TXEN)|(1<<UCSZ2);
	UCSRC=(1<<URSEL)|(1<<UPM1)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
	#if (F_CPU == 16000000)
	UBRRL=51;
	UBRRH=0;
	#else /* F_CPU */
	#error unsupported processor speed or F_CPU not defined, use 16000000
	#endif /* F_CPU */
	#elif defined(__AVR_ATmega644P__)
	UCSR0A=0;
	UCSR0B=(1<<RXEN0)|(1<<TXEN0)|(1<<UCSZ02);
	UCSR0C=(1<<UPM01)|(1<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);
	#if (F_CPU == 16000000)
	UBRR0L=51;
	UBRR0H=0;
	#elif (F_CPU == 18432000) // Baudratenquarz!
	UBRR0L=59;
	UBRR0H=0;
	#elif (F_CPU == 20000000)
	UBRR0L=64;
	UBRR0H=0;
	#else /* F_CPU */
	#error unsupported processor speed or F_CPU not defined, use 16000000
	#endif /* F_CPU */
	#elif defined(__AVR_ATtiny2313__)
	UCSRA=0;
	UCSRB=(1<<RXEN)|(1<<TXEN)|(1<<UCSZ2);
	UCSRC=(1<<UPM1)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
	#if (F_CPU == 16000000)
	UBRRL=51;
	UBRRH=0;
	#elif (F_CPU == 4000000)
	#warning 4MHz is relatively slow for the use of SB9! consider using a higher SystemClock-frequency
	UBRRL = 12;
	UBRRH = 0;
	#else /* F_CPU */
	#error unsupported processor speed or F_CPU not defined, use 16000000
	#endif /* F_CPU */
	#else /* processor type */
	#error unsupported processor type, please add to bus_init()
	#endif
	bus_read_eeprom_address();
	bus_wait_before=bus_address;
	#ifdef BUS_MODE_EVENT
	bus_enable_rx_interrupt(1);
	#endif
}

inline void bus_set_mpcm(uint8_t on) {
	if (on) {
		UART_MPCM_REGISTER |= (1<<UART_MPCM_BIT);
	} else {
		UART_MPCM_REGISTER &= ~(1<<UART_MPCM_BIT);
	}
}

#ifdef BUS_MODE_EVENT
volatile uint8_t bus_tx_active=0;
inline uint8_t bus_tx_ready(void) {
	// Diese Anweisung ist atomar
	return bus_tx_active;
}
#endif
// HIGH-LEVEL RX/TX FUNCTIONS
uint8_t bus_tx_packet(uint8_t destination, uint8_t command, uint8_t len, uint8_t data[], uint8_t rx_data[], uint8_t rx_max_len, uint8_t *rx_len, uint8_t *rx_status) {
	#ifdef BUS_MODE_EVENT
	uint8_t rx_isr_enabled;
	// Wrapper um bus_tx_packet_ gegen mehrmaliges gleichzeitiges Ausführen
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (bus_tx_active) {
			// Fehler! Es wird bereits ein anderes Paket gesendet
			// Soviel Buffer gibt es nicht
			return 0; // return aus ATOMIC_BLOCK geht, sei wird trotzdem automatisch ausgeführt
		}
		// Während dieses Paket noch sendet, darf kein weiteres gesendet werden.
		bus_tx_active=1;
		rx_isr_enabled=bus_rx_interrupt_status();
		bus_enable_rx_interrupt(0);
	}
	#endif
	uint8_t ret=bus_tx_packet_(destination, command, len, data, rx_data, rx_max_len, rx_len, rx_status);
	#ifdef BUS_MODE_EVENT
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Restore RX-ISR-Status
		bus_enable_rx_interrupt(rx_isr_enabled);
		bus_tx_active=0;
	}
	#endif
	return ret;
}



static inline uint8_t bus_tx_packet_(uint8_t destination, uint8_t command, uint8_t len, uint8_t data[], uint8_t rx_data[], uint8_t rx_max_len, uint8_t *rx_len, uint8_t *rx_status) {
	// RETURN 1: OK, erster Versuch erfolgreich
	// RETURN 2/3: OK, zweiter/dritter Versuch erfolgreich
	// RETURN 0: Fehlgeschlagen
	// bus_wait_before wird bei bus_init() und bus_set_address() auf bus_address gesetzt
	uint8_t wait_for_bus_empty=15;
	uint8_t retry;
	bus_set_mpcm(0);
	
	// Sequenznummer für 3.1
	// für jedes Paket um 1 erhöhen
	// bei Wiederholungen nicht erhöhen
	static uint8_t seq=0;
	seq++;
	_delay_ms(1);
	for (retry=0;retry<3;retry++) {
		// 1m.1. "zufällige" Zeit berechnen
		bus_wait_before=(bus_wait_before+157)&0xFF;
		
		uint16_t i;
		int16_t rx=0;
		// 1m.2. warten bis Bus 15ms+x bzw. 7ms+x ms lang auf idle
		for (i=0;i<(wait_for_bus_empty*100)+bus_wait_before;i++) {
			if ((!(UART_RX_PIN & (1<<UART_RX_BIT))) || bus_uart_rx_ready()) {
				// Bus auf LOW, also geht die Wartezeit von vorne los
				#ifdef BUS_MODE_EVENT
				// Datenpaket verarbeiten
				// es ist möglich, dass hier ein Paket empfangen wird. Dann wird die Verarbeitungsfunktion aufgerufen.
				bus_poll();
				bus_set_mpcm(0); // MPCM wieder zurücksetzen
				#else
				// einfacher Modus: Empfang während des Sendens nicht möglich, verwerfen und auf Ende der Übertragung warten
				bus_uart_rx_flush();
				#endif
				i=0;
				bus_wait_before=(bus_wait_before+157)&0xFF;
			}
			_delay_us(10);
		}
		uint8_t chk=0xff;
		// 2.1. Master: Slave-Adresse (9. bit gesetzt)
		if (!bus_tx((1<<8)|destination)) {
			continue;
		}
		chk=bus_checksum(destination,chk);
		// 2.2. Master: Master-Adresse
		if (!bus_tx(bus_address)) {
			continue;
		}
		chk=bus_checksum(bus_address,chk);
		// 2.3. Master: Prüfsumme
		if (!bus_tx(chk)) {
			continue;
		}
		// 2.4. Slave: Prüfsumme (Bits negiert)
		if (bus_rx()!=((~chk)&0xFF)) {
			continue;
		}
		// 3.1. Master: Sequenznummer
		// Seq.Nummernberechnung erfolgt am Anfang der Funktion
		chk=bus_checksum(seq,chk);
		if (!bus_tx(seq)) {
			continue;
		}
		// 3.2. Master: Befehl
		chk=bus_checksum(command,chk);
		if (!bus_tx(command)) {
			continue;
		}
		// 3.3 Master: Länge
		chk=bus_checksum(len,chk);
		if (!bus_tx(len)) {
			continue;
		}
		// 3.4 Master: Daten
		for (i=0;i<len;i++) {
			chk=bus_checksum(data[i],chk);
			if (!bus_tx(data[i])) {
				continue;
			}
		}
		// 3.5.
		wait_for_bus_empty=7;
		// 4. Bestätigung
		// 4.1. Master: Prüfsumme
		if (!bus_tx(chk)) {
			continue;
		}
		// 4.2 Slave: Prüfsumme (Bits negiert)
		if (bus_rx()!=((~chk) & 0xFF)) {
			continue;
		}
		// 4.3 Slave führt Befehl aus
		// 5. Übertragung der Antwort
		chk=0xFF;
		// 5.1. Slave: Status
		rx=bus_rx();
		if (rx&(BUS_RX_ERR|(1<<8))) {
			continue;
		}
		*rx_status=rx&0xFF;
		chk=bus_checksum(rx&0xFF,chk);
		// 5.2. Slave: Länge der Antwort
		rx=bus_rx();
		if (rx&(BUS_RX_ERR|(1<<8))) {
			continue;
		}
		*rx_len=rx&0xFF;
		if (*rx_len>rx_max_len) {
			continue;
		}
		chk=bus_checksum(rx&0xFF,chk);
		// 5.3. Slave: Antwortdaten
		uint8_t rxdata_err=0;
		for (i=0;i<*rx_len;i++) {
			rx=bus_rx();
			if (rx&(BUS_RX_ERR|(1<<8))) {
				rxdata_err=1;
				break;
			}
			rx_data[i]=rx&0xFF;
			chk=bus_checksum(rx&0xFF,chk);
		}
		if (rxdata_err) {
			continue;
		}
		// 5.4.: Slave: Prüfsumme von Status, Länge, Daten
		if (bus_rx()!=chk) {
			continue;
		}
		// Ende der Übertragung
		bus_set_mpcm(1);
		return retry+1;
	}
	bus_set_mpcm(1);
	return 0;
}

inline void bus_store_error(uint8_t err, uint16_t info) {
	// empty, debug off
}
// #warning debug
// void lcd_hex(uint8_t x);
// void bus_store_error(uint8_t err, uint16_t info) {
// 	lcd_clear();
// 	lcd_hex(err);
// 	lcd_putchar(':');
// 	lcd_hex((info>>8)&0xFF);
// 	lcd_putchar(' ');
// 	lcd_hex(info&0xFF);
// 	delayms(1000);
// }

uint8_t bus_rx_packet(uint8_t *sender, uint8_t *command, uint8_t *len, uint8_t max_len, uint8_t data[], uint8_t *repeated) {
	uint8_t chk=0xff;
	bus_set_mpcm(1);
	// 1s.1. Ruhezustand
	if (!bus_uart_rx_ready()) {
		bus_set_mpcm(1);
		return 0;
	}
	bus_set_mpcm(0); // TODO
	
	// TODO überarbeitetes Sequenzzeug mit toggle() stresstesten: dauernd toggle und Antwort auf toggeln überprüfen, dabei zwischendurch einzelne Störimpulse einspeisen
	
	static uint8_t clear_cache_counter=0;
	static uint16_t seq=0xFFFF; // start with invalid seq.
	static uint8_t last_sender=0;
	if (clear_cache_counter>5) {
		// invalidate seq. number cache after 5 packets
		seq=0xFFFF;
	} else {
		clear_cache_counter++;
	}
	
	
	uint16_t rx;
	rx=bus_rx();
	if (rx&BUS_RX_ERR) {
		bus_set_mpcm(1);
		bus_store_error(0x20,rx);
		return 0;
	}
	
	// 2.1. Slave-Adresse
	if (rx!=((1<<8)|bus_address)) {
		// NO error, the packet is just not for us
		bus_set_mpcm(1);
		return 0;
	}
	chk=bus_checksum(bus_address,chk);
	// 2.2. Master-Adresse
	rx=bus_rx();
	if (rx&((1<<8)|BUS_RX_ERR)) {
		bus_store_error(0x22,rx);
		bus_set_mpcm(1);
		return 0;
	}
	*sender=rx&0xff;
	chk=bus_checksum(rx&0xff,chk);
	// 2.3. Master:	Prüfsumme
	if (bus_rx()!=chk) {
		bus_store_error(0x23,(rx<<8)|chk);
		bus_set_mpcm(1);
		return 0;
	}
	// 2.4. Slave:	Prüfsumme (Bits negiert)
	if (!bus_tx((~chk)&0xff)) {
		bus_store_error(0x24,rx);
		bus_set_mpcm(1);
		return 0;
	}
	// 3.1. Master: Sequenznummer
	rx=bus_rx();
	if (rx&((1<<8)|BUS_RX_ERR)) {
		bus_store_error(0x25,rx);
		bus_set_mpcm(1);
		return 0;
	}
	chk=bus_checksum(rx&0xff,chk);
	uint16_t old_seq=seq;
	// Repetition means that a query is processed twice because it was resent after the answer got lost.
	// If the function exits early because of an error, it must not save the new sequence number because the packet was not processed.
	// The new sequence number is only stored into the static variable seq after the function has completed receiving, which means the packet will be processed.
	uint16_t new_seq=rx&0xff;
	uint8_t new_last_sender=*sender;
	
	*repeated=0;
	if ((new_seq==old_seq) && (last_sender==*sender)) {
		*repeated=1;
	}
	
	// 3.2. Master: Befehl
	rx=bus_rx();
	if (rx&((1<<8)|BUS_RX_ERR)) {
		bus_store_error(0x32,rx);
		bus_set_mpcm(1);
		return 0;
	}
	*command=rx&0xff;
	chk=bus_checksum(rx&0xff,chk);
	// 3.3 Master: Länge
	rx=bus_rx();
	if (rx&((1<<8)|BUS_RX_ERR)) {
		bus_store_error(0x33,rx);
		bus_set_mpcm(1);
		return 0;
	}
	chk=bus_checksum(rx&0xff,chk);
	*len=rx&0xFF;
	if (*len>max_len) {
		bus_store_error(0x33,rx);
		// TODO auto-answer BUS_ERROR_LEN
		bus_set_mpcm(1);
		return 0;
	}
	// 3.4 Daten
	uint16_t i;
	for (i=0;i<*len;i++) {
		
		rx=bus_rx();
		if (rx&((1<<8)|BUS_RX_ERR)) {
			bus_store_error(0x34,rx);
			bus_set_mpcm(1);
			return 0;
		}
		chk=bus_checksum(rx&0xff,chk);
		data[i]=rx&0xff;
	}
	// 4.1. Master:	Prüfsumme
	if (bus_rx()!=chk) {
		bus_store_error(0x41,(rx<<8)|chk);
		bus_set_mpcm(1);
		return 0;
	}
	// 4.2 Slave: Prüfsumme (Bits negiert)
	if (!bus_tx((~chk)&0xff)) {
		bus_store_error(0x42,0);
		bus_set_mpcm(1);
		return 0;
	}
	
	// store the new sequence-number so re-processing the same packet will be detected
	seq=new_seq;
	last_sender=new_last_sender;
	
	// do not set mpcm to 1 here, bus_rx_send_answer is not yet called
	return 1;
	// answer is computed, then bus_rx_send_answer is called by the program
}

static inline uint8_t bus_rx_send_answer(uint8_t status, uint8_t answer_data[], uint8_t len) {
	uint8_t chk=0xff;
	// 5.1. Slave: Status
	if (!bus_tx(status)) {
		bus_set_mpcm(1);
		return 0;
	}
	chk=bus_checksum(status,chk);
	// 5.2. Slave: Länge der Antwort (bzw. 0 für keine Antwortdaten)
	if (!bus_tx(len)) {
		bus_set_mpcm(1);
		return 0;
	}
	chk=bus_checksum(len,chk);
	// 5.3. Slave: Antwortdaten
	uint16_t i;
	for (i=0;i<len;i++) {
		if (!bus_tx(answer_data[i])) {
			bus_set_mpcm(1);
			return 0;
		}
		chk=bus_checksum(answer_data[i],chk);
	}
	// 5.4. Slave: Prüfsumme von Status, Länge und Daten (auch bei Länge=0)
	if (!bus_tx(chk)) {
		bus_set_mpcm(1);
		return 0;
	}
	bus_set_mpcm(1);
	return 1;
	
	
}

#if defined BUS_MODE_EVENT
#if defined(UART_RX_ISR_REGISTER) && defined(UART_RX_ISR_FLAGS)
// UART-Setting already defined (e.g. to overwrite default settings)
#elif defined(__AVR_ATmega8__)
#define UART_RX_COMPLETE_VECTOR USART_RXC_vect
#define UART_RX_ISR_REGISTER UCSRB
#define UART_RX_ISR_FLAGS (1<<RXCIE)

#elif defined(__AVR_ATmega644P__)
#define UART_RX_COMPLETE_VECTOR USART0_RX_vect
#define UART_RX_ISR_REGISTER UCSR0B
#define UART_RX_ISR_FLAGS (1<<RXCIE0)

#elif defined(__AVR_ATtiny2313__)
#define UART_RX_COMPLETE_VECTOR USART_RX_vect
#define UART_RX_ISR_REGISTER UCSRB
#define UART_RX_ISR_FLAGS (1<<RXCIE)

#else
#error unsupported processor type, please add definitions for UART_RX_ISR_REGISTER (e.g. UCSRB), UART_RX_ISR_FLAGS (e.g. 1<<RXCIE), UART_RX_COMPLETE_VECTOR (e.g. USART_RXC_vect)
#endif
const PROGMEM char bus_ident_pstr[]=BUS_IDENT_STRING;
volatile uint8_t bus_poll_active=0;
void bus_poll(void) {
	uint8_t rx_isr_enabled;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (bus_poll_active) {
			// Es läuft bereits eine Instanz, Abbruch!
			return; // return aus ATOMIC_BLOCK geht, sei() wird trotzdem automatisch ausgeführt
		}
		bus_poll_active=1;
		rx_isr_enabled=bus_rx_interrupt_status();
		bus_enable_rx_interrupt(0);
	}
	
	// RX-Zwischenspeicher
	uint8_t bus_rx_sender;
	uint8_t bus_rx_command;
	uint8_t bus_rx_len;
	uint8_t bus_rx_data[BUS_RX_BUF_LENGTH];
	uint8_t bus_rx_repeated;
	
	// RX-Antwort-Zwischenspeicher: static, damit bei repeated die alte Antwort gleich wieder gesendet werden kann
	static uint8_t bus_answer_status=BUS_ERR_INTERNAL;
	static uint8_t bus_answer_data[BUS_ANSWER_BUF_LENGTH];
	static uint8_t bus_answer_len=0;
	
	if (bus_rx_packet(&bus_rx_sender, &bus_rx_command, &bus_rx_len, BUS_RX_BUF_LENGTH, bus_rx_data, &bus_rx_repeated)) {
		// 		uint8_t reset_pending=0;
		if (bus_rx_repeated) {
			// leave answer data, status and length unchanged
			// TODO test this
		} else {
			bus_answer_status=BUS_ERR_CMD;
			bus_answer_len=0;
			
			// call bus_rx_event to handle the command - if it doesnt know the command, it sets the status to BUS_ERR_CMD
			bus_rx_event(bus_rx_sender, bus_rx_command, bus_rx_data, bus_rx_len, &bus_answer_status, bus_answer_data, &bus_answer_len, BUS_ANSWER_BUF_LENGTH);
			
			if (bus_answer_status!=BUS_ERR_CMD) {
				// bus_rx_event handled the command
				// the response is sent
				
				// some default commands are processed in the next else-if clauses if they were not answered by bus_rx_event()
			} else if (bus_rx_command==BUS_CMD_SET_ADDR) { // Set new address
				if (bus_rx_len==6) {
					// three times the new address
					// three times the bit-inverted new address
					if ((bus_rx_data[0]==bus_rx_data[1]) && (bus_rx_data[0]==bus_rx_data[2]) && (((~bus_rx_data[0])&0xFF)==bus_rx_data[3]) && (bus_rx_data[3]==bus_rx_data[4]) && (bus_rx_data[4]==bus_rx_data[5])) {
						if (eeprom_is_ready()) {
							bus_answer_status=BUS_OK;
							bus_set_address(bus_rx_data[0]);
						} else {
							// another EEPROM operation is pending, bus_set_address would take too long
							bus_answer_status=BUS_ERR_INTERNAL;
						}
						
					} else {
						bus_answer_status=BUS_ERR_PARAM;
					}
				} else {
					bus_answer_status=BUS_ERR_LEN;
				}
			} else if (bus_rx_command==BUS_CMD_LOOPBACK) {
				if (bus_rx_len<=BUS_ANSWER_BUF_LENGTH) {
					bus_answer_status=BUS_OK;
					bus_answer_len=bus_rx_len;
					for (uint8_t i=0; i<bus_rx_len; i++) {
						bus_answer_data[i]=bus_rx_data[i];
					}
					// 					memcpy(bus_answer_data,bus_rx_data,bus_rx_len);
				} else {
					// packet is too long for answer buffer
					// packets too long for the rx buffer will be dismissed by bus_rx_packet() (TODO fix this), they will simply timeout
					bus_answer_status=BUS_ERR_LEN;
				}
				// TODO Reset not fully implemented
				// 		} else if (command==BUS_CMD_RESET) {
				// 			if (len==0) {
				// 				bus_answer_status=BUS_OK;
				// 				reset_pending=1;
				// 			} else {
				// 				bus_answer_status=BUS_ERR_LEN;
				// 			}
			} else if (bus_rx_command==BUS_CMD_IDENT) {
				if (bus_rx_len==0) {
					bus_answer_status=BUS_OK;
					// copy ident-string into answer data, but only until the buffer is full
					// the answer does not include the nullbyte!
					for (uint8_t i=0;i<BUS_ANSWER_BUF_LENGTH;i++) {
						bus_answer_data[i]=bus_ident_pstr[i];
						if (bus_answer_data[i]==0) {
							break;
						} else {
							bus_answer_len++;
						}
					}
					
				} else {
					bus_answer_status=BUS_ERR_LEN;
				}
			} else {
				bus_answer_status=BUS_ERR_CMD;
			}
		}
		bus_rx_send_answer(bus_answer_status, bus_answer_data, bus_answer_len);
		
		// 		// do reset if called to do so
		// 		if (reset_pending) {
		// 			wdt_enable(WDTO_15MS);
		// 			while(1) {}
		// 		}
	}
	
	// Sperre wieder freigeben
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		bus_poll_active=0;
		// Restore RX-ISR-Status
		bus_enable_rx_interrupt(rx_isr_enabled);
	}
}

inline void bus_enable_rx_interrupt(uint8_t enable) {
	if (enable) {
		UART_RX_ISR_REGISTER |= UART_RX_ISR_FLAGS;
	} else {
		UART_RX_ISR_REGISTER &= ~(UART_RX_ISR_FLAGS);
	}
}

inline uint8_t bus_rx_interrupt_status(void) {
	return UART_RX_ISR_REGISTER & (UART_RX_ISR_FLAGS);
}

// Call this in the UART-RX-interrupt
inline void bus_rx_interrupt(void) {
	if (bus_poll_active || bus_tx_active) {
		// ERROR, this should not happen
		// #warning debug
		// 		lcd_clear();
		// 		lcd_putstr(PSTR("BusErrISR-Loop"));
		// 		while(1) {}
		return;
	}
	bus_poll();
}

ISR(UART_RX_COMPLETE_VECTOR,ISR_BLOCK) {
	// 	#warning debug scheiß!
	// 	PORTB |= (1<<PB7);
	bus_rx_interrupt();
	// 	PORTB &= ~(1<<PB7);
}
#endif /* defined BUS_MODE_EVENT */