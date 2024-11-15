
.text
_main:
                                pushl %ebp
                                movl  %esp, %ebp
                                subl  $8, %esp
                                subl  $4, %esp
                                jmp   L1
.data
L0:
.byte 'S','e','r','a','h',' ','i','m','p','r','e','s','s','o',' '
.byte 0

.text
L1:
                                leal  L0, %ebx
                                movl  %ebx, 0(%esp)
                                call  _puts
                                addl  $4, %esp
                                subl  $4, %esp
                                movl  $123, %ebx
                                movl  %ebx, 0(%esp)
                                call  _putint
                                addl  $4, %esp
                                subl  $4, %esp
                                jmp   L3
.data
L2:
.byte '\','n'
.byte 0

.text
L3:
                                leal  L2, %ebx
                                movl  %ebx, 0(%esp)
                                call  _puts
                                addl  $4, %esp
                                movl  %ebp, %esp
                                popl  %ebp
                                ret
.global _start
_start:
                                call  _main
                                movl  %eax, %ebx
                                movl  $1, %eax
                                int   $0x80
