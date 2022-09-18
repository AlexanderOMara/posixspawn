/* 
 * Copyright (c) 2015-2022 Alexander O'Mara
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <spawn.h>

// Define Apple's private constants (from bsd/sys/spawn.h).
#ifndef POSIX_SPAWN_OSX_TALAPP_START
	#define POSIX_SPAWN_OSX_TALAPP_START 0x0400
#endif
#ifndef POSIX_SPAWN_OSX_WIDGET_START
	#define POSIX_SPAWN_OSX_WIDGET_START 0x0800
#endif
#ifndef POSIX_SPAWN_OSX_DBCLIENT_START
	#define POSIX_SPAWN_OSX_DBCLIENT_START 0x0800
#endif
#ifndef POSIX_SPAWN_OSX_RESVAPP_START
	#define POSIX_SPAWN_OSX_RESVAPP_START 0x1000
#endif
#ifndef _POSIX_SPAWN_DISABLE_ASLR
	#define _POSIX_SPAWN_DISABLE_ASLR 0x0100
#endif
#ifndef _POSIX_SPAWN_ALLOW_DATA_EXEC
	#define _POSIX_SPAWN_ALLOW_DATA_EXEC 0x2000
#endif

extern char **environ;

struct constant {
	char *name;
	short flag;
};

struct argdata {
	short flags;
	char *path;
	bool wait;
	char **args;
};

struct constant poxis_spawn_items[] = {
	{ .name = "POSIX_SPAWN_RESETIDS"           , .flag = POSIX_SPAWN_RESETIDS           },
	{ .name = "POSIX_SPAWN_SETPGROUP"          , .flag = POSIX_SPAWN_SETPGROUP          },
	{ .name = "POSIX_SPAWN_SETSIGDEF"          , .flag = POSIX_SPAWN_SETSIGDEF          },
	{ .name = "POSIX_SPAWN_SETSIGMASK"         , .flag = POSIX_SPAWN_SETSIGMASK         },
	{ .name = "POSIX_SPAWN_SETEXEC"            , .flag = POSIX_SPAWN_SETEXEC            },
	{ .name = "POSIX_SPAWN_START_SUSPENDED"    , .flag = POSIX_SPAWN_START_SUSPENDED    },
	{ .name = "POSIX_SPAWN_CLOEXEC_DEFAULT"    , .flag = POSIX_SPAWN_CLOEXEC_DEFAULT    },
	// Private constants.
	{ .name = "POSIX_SPAWN_OSX_TALAPP_START"   , .flag = POSIX_SPAWN_OSX_TALAPP_START   },
	{ .name = "POSIX_SPAWN_OSX_WIDGET_START"   , .flag = POSIX_SPAWN_OSX_WIDGET_START   },
	{ .name = "POSIX_SPAWN_OSX_DBCLIENT_START" , .flag = POSIX_SPAWN_OSX_DBCLIENT_START },
	{ .name = "POSIX_SPAWN_OSX_RESVAPP_START"  , .flag = POSIX_SPAWN_OSX_RESVAPP_START  },
	{ .name = "_POSIX_SPAWN_DISABLE_ASLR"      , .flag = _POSIX_SPAWN_DISABLE_ASLR      },
	{ .name = "_POSIX_SPAWN_ALLOW_DATA_EXEC"   , .flag = _POSIX_SPAWN_ALLOW_DATA_EXEC   }
};

void usage() {
	printf(
		"posixspawn -- The power of posix_spawn in your shell.\n"
		"Version 1.0.0\n"
		"Copyright (c) 2015-2022 Alexander O'Mara\n"
		"Licensed under MPL 2.0 <http://mozilla.org/MPL/2.0/>\n"
		"\n"
		"Usage: posixspawn [options...] [--] [args...]\n"
		"\n"
		"options:\n"
		"  -f <flags> Pipe-delimited list of flags (see flags section below).\n"
		"  -p <path>  Set the executable path separate from arg 0.\n"
		"  -w         Wait for child process to complete before returning?\n"
		"\n"
		"args:\n"
		"  The remaining arguments are passed to the child process.\n"
		"\n"
		"flags:\n"
		"  The flags arguments is a pipe-delimited list of constants or integers.\n"
		"  A flag can be a string constant, a base-16 string, or a base-10 string.\n"
		"  The flag uses the short data type, with each flag a maximum 2 bytes.\n"
		"  Example argument:\n"
		"    \"EXAMPLE_CONSTANT|0xF0|16\"\n"
		"  The following string constants are supported:\n"
	);
	size_t constant_length = sizeof(poxis_spawn_items) / sizeof(struct constant);
	for (size_t i = 0; i < constant_length; i++) {
		printf("    0x%04x  %s\n", poxis_spawn_items[i].flag, poxis_spawn_items[i].name);
	}
	printf("\n");
}

short parse_constant(char *s, struct constant *c, size_t constant_size) {
	size_t constant_length = constant_size / sizeof(struct constant);
	size_t s_s = 0;
	size_t s_e = 0;
	short r = 0;
	size_t s_length = strlen(s);
	for (size_t i = 0; i < s_length; i++) {
		bool last = i == s_length - 1;
		if (last || s[i] == '|') {
			// Set the endpoint for the current segment.
			s_e = last ? i + 1 : i;
			// Calculate the length of this segment.
			size_t s_l = s_e - s_s;
			// If this segment has length, look for a match.
			if (s_l) {
				short flag = 0;
				// Loop through the constants looking for a match.
				for (size_t j = 0; j < constant_length; j++) {
					// If a match, break the loop and set the value.
					if (s_l == strlen(c[j].name) && !strncmp(&s[s_s], c[j].name, s_l)) {
						flag = c[j].flag;
						break;
					}
				}
				// If no flags identified, maybe hex or integer.
				if (!flag) {
					// Maybe hex.
					if (s_l > 2  && s[s_s] == '0' && (s[s_s + 1] == 'x' || s[s_s + 1] == 'X')) {
						flag = (short)strtol(&s[s_s], NULL, 0);
					}
					// If still zero, maye just integer.
					if (!flag) {
						flag = atoi(&s[s_s]);
					}
				}
				// Update the return value with the new bits.
				r |= flag;
			}
			// Update segment start position.
			s_s = i + 1;
		}
	}
	return r;
}

struct argdata parse_args(int argc, char **argv) {
	struct argdata args;
	// Check for the minimum 1 argument.
	if (argc <= 1) {
		usage();
		exit(EXIT_FAILURE);
	}
	// Initialize the return data.
	args.flags = 0;
	args.path = NULL;
	args.wait = FALSE;
	args.args = NULL;
	// Parse arguments.
	for (int opt; (opt = getopt(argc, argv, "f:p:w")) != -1;) {
		switch (opt) {
			case 'f':
				args.flags = parse_constant(optarg, poxis_spawn_items, sizeof(poxis_spawn_items));
				break;
			case 'p':
				args.path = optarg;
				break;
			case 'w':
				args.wait = TRUE;
				break;
			default:
				usage();
				exit(EXIT_FAILURE);
		}
	}
	// Compute remaining arguments, requiring at least one more.
	int args_count = argc - optind;
	if (args_count <= 0 && !args.path) {
		usage();
		exit(EXIT_FAILURE);
	}
	// Initialize memory for the arguments array, plus a null terminator.
	args.args = malloc((args_count + 1) * sizeof(char *));
	// Loop over remaining arguments, inserting them.
	for (int i = optind; i < argc; i++) {
		args.args[i - optind] = argv[i];
	}
	// Add the null terminator.
	args.args[args_count] = NULL;
	return args;
}

int main(int argc, char **argv) {
	struct argdata args = parse_args(argc, argv);
	pid_t pid;
	posix_spawnattr_t attr;
	posix_spawnattr_init(&attr);
	posix_spawnattr_setflags(&attr, args.flags);
	int status = posix_spawn(&pid, args.path ? args.path : args.args[0], NULL, &attr, args.args, environ);
	if (status) {
		printf("ERROR: posix_spawn: %s\n", strerror(status));
		exit(EXIT_FAILURE);
	}
	printf("PID: %i\n", pid);
	if (args.wait) {
		if (waitpid(pid, &status, 0) != -1) {
			printf("EXIT: %i\n", status);
		}
		else {
			perror("ERROR: waitpid");
		}
	}
	return EXIT_SUCCESS;
}
