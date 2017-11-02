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

#include <grisp/eeprom.h>
#include <bsp/i2c.h>
#include <dev/i2c/eeprom.h>
#include <dev/i2c/i2c.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

static const char eeprom_path[] = "/dev/i2c-0.eeprom-0";
#define EEPROM_ADDR 0x57
#define EEPROM_PAGES 16
#define EEPROM_PAGE_SIZE 16
#define EEPROM_SIZE (EEPROM_PAGE_SIZE * EEPROM_PAGES)

#define EEPROM_CRC_START_VALUE 0

int
grisp_eeprom_init(void)
{
	int rv;

	rv = i2c_dev_register_eeprom(
	    ATSAM_I2C_0_BUS_PATH,
	    &eeprom_path[0],
	    EEPROM_ADDR,
	    1,
	    EEPROM_PAGE_SIZE,
	    EEPROM_SIZE,
	    0
	);
	return rv;
}

int
grisp_eeprom_get(struct grisp_eeprom *eeprom)
{
	int fd;
	int rv;
	uint16_t crc = EEPROM_CRC_START_VALUE;

	fd = open(&eeprom_path[0], O_RDONLY);
	if (fd == -1) {
		return fd;
	}

	rv = read(fd, eeprom, sizeof(*eeprom));
	close(fd);

	if (rv != sizeof(*eeprom)) {
		return -1;
	}

	if (eeprom->sig_version != GRISP_EEPROM_SIG_VERSION) {
		return -1;
	}

	crc = grisp_eeprom_crc16(crc, (void*)eeprom,
	    sizeof(*eeprom) - sizeof(eeprom->crc16));
	if (crc != eeprom->crc16) {
		return -1;
	}

	return 0;
}

int
grisp_eeprom_set(struct grisp_eeprom *eeprom)
{
	int fd;
	int rv;
	uint16_t crc = EEPROM_CRC_START_VALUE;

	crc = grisp_eeprom_crc16(crc, (void*)eeprom,
	    sizeof(*eeprom) - sizeof(eeprom->crc16));
	eeprom->crc16 = crc;

	fd = open(&eeprom_path[0], O_WRONLY);
	if (fd == -1) {
		return fd;
	}

	rv = write(fd, eeprom, sizeof(*eeprom));
	close(fd);

	if (rv != sizeof(*eeprom)) {
		return -1;
	}

	return 0;
}

void
grisp_eeprom_dump(struct grisp_eeprom *eeprom)
{
	printf("sig_version: %u\n"
	    "serial: %lu\n"
	    "batch_nr: %u\n"
	    "prod_year: %u\n"
	    "prod_month: %u\n"
	    "prod_day: %u\n"
	    "vers_major: %u\n"
	    "vers_minor: %u\n"
	    "ass_var: %u\n"
	    "mac_addr: %02x:%02x:%02x:%02x:%02x:%02x\n"
	    "crc16: %04x\n",
	    eeprom->sig_version,
	    eeprom->serial,
	    eeprom->batch_nr,
	    eeprom->prod_year,
	    eeprom->prod_month,
	    eeprom->prod_day,
	    eeprom->vers_major,
	    eeprom->vers_minor,
	    eeprom->ass_var,
	    eeprom->mac_addr[0],
	    eeprom->mac_addr[1],
	    eeprom->mac_addr[2],
	    eeprom->mac_addr[3],
	    eeprom->mac_addr[4],
	    eeprom->mac_addr[5],
	    eeprom->crc16
	);
}
