/*
*   Copyright (c) 2018, Masatake YAMATO
*   Copyright (c) 2018, Red Hat, Inc.
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License version 2 or (at your option) any later version.
*
*   Managing parallel execution
*/

/*
*   INCLUDE FILES
*/
#include "general.h"  /* must always come first */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>

#include "routines.h"
#include "vstring.h"

struct worker
{
#define MANAGER 0
#define WORKER  1
	pid_t pid;
	int sv[2];
};

struct consumer
{
	pid_t pid;
	struct worker *workers;
	FILE **producers;
};

struct PManager
{
	int count;
	int current_worker;
	vString *fnbuf;
	struct consumer consumer;
	struct worker* workers;
};

static void drain_workers (int count, struct consumer *consumer)
{
	int max_fd = 0;

	consumer->producers = eCalloc (count + 1, sizeof (FILE *));
	for (int i = 0; i < count; i++)
	{
		FILE *fp = fdopen (consumer->workers[i].sv[MANAGER], "r");
		if (!fp)
			error (FATAL | PERROR, "cannot make FILE object for producer: %d",
				   consumer->workers[i].sv[MANAGER]);
		consumer->producers[i] = fp;
	}

	for (int i = 0; i < count; i++)
	{
		if (max_fd < consumer->workers[i].sv[MANAGER])
			max_fd = consumer->workers[i].sv[MANAGER];
	}


	fd_set rfds;
	vString *line;
	int n_active_worker;

	line = vStringNew();

 next:
	n_active_worker = 0;
	FD_ZERO(&rfds);
	for (int i = 0; i < count; i++)
	{
		if (consumer->workers[i].sv[MANAGER] != -1)
		{
			FD_SET(consumer->workers[i].sv[MANAGER], &rfds);
			n_active_worker++;
		}
	}
	if (n_active_worker == 0)
		goto out;

	int r = select(max_fd + 1, &rfds, NULL, NULL, NULL);
	if (r < 0)
			error (FATAL | PERROR, "select producers");
	else if (r)
	{
		for (int i = 0; i < count; i++)
		{
			if (FD_ISSET(consumer->workers[i].sv[MANAGER], &rfds))
			{
				if (feof (consumer->producers[i]))
				{
				eof:
					shutdown (consumer->workers[i].sv[MANAGER], SHUT_RD);
					fclose(consumer->producers[i]);
					consumer->workers[i].sv[MANAGER] = -1;
				}
				else
				{
					vStringClear(line);
					char *str = vStringValue(line);
					size_t size = vStringSize(line);

					for (;;)
					{
						bool newLine;
						bool eof = false;

						if (fgets (str, size, consumer->producers[i]) == NULL)
						{
							eof = feof (consumer->producers[i]);
							if (!eof)
								error (FATAL | PERROR, "reading line form producers");
						}
						vStringSetLength (line);
						newLine = vStringLength (line) > 0 && vStringLast (line) == '\n';
						if (newLine)
						{
							if (vStringLength (line) > 0)
							{
								str = vStringValue(line);
								fputs(str, stdout);
								size = vStringSize(line);
							}
							if (eof)
								goto out;
							break;
						}
						else if (eof)
							goto eof;
						else
						{
							vStringResize(line, vStringLength(line) * 2);
							str = vStringValue(line) + vStringLength (line);
							size = vStringSize(line) - vStringLength (line);
							if (eof)
								goto eof;
						}
					}
				}
			}
		}
		goto next;
	}

 out:
	vStringDelete(line);
	eFree (consumer->producers);
	fclose (stdout);
	_exit (0);
}

struct PManager* pmanager_new (int count, int argc, const char** argv)
{
	struct PManager *pmanager = eMalloc(sizeof (struct PManager));

	pmanager->count = count;
	pmanager->workers = eCalloc (count, sizeof (struct worker));
	pmanager->current_worker = 0;
	pmanager->fnbuf = NULL;;

	for (int i = 0; i < pmanager->count; i++)
	{
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, pmanager->workers[i].sv) < 0)
			error (FATAL | PERROR, "cannot make sockerpair for communicating with worker");
		pmanager->workers[i].pid = fork ();
		if (pmanager->workers[i].pid < 0)
			error (FATAL | PERROR, "cannot make a process for worker");
		else if (pmanager->workers[i].pid > 0)
			close (pmanager->workers[i].sv[WORKER]);
		else
		{
			char *temp_argv[] = {"ctags-worker", "--sort=no", "-L", "-", "-o", "-", NULL};

			close (pmanager->workers[i].sv[MANAGER]);

			close (0);
			if (dup2 (pmanager->workers[i].sv[WORKER], 0) < 0)
				error (FATAL | PERROR,
					   "cannot duplicate file description for communicating with manager");
			close (1);
			if (dup2 (pmanager->workers[i].sv[WORKER], 1) < 0)
				error (FATAL | PERROR,
					   "cannot duplicate file description for communicating with manager");

			extern char *program_invocation_name;
			extern char **environ;
			if (execve (program_invocation_name, temp_argv, environ) < 0)
				error (FATAL | PERROR,
					   "cannot exec ctags in worker process");
		}
	}



	pmanager->consumer.workers = pmanager->workers;
	pmanager->consumer.pid = fork ();
	if (pmanager->consumer.pid < 0)
		error (FATAL | PERROR, "cannot make a process for consuming the output of workers");
	else if (pmanager->consumer.pid == 0)
		drain_workers(pmanager->count, &pmanager->consumer);
	else
		pmanager->fnbuf = vStringNew ();

	return pmanager;
}

int pmanager_dispatch(struct PManager *pmanager, const char *file)
{
	vStringCopyS(pmanager->fnbuf, file);
	vStringPut(pmanager->fnbuf, '\n');
	if (write (pmanager->workers[(pmanager->current_worker++)%pmanager->count].sv[MANAGER],
			   vStringValue(pmanager->fnbuf), vStringLength(pmanager->fnbuf)) < 0)
		error (FATAL | PERROR, "cannot pass the input file name to worker");
	return 0;
}

int pmanager_delete  (struct PManager *pmanager)
{

	for (int i = 0; i < pmanager->count; i++)
		shutdown (pmanager->workers[i].sv[MANAGER], SHUT_WR);

	for (int i = 0; i < pmanager->count; i++)
		close (pmanager->workers[i].sv[MANAGER]);

	for (int i = 0; i < pmanager->count; i++)
		waitpid (pmanager->workers[i].pid, NULL, 0);

	waitpid (pmanager->consumer.pid, NULL, 0);

	vStringDelete (pmanager->fnbuf);
	eFree (pmanager->workers);
	eFree (pmanager);

	return 0;
}
