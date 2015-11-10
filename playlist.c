#include <stdio.h>
#include <stdlib.h>

#include "sort.h"
#include "remote.h"

#include "playlist.h"

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
	PlayList *thisList;
	while (loop != NULL) {
		thisList = getRecursive (loop, allfiles);
		if (thisList != NULL) {
			if (p == NULL) {
				p = thisList;
				tail = getTail (p);
			}
			else {
				tail->next = thisList;
				thisList->prev = tail;
				tail = getTail(thisList);
			}
		}
		loop = loop->next;
	}
	return p;
}

char startPlayer(PlayList *p, PlayerStatus *status) {
	if (p != NULL) {
		mpg123Play (status, p->v);
		return 1;
	}
	return 0;
}

void recursiveAdd (PlayList **playlist, Repository **curdir, ListNode **allfiles) {
	PlayList *p;
	PlayList *q;
	p = getRecursive (*curdir, *allfiles);
	if (*playlist == NULL) *playlist = p;
	else {
		q = getTail(p);
		q->next = p;
		if (p != NULL) p->prev = q;
	}
}

void clearPlaylist (PlayList **playlist, PlayerStatus *status) {
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
	if (*playlist != NULL) {
		Repository *thisOne = (*playlist)->v->repository;
		while ((*playlist)->prev != NULL && (*playlist)->v->repository == thisOne) {
			*playlist = (*playlist)->prev;
		}
		if (*playlist != NULL && !hasFinished(status)) {
			startPlayer(*playlist, status);
		}
		else mpg123Stop(status);
	}
}

void chooseNextRepository(PlayList **playlist, PlayerStatus *status) {
	if (*playlist != NULL) {
		Repository *thisOne = (*playlist)->v->repository;
		while ((*playlist)->next != NULL && (*playlist)->v->repository == thisOne) {
			*playlist = (*playlist)->next;
		}
		if (*playlist != NULL && !hasFinished(status)) {
			startPlayer(*playlist, status);
		}
		else mpg123Stop(status);
	}
}

void chooseNextSong(PlayList **playlist, PlayerStatus *status) {
	if (*playlist != NULL && (*playlist)->next != NULL) {
		*playlist=(*playlist)->next;
		if (!hasFinished(status)) {
			startPlayer(*playlist, status);
		}
	}
}

void choosePreviousSong (PlayList **playlist, PlayerStatus *status) {
	if (*playlist != NULL) {
		if ((*playlist)->prev != NULL) (*playlist)=(*playlist)->prev;
		if (!hasFinished(status)) {
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
	char *aname = getCanonicalName(alist->v);
	char *bname = getCanonicalName(blist->v);
	int i = strcasecmp(aname, bname);
	free (aname);
	free (bname);
	return i;
}

int playlistRandomizeCompare (void *a, void *b) {
        struct PlayList *alist = (struct PlayList *) a;
        struct PlayList *blist = (struct PlayList *) b;

	int i = alist->randval - blist->randval;
	if (i==0) {
		return strcasecmp (alist->v->name, blist->v->name);
	}
	return i;
}

void *playlistPrev (void *node) {
        return ((struct PlayList *) node)->prev;
}

void *playlistNext (void *node) {
        return ((struct PlayList *) node)->next;
}

void sortPlaylist (PlayList *p) {
	quicksort(p, &playlistPrev, &playlistNext, &playlistNodeSwap, &playlistSortCompare);
}

void randomizePlaylistByAlbum (PlayList *p) {
	PlayList *l = p;
	int myRand;
	Repository *theRepository = NULL;

	quicksort(p, &playlistPrev, &playlistNext, &playlistNodeSwap, &playlistSortCompare);

	// Assign random ranks
	while (l != NULL) {
		if (theRepository != l->v->repository) {
			myRand = random();
			theRepository = l->v->repository;
		}
		l->randval = myRand;
		l = l->next;
	}
	quicksort(p, &playlistPrev, &playlistNext, &playlistNodeSwap, &playlistRandomizeCompare);
}

void randomizePlaylist (PlayList *p) {
	PlayList *l = p;

	while (l != NULL) {
		l->randval = random();
		l = l->next;
	}

	quicksort(p, &playlistPrev, &playlistNext, &playlistNodeSwap, &playlistRandomizeCompare);
}

