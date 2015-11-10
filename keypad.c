#include "parapin.h"
#include "keypad.h"

unsigned long kpStatus = 0;

unsigned char initKeypad() {
	if (pin_init_user (LPT1) < 0) return 0;
	pin_output_mode (LP_DATA_PINS);
        pin_input_mode (LP_SWITCHABLE_PINS);
	return 1;
}

void setLight(unsigned char state) {
	if (state) set_pin(LP_PIN07);
	else clear_pin(LP_PIN07);
}

unsigned long readKeypad() {
	int i;
	unsigned long pin15, pin13, pin12, pin11;
	unsigned long col1=0, col2=0, col3=0, col4=0;

	clear_pin (LP_PIN02 | LP_PIN03 | LP_PIN04 | LP_PIN05 | LP_PIN06);
	for (i=0; i < 5; i++) {
		set_pin(LP_PIN[i + 2]);
		pin15 = pin_is_set(LP_PIN15);
		pin13 = pin_is_set(LP_PIN13);
		pin12 = pin_is_set(LP_PIN12);
		pin11 = pin_is_set(LP_PIN11);
//			printf ("15: %i 13: %i 12: %i 11: %i\n", pin15 != 0, pin13 != 0, pin12 != 0, pin11 != 0);
		switch (i) {
			case 0:
				if (pin15) col1 |= BUTTON_SEVEN;
				if (pin13) col2 |= BUTTON_EIGHT;
				if (pin12) col3 |= BUTTON_NINE;
				if (pin11) col4 |= BUTTON_PLUS;
				break;
			case 1:
				if (pin15) col1 |= BUTTON_FOUR;
				if (pin13) col2 |= BUTTON_FIVE;
				if (pin12) col3 |= BUTTON_SIX;
				if (pin11) col4 |= BUTTON_PERIOD;
				break;
			case 2:
				if (pin15) col1 |= BUTTON_ONE;
				if (pin13) col2 |= BUTTON_TWO;
				if (pin12) col3 |= BUTTON_THREE;
				if (pin11) col4 |= BUTTON_ZERO;
				break;
			case 3:
				if (pin12) col3 |= BUTTON_STAR;
				if (pin11) col4 |= BUTTON_MINUS;
				if (pin13) col2 |= BUTTON_NUMLOCK;
				break;
			case 4:
				if (pin15) col1 |= BUTTON_SLASH;
				break;
		}
		clear_pin(LP_PIN[i + 2]);
	}
	if ((col1 & BUTTON_SEVEN) && (col1 & BUTTON_FOUR) && (col1 & BUTTON_ONE) && (col1 & BUTTON_SLASH)) col1 = 0;
	if ((col2 & BUTTON_EIGHT) && (col2 & BUTTON_FIVE) && (col2 & BUTTON_TWO) && (col2 & BUTTON_NUMLOCK)) col2 = 0;
	if ((col3 & BUTTON_NINE) && (col3 & BUTTON_SIX) && (col3 & BUTTON_THREE) && (col3 & BUTTON_STAR)) col3 = 0;
	if ((col4 & BUTTON_PLUS) && (col4 & BUTTON_PERIOD) && (col4 & BUTTON_ZERO) && (col4 & BUTTON_MINUS)) col4 = 0;

	return col1 + col2 + col3 + col4;
}

void processKeypad (unsigned long *ups, unsigned long *downs) {
	unsigned long newStatus = readKeypad();
	unsigned long looper;

	unsigned long myups=0, mydowns=0;
	if (newStatus != kpStatus) {
		newStatus = readKeypad();
		for (looper = BUTTON_NUMLOCK; looper <= BUTTON_ENTER; looper *= 2) {
			if ((newStatus & looper) && !(kpStatus & looper)) {
				mydowns |= looper;
			}
			if (!(newStatus & looper) && (kpStatus & looper)) {
				myups |= looper;
			}
		}
	}

	kpStatus = newStatus;

	(*ups) = myups;
	(*downs) = mydowns;
}

