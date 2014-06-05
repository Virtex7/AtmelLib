/*
 *    Filename: keyboard-8x8.c
 *     Version: 0.0.1
 * Description: Library for 8x8 keyboard on AVR
 *     License: Public Domain
 *
 *      Author: Copyright (C) Max Gaukler <development@maxgaukler.de>
 *        Date: 2010
 *
 * I, the copyright holder of this work, hereby release it into the public domain. This applies worldwide.
 * In case this is not legally possible:
 * I grant anyone the right to use this work for any purpose, without any conditions, unless such conditions are required by law.
 *
 * This program is provided "as is" with absolutely no warranty, neither explicit nor implicit, as far as this is possible by law.
 *
 */

#ifndef KEYBOARD_4x4_C_INCLUDED
#define KEYBOARD_4x4_C_INCLUDED

// Merge strings
#define __KEYB__CONCATENATE(a,b) a##b

#define _KEYB_DDR(x) __KEYB__CONCATENATE(DDR,x)
#define _KEYB_PORT(x) __KEYB__CONCATENATE(PORT,x)
#define _KEYB_PIN(x) __KEYB__CONCATENATE(PIN,x)

#define KEYBOARD_DDR_SET_INPUT(n) do { _KEYB_DDR(KEYBOARD_PORT_##n)&=~(1<<KEYBOARD_PIN_##n); } while (0)
#define KEYBOARD_DDR_SET_OUTPUT(n) do { _KEYB_DDR(KEYBOARD_PORT_##n)|=(1<<KEYBOARD_PIN_##n); } while (0)

#define KEYBOARD_PIN_ON(n) do { _KEYB_PORT(KEYBOARD_PORT_##n)|=(1<<KEYBOARD_PIN_##n); } while (0)
#define KEYBOARD_PIN_OFF(n) do { _KEYB_PORT(KEYBOARD_PORT_##n)&=~(1<<KEYBOARD_PIN_##n); } while (0)

#define KEYBOARD_READ_PIN(n) (_KEYB_PIN(KEYBOARD_PORT_##n)&(1<<KEYBOARD_PIN_##n))

#if !defined(KEYBOARD_PORT_1) || !defined(KEYBOARD_PIN_1) || !defined(KEYBOARD_PORT_2) || !defined(KEYBOARD_PIN_2) || !defined(KEYBOARD_PORT_3) || !defined(KEYBOARD_PIN_3) || !defined(KEYBOARD_PORT_4) || !defined(KEYBOARD_PIN_4) || !defined(KEYBOARD_PORT_5) || !defined(KEYBOARD_PIN_5) || !defined(KEYBOARD_PORT_6) || !defined(KEYBOARD_PIN_6) || !defined(KEYBOARD_PORT_7) || !defined(KEYBOARD_PIN_7) || !defined(KEYBOARD_PORT_8) || !defined(KEYBOARD_PIN_8)
#error you need to define KEYBOARD_PORT_1 ... KEYBOARD_PORT_8  (example value: B for PORTB) and KEYBOARD_PIN_1 ... KEYBOARD_PIN_8 (example value: 3 for PB3)
#endif

#if !defined(KEYBOARD_DEBOUNCE_CYCLES)
#error you need to define KEYBOARD_DEBOUNCE_CYCLES, e.g. to 20. It is the number of times keyboard_poll() needs to be called before a new key event is output by keyboard_read().
#endif

#if !defined(KEYBOARD_IO_WAIT_TIME)
#error you need to define KEYBOARD_IO_WAIT_TIME, e.g. to 100. It is the number of microseconds keyboard_poll() waits for the IO pins to settle after pulling up.
#endif

// optional: delay-time in keyboard_wait() between calls of keyboard_poll()
// default value: same as KEYBOARD_IO_WAIT_TIME*10
#if !defined(KEYBOARD_DEBOUNCE_WAIT_TIME)
#define KEYBOARD_DEBOUNCE_WAIT_TIME (KEYBOARD_IO_WAIT_TIME*10)
#endif


/* 4x4 matrix keyboard:
pin | keys
   5| 1 2 3 C
   6| 4 5 6 D
   7| 7 8 9 E
   8| A 0 B F
      _______
pin:  1 2 3 4


example: pressing key "F" connects pins 4 and 5
*/

#define KEYBOARD_NO_KEY 254
#define KEYBOARD_MULTIPLE_KEYS 255
uint8_t keyboard_read_raw(void) {
	// select all columns by pulling them low
	// rows are pulled up
	KEYBOARD_DDR_SET_OUTPUT(1);
	KEYBOARD_DDR_SET_OUTPUT(2);
	KEYBOARD_DDR_SET_OUTPUT(3);
	KEYBOARD_DDR_SET_OUTPUT(4);
	KEYBOARD_PIN_OFF(1);
	KEYBOARD_PIN_OFF(2);
	KEYBOARD_PIN_OFF(3);
	KEYBOARD_PIN_OFF(4);
	KEYBOARD_DDR_SET_INPUT(5);
	KEYBOARD_DDR_SET_INPUT(6);
	KEYBOARD_DDR_SET_INPUT(7);
	KEYBOARD_DDR_SET_INPUT(8);
	KEYBOARD_PIN_ON(5);
	KEYBOARD_PIN_ON(6);
	KEYBOARD_PIN_ON(7);
	KEYBOARD_PIN_ON(8);
	_delay_us(KEYBOARD_IO_WAIT_TIME);
#define KEYBOARD_NO_ROW 255
	uint8_t row=KEYBOARD_NO_ROW;
	// find out in which row a key has been pressed
	if (!KEYBOARD_READ_PIN(5)) {
		row=0;
	}
	if (!KEYBOARD_READ_PIN(6)) {
		if (row==KEYBOARD_NO_ROW) {
			row=1;
		} else {
			// multiple keys pressed
			return KEYBOARD_MULTIPLE_KEYS;
		}
	}
	if (!KEYBOARD_READ_PIN(7)) {
		if (row==KEYBOARD_NO_ROW) {
			row=2;
		} else {
			// multiple keys pressed
			return KEYBOARD_MULTIPLE_KEYS;
		}
	}
	if (!KEYBOARD_READ_PIN(8)) {
		if (row==KEYBOARD_NO_ROW) {
			row=3;
		} else {
			// multiple keys pressed
			return KEYBOARD_MULTIPLE_KEYS;
		}
	}

	// now we know the key's row
	// next one is the column

	// pullup all columns
	KEYBOARD_DDR_SET_INPUT(1);
	KEYBOARD_DDR_SET_INPUT(2);
	KEYBOARD_DDR_SET_INPUT(3);
	KEYBOARD_DDR_SET_INPUT(4);
	KEYBOARD_PIN_ON(1);
	KEYBOARD_PIN_ON(2);
	KEYBOARD_PIN_ON(3);
	KEYBOARD_PIN_ON(4);

	// switch all rows to high-impedance state
	KEYBOARD_DDR_SET_INPUT(5);
	KEYBOARD_DDR_SET_INPUT(6);
	KEYBOARD_DDR_SET_INPUT(7);
	KEYBOARD_DDR_SET_INPUT(8);
	KEYBOARD_PIN_OFF(5);
	KEYBOARD_PIN_OFF(6);
	KEYBOARD_PIN_OFF(7);
	KEYBOARD_PIN_OFF(8);

	// pull the single selected row low
	if (row==KEYBOARD_NO_ROW) {
		return KEYBOARD_NO_KEY;
	} else if (row==0) {
		KEYBOARD_DDR_SET_OUTPUT(5);
	} else if (row==1) {
		KEYBOARD_DDR_SET_OUTPUT(6);
	} else if (row==2) {
		KEYBOARD_DDR_SET_OUTPUT(7);
	} else if (row==3) {
		KEYBOARD_DDR_SET_OUTPUT(8);
	}

	_delay_us(KEYBOARD_IO_WAIT_TIME);
#define KEYBOARD_NO_COL 254
	uint8_t col=KEYBOARD_NO_COL;
	if (!KEYBOARD_READ_PIN(1)) {
		col=0;
	}
	if (!KEYBOARD_READ_PIN(2)) {
		if (col==KEYBOARD_NO_COL) {
			col=1;
		} else {
			return KEYBOARD_MULTIPLE_KEYS;
		}
	}
	if (!KEYBOARD_READ_PIN(3)) {
		if (col==KEYBOARD_NO_COL) {
			col=2;
		} else {
			return KEYBOARD_MULTIPLE_KEYS;
		}
	}
	if (!KEYBOARD_READ_PIN(4)) {
		if (col==KEYBOARD_NO_COL) {
			col=3;
		} else {
			return KEYBOARD_MULTIPLE_KEYS;
		}
	}

	return row*4+col;
}

volatile uint8_t keyboard_current_key=0;
volatile uint8_t keyboard_key_is_new=0;

void keyboard_poll(void) {
	static uint8_t keyboard_debounce_lock=0;
	// call this function often, e.g. in a timer interrupt or in a short main loop
	uint8_t key=keyboard_read_raw();
	if (keyboard_debounce_lock>0) {
		// ignore keys until debounce time has passed
		keyboard_debounce_lock--;
	} else {
		// debounce time is over, process new key press
		keyboard_current_key=key;
		// don't trigger events for "no key"
		if ((key != KEYBOARD_NO_KEY)) {
			keyboard_key_is_new=1;
		}
	}
	if (key==keyboard_current_key) {
		// ignore new keys or key-release until the key has been released and debouncing wait-time has passed
		keyboard_debounce_lock=KEYBOARD_DEBOUNCE_CYCLES;
	}
}

const PROGMEM uint8_t keyboard_keycodes[]= {
	1,    2,    3, 0x0C,
 4,    5,    6, 0x0D,
 7,    8,    9, 0x0E,
 0x0A,    0, 0x0B, 0x0F
};

uint8_t keyboard_translate_key(uint8_t raw_key) {
	if (raw_key>16) {
		return raw_key;
	}
	return pgm_read_byte(keyboard_keycodes+raw_key);
}


uint8_t keyboard_ready(void) {
	return keyboard_key_is_new;
}

uint8_t keyboard_read(void) {
	// keyboard_poll must not be running while this is active!
	// if keyboard_poll is called from an interrupt, call cli() before calling this function!
	if (keyboard_key_is_new) {
		keyboard_key_is_new=0;
		return keyboard_translate_key(keyboard_current_key);
	} else {
		return KEYBOARD_NO_KEY;
	}
}

void keyboard_flush(void) {
	keyboard_key_is_new=0;
}

void keyboard_wait(void) {
	while (!keyboard_ready()) {
		keyboard_poll();
		_delay_us(KEYBOARD_DEBOUNCE_WAIT_TIME);
	}
}

#endif /* ifndef KEYBOARD_8x8_C_INCLUDED */