/*
 * Copyright (c) 2017 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <machine/rtems-bsd-commands.h>

#include <rtems.h>

#include <grisp/init.h>

static char *wpa_supplicant_cmd[] = {
	"wpa_supplicant",
	"-Dbsd",
	"-iwlan0",
	"-c",
	NULL, /* Will be replaced with path to config. */
	NULL
};

static void
wpa_supplicant_watcher_task(rtems_task_argument arg)
{
	int argc;
	char ** argv;
	int err;
	(void) arg;

	argv = wpa_supplicant_cmd;
	argc = sizeof(wpa_supplicant_cmd)/sizeof(wpa_supplicant_cmd[0])-1;

	while (true) {
		rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(2000));
		err = rtems_bsd_command_wpa_supplicant(argc, argv);
		printf("wpa_supplicant returned with %d\n", err);
	}
}

static int file_exists(const char *filename) {
    struct stat st;
    int result = stat(filename, &st);
    return result == 0;
}

/*
 * FIXME: This currently starts an ugly hack (wpa_supplicant_watcher) that just
 * restarts the wpa_supplicant if it doesn't run anymore. It should be replaced
 * by a proper event handling.
 */
void
grisp_init_wpa_supplicant(const char *conf_file, rtems_task_priority prio)
{
	char *conf;
	size_t pos;
	rtems_status_code sc;
	rtems_id id;

	if (!file_exists(conf_file)) {
		printf("ERROR: wpa configuration does not exist: %s\n",
		    conf_file);
		return;
	}

	pos = sizeof(wpa_supplicant_cmd)/sizeof(wpa_supplicant_cmd[0]) - 2;
	conf = strdup(conf_file);
	wpa_supplicant_cmd[pos] = conf;

	sc = rtems_task_create(
		rtems_build_name('W', 'P', 'A', 'W'),
		prio,
		32 * 1024,
		RTEMS_DEFAULT_MODES,
		RTEMS_FLOATING_POINT,
		&id
	);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_task_start(id, wpa_supplicant_watcher_task, 0);
	assert(sc == RTEMS_SUCCESSFUL);
}
