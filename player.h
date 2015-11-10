#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "parapin.h"

// player.h: Defaults for the mp3 player thing.

void inline showError(const char *msg) {
	#ifdef DEBUG
		printf ("%s", msg);
	#endif
}

const char *fifoDir = "/var/tmp/ehessel-fifo";
const char *repositoryDir = "/var/mp3";

const unsigned long outpins[] = {LP_PIN01, LP_PIN14, LP_PIN16, LP_PIN17};
char states[] = {0, 0, 0, 0,};
char bouncestates[] = {0, 0, 0, 0, 0};

const unsigned long morse_key_pin = LP_PIN04;

const char is_playing = 0;
const char is_paused = 1;
const char is_ready = 2;
const char is_listmode = 3;

inline char getLedState (int state) {
	return states[state];
}
inline char setLedState (int state, char value) {
	states[state] = value;
	if (value) {
		set_pin (outpins[state]);
	}
	else {
		clear_pin (outpins[state]);
	}
}

int dialpins[] = {LP_PIN08, LP_PIN07, LP_PIN12, LP_PIN13, LP_PIN05, LP_PIN06, LP_PIN09, LP_PIN11, LP_PIN03, LP_PIN02, LP_PIN15, -1};

int getDialSetting() {
	int accumulator=0;
	int i;
	for (i = 0; dialpins[i] != -1; i++) {
		if (!pin_is_set(dialpins[i])) switch (i) {
			case 8: accumulator += 1; break;
			case 9: accumulator += 2; break;
			case 10: accumulator += 8; break;
			case 4: accumulator += 10; break;
			case 5: accumulator += 20; break;
			case 6: accumulator += 40; break;
			case 7: accumulator += 80; break;
			case 0: accumulator += 100; break;
			case 1: accumulator += 200; break;
			case 2: accumulator += 400; break;
			case 3: accumulator += 800; break;
		}
	}
	return accumulator;
}
#endif

