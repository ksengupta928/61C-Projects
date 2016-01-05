addi $s0, $0, 0xff
lui $s0, 0xffff
ori $s0, $s0, 0x0101
sb $s0, 4($0)
lb $s1, 4($0)
lbu $s1, 4($0)
addi $s0, $0, 0x1000
addi $s2, $0, 0
swinc $s0, 8($s2)
addi $s0, $0, 0
lw $s0, 0($s2)