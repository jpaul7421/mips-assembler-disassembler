# Module 5 FizzBuzz source: real MIPS instructions only.
# Target: MIPS32, big-endian machine-code words, text at 0x00400000,
# data at 0x10010000.  This source is intended for assembler.py.

.data
fizz:     .asciiz "Fizz"
buzz:     .asciiz "Buzz"
fizzbuzz: .asciiz "FizzBuzz"

.text
addiu $t1, $zero, 1
addiu $t2, $zero, 101
addiu $t3, $zero, 3
addiu $t4, $zero, 5
addiu $t5, $zero, 15

loop1:
j PrintItem
PrintItemEnd:
j PrintNewLine
PrintNewLineEnd:
addiu $t1, $t1, 1
bne $t1, $t2, loop1
j Exit

PrintItem:
div $t1, $t5
mfhi $t6
beq $t6, $zero, PrintFizzBuzz
div $t1, $t3
mfhi $t6
beq $t6, $zero, PrintFizz
div $t1, $t4
mfhi $t6
beq $t6, $zero, PrintBuzz
j PrintInt
PrintIntEnd:
j PrintItemEnd

PrintInt:
addiu $v0, $zero, 1
addu $a0, $t1, $zero
syscall
j PrintIntEnd

PrintFizzBuzz:
addiu $v0, $zero, 4
lui $a0, %hi(fizzbuzz)
ori $a0, $a0, %lo(fizzbuzz)
syscall
j PrintItemEnd

PrintFizz:
addiu $v0, $zero, 4
lui $a0, %hi(fizz)
ori $a0, $a0, %lo(fizz)
syscall
j PrintItemEnd

PrintBuzz:
addiu $v0, $zero, 4
lui $a0, %hi(buzz)
ori $a0, $a0, %lo(buzz)
syscall
j PrintItemEnd

PrintNewLine:
addiu $a0, $zero, 10
addiu $v0, $zero, 11
syscall
j PrintNewLineEnd

Exit:
addiu $v0, $zero, 10
syscall
