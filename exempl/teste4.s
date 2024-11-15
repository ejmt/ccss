
.text
_main:
                                pushl %ebp
                                movl  %esp, %ebp
                                subl  $12, %esp
                                movl  $1, %ebx
                                movl  %ebx, -4(%ebp)
                                movl  $2, %ebx
                                movl  %ebx, -8(%ebp)
                                jmp   L1
.data
L0:
.byte 'a'
.byte 0

.text
L1:
                                leal  L0, %ebx
                                movl  %ebx, -12(%ebp)
                                movl  -4(%ebp), %ebx
                                movl  -8(%ebp), %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, %eax
                                movl  %ebp, %esp
                                popl  %ebp
                                ret
.global _start
_start:
                                call  _main
                                movl  %eax, %ebx
                                movl  $1, %eax
                                int   $0x80
