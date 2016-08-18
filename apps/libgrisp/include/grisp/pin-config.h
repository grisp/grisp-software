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

#include <bsp/pin-config.h>

#define GRISP_PIN_CONFIG                                                      \
  /* Console */                                                               \
  {PIO_PD25C_URXD2 | PIO_PD26C_UTXD2,                                         \
      PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT},                              \
                                                                              \
  /* SDRAM */                                                                 \
  {0x000000FF, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* D0 .. D7 */      \
  {0x0000003F, PIOE, ID_PIOE, PIO_PERIPH_A, PIO_DEFAULT}, /* D8 .. D13 */     \
  {0x00018000, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}, /* D14 .. D15 */    \
  {0xBFF00000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* A0 .. A9, A11 */ \
  {0x00002000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* A10 */           \
  {0x00040000, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}, /* A12 */           \
  {0x00100000, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}, /* BA0 */           \
  {0x00008000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* CS */            \
  {0x00040000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}, /* LDQM / NBS0 */   \
  {0x00008000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* UDQM / NBS1 */   \
  {0x00004000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* CKE */           \
  {0x00010000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* RAS */           \
  {0x00020000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* CAS */           \
  {0x00800000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* CLK */           \
  {0x20000000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}, /* WE */            \
                                                                              \
  /* Trace port */                                                            \
  {0x000000F0, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}, /* TRACE0..3 */     \
  {0x00000100, PIOC, ID_PIOC, PIO_PERIPH_D, PIO_DEFAULT}, /* TRACECLK */      \
                                                                              \
  /* SD-Card */                                                               \
  {PIO_PA25D_MCCK, PIOA, ID_PIOA, PIO_PERIPH_D, PIO_DEFAULT},                 \
  {PIO_PA26C_MCDA2 | PIO_PA27C_MCDA3 | PIO_PA28C_MCCDA | PIO_PA30C_MCDA0 |    \
      PIO_PA31C_MCDA1, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT},             \
  {PIO_PD18A_SDCS, PIOD, ID_PIOD, PIO_PERIPH_A, PIO_DEFAULT}, /* SD_CD */     \
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
  /* Jumpers */                                                               \
  {PIO_PC14 | PIO_PC11 | PIO_PC10 | PIO_PC9 | PIO_PC8,                        \
      PIOC, ID_PIOC, PIO_INPUT, PIO_PULLUP},                                  \
                                                                              \
  /* LEDs */                                                                  \
  {PIO_PA24 | PIO_PA23 | PIO_PA22 | PIO_PA21 | PIO_PA19 | PIO_PA17,           \
      PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT},

#define GRISP_MATRIX_CCFG_SYSIO 0x20400000
