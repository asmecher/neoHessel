#define DEBUG

#include "player.h"

#ifdef DEBUG
#undef DEBUG
#endif

#include <stdio.h>

#include "parapin.h"
#include "morse.h"
#include "time.h"
#include "speech.h"
#include "remote.h"

typedef struct PlayList {
	ListNode *v;
	long int randval;
	struct PlayList *next;
	struct PlayList *prev;
} PlayList;

void doOkBlink() {
	int i;
	int bit = is_ready;
	int readyState = getLedState(bit);
	for (i=0; i < 2; i++) {
		if (i % 2 == 0) setLedState (bit, !readyState);
		else setLedState (bit, readyState);
		usleep (100000);
	}
}

void doErrorBlink() {
	int i;
	int bit = is_ready;
	int readyState = getLedState(bit);
	for (i=0; i < 8; i++) {
		if (i % 2 == 0) setLedState (bit, !readyState);
		else setLedState (bit, readyState);
		usleep (100000);
	}
}

void doFlashyBits() {
	int i;
	for (i=0; i < 16; i++) {
		switch (i % 4) {
			case 0:
				setLedState (is_playing, 1);
				setLedState (is_paused, 0);
				setLedState (is_ready, 0);
				setLedState (is_listmode, 0);
				break;
			case 1:
				setLedState (is_playing, 0);
				setLedState (is_paused, 1);
				setLedState (is_ready, 0);
				setLedState (is_listmode, 0);
				break;
			case 2:
				setLedState (is_playing, 0);
				setLedState (is_paused, 0);
				setLedState (is_ready, 1);
				setLedState (is_listmode, 0);
				break;
			case 3:
				setLedState (is_playing, 0);
				setLedState (is_paused, 0);
				setLedState (is_ready, 0);
				setLedState (is_listmode, 1);
				break;
		}
		usleep (100000);

	}
}

PlayList *getTail (PlayList *p) {
	PlayList *l = p;
	if (p == NULL) return p;
	while (l->next != NULL) l = l->next;
	return l;
}

PlayList *getMembers (Repository *base, ListNode *allfiles) {
	ListNode *l = allfiles;
	PlayList *returner = NULL;
	PlayList *tail = NULL;

	while (l != NULL) {
		if (l->repository == base) {
			if (returner == NULL) {
				returner = (PlayList *) malloc (sizeof(PlayList));
				returner->v = l;
				returner->next = NULL;
				returner->prev = NULL;
				tail = returner;
			}
			else {
				tail->next = (PlayList *) malloc (sizeof(PlayList));
				tail->next->v = l;
				tail->next->next = NULL;
				tail->next->prev = tail;
				tail = tail->next;
			}
		}
		l = l->next;
	}
	return returner;
}

PlayList *getRecursive (Repository *base, ListNode *allfiles) {
	Repository *loop = base->children;
	PlayList *p = getMembers (base, allfiles);
	PlayList *tail = getTail(p);
	PlayList *this;
	while (loop != NULL) {
		printf ("Recursing on %s...\n", loop->name);
		this = getRecursive (loop, allfiles);
		if (this != NULL) {
			if (p == NULL) {
				p = this;
				tail = getTail (p);
			}
			else {
				tail->next = this;
				this->prev = tail;
				tail = getTail(this);
			}
		}
		loop = loop->next;
	}
	return p;
}

char startPlayer(PlayList *p, PlayerStatus *status) {
	if (p != NULL) {
		// mpg123Launch (p->v, fifo);
		mpg123Play (status, p->v);
		setLedState (is_playing, 1);
		return 1;
	}
	return 0;
}

void recursiveAdd (PlayList **playlist, Repository **curdir, ListNode **allfiles) {
	PlayList *p;
	PlayList *q;
	printf ("Recursive add pressed!\n");
	p = getRecursive (*curdir, *allfiles);
	if (*playlist == NULL) *playlist = p;
	else {
		q = getTail(p);
		q->next = p;
		if (p != NULL) p->prev = q;
	}
	doOkBlink();
	printf ("Done...\n");
}

void clearPlaylist (PlayList **playlist, PlayerStatus *status) {
	printf ("Clear playlist pressed.\n");
	if (*playlist == NULL) doErrorBlink();
	else {
		PlayList *next;
		mpg123Stop(status);
		do {
			*playlist=(*playlist)->prev;
		} while (*playlist != NULL);
		while (*playlist != NULL) {
			next = *playlist;
			if (next != NULL) *playlist = (*playlist)->next;
			else free (next);
		}
		doOkBlink();
	}
}

void togglePlaying(PlayList *playlist, PlayerStatus *status) {
	printf ("Play button pressed!\n");
	if (getLedState(is_playing)) {
		mpg123Stop(status);
		doOkBlink();
	}
	else if (playlist != NULL) {
		mpg123Play (status, playlist->v);
		setLedState (is_playing, 1);
		setLedState (is_paused, 0);
		doOkBlink();
	}
	else doErrorBlink();
}

void choosePreviousRepository (PlayList **playlist, PlayerStatus *status) {
	printf ("Previous Repository button pressed!\n");
	if (*playlist == NULL) doErrorBlink();
	else {
		Repository *thisOne = (*playlist)->v->repository;
		do {
			*playlist = (*playlist)->prev;
		} while (*playlist != NULL && (*playlist)->v->repository == thisOne);
		if (*playlist != NULL && getLedState(is_playing)) {
			startPlayer(*playlist, status);
		}
		else mpg123Stop(status);
		doOkBlink();
	}
}

void chooseNextRepository(PlayList **playlist, PlayerStatus *status) {
	printf ("Next Repository button pressed!\n");
	if (*playlist == NULL) doErrorBlink();
	else {
		Repository *thisOne = (*playlist)->v->repository;
		do {
			*playlist = (*playlist)->next;
		} while (*playlist != NULL && (*playlist)->v->repository == thisOne);
		if (*playlist != NULL && getLedState(is_playing)) {
			startPlayer(*playlist, status);
		}
		else mpg123Stop(status);
		doOkBlink();
	}
}

void chooseNextSong(PlayList **playlist, PlayerStatus *status) {
	printf ("FF button pressed!\n");
	if (*playlist != NULL && (*playlist)->next != NULL) {
		*playlist=(*playlist)->next;
		if (getLedState(is_playing)) {
			startPlayer(*playlist, status);
		}
		doOkBlink();
	}
	else doErrorBlink();
}

void choosePreviousSong (PlayList **playlist, PlayerStatus *status) {
	printf ("Rewind button pressed!\n");
	if (*playlist != NULL) {
		if ((*playlist)->prev != NULL) (*playlist)=(*playlist)->prev;
		if (getLedState(is_playing)) {
			startPlayer(*playlist, status);
		}
		doOkBlink();
	}
	else doErrorBlink();
}

void pausePlaying(PlayerStatus *status) {
	if (!mpg123Pause(status)) doErrorBlink();
	else doOkBlink();
}

void rotateLights() {
	int i;
	for (i=0; i < 8; i++) {
		setLedState (is_listmode, !getLedState (is_listmode));
		setLedState (is_paused, !getLedState (is_paused));
		setLedState (is_ready, !getLedState (is_ready));
		setLedState (is_playing, !getLedState (is_playing));
		usleep (100000);
	}
}

char backListmode, backPaused, backReady, backPlaying;
void stealthify() {
	backListmode = getLedState (is_listmode);
	backPaused = getLedState(is_paused);
	backReady = getLedState(is_ready);
	backPlaying = getLedState(is_playing);

	setLedState (is_listmode, 0);
	setLedState (is_paused, 0);
	setLedState (is_ready, 0);
	setLedState (is_playing, 0);
}

void restoreLights() {
	setLedState (is_listmode, backListmode);
	setLedState (is_paused, backPaused);
	setLedState (is_ready, backReady);
	setLedState (is_playing, backPlaying);
}

void addSongsWithKeyword (char *keyword, PlayList **playlist, ListNode *allfiles) {
	ListNode *l = allfiles;
	PlayList *t = getTail(*playlist);
	while (l != NULL) {
		if (strlen(keyword) <= strlen(l->name)) {
			int i;
			char *rName = malloc(strlen(keyword));
			for (i=0; i <= strlen(l->name)-strlen(keyword); i++) {
				strncpy (rName, l->name + i, strlen(keyword));
				rName[strlen(keyword)] = 0;
				if (strcasecmp(keyword, rName)==0) {
					if (t == NULL) {
						(*playlist) = (PlayList *) malloc(sizeof(PlayList));
						(*playlist)->next = NULL;
						(*playlist)->prev = NULL;
						(*playlist)->v = l;
						t = *playlist;
					}
					else {
						t->next = (PlayList *) malloc(sizeof(PlayList));
						t->next->prev = t;
						t->next->next = NULL;
						t->next->v = l;
						t = t->next;
					}
					break;
				}
			}
			free (rName);
		}
		l = l->next;
	}
}

void addRepositoriesWithKeyword (char *keyword, PlayList **playlist, ListNode *allfiles) {
	ListNode *l = allfiles;
	PlayList *t = getTail(*playlist);
	while (l != NULL) {
		char *canonical = getCanonicalRepositoryName(l->repository);
		if (strlen(keyword) <= strlen(canonical)) {
			int i;
			char *rName = malloc(strlen(keyword));
			for (i=0; i <= strlen(canonical)-strlen(keyword); i++) {
				strncpy (rName, canonical + i, strlen(keyword));
				rName[strlen(keyword)] = 0;
				if (strcasecmp(keyword, rName)==0) {
					if (t == NULL) {
						(*playlist) = (PlayList *) malloc(sizeof(PlayList));
						(*playlist)->next = NULL;
						(*playlist)->prev = NULL;
						(*playlist)->v = l;
						t = *playlist;
					}
					else {
						t->next = (PlayList *) malloc(sizeof(PlayList));
						t->next->prev = t;
						t->next->next = NULL;
						t->next->v = l;
						t = t->next;
					}
					break;
				}
			}
			free (rName);
		}
		free (canonical);
		l = l->next;
	}
}

void displayPlaylist (PlayList *p) {
	PlayList *l = p;
	if (l == NULL) printf ("Playlist is blank!\n");
	else while (l != NULL) {
		printf ("Item: %s\n", l->v->name);
		l = l->next;
	}
}

void randomizePlaylistByAlbum (PlayList *p) {
	PlayList *l = p;
	PlayList *s;
	PlayList *curShortest;

	ListNode *tempList;
	long int tempRandVal;
	
	printf ("Randomizing playlist by album... ");
	
	if (l == NULL) doErrorBlink();
	else {
		// Assign random ranks
          int myRand = random();
          Repository *theRepository = NULL;
	  while (l != NULL) {
			if (theRepository != l->v->repository) {
				myRand = random();
				theRepository = l->v->repository;
			}
			l->randval = myRand;
			l = l->next;
		}
		// Sort by those random ranks
		l = p;
		while (l != NULL) {
			// Find the current smallest element
			curShortest = l;
			for (s = l->next; s != NULL; s=s->next) {
				if (s->randval < curShortest->randval) curShortest = s;
			}
			
			// Swap curShortest with l
			if (l != curShortest) {
				tempList = l->v;
				l->v = curShortest->v;
				curShortest->v = tempList;
				
				tempRandVal = l->randval;
				l->randval = curShortest->randval;
				curShortest->randval = tempRandVal;
			}
			
			l = l->next;
		}
	}
	printf ("Done.\n");
	doOkBlink();
}

void randomizePlaylist (PlayList *p) {
	PlayList *l = p;
	PlayList *s;
	PlayList *curShortest;

	ListNode *tempList;
	long int tempRandVal;
	
	printf ("Randomizing playlist... ");
	
	if (l == NULL) doErrorBlink();
	else {
		// Assign random ranks
	  while (l != NULL) {
			l->randval = random();
			l = l->next;
		}
		// Sort by those random ranks
		l = p;
		while (l != NULL) {
			// Find the current smallest element
			curShortest = l;
			for (s = l->next; s != NULL; s=s->next) {
				if (s->randval < curShortest->randval) curShortest = s;
			}
			
			// Swap curShortest with l
			if (l != curShortest) {
				tempList = l->v;
				l->v = curShortest->v;
				curShortest->v = tempList;
				
				tempRandVal = l->randval;
				l->randval = curShortest->randval;
				curShortest->randval = tempRandVal;
			}
			
			l = l->next;
		}
	}
	printf ("Done.\n");
	doOkBlink();
}

void showDebugInfo(PlayList *playlist, ListNode *allfiles) {
	PlayList *loopy = playlist;
	int curindex = 1;
	int totalsongs = 1;
	if (playlist == NULL) {
		printf ("Playlist is empty!");
	}
	else {
		while (loopy->prev != NULL) {
			loopy = loopy->prev;		
			curindex++;
		}
		while (loopy->next != NULL) {
			totalsongs++;
			loopy = loopy->next;
		}
		printf ("You are currently at song %i of %i.\n", curindex, totalsongs);
		
	}
}

void doDialThing(int newDialSetting, int oldDialSetting, PlayList **playlist, PlayerStatus *status) {
	int try1;
	int try2;

	int difference;

	usleep (10000);
	try1 = getDialSetting();
	usleep (10000);
	try2 = getDialSetting();

	if (try1 != try2 || try1 != newDialSetting || try2 != newDialSetting) {
		if (try1 == try2 && try1 != newDialSetting) newDialSetting = try1;
		if (try1 != try2 && try1 != newDialSetting && try2 != newDialSetting) {
			fprintf (stderr, "I'm terribly confused.\n");
		}
	}

	difference = newDialSetting - oldDialSetting;

	if (difference != 0) {
		if (difference == 1 || difference == -3 || difference == 5 || difference == -9) {
			mpg123Forward(status);
		}
		else if (difference == -1 || difference == 3 || difference == -5 || difference == 9) {
			mpg123Back(status);
		}
		else if (difference == 10 || difference == -90) {
			chooseNextSong(playlist, status);
		}
		else if (difference == -10 || difference == 90) {
			choosePreviousSong(playlist, status);
		}
		else if (difference == 100 || difference == -900) {
			chooseNextRepository(playlist, status);
		}
		else if (difference == -100 || difference == 900) {
			choosePreviousRepository(playlist, status);
		}
		else fprintf (stderr, "I'm confused by %i.\n", difference);
	}
}

int main() {
	ListNode *allfiles;
	PlayList *playlist = NULL;
	MorseKey *mykey;
	char thekey;
	char isLocked = 0;
	char *keyword = NULL;
	int oldDialSetting = -1;
	char isStringInput = 0;
	Repository *repository = makeRepositoryNode (NULL, NULL, NULL, repositoryDir);
	Repository *curdir = repository;
	PlayerStatus status;
	if (pin_init_user (LPT1) < 0) {
		printf ("Error initializing lpt1!\n");
		return(-1);
	}
	pin_input_mode (LP_DATA_PINS);
	pin_output_mode (LP_SWITCHABLE_PINS);

	mykey = initKey (morse_key_pin);

	allfiles = readDir (repository);

	initSpeech();

	say ("welcome");
	doFlashyBits();

	setLedState (is_playing, 0);
	setLedState (is_paused, 0);
	setLedState (is_ready, 1);
	setLedState (is_listmode, 1);
	
	srandom (time(NULL));

	mpg123Launch(&status);

	while (1) {
		int newDialSetting = getDialSetting();
		if (newDialSetting != oldDialSetting) {
			doDialThing (newDialSetting, oldDialSetting, &playlist, &status);
			oldDialSetting = newDialSetting;
		}

		if (hasFinished(&status) && getLedState(is_playing)) {
			if (playlist->next != NULL) {
				playlist = playlist->next;
				mpg123Play (&status, playlist->v);
			}
			else {
				setLedState (is_playing, 0);
			}
		}
		usleep (1000);
		doKey (mykey);
		mpg123ProcessStatus (&status);
		thekey = getCharacter(mykey);
		if (thekey != 0) {
			if (isStringInput != 0) {
				if (thekey == ',') {
		                        if (!getState(&status, STATE_PLAYING))
					  say ("Adding songs by keyword");
					setLedState (is_ready, !getLedState(is_ready));
					addSongsWithKeyword (keyword, &playlist, allfiles);
					free (keyword);
					keyword = NULL;
					isStringInput = 0;
					doOkBlink();
				}
				else if (thekey == '.') {
		                        if (!getState(&status, STATE_PLAYING))
					  say ("Adding albums by keyword");
						setLedState (is_ready, !getLedState(is_ready));
					addRepositoriesWithKeyword (keyword, &playlist, allfiles);
					free (keyword);
					keyword = NULL;
					isStringInput = 0;
					doOkBlink();
				}
				else if (thekey == '?') {
		                        if (!getState(&status, STATE_PLAYING))
					  say (keyword);
				}
				else if (thekey == '*') {
		                        if (!getState(&status, STATE_PLAYING))
					  say ("backspace");
					if (keyword != NULL && strlen(keyword) != 0) {
						keyword[strlen(keyword)-1]=0;
					}
				}
				else {
					if (keyword == NULL) {
						keyword = malloc(2);
						keyword[0] = keyword[1] = 0;
					}
					else {
						keyword = (char *)realloc(keyword, strlen(keyword) + 2);
						keyword[strlen(keyword)+1] = 0;
					}
					keyword[strlen(keyword)] = thekey;
				}
				doOkBlink();
			}
			else if (isLocked) {
				if (thekey == 'l') {
		                        if (!getState(&status, STATE_PLAYING))
					  say ("Unlocking");
					isLocked = 0;
					restoreLights();
				}
				else {
		                        if (!getState(&status, STATE_PLAYING))
					  say ("locked");
					rotateLights();
				}
			}
			else switch (thekey) {
				case 'v':
					recursiveAdd(&playlist, &curdir, &allfiles);
					break;
				case 'c':
					clearPlaylist (&playlist, &status);
		                        if (!getState(&status, STATE_PLAYING))
					  say ("clearing playlist");
					break;
				case 'p': togglePlaying (playlist, &status); break;
				case 'r': choosePreviousRepository (&playlist, &status); break;
				case 'f': chooseNextRepository (&playlist, &status); break;
				case 'e': chooseNextSong (&playlist, &status); break;
				case 't': choosePreviousSong (&playlist, &status); break;
				case 'a': pausePlaying(&status); break;
				case 'z':
					randomizePlaylist(playlist);
		                        if (!getState(&status, STATE_PLAYING))
					  say ("randomizing");
					break;
				case 'k':
					randomizePlaylistByAlbum(playlist);
		                        if (!getState(&status, STATE_PLAYING))
					  say ("randomizing by album");
					break;
				case 's':
					isStringInput = 1;
		                        if (!getState(&status, STATE_PLAYING))
					  say ("search");
					setLedState (is_ready, !getLedState(is_ready));
					doOkBlink();
					break;
				case 'l':
		                        if (!getState(&status, STATE_PLAYING))
					  say ("Locking");
					stealthify();
					isLocked = 1;
					break;
				case '5':
					showDebugInfo(playlist, allfiles);
		                        if (!getState(&status, STATE_PLAYING))
					  say (playlist->v->name);
					break;
				default: {
					doErrorBlink();
					// printf ("Unknown character pressed: %c!\n", thekey);
				} break;
			}
		}
	}
	printf ("End loop.\n");
	close (status);
}

