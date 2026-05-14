# Hardware Validation of Zynq-7000 AXI-Lite Crypto IP#

This project implements a custom hardware accelerator for basic XOR-based encryption/decryption. The IP is designed with an **AXI-Lite interface**, allowing a Zynq-7000 processing system (PS) to control hardware registers via software.

## Project Structure##

```text```
.
├── Verilog
│   ├── crypto.v           # RTL implementation of the Crypto IP
│   ├── test_crypto.v      # Verilog Testbench for simulation
│   └── README.md          # RTL-specific notes
├── standalone_test
│   ├── vitis              # C Drivers and standalone application
│   │   ├── helloworld.c   # Main validation software
│   │   ├── lscript.ld     # Linker script
│   │   └── platform.c     # Platform initialization
│   └── vivado             # Hardware handoff files
│       ├── design_1.hwh   # Hardware description
│       ├── design_1_wrapper.bit  # FPGA Bitstream
│       └── design_1_wrapper.xsa  # Hardware Specification for Vitis
└── Vivado_xsct_test       # Validation documentation and screenshots
    ├── block_design.png   # Vivado Block Design screenshot
    └── vivado_validation.png # XSCT Console validation


## Hardware Architecture: The AXI-Lite Wrapper##

The core of this project is the **AXI-Lite Wrapper** integrated into `crypto.v`. This serves as the communication bridge between the ARM Processor (PS) and the FPGA Logic (PL)[cite: 1]. It allows the CPU to treat hardware registers as simple memory addresses.

### 1. The Communication Bridge##
The module implements a standard **AXI4-Lite Slave interface**, which is optimized for low-latency, register-based communication[cite: 1]. 
*   **Memory-Mapped I/O:** The Zynq Processing System (PS) accesses the Crypto IP via a specific base address (e.g., `0x43C00000`) defined in the Vivado Address Editor[cite: 1].
*   **32-bit Data Width:** All control, data, and result registers are 32-bit wide to align with the ARM Cortex-A9 architecture[cite: 1].

### 2. Internal Register Map###
The wrapper maps AXI bus transactions to four internal hardware registers[cite: 1]:

| Offset | Register | Access | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | **CTRL** | R/W | **Control:** Bit 0 = Start Engine; Bit 1 = Reset Result[cite: 1]. |
| `0x04` | **DATA** | R/W | **Input:** Holds the 32-bit word to be encrypted/decrypted[cite: 1]. |
| `0x08` | **KEY** | R/W | **Secret:** The 32-bit key used for the XOR operation[cite: 1]. |
| `0x0C` | **RESULT** | RO | **Output:** Holds the processed `DATA ^ KEY` value[cite: 1]. |

### 3. Data Flow & Synchronization###`
The AXI-Lite wrapper manages the handshaking signals (`s_axi_awvalid`, `s_axi_wready`, etc.) to ensure reliable data transfer[cite: 1]:
1.  **Write Logic:** When the PS sends a write request, the wrapper decodes the `addr` and updates the corresponding register (`ctrl`, `data`, or `key`) on the next `clk` edge[cite: 1].
2.  **Execution:** Once the `ctrl[0]` bit is high, the hardware logic performs the XOR operation in a single clock cycle[cite: 1].
3.  **Read Logic:** When the PS reads from offset `0x0C`, a multiplexer (Read Mux) routes the `result` register back onto the AXI `rdata` bus[cite: 1].

---

## Standalone Validation (Hardware-in-the-Loop) ##

The `standalone_test` directory contains the files used to verify the AXI-Lite bridge and RTL logic on physical hardware[cite: 1].

### The Workflow###:
1.  **Bitstream Generation:** The `crypto.v` core was packaged as a custom IP and integrated into a Vivado block design[cite: 1].
2.  **Hardware Handoff:** The `.xsa` file was exported to Vitis, which contains the hardware's address map[cite: 1].
3.  **Software Driver (`helloworld.c`):** 
    *   Directly writes to the memory-mapped offsets mentioned above[cite: 1].
    *   Reads back the hardware result and compares it against a software XOR calculation to ensure the AXI bridge is transparent and accurate[cite: 1].
    *   Validation is confirmed via UART terminal output[cite: 1].
