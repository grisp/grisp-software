/*
 * Copyright (c) 2013-2017 embedded brains GmbH.  All rights reserved.
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

#if !defined(GRISP_LIBBSD_NEXUS_CONFIG_H)
#define GRISP_LIBBSD_NEXUS_CONFIG_H

#include <bsp.h>

#include <rtems/bsd/bsd.h>
#include <machine/rtems-bsd-nexus-bus.h>

#include <libchip/chip.h>

RTEMS_BSD_DRIVER_AT91_MCI0((unsigned long)HSMCI, HSMCI_IRQn);
RTEMS_BSD_DRIVER_MMC;
#ifndef GRISP_IS_BOOTLOADER
RTEMS_BSD_DRIVER_USB;
RTEMS_BSD_DRIVER_USB_MASS;
RTEMS_BSD_DRIVER_USB_SAF1761_OTG((unsigned long)EBI_CS0_ADDR, PIOC_IRQn);
SYSINIT_MODULE_REFERENCE(wlan_ratectl_none);
SYSINIT_MODULE_REFERENCE(wlan_sta);
SYSINIT_MODULE_REFERENCE(wlan_amrr);
SYSINIT_MODULE_REFERENCE(wlan_wep);
SYSINIT_MODULE_REFERENCE(wlan_tkip);
SYSINIT_MODULE_REFERENCE(wlan_ccmp);
SYSINIT_DRIVER_REFERENCE(rtwn_usb, uhub);
SYSINIT_REFERENCE(rtwn_rtl8188eufw);
#endif /* GRISP_IS_BOOTLOADER */

#endif /* GRISP_LIBBSD_NEXUS_CONFIG_H */
