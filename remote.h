#ifndef REMOTE_H
#define REMOTE_H

#include <stdio.h>

#include "repository.h"

#define STATE_PAUSED 1
#define STATE_PLAYING 4

typedef struct PlayerStatus {
	FILE *in;
	FILE *out;
	unsigned char state;
	int pid;
} PlayerStatus;

inline char setState (PlayerStatus *who, int state, char status);
inline char getState (PlayerStatus *who, int state);
inline char hasFinished(PlayerStatus *player);
void mpg123Close(PlayerStatus *who);
void mpg123ProcessStatus(PlayerStatus *who);
void mpg123Launch (PlayerStatus *status);
void mpg123Play (PlayerStatus *status, ListNode *who);
char mpg123Pause(PlayerStatus *status);
void mpg123Stop(PlayerStatus *status);
void mpg123Forward(PlayerStatus *status);
void mpg123Back(PlayerStatus *status);
void mpg123Decree(PlayerStatus *status, const char *statement);

#endif
