/*
 * Copyright (c) 2016-2017 embedded brains GmbH.  All rights reserved.
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
#include <rtems.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/ioctl.h>

#include <bsp.h>
#include <bsp/atsam-spi.h>
#include <bsp/spi.h>
#include <bsp/i2c.h>

#include <dev/spi/spi.h>
#include <dev/i2c/eeprom.h>
#include <dev/i2c/i2c.h>

#include <grisp/init.h>
#include <grisp/led.h>
#include <grisp/pin-config.h>
#include <grisp/eeprom.h>

#include <inih/ini.h>
#include <machine/rtems-bsd-commands.h>

#define N_ELE(arr) (sizeof(arr)/sizeof(arr[0]))

#define PMOD_CMPS_ADDR 0x1e
#define PMOD_CMPS_SIZE 13
#define PMOD_CMPS_PAGE_SIZE 1

#define MAX_SSID_LEN 32
#define MAX_IP_LEN (sizeof("255.255.255.255"))
#define WLAN_NAME "wlan0"

#define DIO11 {PIO_PC12, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define DIO21 {PIO_PC13, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define DIO31 {PIO_PA21, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define DIO41 {PIO_PD30, PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}
#define DIO51 {PIO_PD0, PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT}
#define DIO61 {PIO_PD1, PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT}
#define DIO71 {PIO_PD2, PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT}
#define DIO81 {PIO_PD3, PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT}

#define DIO12 {PIO_PC12, PIOC, ID_PIOC, PIO_INPUT, PIO_DEFAULT}
#define DIO22 {PIO_PC13, PIOC, ID_PIOC, PIO_INPUT, PIO_DEFAULT}
#define DIO32 {PIO_PA21, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define DIO42 {PIO_PD30, PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT}
#define DIO52 {PIO_PD0, PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}
#define DIO62 {PIO_PD1, PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}
#define DIO72 {PIO_PD2, PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}
#define DIO82 {PIO_PD3, PIOD, ID_PIOD, PIO_OUTPUT_0, PIO_DEFAULT}

static const Pin DPIO1[] = {DIO11, DIO21, DIO31, DIO41, DIO51, DIO61, DIO71, DIO81};
static const Pin DPIO2[] = {DIO12, DIO22, DIO32, DIO42, DIO52, DIO62, DIO72, DIO82};

const Pin atsam_pin_config[] = {GRISP_PIN_CONFIG};
const size_t atsam_pin_config_count = PIO_LISTSIZE(atsam_pin_config);
const uint32_t atsam_matrix_ccfg_sysio = GRISP_MATRIX_CCFG_SYSIO;

static uint8_t test_rx_array[3];
static uint8_t test_tx_array[3];
static const char cmps_path[] = "/dev/i2c-0.compass-0";
static const char uart_path[] = "/dev/ttyUSART0";
static const char ini_file[] = "/media/mmcsd-0-0/grisp.ini";
static char ssid[MAX_SSID_LEN + 1] = "";
static char ping_ip[MAX_IP_LEN] = "";
static char net_ip[MAX_IP_LEN] = "";
static char net_mask[MAX_IP_LEN] = "";

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

	if (strcmp(section, "hw-test") == 0) {
		if (strcmp(name, "wlan_name") == 0) {
			ok = ini_value_copy(ssid, sizeof(ssid), value);
		}
		if (strcmp(name, "net_ip") == 0) {
			ok = ini_value_copy(net_ip, sizeof(net_ip), value);
		}
		if (strcmp(name, "net_mask") == 0) {
			ok = ini_value_copy(net_mask, sizeof(net_mask), value);
		}
		if (strcmp(name, "ping_ip") == 0) {
			ok = ini_value_copy(ping_ip, sizeof(ping_ip), value);
		}
	} else {
		/* All other sections are not relevant */
		ok = 1;
	}

	if (!ok) {
		printf("boot: error in configuration file: section \"%s\", name \"%s\", value \"%s\"\n",
		    section, name, value);
		ok = 1;
	}

	return ok;
}

static bool
evaluate_ini_file(const char *filename)
{
	int rv;
	bool passed = true;

	puts("----- parse configuration");

	if (passed) {
		rv = ini_parse(filename, ini_file_handler, NULL);
		passed = (rv == 0);
	}
	if (passed) {
		passed = (strnlen(ssid, sizeof(ssid)) > 1);
	}
	if (passed) {
		passed = (strnlen(net_ip, sizeof(net_ip)) > 1);
	}
	if (passed) {
		passed = (strnlen(net_mask, sizeof(net_mask)) > 1);
	}
	if (passed) {
		passed = (strnlen(ping_ip, sizeof(ping_ip)) > 1);
	}
	if (rv == 0) {
		puts("***** parse configuration passed.");
	} else {
		puts("EEEEE parse configuration FAILED.");
	}
	return (rv == 0);
}

static int
init_spi(void)
{
	int rv;
	int fd;
	uint32_t speed = 100000;

	/* bus registration */
	rv = spi_bus_register_atsam(
	    ATSAM_SPI_0_BUS_PATH, ID_SPI0, SPI0, NULL, 0);
	assert(rv == 0);

	fd = open(ATSAM_SPI_0_BUS_PATH, O_RDWR);
	assert(fd != -1);

	rv = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	assert(rv == 0);

	return fd;
}

static void
test_set_default_msg(struct spi_ioc_transfer *msg)
{
	msg->rx_buf = test_rx_array;
	msg->tx_buf = test_tx_array;
	msg->len = 3;
	msg->speed_hz = 100000;
	msg->delay_usecs = 1;
	msg->bits_per_word = 8;
	msg->cs_change = 1;
	msg->rx_nbits = 0;
	msg->tx_nbits = 0;
	msg->mode = 0;
	msg->cs = 2;
}

static void
read_spi_register(int fd, uint8_t addr, uint8_t *val)
{
	int rv;
	struct spi_ioc_transfer msg;

	test_tx_array[0] = 0x0b;
	test_tx_array[1] = addr;
	test_tx_array[2] = 0xff;

	test_set_default_msg(&msg);
	rv = ioctl(fd, SPI_IOC_MESSAGE(1), &msg);
	assert(rv == 0);
	*val = test_rx_array[2];
}

static void
init_cmps(int *fd_in, int *fd_out)
{
	int rv;

	rv = i2c_dev_register_eeprom(
	    ATSAM_I2C_0_BUS_PATH,
	    &cmps_path[0],
	    PMOD_CMPS_ADDR,
	    1,
	    PMOD_CMPS_PAGE_SIZE,
	    PMOD_CMPS_SIZE,
	    0
	);
	assert(rv == 0);

	*fd_in = open(&cmps_path[0], O_RDWR);
	assert(*fd_in != -1);
	*fd_out = open(&cmps_path[0], O_RDWR);
	assert(*fd_out != -1);
}

static void
read_write_pmod_cmps(int fd_in, uint8_t *out, size_t lenr)
{
	ssize_t rb;
	off_t seekresult;

	seekresult = lseek(fd_in, 9, SEEK_SET);
	assert(seekresult >= 0);
	rb = read(fd_in, &out[0], lenr);
	assert(rb == (ssize_t)lenr);
}

static void
init_gpio1(void) {
	PIO_Configure(DPIO1, sizeof(DPIO1)/sizeof(DPIO1[0]));
}

static void
init_gpio2(void) {
	PIO_Configure(DPIO2, sizeof(DPIO2)/sizeof(DPIO2[0]));
}

static void
prepare_i2c(void)
{
	int rv;
	rv = atsam_register_i2c_0();
	assert(rv == 0);
}

static void
destroy_i2c(void)
{
	int rv;
	rv = unlink(ATSAM_I2C_0_BUS_PATH);
	assert(rv == 0);
}

static bool
test_spi(void)
{
	int err = 0;
	int fd_spi;
	const uint8_t addr[] = {0x0, 0x1, 0x2, 0x3, 0x8, 0x9, 0xA, 0xB};
	const uint8_t acl2_compare_value[] = {0xad, 0x1d, 0xf2};
	uint8_t val;
	size_t i;

	puts("----- SPI test");
	fd_spi = init_spi();
	for (i=0; i < N_ELE(addr); i++) {
		read_spi_register(fd_spi, addr[i], &val);
		printf("value of register %x: %x\n",addr[i], val);
		if(i < N_ELE(acl2_compare_value)) {
			if (val != acl2_compare_value[i]) {
				err = 1;
			}
		}
	}
	if (err != 0) {
		puts("EEEEE SPI test FAILED. Try right module PmodACL2 on port SPI1.");
	}
	else {
		puts("***** SPI test passed.");
		return true;
	}

	return false;
}

static bool
test_i2c_eeprom(void)
{
	int err = 0;
	int rv;
	struct grisp_eeprom eeprom;

	puts("----- EEPROM test");

	err = grisp_eeprom_init();
	assert(err == 0);

	memset(&eeprom.mac_addr, 0, sizeof(eeprom.mac_addr));

	if (err == 0) {
		eeprom.sig_version = GRISP_EEPROM_SIG_VERSION;
		/* Ask for values */
		puts("Please enter the new EEPROM content in the following format:\n"
		    "Format (integers): serial, batch_nr, prod_year, prod_month, prod_day, vers_major, vers_minor, ass_var");
		rv = scanf("%lu, %hu, %hu, %hhu, %hhu, %hhu, %hhu, %hhu",
		    &eeprom.serial, &eeprom.batch_nr, &eeprom.prod_year,
		    &eeprom.prod_month, &eeprom.prod_day, &eeprom.vers_major,
		    &eeprom.vers_minor, &eeprom.ass_var);
		if (rv != 8) {
			printf("Invalid input (%s). Don't write EEPROM.\n",
			    strerror(errno));
			err = 1;
		}
	}

	if (err == 0) {
		puts("Write EEPROM.");
		err = grisp_eeprom_set(&eeprom);
	}

	if (err == 0) {
		puts("Verify EEPROM CRC.");
		err = grisp_eeprom_set(&eeprom);
		/* Dump only for the log */
		grisp_eeprom_dump(&eeprom);
	}

	if (err != 0) {
		puts("EEEEE EEPROM test FAILED.");
	}
	else {
		puts("***** EEPROM test passed.");
		return true;
	}

	return false;
}

static bool
test_i2c_cmps(void)
{
	int err = 0;
	int fd_in_cmps, fd_out_cmps;
	uint8_t cmps_compare_value[] = {0x48, 0x34, 0x33};
	uint8_t cmps_in[] = {0x09};
	uint8_t cmps_out[] = {0xff,0xff, 0xff, 0xff};
	size_t i;
	int rv;

	puts("----- External I2C test");

	init_cmps(&fd_in_cmps, &fd_out_cmps);

	read_write_pmod_cmps(fd_in_cmps, &cmps_out[0], sizeof (cmps_out)/sizeof(cmps_out[0]));

	printf("Compass register read start: ");
	for (i=0; i < N_ELE(cmps_in); i++) {
		printf("%x ",cmps_in[i]);
	}
	puts("");

	printf("Compass register values: ");
	for (i=0; i < N_ELE(cmps_out); i++) {
		printf("%x ",cmps_out[i]);
	}
	puts("");
	for (i=0; i < 3; i++) {
		if (cmps_out[i+1] != cmps_compare_value[i]) {
			err = 1;
		}
	}

	rv = close(fd_in_cmps);
	assert(rv == 0);

	rv = close(fd_out_cmps);
	assert(rv == 0);

	rv = unlink(&cmps_path[0]);
	assert(rv == 0);

	if (err != 0) {
		puts("EEEEE External I2C test FAILED. Try right module PmodCMPS on I2C.");
	}
	else {
		puts("***** External I2C test passed.");
		return true;
	}

	return false;
}

static bool
test_gpio(void)
{
	bool passed = true;
	uint32_t ans;

	puts("----- GPIO test");

	init_gpio1();

	if (passed) {
		puts("\nGPIO test");
		puts("Switch GPIO1 high and measure GPIO2");
		PIO_Set(&DPIO1[0]);
		PIO_Clear(&DPIO1[1]);
		PIO_Set(&DPIO1[2]);
		PIO_Clear(&DPIO1[3]);
		ans = PIO_Get(&DPIO1[4]);
		if (ans == 0) {
			puts ("GPIO fail between DIO1 and DIO5");
			passed = false;
		}
		ans = PIO_Get(&DPIO1[5]);
		if (ans) {
			puts ("GPIO fail between DIO2 and DIO6");
			passed = false;
		}
		ans = PIO_Get(&DPIO1[6]);
		if (ans == 0) {
			puts ("GPIO fail between DIO3 and DIO7");
			passed = false;
		}
		ans = PIO_Get(&DPIO1[7]);
		if (ans) {
			puts ("GPIO fail between DIO4 and DIO8");
			passed = false;
		}
		if (passed) {
			puts ("Pattern 1010 read");
		}
	}
	if (passed) {
		PIO_Clear(&DPIO1[0]);
		PIO_Set(&DPIO1[1]);
		PIO_Clear(&DPIO1[2]);
		PIO_Set(&DPIO1[3]);
		ans = PIO_Get(&DPIO1[4]);
		if (ans) {
			puts ("GPIO fail between DIO1 and DIO5");
			passed = false;
		}
		ans = PIO_Get(&DPIO1[5]);
		if (ans == 0) {
			puts ("GPIO fail between DIO2 and DIO6");
			passed = false;
		}
		ans = PIO_Get(&DPIO1[6]);
		if (ans) {
			puts ("GPIO fail between DIO3 and DIO7");
			passed = false;
		}
		ans = PIO_Get(&DPIO1[7]);
		if (ans == 0) {
			puts ("GPIO fail between DIO4 and DIO8");
			passed = false;
		}
		if (passed) {
			puts ("Pattern 0101 read");
		}
	}

	init_gpio2();

	if (passed) {
		puts("\nGPIO test");
		puts("Switch GPIO2 high and measure GPIO1");
		PIO_Set(&DPIO2[4]);
		PIO_Clear(&DPIO2[5]);
		PIO_Set(&DPIO2[6]);
		PIO_Clear(&DPIO2[7]);
		ans = PIO_Get(&DPIO2[0]);
		if (ans == 0) {
			puts ("GPIO fail between DIO1 and DIO5");
			passed = false;
		}
		ans = PIO_Get(&DPIO2[1]);
		if (ans) {
			puts ("GPIO fail between DIO2 and DIO6");
			passed = false;
		}
		ans = PIO_Get(&DPIO2[2]);
		if (ans == 0) {
			puts ("GPIO fail between DIO3 and DIO7");
			passed = false;
		}
		ans = PIO_Get(&DPIO2[3]);
		if (ans) {
			puts ("GPIO fail between DIO4 and DIO8");
			passed = false;
		}
		if (passed) {
			puts ("Pattern 1010 read");
		}
	}
	if (passed) {
		PIO_Clear(&DPIO2[4]);
		PIO_Set(&DPIO2[5]);
		PIO_Clear(&DPIO2[6]);
		PIO_Set(&DPIO2[7]);
		ans = PIO_Get(&DPIO2[0]);
		if (ans) {
			puts ("GPIO fail between DIO1 and DIO5");
			passed = false;
		}
		ans = PIO_Get(&DPIO2[1]);
		if (ans == 0) {
			puts ("GPIO fail between DIO2 and DIO6");
			passed = false;
		}
		ans = PIO_Get(&DPIO2[2]);
		if (ans) {
			puts ("GPIO fail between DIO3 and DIO7");
			passed = false;
		}
		ans = PIO_Get(&DPIO2[3]);
		if (ans == 0) {
			puts ("GPIO fail between DIO4 and DIO8");
			passed = false;
		}
		if (passed) {
			puts ("Pattern 0101 read");
		}
	}

	if (!passed) {
		puts("EEEEE GPIO test FAILED.");
	}
	else {
		puts("***** GPIO test passed.");
	}

	return passed;
}

static bool
test_uart(void)
{
	bool passed = true;
	int fd;
	int rv;
	struct termios options;
	char rx = '\0';

	puts("----- UART test");

	fd = open(uart_path, O_RDWR);
	assert(fd != -1);

	/* Setup interface */
	rv = tcgetattr(fd, &options);
	assert(rv == 0);
	rv = cfsetispeed(&options, B19200);
	assert(rv == 0);
	rv = cfsetospeed(&options, B19200);
	assert(rv == 0);
	options.c_cflag |= CRTSCTS;
	rv = tcsetattr(fd, TCSANOW, &options);
	assert(rv == 0);

	rv = write(fd, "x", 1);
	if (rv != 1) {
		passed = false;
		perror("uart write failed");
	}

	if (passed) {
		rv = read(fd, &rx, 1);
		if (rv != 1) {
			perror("uart read failed");
			passed = false;
		} else if (rx != 'x') {
			passed = false;
			printf("received wrong character: %c\n", rx);
		}
	}

	rv = close(fd);
	assert(rv == 0);

	if (!passed) {
		puts("EEEEE UART test FAILED.");
	}
	else {
		puts("***** UART test passed.");
	}

	return passed;
}

static bool
prepare_libbsd(void)
{
	rtems_status_code sc;
	int exit_code;
	char *ifcfg[] = {
		"ifconfig", WLAN_NAME, "create", "wlandev", "rtwn0", "up", NULL
	};
	bool passed = true;

	puts("----- prepare libbsd");
	grisp_init_sd_card();
	grisp_init_lower_self_prio();
	grisp_init_libbsd();

	if (passed) {
		sc = grisp_init_wait_for_sd();
		if (sc == RTEMS_SUCCESSFUL) {
			printf("SD: OK\n");
		} else {
			printf("ERROR: SD could not be mounted after timeout\n");
			passed = false;
		}
	}

	if (passed) {
		/* Some time for USB device to be detected. */
		rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(4000));
		exit_code = rtems_bsd_command_ifconfig(
		    RTEMS_BSD_ARGC(ifcfg), ifcfg);
		if(exit_code != EXIT_SUCCESS) {
			printf("ERROR while creating wlan.");
			passed = false;
		}
	}

	if (passed) {
		puts("***** prepare libbsd passed.");
	} else {
		puts("EEEEE prepare libbsd FAILED.");
	}

	return passed;
}

static bool
set_up_wifi(void)
{
	bool passed = true;
	int exit_code;
	char *ifcfg_scan[] = {
		"ifconfig", WLAN_NAME, "scan", NULL
	};
	char *ifcfg_ssid[] = {
		"ifconfig", WLAN_NAME, "ssid", ssid, NULL
	};
	char *ifcfg_ip[] = {
		"ifconfig", WLAN_NAME, net_ip, "netmask", net_mask, NULL
	};
	char *ifcfg[] = {
		"ifconfig", WLAN_NAME, NULL
	};

	puts("----- set up wifi");

	if (passed) {
		exit_code = rtems_bsd_command_ifconfig(
		    RTEMS_BSD_ARGC(ifcfg_scan), ifcfg_scan);
		if(exit_code != EXIT_SUCCESS) {
			printf("ERROR while scanning with wlan.");
			passed = false;
		}
	}

	if (passed) {
		exit_code = rtems_bsd_command_ifconfig(
		    RTEMS_BSD_ARGC(ifcfg_ssid), ifcfg_ssid);
		if(exit_code != EXIT_SUCCESS) {
			printf("ERROR while setting ssid of wlan.");
			passed = false;
		}
	}

	rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(2000));

	if (passed) {
		exit_code = rtems_bsd_command_ifconfig(
		    RTEMS_BSD_ARGC(ifcfg_ip), ifcfg_ip);
		if(exit_code != EXIT_SUCCESS) {
			printf("ERROR while setting ip on wlan.");
			passed = false;
		}
	}

	(void) rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(ifcfg), ifcfg);

	if (passed) {
		puts("***** set up wifi passed.");
	} else {
		puts("EEEEE set up wifi FAILED.");
	}

	return passed;
}

static bool
test_ping(void)
{
	bool passed = true;
	int exit_code;
	char *ping[] = {
		"ping", ping_ip, NULL
	};
	const int maxtries = 10;
	const int delay = 1;
	int tries = 0;

	puts("----- do ping test");

	do {
		exit_code = rtems_bsd_command_ping(RTEMS_BSD_ARGC(ping), ping);
		if(exit_code != EXIT_SUCCESS) {
			printf("Ping failed.");
			passed = false;
		} else {
			passed = true;
		}
		if (!passed && tries < maxtries) {
			++tries;
			rtems_task_wake_after(
			    RTEMS_MILLISECONDS_TO_TICKS(delay * 1000));
		}
	} while (!passed && tries <= maxtries);

	if (passed) {
		puts("***** ping test passed.");
	} else {
		puts("EEEEE ping test FAILED.");
	}

	return passed;
}

static void
Init(rtems_task_argument arg)
{
	bool passed = true;
	char choice;
	int  err;

	(void)arg;
	puts("");
	puts("Start testing Grisp hardware");
	puts("Contect the following modules:");
	puts(" - PmodACL2 on SPI1");
	puts(" - PmodCMPS on I2C");
	puts(" - Bridge between GPIO1 and GPIO2");
	puts(" - Loopback on UART (TxD -> RxD, RTS -> CTS)");
	puts("Prepare a WiFi access point with a configuration that matches the one in grisp.ini.");

	grisp_led_set1(false, false, false);
	grisp_led_set2(false, false, false);

	if (passed) {
		passed = prepare_libbsd();
	}
	if (passed) {
		passed = evaluate_ini_file(ini_file);
	}
	if (passed) {
		passed = set_up_wifi();
	}
	if (passed) {
		passed = test_spi();
	}
	if (passed) {
		passed = test_gpio();
	}
	if (passed) {
		passed = test_uart();
	}
	if (passed) {
		int c;
		puts("\nLED test red");
		grisp_led_set1(true, false, false);
		grisp_led_set2(true, false, false);
		puts("Only red LEDS on? y/n");
		err = scanf("%c", &choice);
		assert(err == 1);
		while ((c = getchar()) != '\n' && c != EOF); /* Flush stdin. */
		if (choice != 'y') {
			passed = false;
		}
	}
	if (passed) {
		int c;
		puts("\nLED test green");
		grisp_led_set1(false, true, false);
		grisp_led_set2(false, true, false);
		puts("Only green LEDS on? y/n");
		err = scanf("%c", &choice);
		assert(err == 1);
		while ((c = getchar()) != '\n' && c != EOF); /* Flush stdin. */
		if (choice != 'y') {
			passed = false;
		}
	}
	if (passed) {
		int c;
		puts("\nLED test blue");
		grisp_led_set1(false, false, true);
		grisp_led_set2(false, false, true);
		puts("Only blue LEDS on? y/n");
		err = scanf("%c", &choice);
		assert(err == 1);
		while ((c = getchar()) != '\n' && c != EOF); /* Flush stdin. */
		if (choice != 'y') {
			passed = false;
		}
	}
	if (passed) {
		passed = test_ping();
	}
	prepare_i2c();
	if (passed) {
		passed = test_i2c_cmps();
	}
	if (passed) {
		passed = test_i2c_eeprom();
	}
	destroy_i2c();

	if (passed) {
		puts("**** Test successful ****");
	}
	else {
		puts("**** Test FAILED ****");
	}

	while(true) {
		sleep(1);
	}

	rtems_task_delete(RTEMS_SELF);
}

/*
 * Configure LibBSD.
 */
#define RTEMS_BSD_CONFIG_BSP_CONFIG
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

#define CONFIGURE_INIT_TASK_STACK_SIZE (64 * 1024)
#define CONFIGURE_INIT_TASK_INITIAL_MODES RTEMS_DEFAULT_MODES
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE (32 * 1024)
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS 4
#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE (1 * 1024 * 1024)
#define CONFIGURE_BDBUF_READ_AHEAD_TASK_PRIORITY 97
#define CONFIGURE_SWAPOUT_TASK_PRIORITY 97

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT

#include <rtems/confdefs.h>

/*
 * Configure Shell.
 */
#include <rtems/netcmds-config.h>
#include <bsp/irq-info.h>
#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
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
  &rtems_shell_BLKSTATS_Command

#include <rtems/shellconfig.h>
