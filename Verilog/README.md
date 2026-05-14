
# Crypto XOR Module - Verilog Project

This project implements a simple hardware-based encryption/decryption demonstration module using Verilog. It features a register-based bus interface to perform bitwise XOR operations.

## Project Structure
* **crypto.v**: The core logic module containing control, data, key, and result registers.
* **test_crypto.v**: The Testbench used to simulate clock/reset cycles, write data to the bus, and verify the XOR results.

## Register Map
The module utilizes an 8-bit address line and a 32-bit data line:

| Address | Name   | Description |
| :--- | :--- | :--- |
| `8'h00` | **CTRL** | Control Register. `bit[0]` enables calculation; `bit[1]` resets the result. |
| `8'h04` | **DATA** | Input data to be processed. |
| `8'h08` | **KEY** | Secret key used for encryption/decryption. |
| `8'h0C` | **RESULT** | Read-only register containing the `DATA ^ KEY` result. |

## Functional Description
1. **Writing Data**: Set `we` (Write Enable) and the target address to load data and keys into the registers.
2. **Starting Computation**: Write `1` to the 0th bit of the `CTRL` register.
3. **Reading Results**: The module calculates the XOR value immediately. Read from address `8'h0C` to retrieve the output.

## Simulation & Verification
The testbench automatically performs the following:
1. Writes data `32'hAAAA_5555`.
2. Writes key `32'h1234_5678`.
3. Enables calculation and reads address `8'h0C`.
4. Compares hardware results with expected software values and prints `SUCCESS` or `ERROR`.

## How to Run
Use any Verilog-2001 compliant simulator (e.g., Icarus Verilog, Vivado, or ModelSim):
```bash
# Example using iVerilog
iverilog -o sim_out test_crypto.v crypto.v
vvp sim_out
