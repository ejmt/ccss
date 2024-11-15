
.text
_main:
                                pushl %ebp
                                movl  %esp, %ebp
                                subl  $8, %esp
                                movl  $123, %ebx
                                movl  %ebx, -4(%ebp)
                                jmp   L1
.data
L0:
.byte 'A'
.byte 0

.text
L1:
                                leal  L0, %ebx
                                movl  %ebx, -8(%ebp)
                                subl  $12, %esp
                                jmp   L3
.data
L2:
.byte 'i',' ','=',' ','%','d',' ','e',' ','c',' ','=',' ','%','c','\','n'
.byte 0

.text
L3:
                                leal  L2, %ebx
                                movl  %ebx, 0(%esp)
                                movl  -4(%ebp), %ebx
                                movl  %ebx, 4(%esp)
                                movl  -8(%ebp), %ebx
                                movl  %ebx, 8(%esp)
                                call  _printf
                                addl  $12, %esp
                                movl  $0, %ebx
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
