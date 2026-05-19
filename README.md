# Hardware Validation of Zynq-7000 AXI-Lite Crypto IP #

This project implements a custom hardware accelerator for basic XOR-based encryption/decryption. The IP is designed with an **AXI-Lite interface**, allowing a Zynq-7000 processing system (PS) to control hardware registers via software. 

The project has evolved from a bare-metal (**Standalone**) hardware-in-the-loop verification into a full-stack Embedded Linux environment (**PetaLinux**), featuring a custom **Linux Kernel Module (char device driver)** and a **userspace verification application**.

---

## Project Structure ##

```text
.
├── end_to_end_first_stage
│   ├── petalinux                    # Embedded Linux Full-Stack Files
│   │   ├── configs                  # Subsystem & Hardware Kconfig backups
│   │   │   ├── Kconfig
│   │   │   └── Kconfig.syshw
│   │   ├── driver                   # Custom Linux Character Device Driver Source
│   │   │   ├── crypto-driver.bb     # Yocto/PetaLinux BitBake Recipe
│   │   │   ├── crypto-driver.c      # Kernel Module handling MMIO and file operations
│   │   │   └── Makefile
│   │   ├── rootfsconfigs            # Root File System Configuration templates
│   │   ├── usrspace                 # Userspace Validation Application
│   │   │   ├── crypto-test.bb       # App BitBake Recipe
│   │   │   ├── crypto-test.c        # Tests hardware via /dev/crypto-driver
│   │   │   └── Makefile
│   │   ├── system.xsa               # Unified Xilinx Support Archive (Hardware Source)
│   │   └── Screenshot from 2026-05-18 21-28-07.png
│   ├── standalone_test              # Bare-Metal Hardware-in-the-loop Validation
│   │   ├── vitis                    # Bare-metal test stack (helloworld C source)
│   │   │   ├── helloworld.c
│   │   │   ├── lscript.ld
│   │   │   └── platform.c
│   │   └── vivado                   # Exported Hardware Hand-off data (.bit, .xsa)
│   │       ├── design_1.hwh
│   │       ├── design_1_wrapper.bit
│   │       └── design_1_wrapper.xsa
│   └── Verilog                      # Pure RTL Core Source Files
│       ├── crypto.v                 # Top-level module with AXI-Lite slave bridge
│       ├── README.md
│       └── test_crypto.v            # Initial simulation testbench
└── README.md                        # Master Documentation

## Hardware Architecture: The AXI-Lite Wrapper ##

The core of this project is the **AXI-Lite Wrapper** integrated into `crypto.v`. This serves as the communication bridge between the ARM Processor (PS) and the FPGA Logic (PL)[cite: 1]. It allows the CPU to treat hardware registers as simple memory addresses.

### 1. The Communication Bridge ##
The module implements a standard **AXI4-Lite Slave interface**, which is optimized for low-latency, register-based communication[cite: 1]. 
*   **Memory-Mapped I/O:** The Zynq Processing System (PS) accesses the Crypto IP via a specific base address (e.g., `0x43C00000`) defined in the Vivado Address Editor[cite: 1].
*   **32-bit Data Width:** All control, data, and result registers are 32-bit wide to align with the ARM Cortex-A9 architecture[cite: 1].

### 2. Internal Register Map ###
The wrapper maps AXI bus transactions to four internal hardware registers[cite: 1]:

| Offset | Register | Access | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | **CTRL** | R/W | **Control:** Bit 0 = Start Engine; Bit 1 = Reset Result[cite: 1]. |
| `0x04` | **DATA** | R/W | **Input:** Holds the 32-bit word to be encrypted/decrypted[cite: 1]. |
| `0x08` | **KEY** | R/W | **Secret:** The 32-bit key used for the XOR operation[cite: 1]. |
| `0x0C` | **RESULT** | RO | **Output:** Holds the processed `DATA ^ KEY` value[cite: 1]. |

### 3. Data Flow & Synchronization ###`
The AXI-Lite wrapper manages the handshaking signals (`s_axi_awvalid`, `s_axi_wready`, etc.) to ensure reliable data transfer[cite: 1]:
1.  **Write Logic:** When the PS sends a write request, the wrapper decodes the `addr` and updates the corresponding register (`ctrl`, `data`, or `key`) on the next `clk` edge[cite: 1].
2.  **Execution:** Once the `ctrl[0]` bit is high, the hardware logic performs the XOR operation in a single clock cycle[cite: 1].
3.  **Read Logic:** When the PS reads from offset `0x0C`, a multiplexer (Read Mux) routes the `result` register back onto the AXI `rdata` bus[cite: 1].

---

## Standalone Validation (Hardware-in-the-Loop) ##

The `standalone_test` directory contains the files used to verify the AXI-Lite bridge and RTL logic on physical hardware[cite: 1].

### The Workflow ###:
1.  **Bitstream Generation**:  The `crypto.v` core was packaged as a custom IP and integrated into a Vivado block design[cite: 1].
2.  **Hardware Handoff**:  The `.xsa` file was exported to Vitis, which contains the hardware's address map[cite: 1].
3.  **Software Driver (`helloworld.c`):** 
    *   Directly writes to the memory-mapped offsets mentioned above[cite: 1].
    *   Reads back the hardware result and compares it against a software XOR calculation to ensure the AXI bridge is transparent and accurate[cite: 1] using **Xil_out32 Xil_in32**.
      
## Full-Stack PetaLinux Bring-Up (Embedded OS) ##
To scale the project into a true production-like infrastructure, a custom embedded OS pipeline was built using Xilinx PetaLinux tools.

### 1. Device Tree Integration (system-user.dtsi) ###
The AXI Crypto IP block is registered into the Linux device tree framework under the AMBA bus loop. This notifies the Linux Kernel about the hardware's physical base address (0x43C00000) and address range layout during boot time.

### 2. Custom Linux Kernel Module (crypto-driver.c) ###
Instead of allowing userspace to touch dangerous raw memory, a robust Character Device Driver was engineered:

Resource Mapping: The driver claims the hardware memory region using request_mem_region() and maps it safely into virtual kernel memory spaces via ioremap().

File Operations Handlers: Implements standard POSIX filesystem operations (open, release, read, write, ioctl).

Memory Safety: Seamlessly handles the architectural boundaries between hardware registers, kernel memory space, and user memory spaces via proper streaming loops.

### 3. Userspace Testing Application (crypto-test.c) ###
A validation test application is baked directly into the PetaLinux root filesystem image (rootfs):

It accesses the physical crypto hardware smoothly by calling a standard open("/dev/crypto-driver", O_RDWR) system call.

It passes down arbitrary processing buffers, monitors execution status, retrieves the encrypted hardware results, and runs validation checkers to confirm full data-path precision under standard Linux process constraints.

