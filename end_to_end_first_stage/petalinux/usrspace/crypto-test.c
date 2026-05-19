/*
* Copyright (C) 2013-2022  Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in this
* Software without prior written authorization from Xilinx.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>


#define DEVICE_PATH "/dev/crypto-driver"
#define REG_SIZE    4096  // Zynq 架构通常映射一个内存页 (4KB), but  the reg size in devicetree

/* 💡 精准对齐你的 Verilog addr 映射：
 * 8'h00 -> ctrl
 * 8'h04 -> data
 * 8'h08 -> key
 * 8'h0C -> result
 */
struct crypto_regs {
    volatile uint32_t ctrl;   // 偏移量 0x00 (对应 8'h00)
    volatile uint32_t data;   // 偏移量 0x04 (对应 8'h04)
    volatile uint32_t key;    // 偏移量 0x08 (对应 8'h08)
    volatile uint32_t result; // 偏移量 0x0C (对应 8'h0C)
};

int main(int argc, char *argv[]) {
    int fd;
    void *map_base = NULL;
    struct crypto_regs *regs = NULL;

    printf("=========================================\n");
    printf("     Crypto Verilog IP Userspace Test    \n");
    printf("=========================================\n");

    // 1. 打开驱动创建的字符设备节点
    fd = open(DEVICE_PATH, O_RDWR | O_SYNC);
    if (fd < 0) {
        perror(" 打开设备节点失败，请检查驱动是否加载");
        return EXIT_FAILURE;
    }

    // 2. 物理地址映射到虚拟地址
    map_base = mmap(NULL, REG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_base == MAP_FAILED) {
        perror(" mmap 映射失败");
        close(fd);
        return EXIT_FAILURE;
    }

    // 3. 将虚拟地址指针转换为匹配你 Verilog 寄存器的结构体指针
    regs = (struct crypto_regs *)map_base;

    printf("\n--- 开始按硬件时序进行数据写入 ---\n");

    // 写入你的加密因子 (Key 和 Data)
    uint32_t my_data = 0xAAAA5555;
    uint32_t my_key  = 0x12345678;

    printf("[写] 往 8'h04 (data) 写入数据: 0x%08X\n", my_data);
    regs->data = my_data; // 触发 Verilog 的 case(8'h04): data <= wdata;

    printf("[写] 往 8'h08 (key)  写入密钥: 0x%08X\n", my_key);
    regs->key = my_key;   // 触发 Verilog 的 case(8'h08): key  <= wdata;

    __asm__ __volatile__("" ::: "memory");  //forbit store reorder load reorder cache-based optimization assumpions
    __sync_synchronize();
    // 4. 触发计算 (你的 Verilog 中，if (ctrl[0]) 会执行 key ^ data)
    printf("[控] 往 8'h00 (ctrl) 写入启动信号 0x00000001 (ctrl[0]=1)...\n");
    regs->ctrl = 0x00000001; 
	usleep(1000);
    // 5. 读取结果
    // 因为你的异或运算是组合逻辑/单时钟触发，在我们读取时硬件一定已经算完了
    asm volatile ("" ::: "memory");
    uint32_t crypto_res = regs->result; // 触发 Verilog 的 case(8'h0C): rdata = result;
    
    printf("\n--- 读取硬件反馈结果 ---\n");
    printf("[读] 从 8'h0C (result) 读出的异或值: 0x%08X\n", crypto_res);

    // 验证软件算出来的结果是否跟硬件一致
    uint32_t software_res = my_key ^ my_data;
    if (crypto_res == software_res) {
        printf(" 测试成功！硬件异或计算与软件预期完全一致！\n");
    } else {
        printf(" 测试失败！硬件返回 0x%08X，但预期为 0x%08X\n", crypto_res, software_res);
    }

    // 6. 释放资源
    munmap(map_base, REG_SIZE);
    close(fd);
    return EXIT_SUCCESS;
}

    
