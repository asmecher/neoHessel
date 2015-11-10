#ifndef SPEECH_H
#define SPEECH_H

#include <stdio.h>

#include "player.h"

// Does speech stuff.

typedef struct SpeechAtom {
	struct SpeechAtom *next;
	char *atom;
	char *filename;
} SpeechAtom;

SpeechAtom *rootSpeechNode = NULL;

void initSpeech() {
	const char speechdir[] = "/var/sounds/various";
	DIR *dir = opendir(speechdir);
	struct dirent *entry;

	while (1) {
		entry = readdir(dir);
		if (entry == NULL) break;
		if (entry == (struct dirent *) EOF) {
			showError ("initSpeech returned error on dir.");
			break;
		}
		if (strlen(entry->d_name) >= 4 && strcasecmp (entry->d_name + strlen(entry->d_name) - 4, ".mp3")==0) {
			SpeechAtom *newnode = (SpeechAtom *) malloc(sizeof(SpeechAtom));
			newnode->filename = (char *) malloc(2 + strlen(speechdir) + strlen(entry->d_name));
			sprintf (newnode->filename, "%s/%s", speechdir, entry->d_name);
			newnode->atom = (char *) malloc(strlen(entry->d_name)-3);
			strncpy (newnode->atom, entry->d_name, strlen(entry->d_name)-4);
			newnode->atom[strlen(entry->d_name)-4] = 0;
			newnode->next = NULL;

			if (rootSpeechNode == NULL) rootSpeechNode = newnode;
			else {
				// Do in-order insert.
				SpeechAtom *curpos = rootSpeechNode;
				SpeechAtom *parentNode = NULL;
				while (1) {
					int minsize;
					if (curpos->next == NULL) {
						curpos->next = newnode;
						break;
					}
					if (strlen(curpos->atom) < strlen(newnode->atom)) minsize = strlen(curpos->atom);
					else minsize = strlen(newnode->atom);

					if (strncasecmp(curpos->atom, newnode->atom, minsize) >= 0) {
						newnode->next = curpos;
						if (parentNode == NULL) rootSpeechNode = newnode;
						else parentNode->next = newnode;
						break;
					}
					parentNode = curpos;
					curpos = curpos->next;
				}
				// tail->next = newnode;
			}
			printf ("Added atom %s (%s)...\n", newnode->atom, newnode->filename);
		}
	}
	closedir(dir);
}

void say (char *what) {
	printf ("Speech disabled until mixing support is in.\n");
	/* int looperpid = fork();
	printf ("Called say on %s\n", what);
	if (looperpid == 0) {
		char *looper = what;
		int size = sizeof(char *) * (4 + strlen(what));
		int location = 2;

		char **curstrings = malloc(size);

		char *aggressiveParam = "--aggressive";
		char *bufParam = "-b 1024";

		setpriority (PRIO_PROCESS, 0, +20);

		memset (curstrings, 0, size);
		curstrings[0] = bufParam;
		curstrings[1] = aggressiveParam;
		while (*looper != 0) {
			SpeechAtom *node = rootSpeechNode;
			while (node != NULL && strncasecmp(looper, node->atom, strlen(node->atom))!=0) node = node->next;
			if (node != NULL) {
				curstrings[location++] = node->filename;
				if (strlen(looper)<strlen(node->atom)) looper += strlen(looper);
				else looper += strlen(node->atom);
			}
			else looper++;
		}
		execv ("/usr/bin/mpg123", curstrings);
	} */
}
#endif
