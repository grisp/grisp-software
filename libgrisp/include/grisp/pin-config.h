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

#ifndef GRISP_PIN_CONFIG_H
#define GRISP_PIN_CONFIG_H

#include <rtems.h>
#include <bsp/pin-config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GRISP_LED_1R	{PIO_PA24, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_1G	{PIO_PA17, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_1B	{PIO_PA23, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_2R	{PIO_PA13, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_2G	{PIO_PA5 , PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_LED_2B	{PIO_PA11, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_SAF_RESET	{PIO_PA12, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define GRISP_SAF_IRQ	{PIO_PC16, PIOC, ID_PIOC, PIO_INPUT,    PIO_PULLUP | PIO_IT_LOW_LEVEL}
#define GRISP_WLAN_EN	{PIO_PA22, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define GRISP_MODE_1	{PIO_PA6 , PIOA, ID_PIOA, PIO_INPUT,    PIO_PULLUP}
#define GRISP_MODE_2	{PIO_PD9 , PIOD, ID_PIOD, PIO_INPUT,    PIO_PULLUP}
#define GRISP_MODE_3	{PIO_PC17, PIOC, ID_PIOC, PIO_INPUT,    PIO_PULLUP}
#define GRISP_MODE_4	{PIO_PD11, PIOD, ID_PIOD, PIO_INPUT,    PIO_PULLUP}
#define GRISP_MODE_5	{PIO_PC10, PIOC, ID_PIOC, PIO_INPUT,    PIO_PULLUP}

#define GRISP_PIN_CONFIG                                                      \
  /* Console */                                                               \
  {PIO_PD25C_URXD2 | PIO_PD26C_UTXD2,                                         \
      PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT},                              \
                                                                              \
  /* SDRAM, EBI */                                                            \
  {0x000000FF, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* D0 .. D7 */      \
  {0x0000003F, PIOE, ID_PIOE, PIO_PERIPH_A, PIO_DEFAULT}, /* D8 .. D13 */     \
  {0x00018000, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}, /* D14 .. D15 */    \
  {0xFFF80000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* (SAF_A1 .. SAF_A13) == (n.u., RAM_A0 .. RAM_A9, n.u., RAM_A11) */ \
  {0x00002000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAM_A10 */       \
  {0x00040000, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}, /* RAM_A12 == SAF_A14 */ \
  {0x00180001, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}, /* (SAF_A15 .. SAF_A17) == (n.u., RAM_BA0, RAM_BA1) */ \
  {0x00008000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* RAM_CS */        \
  {0x00040000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* RAM_DQML / NBS0 */ \
  {0x00008000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAM_DQMH / NBS1 */ \
  {0x00004000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAM_CKE */       \
  {0x00010000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAM_RAS */       \
  {0x00020000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAM_CAS */       \
  {0x00800000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAM_CLK */       \
  {0x20000000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAM_WE */        \
  {PIO_PA1,    PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}, /* SAF_HC_DACK */   \
  GRISP_SAF_RESET,                                        /* SAF_RESET_N */   \
  {PIO_PC8A_NWE, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* SAF_WR */      \
  {PIO_PC11A_NRD, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* SAF_RD */     \
  {PIO_PC14A_NCS0, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* SAF_CS */    \
  GRISP_SAF_IRQ,                                          /* SAF_IRQ */       \
  {PIO_PD10,   PIOD, ID_PIOD, PIO_INPUT,    PIO_PULLUP }, /* SAF_DREQ */      \
                                                                              \
  /* Trace port */                                                            \
  {0x000000F0, PIOD, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}, /* TRACE0..3 */     \
  {0x00000100, PIOD, ID_PIOC, PIO_PERIPH_D, PIO_DEFAULT}, /* TRACECLK */      \
                                                                              \
  /* SD-Card */                                                               \
  {PIO_PA25D_MCCK, PIOA, ID_PIOA, PIO_PERIPH_D, PIO_DEFAULT},                 \
  {PIO_PA26C_MCDA2 | PIO_PA27C_MCDA3 | PIO_PA28C_MCCDA | PIO_PA30C_MCDA0 |    \
      PIO_PA31C_MCDA1, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT},             \
  {PIO_PD18, PIOD, ID_PIOD, PIO_INPUT, PIO_DEFAULT}, /* DETECT */             \
                                                                              \
  /* UART */                                                                  \
  {PIO_PB0C_RXD0 | PIO_PB1C_TXD0 | PIO_PB2C_CTS0 | PIO_PB3C_RTS0,             \
      PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT},                              \
                                                                              \
  /* I2C */                                                                   \
  {PIO_PA3A_TWD0 | PIO_PA4A_TWCK0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}, \
                                                                              \
  /* SPI */                                                                   \
  {PIO_PA2, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP},                            \
  {PIO_PB12, PIOB, ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT},                       \
  {PIO_PD12C_SPI0_NPCS2, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT},           \
  {PIO_PD20B_SPI0_MISO | PIO_PD21B_SPI0_MOSI | PIO_PD22B_SPI0_SPCK |          \
      PIO_PD27B_SPI0_NPCS3, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT},        \
                                                                              \
  /* WLAN Enable */                                                           \
  GRISP_WLAN_EN,                                                              \
                                                                              \
  /* Jumpers */                                                               \
  GRISP_MODE_1,                                                               \
  GRISP_MODE_2,                                                               \
  GRISP_MODE_3,                                                               \
  GRISP_MODE_4,                                                               \
  GRISP_MODE_5,                                                               \
                                                                              \
  /* LEDs */                                                                  \
  GRISP_LED_1R, GRISP_LED_1G, GRISP_LED_1B,                                   \
  GRISP_LED_2R, GRISP_LED_2G, GRISP_LED_2B

#define GRISP_MATRIX_CCFG_SYSIO 0x20400000

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GRISP_PIN_CONFIG_H */
