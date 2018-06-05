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
#include <rtems/bsd/bsd.h>
#include <rtems/bdbuf.h>
#include <rtems/console.h>
#include <rtems/malloc.h>
#include <rtems/media.h>
#include <rtems/score/armv7m.h>
#include <rtems/shell.h>
#include <rtems/stringto.h>
#include <rtems/ftpd.h>
#include <machine/rtems-bsd-commands.h>

#include <bsp.h>

#include <grisp/pin-config.h>
#include <grisp/led.h>
#include <grisp/init.h>

#define STACK_SIZE_INIT_TASK	(64 * 1024)
#define STACK_SIZE_SHELL	(64 * 1024)

#define PRIO_SHELL		150
#define PRIO_LED_TASK		(RTEMS_MAXIMUM_PRIORITY - 1)
#define PRIO_DHCP		(RTEMS_MAXIMUM_PRIORITY - 1)
#define PRIO_WPA		(RTEMS_MAXIMUM_PRIORITY - 1)

const char *wpa_supplicant_conf = "/media/mmcsd-0-0/wpa_supplicant.conf";

const Pin atsam_pin_config[] = {GRISP_PIN_CONFIG};
const size_t atsam_pin_config_count = PIO_LISTSIZE(atsam_pin_config);
const uint32_t atsam_matrix_ccfg_sysio = GRISP_MATRIX_CCFG_SYSIO;

struct rtems_ftpd_configuration rtems_ftpd_configuration = {
	.priority = 100,
	.max_hook_filesize = 0,
	.port = 21,
	.hooks = NULL,
	.root = NULL,
	.tasks_count = 4,
	.idle = 5 * 60,
	.access = 0
};

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
create_wlandev(void)
{
	int exit_code;
	char *ifcfg[] = {
		"ifconfig",
		"wlan0",
		"create",
		"wlandev",
		"rtwn0",
		"up",
		NULL
	};

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(ifcfg), ifcfg);
	if(exit_code != EXIT_SUCCESS) {
		printf("ERROR while creating wlan0.");
	}
}

static void
led_task(rtems_task_argument arg)
{
	bool state = false;

	(void)arg;

	while(true) {
		state = !state;
		grisp_led_set1(!state, !state, state);
		grisp_led_set2(state, state, !state);
		rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(250));
	}
}

static void
init_led(void)
{
	rtems_status_code sc;
	rtems_id id;

	sc = rtems_task_create(
		rtems_build_name('L', 'E', 'D', ' '),
		PRIO_LED_TASK,
		RTEMS_MINIMUM_STACK_SIZE,
		RTEMS_DEFAULT_MODES,
		RTEMS_DEFAULT_ATTRIBUTES,
		&id
	);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_task_start(id, led_task, 0);
	assert(sc == RTEMS_SUCCESSFUL);
}

static int
command_startftp(int argc, char *argv[])
{
	rtems_status_code sc;

	(void) argc;
	(void) argv;

	sc = rtems_initialize_ftpd();
	if(sc == RTEMS_SUCCESSFUL) {
		printf("FTP started.\n");
	} else {
		printf("ERROR: FTP could not be started.\n");
	}

	return 0;
}

rtems_shell_cmd_t rtems_shell_STARTFTP_Command = {
	"startftp",          /* name */
	"startftp",          /* usage */
	"net",               /* topic */
	command_startftp,    /* command */
	NULL,                /* alias */
	NULL,                /* next */
	0, 0, 0
};

static void
Init(rtems_task_argument arg)
{
	rtems_status_code sc;
	Pin mode1 = GRISP_MODE_1;

	(void)arg;

	grisp_led_set1(false, false, false);
	grisp_led_set2(true, false, false);
	puts("\nGRISP RTEMS SD Demo\n");
	grisp_init_sd_card();
	grisp_init_lower_self_prio();
	grisp_init_libbsd();

	/* Wait for the SD card */
	grisp_led_set2(true, false, true);
	sc = grisp_init_wait_for_sd();
	if (sc == RTEMS_SUCCESSFUL) {
		printf("SD: OK\n");
	} else {
		printf("ERROR: SD could not be mounted after timeout\n");
		grisp_led_set1(true, false, false);
	}

	grisp_init_dhcpcd(PRIO_DHCP);

	grisp_led_set2(false, false, true);
	if (PIO_Get(&mode1) != 0) {
		printf("Mode 1 set: Create WLAN device.\n");
		/* Some time for USB device to be detected. */
		rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(4000));
		create_wlandev();
	} else {
		printf("Mode 1 cleared: Skip creating WLAN device.\n");
	}
	grisp_init_wpa_supplicant(wpa_supplicant_conf, PRIO_WPA);

	init_led();
	start_shell();

	exit(0);
}

/*
 * Configure LibBSD.
 */
#include <grisp/libbsd-nexus-config.h>
#define RTEMS_BSD_CONFIG_TERMIOS_KQUEUE_AND_POLL
#define RTEMS_BSD_CONFIG_INIT

#include <machine/rtems-bsd-config.h>

/*
 * Configure RTEMS.
 */
#define CONFIGURE_MICROSECONDS_PER_TICK 10000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

#define CONFIGURE_FILESYSTEM_DOSFS
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 1

#define CONFIGURE_INIT_TASK_STACK_SIZE STACK_SIZE_INIT_TASK
#define CONFIGURE_INIT_TASK_INITIAL_MODES RTEMS_DEFAULT_MODES
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE (32 * 1024)
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS 4
#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE (1 * 1024 * 1024)
#define CONFIGURE_BDBUF_READ_AHEAD_TASK_PRIORITY 97
#define CONFIGURE_SWAPOUT_TASK_PRIORITY 97

//#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT

#include <rtems/confdefs.h>

/*
 * Configure Shell.
 */
#include <rtems/netcmds-config.h>
#include <bsp/irq-info.h>
#define CONFIGURE_SHELL_COMMANDS_INIT

#define CONFIGURE_SHELL_USER_COMMANDS \
  &bsp_interrupt_shell_command, \
  &rtems_shell_ARP_Command, \
  &rtems_shell_PFCTL_Command, \
  &rtems_shell_PING_Command, \
  &rtems_shell_IFCONFIG_Command, \
  &rtems_shell_ROUTE_Command, \
  &rtems_shell_NETSTAT_Command, \
  &rtems_shell_DHCPCD_Command, \
  &rtems_shell_HOSTNAME_Command, \
  &rtems_shell_SYSCTL_Command, \
  &rtems_shell_VMSTAT_Command, \
  &rtems_shell_WLANSTATS_Command, \
  &rtems_shell_STARTFTP_Command, \
  &rtems_shell_BLKSTATS_Command, \
  &rtems_shell_WPA_SUPPLICANT_Command, \
  &rtems_shell_WPA_SUPPLICANT_FORK_Command

#define CONFIGURE_SHELL_COMMANDS_ALL

#include <rtems/shellconfig.h>
