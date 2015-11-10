#include "remote.h"

#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

inline char setState (PlayerStatus *who, int state, char status) {
	char was_set = who->state & state;
	if ((was_set && !status) || (!was_set && status)) {
		who->state ^= state;
		return !status;
	}
	return status;
}

inline char getState (PlayerStatus *who, int state) {
	return (who->state & state);
}

inline char hasFinished(PlayerStatus *player) {
	return (!getState(player, STATE_PLAYING));
}

void mpg123Close(PlayerStatus *who) {
	fprintf (who->in, "QUIT\n");
	waitpid (who->pid, NULL, 0);
	fclose (who->in);
	fclose (who->out);
	who->state = 0;
	fflush (who->in);
}

void mpg123ProcessStatus(PlayerStatus *who) {
	char statusbuffer[1000];
	while (!feof(who->out) && fgets (statusbuffer, sizeof(statusbuffer), who->out) > 0) {
		char *thisOne;
		int tokencount = 0;
		int tokenmode = 0;
		for (thisOne = strtok(statusbuffer, " "); thisOne != NULL; thisOne = strtok(NULL, " ")) {
			if (tokencount == 0) switch (thisOne[1]) {
				case 'R':
					fprintf (stderr, "(mpg123 Hello)\n");
					// who->state = 0;
					break;
				case 'I':
					fprintf (stderr, "(mpg123 file info)\n");
					break;
				case 'S':
					fprintf (stderr, "(mpg123 stream info)\n");
					break;
				case 'F':
					// fprintf (stderr, ".");
					if (!(who->state & STATE_PLAYING)) who->state = who->state & STATE_PLAYING;
					break;
				case 'P':
					fprintf (stderr, "(mpg123 play status)\n");
					thisOne = strtok(NULL, " ");
					switch (atoi(thisOne)) {
						case 1:
							setState (who, STATE_PAUSED, 1);
						case 2:
							setState (who, STATE_PLAYING, 1);
							break;
						default:
							setState (who, STATE_PAUSED, 0);
							setState (who, STATE_PLAYING, 0);
					}
					break;
				case 'E':
					fprintf (stderr, "(mpg123 error)\n");
					break;
			}
			tokencount++;
		}
	}
}

void mpg123Launch (PlayerStatus *status) {
	int stdin_pipe[2];
	int stdout_pipe[2];

	if (pipe (stdin_pipe) < 0 || pipe (stdout_pipe) < 0) {
		exit(-1);
	}

	switch ((status->pid = fork())) {
		case 0:
			dup2 (stdin_pipe[0], 0);
			dup2 (stdout_pipe[1], 1);

			// Actually launch the player.
			setpriority (PRIO_PROCESS, 0, -20);
			// execl ("/usr/bin/mpg123", "-b 1024", "-w", "/usr/bin/spinup.sh", "--aggressive", "-i", "-R", "token-filename-goes-here", NULL);
			execl ("/usr/bin/mpg123", "mpg123", "--aggressive", "-R", "token-filename-goes-here", NULL);
			break;
		default:
			fcntl (stdout_pipe[0], F_SETFL, O_NONBLOCK);
			status->out = fdopen(stdout_pipe[0], "r");
			status->in = fdopen(stdin_pipe[1], "w");
			status->state = 0;
			break;
	}
}

void mpg123Play (PlayerStatus *status, ListNode *who) {
        char *theName;
	theName = getCanonicalName(who);
	fprintf (stderr, "Loading %s\n", theName);
        fprintf (status->in, "LOAD %s\n", theName);
	fflush (status->in);
        free (theName);
	setState (status, STATE_PLAYING, 1);
}

char mpg123Pause(PlayerStatus *status) {
        if (getState(status, STATE_PLAYING)) {
                fprintf (status->in, "PAUSE\n");
		fflush (status->in);
                return 1;
        }
        return 0;
}

void mpg123Stop(PlayerStatus *status) {
        if (getState(status, STATE_PLAYING)) {
                fprintf(status->in, "STOP\n");
                fflush(status->in);
        }
}

void mpg123Decree(PlayerStatus *status, const char *statement) {
        fprintf (status->in, "DECREE %s\n", statement);
        fflush (status->in);
}

void mpg123Forward(PlayerStatus *status) {
        fprintf (status->in, "JUMP +50\n");
        fflush (status->in);
}

void mpg123Back(PlayerStatus *status) {
        fprintf (status->in, "JUMP -50\n");
        fflush (status->in);
}

