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

#include <grisp/led.h>
#include <grisp/pin-config.h>

static const Pin led_rgb1_red = GRISP_LED_1R;
static const Pin led_rgb1_green = GRISP_LED_1G;
static const Pin led_rgb1_blue = GRISP_LED_1B;

static const Pin led_rgb2_red = GRISP_LED_2R;
static const Pin led_rgb2_green = GRISP_LED_2G;
static const Pin led_rgb2_blue = GRISP_LED_2B;

rtems_status_code
grisp_led_set(int led_nr, bool r, bool g, bool b)
{
	const Pin *led_r = NULL;
	const Pin *led_g = NULL;
	const Pin *led_b = NULL;

	switch (led_nr) {
	case 1:
		led_r = &led_rgb1_red;
		led_g = &led_rgb1_green;
		led_b = &led_rgb1_blue;
		break;
	case 2:
		led_r = &led_rgb2_red;
		led_g = &led_rgb2_green;
		led_b = &led_rgb2_blue;
		break;
	default:
		return RTEMS_INVALID_NUMBER;
		break;
	}

	if (r) {
		PIO_Set(led_r);
	} else {
		PIO_Clear(led_r);
	}

	if (g) {
		PIO_Set(led_g);
	} else {
		PIO_Clear(led_g);
	}

	if (b) {
		PIO_Set(led_b);
	} else {
		PIO_Clear(led_b);
	}

	return RTEMS_SUCCESSFUL;
}

void
grisp_led_set1(bool r, bool g, bool b)
{
	rtems_status_code sc;
	sc = grisp_led_set(1, r, g, b);
	assert(sc == RTEMS_SUCCESSFUL);
}

void
grisp_led_set2(bool r, bool g, bool b)
{
	rtems_status_code sc;
	sc = grisp_led_set(2, r, g, b);
	assert(sc == RTEMS_SUCCESSFUL);
}
