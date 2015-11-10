#ifndef KEYPAD_H
#define KEYPAD_H

typedef enum {
	BUTTON_NUMLOCK = 1,
	BUTTON_SLASH = 2,
	BUTTON_STAR = 4,
	BUTTON_MINUS = 8,
	BUTTON_SEVEN = 16,
	BUTTON_EIGHT = 32,
	BUTTON_NINE = 64,
	BUTTON_PLUS = 128,
	BUTTON_FOUR = 256,
	BUTTON_FIVE = 512,
	BUTTON_SIX = 1024,
	BUTTON_ONE = 2048,
	BUTTON_TWO = 4096,
	BUTTON_THREE = 8192,
	BUTTON_ZERO = 16384,
	BUTTON_PERIOD = 32768,
	BUTTON_ENTER = 65536,
} keypad_button;

void processKeypad (unsigned long *ups, unsigned long *downs);

#endif
