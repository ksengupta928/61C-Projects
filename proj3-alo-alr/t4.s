add $s2, $0, $0
beqal $s2, $0, smth

bne $s1, $s2, jum
d: 
addi $s1, $0, 0x7
addi $s2, $0, 0x5
divu $s1, $s2
jal Ex

smth:
addi $s1, $0, 0x7fff
addi $s2, $0, 0x4
multu $s2, $s1
mfhi $s0
mflo $s0
jr $ra

jum:
j d
Ex: