addu $0, $0, $0
addu $0, $0, $0
addiu $s0 $0 -2
lui $at, 0x8000
ori $s1, $at, 0x8000
ori $s0 $s0 -1
j Loop
lui $at, 0x0000
ori $a0, $at, 0x000a
addu $s2, $s0, $0
addu $s0 $s0 $s1
slt $at, $s0, $s1
bne $0, $at, BLT
slt $at, $s0, $s1
beq $0, $at, BGT
addu $at, $s3, $s1
addu $s3, $at, $s2
addu $at, $0, $s2
addu $s2, $0, $t0
addu $t0, $0, $at
lui $s0 -1
mult $sp, $s2
mflo $s0
div $a3, $s2
mflo $a2
div $v0, $s1
mfhi $s1
or $v0 $v0 $s2
sltu $v0 $v0 $s2
jr $ra
sll $s2 $t3 3
lb $t0 0 $v0
lbu $t0 1 $v0
lw $t0 2 $v0
sb $t0 3 $v0
sw $t0 7 $v0
jal BLR
