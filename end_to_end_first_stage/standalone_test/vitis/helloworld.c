/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

// #include <stdio.h>
#include "xil_printf.h"
#include "xil_io.h"
#include "xparameters.h"
#include "bspconfig.h"

#include "xuartps.h"
#include "xil_cache.h"

#define REG_BASE XPAR_CRYPTO_FIRST_IP_0_BASEADDR

#define ADDR_DATA   0x08
#define ADDR_KEY    0x04
#define ADDR_CTRL   0x00
#define ADDR_RESULT 0x0C



volatile u32 debug_reg_data;
volatile u32 debug_reg_key;
volatile u32 debug_reg_result;

int main()
{
    u32 data = 0xAAAA5555;
    u32 key  = 0x12345678;

    u32 result;

    // 写寄存器
    Xil_Out32(REG_BASE + ADDR_DATA, data);
    Xil_Out32(REG_BASE + ADDR_KEY, key);
    Xil_Out32(REG_BASE + ADDR_CTRL, 1);

    // 读寄存器
    result = Xil_In32(REG_BASE + ADDR_RESULT);

    // 👉 关键：保存到变量（debugger 可看）
    debug_reg_data   = data;
    debug_reg_key    = key;
    debug_reg_result = result;

    return 0;
}