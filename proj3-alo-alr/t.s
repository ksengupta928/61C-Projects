addi $s0, $0, 0x30df
andi $s0, $s0, 0xf01f
addi $s1, $0, 0x3ffa
add $s2, $0, $0
addi $s2, $s2, 1
again:addi $s2, $s2, -1
addiu $s1, $0, 0x800f
addu $s1, $s1, $s0
and $s1, $s0, $s2
beq $s2, $0, again

beqal $s2, $0, smth
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