#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/wait.h>

static void die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(1);
}

static int open_ptys(int *master, int *slave) {
	*master = posix_openpt(O_RDWR);
	if (*master < 0) {
		die("Unable to open master pty: %s", strerror(errno));
	}
	if (grantpt(*master) < 0) {
		die("Unable to grant pty permissions: %s", strerror(errno));
	}
	if (unlockpt(*master) < 0) {
		die("Unable to unlock slave pty: %s", strerror(errno));
	}

	const char *ptspath = ptsname(*master);
	if (!ptspath) {
		die("Unable to get slave pty name: %s", strerror(errno));
	}
	*slave = open(ptspath, O_RDWR);
	if (*slave < 0) {
		die("Unable to open slave pty: %s", strerror(errno));
	}
	return 0;
}

static int transfer_all_data(int fdfrom, int fdto) {
	char buf[512];
	for (;;) {
		ssize_t n = read(fdfrom, buf, sizeof(buf));
		if (n < 0 && errno == EINTR) {
			// read interrupted by signal, restart
			continue;
		}
		if (n < 0 && errno == EIO) {
			// linux returns EIO on pty EOF
			break;
		}
		if (n < 0) {
			die("Unexpected error from read: %s\n", strerror(errno));
		}
		if (n == 0) {
			// EOF from child
			break;
		}
		int start = 0;
		while (start < n) {
			ssize_t writ = write(fdto, start+buf, n-start);
			if (writ < 0 && errno == EINTR) {
				// write interrupted by signal, restart
				continue;
			}
			if (writ < 0) {
				die("Unexpected error from write: %s\n", strerror(errno));
			}
			start += writ;
		}
	}
	return 0;
}

// other_program -stdout-> pty_master -> our_program -> pty_slave -orig_stdout->

int main(int argc, char **argv)
{
	if (argc < 2 || argv[1][0] == '-') {
		die("Usage: %s cmd [args]", argv[0]);
	}

	int pty_master, pty_slave;
	if (open_ptys(&pty_master, &pty_slave) < 0) {
		die("Unable to open pseudo-ttys: %s", strerror(errno));
	}

	pid_t child = fork();
	if (child < 0) {
		die("Unable to fork child: %s", strerror(errno));
	}
	if (child == 0) {
		// child process
		close(pty_slave);

		// change stdout to pty master
		dup2(pty_master, STDOUT_FILENO);
		close(pty_master);

		// exec the remaining args as the program
		char **nargv = malloc(sizeof(char *) * argc);
		size_t i;
		for (i = 1; i < argc; i++) {
			nargv[i-1] = argv[i];
		}
		nargv[i-1] = NULL;

		execvp(argv[1], nargv);
		exit(1);
	}
	if (child > 0) {
		// parent process
		close(pty_master);

		// read slave pty and write to original stdout
		if (transfer_all_data(pty_slave, STDOUT_FILENO) < 0) {
			die("Unable to transfer data: %s\n", strerror(errno));
		}
		close(pty_slave);

		int status = 0;
		wait(&status);
		if (WIFEXITED(status)) {
			exit(WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status)) {
			die("Child process died with signal %d", WTERMSIG(status));
		}
		die("Unexpected status %d\n", status);
	}
}
