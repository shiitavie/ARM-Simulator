.text
mov X9, 0x1000
lsl X9, X9, 0x10
adds X8, X1, 0x1
adds X10, X9, X8
mov X1, 0x1000
lsl X1, X1, 16
mov X10, 0x1234
stur X10, [X1, 0x1]
sturb W10, [X1, 0x2]
ldur X13, [X1, 0x0]
mov X3, 0x1
mov X4, 0x2
b foo
add X13, X0, 10
foo:
add X14, X9, 11
b bar
bar:
add X15, X2, X8



cmp X3, X4
beq bcnot
add X16, X16, 1
add X16, X16, 2

bcnot:
add X16, X16, 3
add X16, X16, 4
cmp X3, X3
beq bctake
add X17, X17, 1
add X17, X17, 2

bctake:
add X17, X17, 3
add X17, X17, 4



cmp X3, X4
bne bnnot
add X18, X18, 1
add X18, X18, 2

bnnot:
add X18, X18, 3
add X18, X18, 4

cmp X3, X3
bne bntake
add X19, X19, 1
add X19, X19, 2

bntake:
add X19, X19, 3
add X19, X19, 4

ldur X14, [X1, 0x3]
ldurb W15, [X1, 0x1]
adds X11, X10, 0xff
ldur X10, [X11,0x4]
adds X12, X10, 1
adds X13, X11, 4
HLT 0
