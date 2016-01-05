addi $s0, $0, 1
addi $s1, $0, -1
addi $s2, $0, 1
slti $s0, $s0, -1
slt $s2, $s0, $s1
sltu $s2, $s0, $s1
sll $s2, $s2, 3
lui $s2, 0xffff
ori $s2, $s2, 0xffff
sra $s1, $s2, 3
srl $s1, $s2, 3
addi $s0, $0, 0xf
sw $s2, 0($0)
sb $s0, 4($0)
addi $s0, $0, 0x10
sh $s0, 5($0)
or $s0, $s1, $s2
ori $s1, $s0, 0xfed1
lb $s0, 4($0)
lbu $s0, 4($0)
lhu $s0, 5($0)
lui $s0, 0xf00f
lw $s0, 0($0)
addi $s0, $0, 0x1000
add $s1, $0, $0
swinc $s0, 8($s1)
add $s0, $0, $0
lw $s0, 0($s1)