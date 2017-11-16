#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


void sig_fn(int);


int main(void)
{
	struct sigaction new_sa;
	struct sigaction old_sa;
	sigfillset(&new_sa.sa_mask);
	new_sa.sa_handler = SIG_IGN;
	new_sa.sa_flags = 0;

	if(sigaction(SIGINT, &new_sa, &old_sa)==0 && old_sa.sa_handler != SIG_IGN)
	{
		new_sa.sa_handler = sig_fn;
		sigaction(SIGINT, &new_sa, 0);
	}
	while(1)
		pause();
}

void sig_fn(int sig)
{
	fprintf(stderr, "\nSignal Number: %d\n Ctrl-C is pressed. Try Again", sig);
}
