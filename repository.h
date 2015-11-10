#ifndef REPOSITORY_H
#define REPOSITORY_H

typedef struct Repository {
	char *name;
	struct Repository *children;
	struct Repository *parent;
	struct Repository *next;
} Repository;

typedef struct ListNode {
	struct ListNode *prev;
	struct ListNode *next;
	struct Repository *repository;
	char *name;
} ListNode;

// Repository *makeRepositoryNode (Repository *parent, Repository *next, Repository *children, char const *name);
// ListNode *makeListNode (ListNode *prev, Repository *repository, const char *name, ListNode *next);
// void appendEntry (ListNode *list, ListNode *newguy);
// void removeEntry (ListNode *who);
// char *getRepositoryCanonicalName(Repository *rep);
ListNode *readDir (Repository *theRepository);
void destroyNodesAfterAndIncluding (ListNode *who);
char *getCanonicalName(ListNode *who);
void displayList (ListNode *who);
Repository *blankRepository(const char *repositoryDir);

#endif

