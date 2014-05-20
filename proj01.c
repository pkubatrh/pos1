/*  POS project 1: signal synchronization
 *  Author: xkubat11@stud.fit.vutbr.cz
 *  Description:
 *      The appliaction forks a child and output letters of the alphabet
 *      using signal synchronization.
 *      When one of the processes recieves the SIGUSR2 signal
 *      the output starts again from the letter 'A'
 *
 */

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1 /* XPG 4.2 - needed for usleep() */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int intr;
char counter = 'A';
pid_t chldPid = 0;

/* signal handler, parent version */
void parCatcher(int sig)
{
    if (sig == SIGUSR2)
        counter = 'A';
    else if (sig == SIGINT) {
		kill(chldPid, SIGINT);
		exit(0);
	}
    else
    /* Flag */
    	intr = 1;
}

/* signal handler, child version */
void chldCatcher(int sig)
{
    if (sig == SIGUSR2)
        counter = 'A';
    else
    /* Flag */
    	intr = 1;
}

int main() {
    struct sigaction sigact;
	sigset_t setint;

    	pid_t parPid;
    	pid_t retval;
	char c;
	sigemptyset(&setint);
	sigaddset(&setint, SIGUSR1);
	sigaddset(&setint, SIGUSR2);

	sigprocmask(SIG_BLOCK, &setint, NULL);
	parPid = getpid();
	retval = fork();
	if (retval == 0) {
        /* I am the child */
        /* Signal init */

	    sigact.sa_handler = chldCatcher;
		sigemptyset(&sigact.sa_mask);
		sigact.sa_flags = 0;
		if (sigaction(SIGUSR1, &sigact, NULL) == -1) {
			perror("sigaction()");
			return(1);
		}
		if (sigaction(SIGUSR2, &sigact, NULL) == -1) {
			perror("sigaction()");
			return(1);
		}
		sigprocmask(SIG_UNBLOCK, &setint, NULL);

        /* Child implementation */
        chldPid = getpid();
	while(1){
	    while(intr == 0 && sigsuspend(&sigact.sa_mask) == -1);
            printf("Child (%d) : \'%c\'\n", chldPid, counter);
            kill(parPid, SIGUSR1);
	    if (counter == 'Z')
                counter = 'A';
            else
                counter++;
            intr = 0;
        }
    }
    else {
        /* Im the parent */

        /* Signal init */
	    sigact.sa_handler = parCatcher;
		sigemptyset(&sigact.sa_mask);
		sigact.sa_flags = 0;
		if (sigaction(SIGUSR1, &sigact, NULL) == -1) {
			perror("sigaction()");
			return(1);
		}
		if (sigaction(SIGUSR2, &sigact, NULL) == -1) {
			perror("sigaction()");
			return(1);
		}
		if (sigaction(SIGINT, &sigact, NULL) == -1) {
			perror("sigaction()");
			return(1);
		}
		sigprocmask(SIG_UNBLOCK, &setint, NULL);

        /* Parent implementation */
        while(1) {
            printf("Parent (%d) : \'%c\'\n", parPid, counter);
            kill(retval, SIGUSR1);
            while(intr == 0 && sigsuspend(&sigact.sa_mask) == -1);
            printf("Press enter...");
            if (counter == 'Z')
                counter = 'A';
            else
                counter++;

            while((c = getchar()) != '\n');

            intr = 0;
        }
    }
	return 0;
}
