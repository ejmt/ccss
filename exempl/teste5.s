
.text
_somar:
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
                                movl  $1234, %ebx
                                movl  %ebx, -4(%ebp)
                                movl  $4321, %ebx
                                movl  %ebx, -8(%ebp)
                                subl  $8, %esp
                                movl  -4(%ebp), %ebx
                                movl  %ebx, 0(%esp)
                                movl  -8(%ebp), %ebx
                                movl  %ebx, 4(%esp)
                                call  _somar
                                addl  $8, %esp
                                movl  %eax, %ebx
                                movl  %ebx, -12(%ebp)
                                subl  $4, %esp
                                jmp   L1
.data
L0:
.byte 'a',' ','s','o','m','a',' '
.byte 0

.text
L1:
                                leal  L0, %ebx
                                movl  %ebx, 0(%esp)
                                call  _puts
                                addl  $4, %esp
                                subl  $4, %esp
                                movl  -4(%ebp), %ebx
                                movl  %ebx, 0(%esp)
                                call  _putint
                                addl  $4, %esp
                                subl  $4, %esp
                                jmp   L3
.data
L2:
.byte '+'
.byte 0

.text
L3:
                                leal  L2, %ebx
                                movl  %ebx, 0(%esp)
                                call  _puts
                                addl  $4, %esp
                                subl  $4, %esp
                                movl  -8(%ebp), %ebx
                                movl  %ebx, 0(%esp)
                                call  _putint
                                addl  $4, %esp
                                subl  $4, %esp
                                jmp   L5
.data
L4:
.byte '='
.byte 0

.text
L5:
                                leal  L4, %ebx
                                movl  %ebx, 0(%esp)
                                call  _puts
                                addl  $4, %esp
                                subl  $4, %esp
                                movl  -12(%ebp), %ebx
                                movl  %ebx, 0(%esp)
                                call  _putint
                                addl  $4, %esp
                                subl  $4, %esp
                                jmp   L7
.data
L6:
.byte '\','n'
.byte 0

.text
L7:
                                leal  L6, %ebx
                                movl  %ebx, 0(%esp)
                                call  _puts
                                addl  $4, %esp
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
