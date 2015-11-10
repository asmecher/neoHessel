#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "repository.h"

Repository *makeRepositoryNode (Repository *parent, Repository *next, Repository *children, char const *name) {
	Repository *newNode = (Repository *) malloc(sizeof(Repository));
	if (newNode == NULL) {
		return NULL;
	}
	newNode->name = (char *) malloc(strlen(name)+1);
	if (newNode->name == NULL) {
		free (newNode);
		return NULL;
	}
	strcpy (newNode->name, name);
	newNode->next = next;
	newNode->children = children;
	newNode->parent = parent;
	return newNode;
}

Repository *blankRepository(const char *repositoryDir) {
	return makeRepositoryNode (NULL, NULL, NULL, repositoryDir);
}

ListNode *makeListNode (ListNode *prev, Repository *repository, const char *name, ListNode *next) {
	ListNode *newNode = (ListNode *)malloc(sizeof(ListNode));
	if (newNode == NULL) {
		return NULL;
	}
	newNode->name = (char *) malloc(strlen(name)+1);
	if (newNode->name == NULL) {
		free (newNode);
		return NULL;
	}
	strcpy (newNode->name, name);
	newNode->next = next;
	newNode->prev = prev;
	newNode->repository = repository;
	if (prev != NULL) prev->next = newNode;
	if (next != NULL) next->prev = newNode;
	return newNode;
}

void appendEntry (ListNode *list, ListNode *newguy) {
	ListNode *seeker = list;
	if (newguy == NULL) return;
	if (list == NULL) {
		return;
	}
	if (newguy->prev != NULL) {
		return;
	}
	while (seeker->next != NULL) seeker=seeker->next;
	seeker->next = newguy;
	newguy->prev = seeker;
}

void removeEntry (ListNode *who) {
	if (who == NULL) {
		return;
	}
	if (who->prev != NULL) who->prev->next = who->next;
	if (who->next != NULL) who->next->prev = who->prev;
	free (who->name);
	free (who);
}

char *getRepositoryCanonicalName(Repository *rep) {
	Repository *r = rep;
	char *buf = (char *) malloc(strlen(r->name)+1);
	strcpy (buf, r->name);
	while (r->parent != NULL) {
		r = r->parent;
		buf = (char *) realloc (buf, strlen(buf) + strlen(r->name) + 2);
		memmove (buf + strlen(r->name) + 1, buf, strlen(buf)+1);
		memmove (buf, r->name, strlen(r->name));
		buf[strlen(r->name)] = '/';
	}
	return buf;
}

ListNode *readDir (Repository *theRepository) {
	ListNode *returner = NULL;
	ListNode *tail = NULL;
	struct dirent *entry = NULL;
	char *dirname = getRepositoryCanonicalName(theRepository);
	DIR *dir = opendir (dirname);
	Repository *curchild = NULL;
	free (dirname);
	if (dir == NULL) return NULL;
	while (1) {
		entry = readdir (dir);
		if (entry == NULL) break;
		if (entry == (struct dirent *) EOF) {
			return NULL;
		}
		if (strlen(entry->d_name) >= 4 && strcasecmp (entry->d_name + strlen(entry->d_name) - 4, ".mp3")==0) {
			if (returner == NULL) {
				returner = makeListNode (NULL, theRepository, entry->d_name, NULL);
				tail = returner;
				if (returner == NULL) {
					closedir (dir);
					return NULL;
				}
			}
			else {
				tail = makeListNode (tail, theRepository, entry->d_name, NULL);
				if (tail == NULL) {
					closedir (dir);
					return NULL;
				}
			}
		}
		else {
			// Maybe it's a directory...
			if (strcmp(entry->d_name, "..")!=0 && strcmp(entry->d_name, ".")!=0) {
				ListNode *l;
				Repository *r = makeRepositoryNode (theRepository, NULL, NULL, entry->d_name);
				if (curchild == NULL) {
					curchild = r;
					theRepository->children = r;
				}
				else {
					curchild->next = r;
					curchild = r;
				}
				l = readDir (r);
				if (returner != NULL) appendEntry (returner, l);
				else returner = l;
			}
		}
	}
	closedir (dir);
	return returner;
}

void destroyNodesAfterAndIncluding (ListNode *who) {
	if (who == NULL) return;
	destroyNodesAfterAndIncluding (who->next);
	free (who->name);
	free (who);
}

char *getCanonicalName(ListNode *who) {
	char *buf = getRepositoryCanonicalName(who->repository);
	buf = realloc(buf, strlen(buf) + 1 + strlen(who->name) + 1);
	strcat (buf, "/");
	strcat (buf, who->name);
	return buf;
}

void displayList (ListNode *who) {
	if (who == NULL) return;
	printf ("Entry name: \"%s\"\n", who->name);
	displayList (who->next);
}
