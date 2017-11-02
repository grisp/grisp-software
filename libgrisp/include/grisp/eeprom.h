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

#ifndef GRISP_EEPROM_H
#define GRISP_EEPROM_H

#include <rtems.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GRISP_EEPROM_SIG_VERSION 1

struct grisp_eeprom {
	/** Version of the signature block. Always GRISP_EEPROM_SIG_VERSION. */
	uint8_t sig_version;
	/** Serial number of the board. */
	uint32_t serial;
	/** Production batch. */
	uint16_t batch_nr;
	/** Production year. */
	uint16_t prod_year;
	/** Production month. (1..12) */
	uint8_t prod_month;
	/** Production month. (1..31) */
	uint8_t prod_day;
	/** Mayor hardware version. */
	uint8_t vers_major;
	/** Minor hardware version. */
	uint8_t vers_minor;
	/** Variant of the assembled parts. */
	uint8_t ass_var;
	/** MAC address of the WiFi module. */
	uint8_t mac_addr[6];
	/** CRC for the EEPROM. */
	uint16_t crc16;
};

/**
 * Create EEPROM device.
 */
int	grisp_eeprom_init(void);

/**
 * Read the EEPROM contents into the given structure. Returns 0 if everything
 * went well or != 0 if the EEPROM content is not valid. In the later case, the
 * content of the returned structure is undefined.
 */
int	grisp_eeprom_get(struct grisp_eeprom *eeprom);

/**
 * Write the EEPROM contents. Calculates the correct CRC. Returns 0 if
 * everything went well or != 0 if something went wrong.
 */
int	grisp_eeprom_set(struct grisp_eeprom *eeprom);

/**
 * Dump the given EEPROM structure contents to stdout.
 */
void	grisp_eeprom_dump(struct grisp_eeprom *eeprom);

/**
 * Helper for calculating EEPROM CRC.
 */
uint16_t grisp_eeprom_crc16(uint16_t crc, uint8_t *cp, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GRISP_EEPROM_H */
