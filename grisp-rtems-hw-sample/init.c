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
#include <rtems.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <bsp.h>

#include <bsp/spi.h>
#include <bsp/i2c.h>

#include <dev/spi/spi.h>
#include <dev/i2c/eeprom.h>
#include <dev/i2c/i2c.h>

#include <grisp/led.h>
#include <grisp/pin-config.h>

/** Page size in bytes */
#define EEPROM_PAGE_SIZE 16

/** Amount of pages */
#define EEPROM_PAGES 16

#define EEPROM_SIZE (EEPROM_PAGE_SIZE * EEPROM_PAGES)
#define EEPROM_ADDR 0x57
#define PMOD_CMPS_ADDR 0x1e
#define PMOD_CMPS_SIZE 13
#define PMOD_CMPS_PAGE_SIZE 1

const Pin atsam_pin_config[] = {GRISP_PIN_CONFIG};
const size_t atsam_pin_config_count = PIO_LISTSIZE(atsam_pin_config);
const uint32_t atsam_matrix_ccfg_sysio = GRISP_MATRIX_CCFG_SYSIO;

static uint8_t test_rx_array[3];
static uint8_t test_tx_array[3];
static const char eeprom_path[] = "/dev/i2c-0.eeprom-0";
static const char cmps_path[] = "/dev/i2c-0.compass-0";

static int
init_spi(void)
{
	int rv;
	int fd;
	uint32_t speed = 100000;

	/* bus registration */
	rv = atsam_register_spi_0();
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
init_i2c(int *fd_in, int *fd_out)
{
	int rv;

	rv = atsam_register_i2c_0();
	assert(rv == 0);

	rv = i2c_dev_register_eeprom(
	    ATSAM_I2C_0_BUS_PATH,
	    &eeprom_path[0],
	    EEPROM_ADDR,
	    1,
	    EEPROM_PAGE_SIZE,
	    EEPROM_SIZE,
	    0
	);
	assert(rv == 0);

	*fd_in = open(&eeprom_path[0], O_RDWR);
	assert(*fd_in != -1);
	*fd_out = open(&eeprom_path[0], O_RDWR);
	assert(*fd_out != -1);
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

static int
unlink_test_eeprom_and_bus(void)
{
	int rv;

	rv = unlink(ATSAM_I2C_0_BUS_PATH);
	assert(rv == 0);

	rv = unlink(&eeprom_path[0]);
	assert(rv == 0);

	rv= unlink(&cmps_path[0]);
	assert(rv == 0);

	return rv;
}

static int
close_fds(int in, int out, int in_cmps, int out_cmps)
{
	int rv;

	rv = close(in);
	assert(rv == 0);
	rv = close(out);
	assert(rv == 0);
	rv = close(in_cmps);
	assert(rv == 0);
	rv = close(out_cmps);
	assert(rv == 0);

	return rv;
}

static void
read_write_i2c_eeprom(int fd_in, int fd_out, uint8_t *in, uint8_t *out, size_t len)
{
	ssize_t rb, wb;

	wb = write(fd_out, &in[0], len);
	assert(wb == (ssize_t)len);
	rb = read(fd_in, &out[0], len);
	assert(rb == (ssize_t)len);
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
Init(rtems_task_argument arg)
{
	int rv;
	int fd_spi;
	int fd_in, fd_out;
	int fd_in_cmps, fd_out_cmps;
	uint8_t val;
	uint8_t addr[] = {0x0, 0x1, 0x2, 0x3, 0x8, 0x9, 0xA, 0xB};
	uint8_t acl2_compare_value[] = {0xad, 0x1d, 0xf2};
	uint8_t cmps_compare_value[] = {0x48, 0x34, 0x33};
	uint8_t in[] = {0xc0, 0x1d, 0xc0, 0xff, 0xee};
	uint8_t out[] = {0xff, 0xff, 0xff, 0xff, 0xff};
	uint8_t cmps_in[] = {0x09};
	uint8_t cmps_out[] = {0xff,0xff, 0xff, 0xff};
	int i, err = 0;
	bool passed = true;

	(void)arg;
	puts("");
	puts("Start testing Grisp hardware");
	puts("Contect the following modules: PmodACL2 on SPI1");
	puts("                               PmodCMPS on I2C");

	puts("\nSPI test");
	fd_spi = init_spi();
	for(i=0; i<8; i++) {
		read_spi_register(fd_spi, addr[i], &val);
		printf("value of register %x: %x\n",addr[i], val);
		if(i < 3) {
			if (val != acl2_compare_value[i]) {
				err = 1;
			}
		}
	}
	if (err != 0) {
		puts("***** SPI test FAILED. Try right module PmodACL2 on port SPI1.");
		passed = false;
	}
	else {
		puts("SPI test passed.");
	}

	err = 0;
	puts("\nI2C test");
	init_i2c(&fd_in, &fd_out);

	read_write_i2c_eeprom(fd_in, fd_out, &in[0], &out[0], sizeof (in)/sizeof (in[0]));

	printf("I2C eeprom write data: ");
	for (i=0; i<(int)(sizeof (in)/sizeof (in[0])); i++) {
		printf("%x ",in[i]);
	}
	puts("");
	printf("I2C eeprom read data:  ");
	for (i=0; i<(int)(sizeof (in)/sizeof (in[0])); i++) {
		printf("%x ",out[i]);
	}
	for (i=0; i<(int)(sizeof (in)/sizeof (in[0])); i++) {
		if (in[i] != out[i]) {
			err = 1;
		}
	}
	puts("");
	if (err != 0) {
		puts("***** Internal I2C test FAILED.");
		passed = false;
	}
	else {
		puts("Internal I2C test passed.");
	}
	puts("");

	init_cmps(&fd_in_cmps, &fd_out_cmps);

	read_write_pmod_cmps(fd_in_cmps, &cmps_out[0], sizeof (cmps_out)/sizeof(cmps_out[0]));

	printf("\nCompass register read start: ");
	for (i=0; i<(int)(sizeof (cmps_in)/sizeof (cmps_in[0])); i++) {
		printf("%x ",cmps_in[i]);
	}
	puts("");
	printf("Compass register values : ");
	for (i=0; i<(int)(sizeof (cmps_out)/sizeof (cmps_out[0])); i++) {
		printf("%x ",cmps_out[i]);
	}
	for (i=0; i<3; i++) {
		if (cmps_out[i+1] != cmps_compare_value[i]) {
			err = 1;
		}
	}
	puts("");
	if (err != 0) {
		puts("***** External I2C test FAILED. Try right module PmodCMPS on I2C.");
		passed = false;
	}
	else {
		puts("External I2C test passed.");
	}
	puts("");

	if (passed) {
		puts("**** Test successfull ****");
	}
	else {
		puts("**** Test FAILED ****");
	}

	rv = unlink_test_eeprom_and_bus();
	assert(rv == 0);
	rv = close_fds(fd_in, fd_out, fd_in_cmps, fd_out_cmps);
	assert(rv == 0);
	rtems_task_delete(RTEMS_SELF);
}

/*
 * Configure RTEMS.
 */
#define CONFIGURE_MICROSECONDS_PER_TICK 10000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_INIT_TASK_STACK_SIZE (32 * 1024)

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
