l: addi $s0, $s0, 1
beq $0, $0, l
addi $s2, $s2, 1
again:addi $s2, $s2, -1
beq $s2, $0, again
addi $s2, $s2, 1
addi $s2, $s2, 1