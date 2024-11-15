
.text
_func:
                                pushl %ebp
                                movl  %esp, %ebp
                                movl  %ebp, %esp
                                popl  %ebp
                                ret

.text
_main:
                                pushl %ebp
                                movl  %esp, %ebp
                                subl  $4, %esp
                                subl  $16, %esp
                                movl  $1, %ebx
                                movl  %ebx, 0(%esp)
                                jmp   L1
.data
L0:
.byte 'a'
.byte 0

.text
L1:
                                leal  L0, %ebx
                                movl  %ebx, 4(%esp)
                                movl  $2, %ebx
                                movl  %ebx, 8(%esp)
                                jmp   L3
.data
L2:
.byte 'b'
.byte 0

.text
L3:
                                leal  L2, %ebx
                                movl  %ebx, 12(%esp)
                                call  _func
                                addl  $16, %esp
                                movl  %eax, %ebx
                                movl  %ebx, -4(%ebp)
                                movl  -4(%ebp), %ebx
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
