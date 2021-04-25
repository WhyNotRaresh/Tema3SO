/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "exec_parser.h"

static void handler(int sig_no)
{
	fprintf(stderr, "I CANT BREATH!");
}

static so_exec_t *exec;

int so_init_loader(void)
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handler;
	sigaction(SIGSEGV, &sa, NULL);

	return -1;
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return -1;
}
