# Module 5: MIPS Assembler and Disassembler

## Project files

- `fizzbuzz.asm` — adapted FizzBuzz source using real MIPS instructions only. The pseudoinstructions `li`, `la`, and `move` were replaced with real instruction sequences.
- `assembler.cpp` — assembles the supported MIPS source into MHC-ASCII and MHC-BIN files.
- `disassembler.cpp` — reads either MHC format and reconstructs MIPS assembly.
- `reconstructed_ascii.asm` — assembly reconstructed from the ASCII machine-code file.
- `reconstructed_bin.asm` — assembly reconstructed from the binary machine-code file.
- `fizzbuzz.mhc-ascii` and `fizzbuzz.mhc-bin` — machine-code outputs produced by the assembler.

## Supported features

The assembler supports the following MIPS instructions:

- R-type: `add`, `addu`, `sub`, `div`, `mfhi`, `syscall`
- I-type: `addi`, `addiu`, `ori`, `lui`, `beq`, `bne`
- J-type: `j`

It also supports the `.data`, `.text`, `.asciiz`, and `.byte` directives. Labels are supported for jumps and branches. The symbolic expressions `%hi(label)` and `%lo(label)` are supported for constructing addresses.

Invalid registers, unknown labels, unsupported instructions, malformed directives, unaligned targets, and out-of-range values produce an error message identifying the problem.

## Memory layout and byte order

- Text starts at `0x00400000`.
- Data starts at `0x10010000`.
- Instructions and data are stored in big-endian order.

The MHC-BIN file begins with a header containing a magic value, version, text start address, data start address, and the lengths of the text and data sections. This preserves the distinction between instructions and data when the file is disassembled.

The MHC-ASCII file stores readable binary digits. Each 32-bit instruction uses 32 text characters, while each data byte uses 8 text characters. The MHC-BIN file stores the actual bytes, so each 32-bit instruction uses only four bytes.

## Compile and run

From the project directory, compile both programs:

```bash
g++ -std=c++17 assembler.cpp -o assembler
g++ -std=c++17 disassembler.cpp -o disassembler
```

Assemble the original source:

```bash
./assembler fizzbuzz.asm original_output
```

This creates:

```text
original_output/fizzbuzz.mhc-ascii
original_output/fizzbuzz.mhc-bin
```

Disassemble both output formats:

```bash
./disassembler original_output/fizzbuzz.mhc-ascii reconstructed_ascii.asm
./disassembler original_output/fizzbuzz.mhc-bin reconstructed_bin.asm
```

Reassemble both reconstructed programs:

```bash
./assembler reconstructed_ascii.asm ascii_roundtrip
./assembler reconstructed_bin.asm binary_roundtrip
```

Verify that the reconstructed programs produce identical machine-code bytes:

```bash
cmp original_output/fizzbuzz.mhc-bin ascii_roundtrip/fizzbuzz.mhc-bin
cmp original_output/fizzbuzz.mhc-bin binary_roundtrip/fizzbuzz.mhc-bin
```

The `cmp` commands produce no output when the files match. The verification for this project passed for both reconstructed programs.

## Part 1: Hand assembly

`addi` is an I-type instruction with the fields `opcode | rs | rt | immediate`.

| Instruction | Fields | 32-bit binary | Hexadecimal |
|---|---|---|---|
| `addi $t0, $zero, 3` | `001000 | 00000 | 01000 | 0000000000000011` | `00100000000010000000000000000011` | `0x20080003` |
| `addi $t1, $zero, 5` | `001000 | 00000 | 01001 | 0000000000000101` | `00100000000010010000000000000101` | `0x20090005` |

`add` and `sub` are R-type instructions with the fields `opcode | rs | rt | rd | shamt | funct`. For both instructions, the opcode and shift amount are zero.

| Instruction | Fields | 32-bit binary | Hexadecimal |
|---|---|---|---|
| `add $t2, $t0, $t1` | `000000 | 01000 | 01001 | 01010 | 00000 | 100000` | `00000001001010010101000000100000` | `0x01295020` |
| `sub $t3, $t1, $t0` | `000000 | 01001 | 01000 | 01011 | 00000 | 100010` | `00000001001010000101100000100010` | `0x01285822` |

## Part 2: Machine-code bytes

Using big-endian byte order, the four instructions are represented as follows:

| Instruction | Four binary bytes | Four hexadecimal bytes |
|---|---|---|
| `addi $t0, $zero, 3` | `00100000 00001000 00000000 00000011` | `20 08 00 03` |
| `addi $t1, $zero, 5` | `00100000 00001001 00000000 00000101` | `20 09 00 05` |
| `add $t2, $t0, $t1` | `00000001 00101001 01010000 00100000` | `01 29 50 20` |
| `sub $t3, $t1, $t0` | `00000001 00101000 01011000 00100010` | `01 28 58 22` |

The ASCII format contains visible `0` and `1` characters. The binary format contains the actual byte values. Therefore, one 32-bit instruction takes 32 text characters in MHC-ASCII, excluding line breaks, but only four bytes in MHC-BIN.

## Test result

The project assembled 46 instructions and 19 data bytes. Both reconstructed assembly files were successfully reassembled, and both byte-for-byte comparisons passed.

## Collaboration Acknowledgment

ChatGPT 6.1 Sol was used as a programming aid for portions of the C++ implementation and debugging. I reviewed, compiled, tested, and verified the final project myself.
