#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "repository.h"

Repository *makeRepositoryNode (Repository *parent, Repository *next, Repository *children, char const *name) {
	Repository *newNode = malloc(sizeof(Repository));
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

ListNode *makeListNode (ListNode *prev, Repository *repository, const char *name, ListNode *next) {
	ListNode *newNode = malloc(sizeof(ListNode));
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
	char *buf = malloc(strlen(r->name)+1);
	strcpy (buf, r->name);
	while (r->parent != NULL) {
		r = r->parent;
		buf = realloc (buf, strlen(buf) + strlen(r->name) + 2);
		memmove (buf + strlen(r->name) + 1, buf, strlen(buf)+1);
		memmove (buf, r->name, strlen(r->name));
		buf[strlen(r->name)] = '/';
	}
	printf ("Canonical name of %s: %s\n", rep->name, buf);
	return buf;
}

ListNode *readDir (Repository *this) {
	ListNode *returner = NULL;
	ListNode *tail = NULL;
	struct dirent *entry = NULL;
	char *dirname = getRepositoryCanonicalName(this);
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
				returner = makeListNode (NULL, this, entry->d_name, NULL);
				tail = returner;
				if (returner == NULL) {
					closedir (dir);
					return NULL;
				}
			}
			else {
				tail = makeListNode (tail, this, entry->d_name, NULL);
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
				Repository *r = makeRepositoryNode (this, NULL, NULL, entry->d_name);
				if (curchild == NULL) {
					curchild = r;
					this->children = r;
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

char *getCanonicalRepositoryName(Repository *who) {
	char *buf = NULL;
	Repository *r = who;
	while (r != NULL) {
		int l = (buf==NULL?0:strlen(buf));
		buf = realloc (buf, l+2+strlen(r->name));
		memmove (buf+1+strlen(r->name), buf, l+1);
		memmove (buf, r->name, strlen(r->name));
		buf[strlen(r->name)] = '/';
		r=r->parent;
	}
	return buf;
}

char *getCanonicalName(ListNode *who) {
	char *buf = malloc(strlen(who->name) + 1);
	Repository *r = who->repository;
	strcpy (buf, who->name);
	while (r != NULL) {
		buf = realloc (buf, strlen(buf)+2+strlen(r->name));
		memmove (buf+1+strlen(r->name), buf, strlen(buf)+1);
		memmove (buf, r->name, strlen(r->name));
		buf[strlen(r->name)] = '/';
		r=r->parent;
	}
	return buf;
}

void displayList (ListNode *who) {
	if (who == NULL) return;
	printf ("Entry name: \"%s\"\n", who->name);
	displayList (who->next);
}
