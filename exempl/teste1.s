
.text
_func:
                                pushl %ebp
                                movl  %esp, %ebp
                                movl  8(%ebp), %ebx
                                movl  %ebx, %eax
                                movl  %ebp, %esp
                                popl  %ebp
                                ret

.text
_main:
                                pushl %ebp
                                movl  %esp, %ebp
                                subl  $4, %esp
                                jmp   L1
.data
L0:
.byte 'B'
.byte 0

.text
L1:
                                leal  L0, %ebx
                                movl  %ebx, 0(%esp)
                                call  _func
                                addl  $4, %esp
                                movl  %eax, %ebx
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
