#define DEBUG

#include "player.h"

#ifndef DEBUG
#define DEBUG
#endif

#include <stdio.h>

#include "keypad.h"
#include "parapin.h"
#include "time.h"
#include "remote.h"

int main(int argc, char **argv, char **envp) {
	if (pin_init_user (LPT1) < 0) {
		printf ("Error initializing lpt1!\n");
		return(-1);
	}
	pin_output_mode (LP_DATA_PINS);
	pin_input_mode (LP_SWITCHABLE_PINS);

	while (1) {
		unsigned long ups, downs;
		processKeypad (&ups, &downs);

		switch (downs) {
			case BUTTON_ZERO: printf("0"); break;
			case BUTTON_ONE: printf("1"); break;
			case BUTTON_TWO: printf("2"); break;
			case BUTTON_THREE: printf("3"); break;
			case BUTTON_FOUR: printf("4"); break;
			case BUTTON_FIVE: printf("5"); break;
			case BUTTON_SIX: printf("6"); break;
			case BUTTON_SEVEN: printf("7"); break;
			case BUTTON_EIGHT: printf("8"); break;
			case BUTTON_NINE: printf("9"); break;
		}
		usleep (5000);
	}
}

