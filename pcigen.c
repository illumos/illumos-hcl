/*
 * Given that we've already parsed all the manifests and produced a file, build
 * up a small state machine. This could probably be in some other language, but
 * C will do for now.
 */

#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>

static char *g_pname;

static void
fatal(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
}

int
main(int argc, char *argv)
{
	FILE *f;
	g_pname = basename(argv[0]);

	if (argc != 2) {
		fprintf(stderr, "%s: <input>\n", g_pname);
		return (1);
	}

	f = fopen(argv[1], "rF");
	if (f == NULL) {
		fprintf(stderr, "%s: 
	}

	return (0);
}
