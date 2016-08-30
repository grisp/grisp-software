/*
 * Copyright (c) 2016 embedded brains GmbH.  All rights reserved.
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
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <rtems.h>
#include <rtems/bdbuf.h>
#include <rtems/console.h>
#include <rtems/malloc.h>
#include <rtems/media.h>
#include <rtems/shell.h>
#include <rtems/stringto.h>

#include <bsp.h>

#include <grisp/pin-config.h>
#include <grisp/led.h>

#include <inih/ini.h>

#define STACK_SIZE_SHELL (8 * 1024)
#define STACK_SIZE_MEDIA_SERVER (32 * 1024)

#define PRIO_MEDIA_SERVER 200
#define PRIO_SHELL 10

#define EVT_MOUNTED RTEMS_EVENT_9

const Pin atsam_pin_config[] = {GRISP_PIN_CONFIG};
const size_t atsam_pin_config_count = PIO_LISTSIZE(atsam_pin_config);
const uint32_t atsam_matrix_ccfg_sysio = GRISP_MATRIX_CCFG_SYSIO;

static const char ini_file[] = "/sd/grisp.ini";
static int timeout_in_seconds = 3;
static char image_path[PATH_MAX + 1] = "/sd/grisp.bin";

static rtems_id led_timer_id = RTEMS_INVALID_ID;
static rtems_id wait_mounted_task_id = RTEMS_INVALID_ID;

static int
ini_value_copy(void *dst, size_t dst_size, const char *value)
{
	int ok = 1;
	size_t value_size = strlen(value) + 1;

	if (value_size <= dst_size) {
		memcpy(dst, value, value_size);
	} else {
		ok = 0;
	}

	return ok;
}

static int
ini_file_handler(void *arg, const char *section, const char *name,
    const char *value)
{
	int ok = 0;

	(void)arg;

	if (strcmp(section, "file") == 0) {
		if (strcmp(name, "image_path") == 0) {
			ok = ini_value_copy(&image_path[0], sizeof(image_path),
			    value);
		}
	} else if (strcmp(section, "boot") == 0) {
		if (strcmp(name, "timeout_in_seconds") == 0) {
			rtems_status_code sc = rtems_string_to_int(value,
			    &timeout_in_seconds, NULL, 10);
			ok = sc == RTEMS_SUCCESSFUL;
		}
	}

	if (!ok) {
		printf("boot: error in configuration file: section \"%s\", name \"%s\", value \"%s\"\n",
		    section, name, value);
		ok = 1;
	}

	return ok;
}

static void
led_timer(rtems_id timer, void *arg)
{
	rtems_status_code sc;
	static bool is_on = false;
	bool ok = (bool)arg;
	bool r = false;
	bool g = false;
	bool b = false;

	sc = rtems_timer_reset(timer);
	assert(sc == RTEMS_SUCCESSFUL);

	if (!is_on) {
		if (ok) {
			b = true;
		} else {
			r = true;
		}
	}

	grisp_led_set1(r, g, b);
}

static void
init_led(void)
{
	rtems_status_code sc;

	grisp_led_set1(false, false, false);
	grisp_led_set2(false, false, false);

	sc = rtems_timer_initiate_server(
		250,
		RTEMS_MINIMUM_STACK_SIZE,
		RTEMS_DEFAULT_ATTRIBUTES
	);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_timer_create(rtems_build_name('L', 'E', 'D', ' '),
	    &led_timer_id);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_timer_server_fire_after(
		led_timer_id,
		rtems_clock_get_ticks_per_second() / 2,
		led_timer,
		(void*)true
	);
	assert(sc == RTEMS_SUCCESSFUL);
}

static void
led_not_ok(void)
{
	rtems_status_code sc;

	sc = rtems_timer_cancel(led_timer_id);
	assert(sc == RTEMS_SUCCESSFUL);

	grisp_led_set1(true, false, false);

	sc = rtems_timer_server_fire_after(
		led_timer_id,
		rtems_clock_get_ticks_per_second() / 4,
		led_timer,
		(void*)false
	);
	assert(sc == RTEMS_SUCCESSFUL);
}

static rtems_status_code
media_listener(rtems_media_event event, rtems_media_state state,
    const char *src, const char *dest, void *arg)
{
	printf(
		"media listener: event = %s, state = %s, src = %s",
		rtems_media_event_description(event),
		rtems_media_state_description(state),
		src
	);

	if (dest != NULL) {
		printf(", dest = %s", dest);
	}

	if (arg != NULL) {
		printf(", arg = %p\n", arg);
	}

	printf("\n");

	if (event == RTEMS_MEDIA_EVENT_MOUNT &&
	    state == RTEMS_MEDIA_STATE_SUCCESS) {
		rtems_event_send(wait_mounted_task_id, EVT_MOUNTED);
	}

	return RTEMS_SUCCESSFUL;
}

static rtems_status_code
init_sd_card(void)
{
	rtems_status_code sc;
	const rtems_interval max_mount_time = 3000 /
	    rtems_configuration_get_milliseconds_per_tick();
	rtems_event_set out;

	wait_mounted_task_id = rtems_task_self();

	sc = rtems_bdbuf_init();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_initialize();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_listener_add(media_listener, NULL);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_server_initialize(
	    PRIO_MEDIA_SERVER,
	    STACK_SIZE_MEDIA_SERVER,
	    RTEMS_DEFAULT_MODES,
	    RTEMS_DEFAULT_ATTRIBUTES
	    );
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_event_receive(EVT_MOUNTED, RTEMS_WAIT, max_mount_time, &out);
	assert(sc == RTEMS_SUCCESSFUL || sc == RTEMS_TIMEOUT);

	return sc;
}

static void print_status(bool ok)
{
	if (ok) {
		printf("done\n");
	} else {
		printf("failed\n");
	}
}

static void
load_via_file(const char *file)
{
	static uint8_t *buffer;

	size_t buffer_size = 127 * 1024 * 1024;
	if (buffer == NULL) {
		buffer = rtems_heap_allocate_aligned_with_boundary(buffer_size, 64 * 1024 * 1024, 0);
		assert(buffer != NULL);
	}

	printf("boot: open file \"%s\"... ", file);
	int fd = open(file, O_RDONLY);
	bool ok = fd >= 0;
	print_status(ok);
	if (ok) {
		printf("boot: read file \"%s\"... ", file);
		ssize_t in = read(fd, buffer, buffer_size);
		printf("received %zi bytes\n", in);

		int rv = close(fd);
		assert(rv == 0);

		ssize_t entry = 0x10000;
		if (in > entry) {
			void (*start) ( void ) = (void *)(buffer + entry);
			rtems_interrupt_level level;

			rtems_status_code sc = rtems_timer_cancel(led_timer_id);
			assert(sc == RTEMS_SUCCESSFUL);

			grisp_led_set1(false, true, false);
			sleep(1);

			rtems_interrupt_disable( level );
			(void) level; /* Avoid warning */
			( *start )();
		}
	}
}

static void
evaluate_ini_file(const char *filename)
{
	ini_parse(filename, ini_file_handler, NULL);
}

static void print_message(int fd, int seconds_remaining, void *arg)
{
	(void) fd;
	(void) seconds_remaining;
	(void) arg;

	printf("boot: press key to enter service mode\n");
}

static bool
wait_for_user_input(void)
{
	bool service_mode_requested;
	int fd;

	fd = open(CONSOLE_DEVICE_NAME, O_RDWR);
	assert(fd >= 0);

	rtems_status_code sc = rtems_shell_wait_for_input(
	    fd, timeout_in_seconds, print_message, NULL);
	service_mode_requested = (sc == RTEMS_SUCCESSFUL);

	return service_mode_requested;
}

static void
start_shell(void)
{
	rtems_status_code sc = rtems_shell_init(
		"SHLL",
		STACK_SIZE_SHELL,
		PRIO_SHELL,
		CONSOLE_DEVICE_NAME,
		false,
		true,
		NULL
	);
	assert(sc == RTEMS_SUCCESSFUL);
}

static void
service_mode(void)
{
	start_shell();
}

static void
Init(rtems_task_argument arg)
{
	bool service_mode_requested;
	rtems_status_code sc;

	(void)arg;

	init_led();
	sc = init_sd_card();

	if(sc == RTEMS_SUCCESSFUL) {
		evaluate_ini_file(ini_file);

		if (timeout_in_seconds == 0) {
			service_mode_requested = false;
		} else {
			service_mode_requested = wait_for_user_input();
		}

		if (!service_mode_requested) {
			const char *image = image_path;
			if (strlen(image) > 0) {
				load_via_file(image);
			}

			/* Fallback: Show error and star service mode */
			led_not_ok();
		}
	} else {
		printf("SD is not mounted\n");
		led_not_ok();
	}

	service_mode();

	exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_MICROSECONDS_PER_TICK 1000
#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_STACK_CHECKER_ENABLED
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT

#include <rtems/confdefs.h>

#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL

#include <rtems/shellconfig.h>
