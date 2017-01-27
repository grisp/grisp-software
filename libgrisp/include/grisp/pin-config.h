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

#ifndef GRISP_PIN_CONFIG_H
#define GRISP_PIN_CONFIG_H

#include <rtems.h>
#include <bsp/pin-config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GRISP_LED_1R	{PIO_PA24, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_1G	{PIO_PA22, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_1B	{PIO_PA23, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_2R	{PIO_PA21, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_2G	{PIO_PA19, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_2B	{PIO_PA17, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}

#define GRISP_PIN_CONFIG                                                      \
  /* Console */                                                               \
  {PIO_PA21A_RXD1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT},                 \
  {PIO_PB4D_TXD1, PIOB, ID_PIOB, PIO_PERIPH_D, PIO_DEFAULT},                  \
                                                                              \
  /* SDRAM */                                                                 \
  {0x000000FF, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* D0_7   */        \
  {0x0000003F, PIOE, ID_PIOE, PIO_PERIPH_A, PIO_DEFAULT}, /* D8_13  */        \
  {0x00018000, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}, /* D14_15 */        \
  {0x3FF00000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* A0_9   */        \
  {0x00002000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* SDA10  */        \
  {0x00020000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* CAS    */        \
  {0x00010000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAS    */        \
  {0x00004000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* SDCKE  */        \
  {0x00800000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* SDCK   */        \
  {0x00008000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* SDSC   */        \
  {0x00040000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* NBS0   */        \
  {0x00008000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* NBS1   */        \
  {0x20000000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* SDWE   */        \
  {0x00100000, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}  /* BA0    */        \
                                                                              \
  /* Trace port */                                                            \
  {0x000000F0, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}, /* TRACE0..3 */     \
  {0x00000100, PIOC, ID_PIOC, PIO_PERIPH_D, PIO_DEFAULT}, /* TRACECLK */      \
                                                                              \
  /* SD-Card */                                                               \
  /* FIXME: Adapt */                                                          \
  {PIO_PA25D_MCCK, PIOA, ID_PIOA, PIO_PERIPH_D, PIO_DEFAULT},                 \
  {PIO_PA26C_MCDA2 | PIO_PA27C_MCDA3 | PIO_PA28C_MCCDA | PIO_PA30C_MCDA0 |    \
      PIO_PA31C_MCDA1, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT},             \
  {PIO_PD18, PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT}, /* DETECT */

#define GRISP_MATRIX_CCFG_SYSIO 0x20400000

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GRISP_PIN_CONFIG_H */
