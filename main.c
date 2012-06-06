/**
 * Copyright (c) 2012, Felipe Pena
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "cmd.h"

#define PX_VERSION "0.1.0"

/**
 * Displays the usage of px
 */
static void usage() {
	printf("Usage: px [options]\n\n"
			"Options:\n"
			"	-h, --help	Displays this information\n");
}

int main(int argc, char **argv) {
	char c;
	int opt_index = 0;
	static struct option long_opts[] = {
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	printf("px - Process Examinator (" PX_VERSION ")\n");

	if (argc > 1) {
		while ((c = getopt_long(argc, argv, "h", long_opts, &opt_index)) != -1) {
			switch (c) {
				case 'h':
					usage();
					exit(0);
				case '?':
					break;
			}
		}
	}

	prompt_cmd();

	return 0;
}
