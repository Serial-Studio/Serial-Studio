; Index of most-significant bit

.text
.globl msb_index
msb_index:
    pushl %ebx
    pushl %ecx
    pushl %edx
    bsrl %eax,%eax
    popl %edx
    popl %ecx
    popl %ebx
    ret
