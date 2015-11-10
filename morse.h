#ifndef MORSE_H
#define MORSE_H

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
// #include "player.h"
#include "parapin.h"

typedef struct MorseMap {
	char letter;
	char *sequence;
} MorseMap;

MorseMap morse_table[] = {
	{'a', ".-"},
	{'b', "-..."},
	{'c', "-.-."},
	{'d', "-.."},
	{'e', "."},
	{'f', "..-."},
	{'g', "--."},
	{'h', "...."},
	{'i', ".."},
	{'j', ".---"},
	{'k', "-.-"},
	{'l', ".-.."},
	{'m', "--"},
	{'n', "-."},
	{'o', "---"},
	{'p', ".--."},
	{'q', "--.-"},
	{'r', ".-."},
	{'s', "..."},
	{'t', "-"},
	{'u', "..-"},
	{'v', "...-"},
	{'w', ".--"},
	{'x', "-..-"},
	{'y', "-.--"},
	{'z', "--.."},
	{'0', "-----"},
	{'1', ".----"},
	{'2', "..---"},
	{'3', "...--"},
	{'4', "....-"},
	{'5', "....."},
	{'6', "-...."},
	{'7', "--..."},
	{'8', "---.."},
	{'9', "----."},
	{'.', ".-.-.-"},
	{',', "--..--"},
	{':', "---..."},
	{'?', "..--.."},
	{'\'', ".----."},
	{'/', "-....-"},
	{'(', "-.--.-"},
	{'"', ".-..-."}
};

#define morse_buf_size 2048
#define morse_out_buf_size 10
#define morse_intermediate_buf_size 8
typedef struct MorseKey {
	unsigned char buffer[morse_buf_size];
	int curlocation;
	int bitStartPos;
	unsigned long bit;
	int cycles_since_bit;
	char outbuffer[morse_out_buf_size];
	char dits_and_dahs[morse_intermediate_buf_size];
} MorseKey;

void inline putNewValue (MorseKey *who, char state) {
	who->buffer[who->curlocation] = state;
	who->curlocation = (who->curlocation + 1) % morse_buf_size;
}

inline char checkLastValues (MorseKey *who, int history) {
	int loc = who->curlocation - history - 1;
	if (loc < 0) loc += morse_buf_size;
	return who->buffer[loc];
}

char laststate = 0;

inline char lookForEdge (MorseKey *who, char *newstate) {
	if ((*newstate = checkLastValues(who, 0)) != laststate) {
		laststate = *newstate;
		return 1;
	}
	return 0;
}

int getCharacterCount (MorseKey *who) {
	return strlen(who->outbuffer);
}

char getCharacter (MorseKey *who) {
	char retval = who->outbuffer[0];
	if (retval != 0) memmove (who->outbuffer, who->outbuffer+1, morse_out_buf_size-1);
	return retval;
}
char putCharacter (MorseKey *who, char c) {
	memmove (who->outbuffer+1, who->outbuffer, morse_out_buf_size-2);
	who->outbuffer[0] = c;
}

void finishLetter (MorseKey *who) {
	int i=0;
	char returner=0;
	while ((i * sizeof(MorseMap)) < sizeof(morse_table)) {
		if (strcmp(who->dits_and_dahs, morse_table[i].sequence)==0) {
			returner = morse_table[i].letter;
			break;
		}
		i++;
	}
	if (returner != 0) {
		putCharacter (who, returner);
	}
	else if (strlen(who->dits_and_dahs)!=0) {
		putCharacter (who, '*');
	}
	memset (who->dits_and_dahs, 0, morse_intermediate_buf_size);
}

char putDitOrDah (MorseKey *who, char c) {
	int cursize = strlen(who->dits_and_dahs);
	if (cursize < morse_intermediate_buf_size-1) who->dits_and_dahs[cursize] = c;
}

int getDistance (MorseKey *who, int loc) {
	if (who->curlocation > loc) {
		return (who->curlocation-loc);
	}
	else {
		return (morse_buf_size - (loc - who->curlocation));
	}
}

const int length_threshold = 7;
const int letter_end_threshold = 20;

char inline doKey (MorseKey *who) {
	char curstate;
	int thisdistance;
	
	who->cycles_since_bit++;
	putNewValue (who, ! pin_is_set (who->bit));
	if (who->curlocation == who->bitStartPos) who->bitStartPos = (who->bitStartPos + 1) % morse_buf_size;
	if (lookForEdge (who, &curstate)) {
		if (curstate) {
			who->bitStartPos = who->curlocation;
			if (who->cycles_since_bit > letter_end_threshold) {
				finishLetter (who);
			}
		}
		else {
			thisdistance = getDistance(who, who->bitStartPos);
			putDitOrDah (who, thisdistance>length_threshold?'-':'.');
		}
		who->cycles_since_bit = 0;
	}
	else {
		if (who->cycles_since_bit > letter_end_threshold && strlen(who->dits_and_dahs) != 0) {
		finishLetter (who);
		who->cycles_since_bit = 0;
		}
	}
}

void closeKey(MorseKey *who) {
	free (who);
}

MorseKey *initKey(unsigned long bit) {
	MorseKey *returner = (MorseKey *) malloc (sizeof(MorseKey));
	if (returner == NULL) return returner;
	memset (returner->dits_and_dahs, 0, morse_intermediate_buf_size);
	memset (returner->buffer, 0, morse_buf_size);
	memset (returner->outbuffer, 0, morse_out_buf_size);
	returner->cycles_since_bit = 0;
	returner->bit = bit;
	returner->bitStartPos = 0;
	returner->curlocation = 0;
	return returner;
}

#endif MORSE_H
