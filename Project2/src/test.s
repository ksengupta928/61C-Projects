


BGT:
move $0, $0
move $0, $0
BSS:
addiu $s0, $0, -2
   Loop: li $s1, 0x80008000



ori $s0, $s0, -1
j Loop
li $a0, 10
move $s2, $s0

#comment


addu $s0, $s0, $s1
blt $s0,            $s1,           BLT
bgt $s0, $s1, BGT    #commennntnt



traddu $s3, $s1, $s2
swpr $s2, $t0
lui $s0       , -1
mul $s0, $sp, $s2
div $a2, $a3, $s2
rem $s1, $v0, $s1
BLT: or $v0, $v0, $s2
sltu $v0, $v0, $s2
jr $ra
sll $s2 $t3 3
lb $t0 0($v0)
lbu $t0 1($v0)
lw $t0 2($v0)
sb $t0 3($v0)
sw $t0 7($v0)
jal BLR
