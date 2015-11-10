#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "repository.h"
#include "keypad.h"
#include "remote.h"
#include "playlist.h"

int main(int argc, char **argv, char **envp) {
	ListNode *allfiles;
	PlayList *playlist = NULL;
	Repository *curdir;
	PlayerStatus status;
	char *repositoryDir;

	int volume = 40;
	const int increment = 5;

	if (argc != 2) {
		fprintf (stderr, "Incorrect usage! Please specify repository dir.\n");
		return 0;
	}
	repositoryDir = argv[1];

	fprintf (stderr, "Initializing keypad... ");
	if (!initKeypad()) {
		fprintf (stderr, "Error initializing keypad!\n");
		return(-1);
	}
	fprintf (stderr, "Done.\n");

	fprintf (stderr, "Reading repository... ");
	allfiles = readDir (curdir = blankRepository(repositoryDir));
	fprintf (stderr, "Done.\n");

	srandom (time(NULL));

	fprintf (stderr, "Adding songs to playlist... ");
	recursiveAdd(&playlist, &curdir, &allfiles);
	fprintf (stderr, "Done.\n");

	fprintf (stderr, "Randomizing songs by album... ");
	randomizePlaylistByAlbum(playlist);
	fprintf (stderr, "Done.\n");

	fprintf (stderr, "Launching mpg123... ");
	mpg123Launch(&status);
	fprintf (stderr, "Done.\n");

	fprintf (stderr, "Starting player... ");
	startPlayer(playlist, &status);
	fprintf (stderr, "Done.\n");

	fprintf (stderr, "\nInitialization complete.\n\n");
	setLight(1);

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
			case BUTTON_PERIOD:
				setLight(0);
				execl("/usr/bin/shutdown.sh", "shutdown.sh", NULL); break;
				break;
			case BUTTON_ZERO:
				setLight(0);
				sortPlaylist (playlist);
				setLight(1);
				break;
			case BUTTON_ONE:
				setLight(0);
				randomizePlaylistByAlbum (playlist);
				setLight(1);
				break;
			case BUTTON_THREE:
				setLight(0);
				randomizePlaylist (playlist);
				setLight(1);
				break;
			case BUTTON_PLUS:
				mpg123Decree(&status, playlist->v->name);
				break;
			case BUTTON_MINUS:
				// BLEE! Volume controls go here.
				fprintf (stderr, "Minus!\n");
				break;
			case BUTTON_STAR:
			case BUTTON_SLASH: {
				pid_t childpid;
				int status;
				char text_volume[4];

				if (downs == BUTTON_SLASH) volume -= increment;
				else volume += increment;

				if (volume > 100) volume = 100;
				if (volume < 0) volume = 0;
				fprintf (stderr, "Setting volume to %i\n", volume);

				sprintf (text_volume, "%i", volume);
				if ((childpid = fork())==0) {
                                        execl("/usr/bin/setmixer", "setmixer", "vol", text_volume, NULL);
                                        _exit(0);
                                }
                                waitpid (childpid, &status, 0);
				} break;
			case BUTTON_NUMLOCK:
				fprintf (stderr, "Numlock!\n");
				break;
		}
		usleep (10000);
	}
	// There's no real way of getting here. I salute you if you manage, and
	// feel free to release any stored resources if you wish.
	return 1;
}

