#define DEBUG

#include "repository.h"
#include "player.h"
#include "sort.h"

#ifdef DEBUG
#undef DEBUG
#endif

#include <stdio.h>

#include "keypad.h"
#include "parapin.h"
#include "time.h"
#include "remote.h"

typedef struct PlayList {
	ListNode *v;
	long int randval;
	struct PlayList *next;
	struct PlayList *prev;
} PlayList;

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
}

void clearPlaylist (PlayList **playlist, PlayerStatus *status) {
	printf ("Clear playlist pressed.\n");
	if (*playlist != NULL) {
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
	}
}

void choosePreviousRepository (PlayList **playlist, PlayerStatus *status) {
	printf ("Previous Repository button pressed!\n");
	if (*playlist != NULL) {
		Repository *thisOne = (*playlist)->v->repository;
		do {
			*playlist = (*playlist)->prev;
		} while (*playlist != NULL && (*playlist)->v->repository == thisOne);
		if (*playlist != NULL && getLedState(is_playing)) {
			startPlayer(*playlist, status);
		}
		else mpg123Stop(status);
	}
}

void chooseNextRepository(PlayList **playlist, PlayerStatus *status) {
	printf ("Next Repository button pressed!\n");
	if (*playlist != NULL) {
		Repository *thisOne = (*playlist)->v->repository;
		do {
			*playlist = (*playlist)->next;
		} while (*playlist != NULL && (*playlist)->v->repository == thisOne);
		if (*playlist != NULL && getLedState(is_playing)) {
			startPlayer(*playlist, status);
		}
		else mpg123Stop(status);
	}
}

void chooseNextSong(PlayList **playlist, PlayerStatus *status) {
	printf ("FF button pressed!\n");
	if (*playlist != NULL && (*playlist)->next != NULL) {
		*playlist=(*playlist)->next;
		if (getLedState(is_playing)) {
			startPlayer(*playlist, status);
		}
	}
}

void choosePreviousSong (PlayList **playlist, PlayerStatus *status) {
	printf ("Rewind button pressed!\n");
	if (*playlist != NULL) {
		if ((*playlist)->prev != NULL) (*playlist)=(*playlist)->prev;
		if (getLedState(is_playing)) {
			startPlayer(*playlist, status);
		}
	}
}

void pausePlaying(PlayerStatus *status) {
	mpg123Pause(status);
}

void playlistNodeSwap(void *a, void *b) {
        struct PlayList *alist = (struct PlayList *) a;
        struct PlayList *blist = (struct PlayList *) b;
        struct PlayList temp;

        temp.v = alist->v;
	temp.randval = alist->randval;

        alist->v = blist->v;
        alist->randval = blist->randval;

        blist->v = temp.v;
        blist->randval = temp.randval;
}

int playlistSortCompare (void *a, void *b) {
        struct PlayList *alist = (struct PlayList *) a;
        struct PlayList *blist = (struct PlayList *) b;
	int i;
	if (alist==blist) return 0;
	if (alist==NULL || blist==NULL) return 0;
	if (alist->v == NULL || blist->v == NULL) return 0;
	if (alist->v->repository == NULL || blist->v->repository==NULL) return 0;
	if (alist->v->repository->name==NULL || blist->v->repository->name==NULL) return 0;
	i = strcmp(alist->v->repository->name, blist->v->repository->name);
	if (i != 0) return i;
	if (alist->v->name==NULL || blist->v->name == NULL) return 0;
	return strcmp(alist->v->name, blist->v->name);
}

int playlistRandomizeCompare (void *a, void *b) {
        struct PlayList *alist = (struct PlayList *) a;
        struct PlayList *blist = (struct PlayList *) b;
	return blist->randval - alist->randval;
}

void *playlistNext (void *node) {
        return ((struct PlayList *) node)->next;
}

void randomizePlaylistByAlbum (PlayList *p) {
	PlayList *l = p;

	sort(p, &playlistNext, &playlistNodeSwap, &playlistSortCompare);

	if (l != NULL) {
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
		sort(p, &playlistNext, &playlistNodeSwap, &playlistRandomizeCompare);
	}
	printf ("Done.\n");
}

void randomizePlaylist (PlayList *p) {
	PlayList *l = p;

	printf ("Randomizing playlist... ");
	
	while (l != NULL) {
		l->randval = random();
		l = l->next;
	}

	sort(p, &playlistNext, &playlistNodeSwap, &playlistRandomizeCompare);
	printf ("Done.\n");
}

int main(int argc, char **argv, char **envp) {
	ListNode *allfiles;
	PlayList *playlist = NULL;
	Repository *repository = makeRepositoryNode (NULL, NULL, NULL, repositoryDir);
	Repository *curdir = repository;
	PlayerStatus status;
	PlayList *debugtemp;
	if (pin_init_user (LPT1) < 0) {
		printf ("Error initializing lpt1!\n");
		return(-1);
	}
	pin_output_mode (LP_DATA_PINS);
	pin_input_mode (LP_SWITCHABLE_PINS);

	allfiles = readDir (repository);

	srandom (time(NULL));

	mpg123Launch(&status);

	recursiveAdd(&playlist, &curdir, &allfiles);

	randomizePlaylistByAlbum(playlist);
	startPlayer(playlist, &status);

	while (1) {
		unsigned long ups, downs;
		if (hasFinished(&status)) {
			if (playlist->next != NULL) {
				playlist = playlist->next;
				mpg123Play (&status, playlist->v);
			}
		}
		processKeypad (&ups, &downs);
		mpg123ProcessStatus (&status);

		switch (downs) {
			case BUTTON_EIGHT: choosePreviousRepository (&playlist, &status); break;
			case BUTTON_TWO: chooseNextRepository (&playlist, &status); break;
			case BUTTON_SIX: chooseNextSong (&playlist, &status); break;
			case BUTTON_FOUR: choosePreviousSong (&playlist, &status); break;
			case BUTTON_FIVE: pausePlaying(&status); break;
			case BUTTON_SEVEN: mpg123Back(&status); break;
			case BUTTON_NINE: mpg123Forward(&status); break;
			case BUTTON_PERIOD: execl("/usr/bin/shutdown.sh", NULL); break;
			case BUTTON_ZERO: sort(playlist, &playlistNext, &playlistNodeSwap, &playlistSortCompare); break;
			case BUTTON_ONE: randomizePlaylistByAlbum (playlist); break;
			case BUTTON_THREE: randomizePlaylist (playlist); break;
		}
		usleep (10000);
		/* if (thekey != 0) {
			if (isStringInput != 0) {
				if (thekey == ',') {
		                        if (!getState(&status, STATE_PLAYING))
					  say ("Adding songs by keyword");
					setLedState (is_ready, !getLedState(is_ready));
					addSongsWithKeyword (keyword, &playlist, allfiles);
					free (keyword);
					keyword = NULL;
					isStringInput = 0;
				}
				else if (thekey == '.') {
		                        if (!getState(&status, STATE_PLAYING))
					  say ("Adding albums by keyword");
						setLedState (is_ready, !getLedState(is_ready));
					addRepositoriesWithKeyword (keyword, &playlist, allfiles);
					free (keyword);
					keyword = NULL;
					isStringInput = 0;
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
					// printf ("Unknown character pressed: %c!\n", thekey);
				} break;
			}
		} */
	}
	printf ("End loop.\n");
	close (status);
}

