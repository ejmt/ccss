
.text
_main:
                                pushl %ebp
                                movl  %esp, %ebp
                                subl  $12, %esp
                                movl  $123456, %ebx
                                movl  %ebx, -4(%ebp)
                                movl  $654321, %ebx
                                movl  %ebx, -8(%ebp)
                                movl  -4(%ebp), %ebx
                                movl  -8(%ebp), %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, -12(%ebp)
                                subl  $16, %esp
                                jmp   L1
.data
L0:
.byte '%','d','+','%','d','=','%','d','\','n'
.byte 0

.text
L1:
                                leal  L0, %ebx
                                movl  %ebx, 0(%esp)
                                movl  -4(%ebp), %ebx
                                movl  %ebx, 4(%esp)
                                movl  -8(%ebp), %ebx
                                movl  %ebx, 8(%esp)
                                movl  -12(%ebp), %ebx
                                movl  %ebx, 12(%esp)
                                call  _printf
                                addl  $16, %esp
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
