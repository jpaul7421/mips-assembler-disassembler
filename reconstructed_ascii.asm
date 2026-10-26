# Reconstructed by disassembler_cpp
.data
# data starts at 0x10010000
.byte 70, 105, 122, 122, 0, 66, 117, 122, 122, 0, 70, 105, 122, 122, 66, 117, 122, 122, 0

.text
# text starts at 0x00400000
addiu $t1, $zero, 1
addiu $t2, $zero, 101
addiu $t3, $zero, 3
addiu $t4, $zero, 5
addiu $t5, $zero, 15
L2:
j L0
L12:
j L1
L13:
addiu $t1, $t1, 1
bne $t1, $t2, L2
j L3
L0:
div $t1, $t5
mfhi $t6
beq $t6, $zero, L4
div $t1, $t3
mfhi $t6
beq $t6, $zero, L5
div $t1, $t4
mfhi $t6
beq $t6, $zero, L6
j L7
L9:
j L12
L7:
addiu $v0, $zero, 1
addu $a0, $t1, $zero
syscall
j L9
L4:
addiu $v0, $zero, 4
lui $a0, 0x1001
ori $a0, $a0, 0x000A
syscall
j L12
L5:
addiu $v0, $zero, 4
lui $a0, 0x1001
ori $a0, $a0, 0x0000
syscall
j L12
L6:
addiu $v0, $zero, 4
lui $a0, 0x1001
ori $a0, $a0, 0x0005
syscall
j L12
L1:
addiu $a0, $zero, 10
addiu $v0, $zero, 11
syscall
j L13
L3:
addiu $v0, $zero, 10
syscall
