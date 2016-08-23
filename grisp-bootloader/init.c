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
#include <rtems/console.h>
#include <rtems/shell.h>
#include <rtems/stringto.h>

#include <bsp.h>

#include <grisp/pin-config.h>
#include <grisp/led.h>

#include <inih/ini.h>

#define SHELL_STACK_SIZE (8 * 1024)

const Pin atsam_pin_config[] = {GRISP_PIN_CONFIG};
const size_t atsam_pin_config_count = PIO_LISTSIZE(atsam_pin_config);
const uint32_t atsam_matrix_ccfg_sysio = GRISP_MATRIX_CCFG_SYSIO;

static const char ini_file[] = "/sd/grisp.ini";
static int timeout_in_seconds = 3;
static char image_path[PATH_MAX + 1] = "/sd/grisp.bin";

static rtems_id led_timer_id;

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
		SHELL_STACK_SIZE,
		10,
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

	(void)arg;

	init_led();
	evaluate_ini_file(ini_file);

	if (timeout_in_seconds == 0) {
		service_mode_requested = false;
	} else {
		service_mode_requested = wait_for_user_input();
	}

	if (!service_mode_requested) {
		/* FIXME: boot application */

		/* Fallback: Show error and star service mode */
		led_not_ok();
	}

	service_mode();

	exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
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
#define CONFIGURE_SHELL_NO_COMMAND_MKRFS
#define CONFIGURE_SHELL_NO_COMMAND_FDISK
#define CONFIGURE_SHELL_NO_COMMAND_DEBUGRFS

#include <rtems/shellconfig.h>
