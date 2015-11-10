#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <stdio.h>

typedef struct PlayList {
	ListNode *v;
	long int randval;
	struct PlayList *next;
	struct PlayList *prev;
} PlayList;

PlayList *getTail (PlayList *p);
PlayList *getMembers (Repository *base, ListNode *allfiles);
PlayList *getRecursive (Repository *base, ListNode *allfiles);
char startPlayer(PlayList *p, PlayerStatus *status);
void recursiveAdd (PlayList **playlist, Repository **curdir, ListNode **allfiles);
void clearPlaylist (PlayList **playlist, PlayerStatus *status);
void choosePreviousRepository (PlayList **playlist, PlayerStatus *status);
void chooseNextRepository(PlayList **playlist, PlayerStatus *status);
void chooseNextSong(PlayList **playlist, PlayerStatus *status);
void choosePreviousSong (PlayList **playlist, PlayerStatus *status);
void pausePlaying(PlayerStatus *status);
void randomizePlaylistByAlbum (PlayList *p);
void randomizePlaylist (PlayList *p);
void sortPlaylist (PlayList *p);

#endif
