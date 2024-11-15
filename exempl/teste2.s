
.text
_func:
                                pushl %ebp
                                movl  %esp, %ebp
                                movl  8(%ebp), %ebx
                                movl  12(%ebp), %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, %eax
                                movl  %ebp, %esp
                                popl  %ebp
                                ret

.text
_main:
                                pushl %ebp
                                movl  %esp, %ebp
                                subl  $12, %esp
                                movl  $1, %ebx
                                movl  %ebx, -4(%ebp)
                                movl  $2, %ebx
                                movl  %ebx, -8(%ebp)
                                subl  $8, %esp
                                movl  -4(%ebp), %ebx
                                movl  $1, %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, 0(%esp)
                                movl  -8(%ebp), %ebx
                                movl  $2, %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, 4(%esp)
                                call  _func
                                addl  $8, %esp
                                movl  %eax, %ebx
                                movl  %ebx, -12(%ebp)
                                movl  -12(%ebp), %ebx
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
