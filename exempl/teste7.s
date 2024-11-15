
.text
_somar:
                                pushl %ebp
                                movl  %esp, %ebp
                                movl  8(%ebp), %ebx
                                movl  12(%ebp), %ecx
                                addl  %ecx, %ebx
                                movl  16(%ebp), %ecx
                                addl  %ecx, %ebx
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
.byte 'T','e','s','t','e','.','\','n'
.byte 0

.text
L1:
                                leal  L0, %ebx
                                movl  %ebx, 0(%esp)
                                call  _puts
                                addl  $4, %esp
                                subl  $12, %esp
                                movl  $0, %ebx
                                movl  %ebx, 0(%esp)
                                movl  $1, %ebx
                                movl  %ebx, 4(%esp)
                                movl  $2, %ebx
                                movl  %ebx, 8(%esp)
                                call  _somar
                                addl  $12, %esp
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
