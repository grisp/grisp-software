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
#include <stdlib.h>

#include <rtems.h>
#include <rtems/console.h>
#include <rtems/malloc.h>
#include <rtems/shell.h>

#include <bsp.h>

#include <grisp/led.h>
#include <grisp/pin-config.h>

#define STACK_SIZE_INIT_TASK	(32 * 1024)
#define STACK_SIZE_SHELL	(32 * 1024)
#define PRIO_SHELL		10

const Pin atsam_pin_config[] = {GRISP_PIN_CONFIG};
const size_t atsam_pin_config_count = PIO_LISTSIZE(atsam_pin_config);
const uint32_t atsam_matrix_ccfg_sysio = GRISP_MATRIX_CCFG_SYSIO;

static rtems_id led_timer_id = RTEMS_INVALID_ID;

static void
led_timer(rtems_id timer, void *arg)
{
	rtems_status_code sc;
	static uint8_t state = 0x1;
	bool r, g, b;

	(void)arg;

	sc = rtems_timer_reset(timer);
	assert(sc == RTEMS_SUCCESSFUL);

	++state;
	if (state > 0x7) {
		state = 0x0;
	}

	r = ((state & 0x1) != 0);
	g = ((state & 0x2) != 0);
	b = ((state & 0x4) != 0);

	grisp_led_set2(r, g, b);
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
		NULL
	);
	assert(sc == RTEMS_SUCCESSFUL);
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
Init(rtems_task_argument arg)
{
	(void)arg;

	puts("\nSimple RTEMS sample application\n");
	init_led();
	start_shell();

	exit(0);
}

/*
 * Configure RTEMS.
 */
#define CONFIGURE_MICROSECONDS_PER_TICK 10000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 8

#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_INIT_TASK_STACK_SIZE STACK_SIZE_INIT_TASK

#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT

#include <rtems/confdefs.h>

/*
 * Configure Shell.
 */
#include <rtems/netcmds-config.h>
#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
#define CONFIGURE_SHELL_NO_COMMAND_MKRFS
#define CONFIGURE_SHELL_NO_COMMAND_FDISK
#define CONFIGURE_SHELL_NO_COMMAND_DEBUGRFS

#include <rtems/shellconfig.h>
