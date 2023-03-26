.text
mov x1, 0x1000      
lsl X1, X1, 16
mov x2, 0x1001 
lsl X2, X2, 16
mov x3, 0x1002
lsl X3, X3, 16
mov x4, 0x1003
lsl X4, X4, 16
mov x5, 0x1004
lsl X5, X5, 16
mov x6, 0x1005
lsl X6, X6, 16
mov x7, 0x1006
lsl X7, X7, 16
mov x8, 0x1007
lsl X8, X8, 16
mov x9, 0x1008
lsl X9, X9, 16
mov x10, 0x1009
lsl X10, X10, 16
mov X11, 0x10
mov X12, 0x20
mov X13, 0x30
mov X14, 0x40
mov X15, 0x50
mov X16, 0x60
mov X17, 0x70
mov X18, 0x80
mov X19, 0x90
mov X20, 0x10

stur X11, [x1, 0x24]
stur X21, [x1, 0x24]
stur X12, [x2, 0x0]
stur X22, [x2, 0x0]
stur X13, [x3, 0x0]
stur X23, [x3, 0x0]
stur X14, [x4, 0x0]
stur X24, [x4, 0x0]
stur X15, [x5, 0x0]
stur X25, [x5, 0x0]
stur X16, [x6, 0x0]
stur X26, [x6, 0x0]
stur X17, [x7, 0x0]
stur X27, [x7, 0x0]
stur X18, [x8, 0x0]
stur X28, [x8, 0x0]
stur X19, [x9, 0x0]
stur X29, [x9, 0x0]
stur x20, [x10, 0x0]
stur X30, [x10, 0x0]

add x8, x6, x7
add x9, x8, x2
hlt 0

