/*
 *    Filename: lib/0.1.3/io/serial/i2c.h
 *     Version: 0.0.4
 * Description: I²C lib for transmitting and receiving I²C-Messages via Software-I²C
 *     License: GPLv2 or later
 *     Depends: (none)
 *
 *      Author: Copyright (C) Philipp Hörauf
 *        Date: 2011-03-29
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

#ifndef _HAVE_LIB_io_serial_i2c_h
#define _HAVE_LIB_io_serial_i2c_h

// mit I2C_DELAY_FACTOR >1 kann das Timing verlansgamt werden (Integer)
#ifndef I2C_DELAY_FACTOR
#define I2C_DELAY_FACTOR 1
#endif

#include <math.h>

// Erwartete Definitionen:
// #define I2C_DDR
// #define I2C_PORT
// #define I2C_PIN
// #define SCL
// #define SDA
// uint8_t ERR - Variable (global)

// Ausgelesen werden können:
// ERR - zeigt an, dass ein Fehler aufgetreten ist.


void i2c_init(void) {
	I2C_DDR |= (1<<SCL) | (0<<SDA);
	I2C_PORT |= (1<<SCL) | (1<<SDA);
}

void i2c_clock(void) { // Erzeugt einen Clock-Puls
sbi(I2C_PORT,SCL);
delayus(5*I2C_DELAY_FACTOR);
cbi(I2C_PORT,SCL);
delayus(5*I2C_DELAY_FACTOR);
}

void i2c_start(void) { // Sende Startbedingung
// SCL high, kurz warten, SDA falling edge
sbi(I2C_DDR,SDA);
I2C_PORT |= (1<<SDA);
I2C_PORT |= (1<<SCL);
delayus(10*I2C_DELAY_FACTOR);
cbi(I2C_PORT,SDA);
delayus(5*I2C_DELAY_FACTOR);

// SCL low, jetzt darf SDA auf das erste Datenbit gesetzt werden
cbi(I2C_PORT,SCL);
delayus(5*I2C_DELAY_FACTOR);
}

void i2c_stop(void) { // Sende Stoppbedingung
sbi(I2C_DDR,SDA);
cbi(I2C_PORT,SDA);
sbi(I2C_PORT,SCL);
delayus(5*I2C_DELAY_FACTOR);
sbi(I2C_PORT,SDA);
}

// uint8_t ERR = 0;
void i2c_receive_ack(void) { // Frage ACK ab
cbi(I2C_DDR,SDA);
sbi(I2C_PORT,SDA);
delayus(4*I2C_DELAY_FACTOR);
if(in(I2C_PIN,SDA,1)) {
	ERR = 0;
} else {
	ERR = 1;
// 	#warning DEBUG
// 	PORTC &= ~1; 
// 	uart_tx_strln("I2C ERROR");
// 	while(1) {}
}
delayus(4*I2C_DELAY_FACTOR);
i2c_clock();
delayus(10*I2C_DELAY_FACTOR);
}

void i2c_nak_tx(void) { // erzeuge NAK
	sbi(I2C_DDR,SDA);
	sbi(I2C_PORT,SDA);
	delayus(5*I2C_DELAY_FACTOR);
	i2c_clock();
	delayus(5*I2C_DELAY_FACTOR);
	cbi(I2C_DDR,SDA);
}

void i2c_data_tx(uint8_t byte) { // Sende ein Byte Daten
sbi(I2C_DDR,SDA);
for(uint8_t i=8; i>0; i--) {
	if (byte&_BV(i-1)) {
		sbi(I2C_PORT,SDA);
	} else {
		cbi(I2C_PORT,SDA);
	}
	delayus(2*I2C_DELAY_FACTOR);
	i2c_clock();
}
cbi(I2C_DDR,SDA);
}

void i2c_data_tx_x16(uint16_t byte) { // Sende zwei Bytes Daten (16bit)
sbi(I2C_DDR,SDA);
for(uint8_t i=16; i>0; i--) {
	if (byte&_BV(i-1)) {
		sbi(I2C_PORT,SDA);
	} else {
		cbi(I2C_PORT,SDA);
	}
	if (i == 8) {
		i2c_receive_ack();
	}
	delayus(2*I2C_DELAY_FACTOR);
	i2c_clock();
}
cbi(I2C_DDR,SDA);
}

uint8_t i2c_data_rx(void) { // Empfange ein Byte Daten
sbi(I2C_PORT,SDA);
uint8_t temp = 0;
for(uint8_t i=8; i>0; i--) {
	if (I2C_PIN & (1<<SDA)) {
		// eine 1
		temp |= (1<<(i-1));
	} else {
		// eine 0
		// tut gar nichts, die 0en sind schon da.
	}
	delayus(2*I2C_DELAY_FACTOR);
	i2c_clock();
}
return temp;
}

uint16_t i2c_data_rx_x16(void) { // Empfange zwei Bytes Daten (1 var 16bit)
cbi(I2C_DDR,SDA);
uint16_t temp = 0;
for(uint8_t i=16; i>0; i--) {
	if (I2C_PIN & (1<<SDA)) {
		// eine 1
		temp |= (1<<(i-1));
	} else {
		// eine 0
		// tut gar nichts, die 0en sind schon da.
	}
	if (i == 8) {
		i2c_receive_ack();
	}
	delayus(2*I2C_DELAY_FACTOR);
	i2c_clock();
}
return temp;
}

// data: Nutzdaten
// com:  Kommando / Befehl
// adr:  Adresse

void i2c_tx(uint8_t data, uint8_t com, uint8_t adr) {
	i2c_start();
	i2c_data_tx(adr);
	i2c_receive_ack();
	i2c_data_tx(com);
	i2c_receive_ack();
	i2c_data_tx(data);
	i2c_receive_ack();
	i2c_stop();
}

uint8_t i2c_rx(uint8_t com, uint8_t adr) {
	uint8_t temp;
	i2c_start();
	i2c_data_tx(adr);
	i2c_receive_ack();
	i2c_data_tx(com);
	i2c_receive_ack();
	temp = i2c_data_rx();
	i2c_receive_ack();
	i2c_stop();
	return temp;
}

uint16_t i2c_rx_16(uint8_t com, uint8_t adr) {
	uint8_t temp;
	i2c_start();
	i2c_data_tx(adr);
	i2c_receive_ack();
	i2c_data_tx(com);
	i2c_receive_ack();
	temp = (i2c_data_rx()<<7);
	i2c_receive_ack();
	temp |= i2c_data_rx();
	i2c_stop();
	return temp;
}

uint8_t i2c_rx_DS1307(uint8_t com) {
	uint8_t temp;
	i2c_start();
	i2c_data_tx(0b11010000); // Sende Adresse mit Schreibe-Bit
	i2c_receive_ack();
	i2c_data_tx(com); // RAM-Pointer
	i2c_receive_ack();

	i2c_start(); // "repeated start"
	i2c_data_tx(0b11010001); // Sende Adresse mit Lese-Bit
	i2c_receive_ack();
	temp = i2c_data_rx(); // lese Daten aus
	i2c_nak_tx();
	i2c_stop();
	return temp;
}

// Ansteuerung für einen LM75-Standartsensor:

void i2c_lm75_init(uint8_t adr) { // Auflösung: 12bit
i2c_tx(adr, 1, 0b01100100);	// genaue conf - siehe Datenblatt TMP75 - Texas Instruments
}

float i2c_lm75_read(uint8_t adr) {
	uint16_t tmp;
	float temperatur = 0;
	tmp = i2c_rx_16(0 ,adr);
	if (tmp < 0x800) {
		temperatur = (float)tmp / (float)16;
	} else {
		
		
	}
	return temperatur;
}

#endif
