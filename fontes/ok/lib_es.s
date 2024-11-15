/*----------------------------------*/
/* Autor: Elias Jose Moreira Torres */
/* <ejmt_ce@yahoo.com.br>           */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/

/*---------------------------------------*/
/* lib_es.asm - biblioteca de rotinas de */
/* entrada e saida do compilador CCSS    */
/* Autor: Elias Jose Moreira Torres      */
/* dezembro de 2005                      */
/*---------------------------------------*/

.data

.comm	Digitos, 11, 1

Caract:
.long 	0

Negativo:
.long 	0

Dez:
.long 	10

TamCad:
.long 	129

UltCar:
.long 	10

ProxLin:
.long 	10

Param:
.long	12

.text
/*----------------------------------------*/
/* rotinha para imprimir dados formatados */
/* ponteiro para o formato em 8(%ebp)     */
/*----------------------------------------*/
.global _printf
_printf:
                                pushl	%ebp
	                        movl  	%esp, %ebp
				pushl	%eax
				pushl	%ebx
				pushl	%ecx

				movl	8(%ebp), %ebx
printf_laco:
				movb  	(%ebx), %al
				cmpb  	$0, %al
				je    	printf_fim

				cmpb  	$'%', %al
				jne   	printf_continua
				incl  	%ebx
				movb  	(%ebx), %al
				cmpb  	$'d', %al				
				je    	printf_d
				cmpb  	$'c', %al				
				je    	printf_c
				movl	$'%', %eax
				movl	%eax, Caract
				movl	$Caract, %eax
				pushl	%eax
				call	_putchar
				addl	$4, %esp
				jmp	printf_laco
printf_d:			
				movl	Param, %ecx
				movl	(%ebp, %ecx, 1), %eax
				pushl	%eax
				call	_putint
				addl	$4, %esp
				incl  	%ebx
				addl	$4, Param
				jmp   	printf_laco
printf_c:			
				movl	Param, %ecx
				movl	(%ebp, %ecx, 1), %eax
				pushl	%eax
				call	_putchar
				addl	$4, %esp
				incl  	%ebx
				addl	$4, Param
				jmp   	printf_laco
printf_continua:
				cmpb  	$'\', %al
				jne   	printf_normal
				incl  	%ebx
				movb  	(%ebx), %al
				cmpb  	$'n', %al				
				je    	printf_n
				movl  	$'\\', Caract
				movl  	$Caract, %eax
				pushl 	%eax
				call  	_putchar
				addl  	$4, %esp
				jmp   	printf_normal
printf_n:			
				movl  	ProxLin, %eax
				movl  	%eax, Caract
				movl  	$Caract, %eax
				pushl 	%eax
				call  	_putchar
				addl  	$4, %esp
				incl  	%ebx
				jmp   	printf_laco
printf_normal:			
				pushl 	%ebx
				call  	_putchar
				addl  	$4, %esp
				incl  	%ebx
				jmp   	printf_laco
printf_fim:
				popl	%ecx
				popl	%ebx
				popl	%eax
				movl  	%ebp, %esp
                                popl  	%ebp
                                ret

/*--------------------------------------*/
/* rotina para ler cadeia de caracteres */
/* para buffer apontado por 8(%ebp)     */
/*--------------------------------------*/
.global _gets
_gets:
                                pushl 	%ebp
                                movl  	%esp, %ebp

				pushl 	%eax
				pushl 	%ebx
				pushl 	%ecx
				pushl 	%edx

				movl  	$3, %eax
				movl  	$0, %ebx
				movl  	8(%ebp) ,%ecx
				movl  	$129, %edx
				int   	$0x80
				
				movl  	8(%ebp), %edi
				movl  	UltCar, %eax
				repne 	scasb
				decl  	%edi
				movl  	$0, (%edi)

				popl  	%edx
				popl  	%ecx
				popl  	%ebx
				popl  	%eax

				movl  	%ebp, %esp
                                popl  	%ebp
                                ret

/*--------------------------------*/
/* rotina para imprimir caractere */
/* apontado por 8(%ebp)           */
/*--------------------------------*/
.global _putchar
_putchar:
                                pushl 	%ebp
                                movl  	%esp, %ebp

				pushl 	%eax
				pushl 	%ebx
				pushl 	%ecx
				pushl 	%edx

				movl  	$4, %eax
				movl  	$1, %ebx
				movl	8(%ebp), %ecx
				movl  	$1, %edx
				int   	$0x80

				popl  	%edx
				popl  	%ecx
				popl  	%ebx
				popl  	%eax

                                movl  	%ebp, %esp
                                popl  	%ebp
                                ret

/*-------------------------------------------*/
/* rotina para imprimir cadeia de caracteres */
/* apontada por 8(%ebp)                      */
/*-------------------------------------------*/
.global _puts
_puts:
                                pushl 	%ebp
                                movl  	%esp, %ebp

				pushl 	%eax
				pushl 	%ebx

				movl  	8(%ebp), %ebx
puts_rep:			
				movb  	(%ebx), %al
				cmpb  	$0, %al
				je    	puts_fim
				cmpb  	$'\', %al
				jne   	puts_normal
				incl  	%ebx
				movb  	(%ebx), %al
				cmpb  	$'n', %al				
				je    	puts_n
				movl  	$'\\', Caract
				movl  	$Caract, %eax
				pushl 	%eax
				call  	_putchar
				addl  	$4, %esp
				jmp   	puts_normal
				
puts_n:			
				movl  	ProxLin, %eax
				movl  	%eax, Caract
				movl  	$Caract, %eax
				pushl 	%eax
				call  	_putchar
				addl  	$4, %esp
				incl  	%ebx
				jmp   	puts_rep
puts_normal:			
				pushl 	%ebx
				call  	_putchar
				addl  	$4, %esp
				incl  	%ebx
				jmp   	puts_rep
puts_fim:
				popl  	%ebx
				popl  	%eax

                                movl  	%ebp, %esp
                                popl  	%ebp
                                ret

/*---------------------------------*/
/* rotina para ler um inteiro para */
/* endereco apontado por 8(%ebp)   */
/*---------------------------------*/
.global _getint
_getint:
                                pushl 	%ebp
                                movl  	%esp, %ebp

				pushl 	%eax
				pushl 	%ebx
				pushl 	%ecx
				pushl 	%edx

getinterro:                     
				movl  	$Digitos, %ebx
				pushl 	%ebx
				call  	_gets
				addl  	$4, %esp
				movl  	$0, %eax
                                movl  	$0, %ebx
				movl  	$0, %ecx
getintcx1:                      
				movb  	Digitos(%ebx), %dl
				cmpb  	$10, %dl
				je    	getintcx2
				incl  	%ecx
				incl  	%ebx
				jmp   	getintcx1
getintcx2:				
				jecxz 	getintfim
				movl  	$0, %ebx
				movb  	Digitos(%ebx), %dl
                                cmpb  	$'-', %dl
                                jne   	getintpositivo
                                movl  	$1, Negativo
                                incl  	%ebx
				decl  	%ecx
                                jmp   	getintcontinua
getintpositivo:                 
				movl  	$0, Negativo
getintcontinua:                 
				mull  	Dez
                                movb  	Digitos(%ebx), %dl
                                cmpb  	$'0', %dl
                                jb    	getinterro
                                cmpb  	$'9', %dl
                                ja    	getinterro
                                subb  	$'0', %dl
                                movb  	$0, %dh
                                addl  	%edx, %eax
                                inc   	%ebx
                                loop  	getintcontinua
                                cmpl  	$1, Negativo
                                jne   	getintfim
                                neg   	%eax
getintfim:                      
				movl  	8(%ebp), %ebx
                                movl  	%eax, (%ebx)

				popl  	%edx
				popl  	%ecx
				popl  	%ebx
				popl  	%eax

				movl  	%ebp, %esp
                                popl  	%ebp
                                ret

/*-----------------------------------------*/
/* rotina para imprimir inteiro em 8(%ebp) */
/*-----------------------------------------*/
.global _putint
_putint:
                                pushl 	%ebp
                                movl  	%esp, %ebp

				pushl 	%eax
				pushl 	%ecx
				pushl 	%edx

                                movl  	8(%ebp), %eax
                                cmpl  	$0, %eax
                                je    	putintzero
                                jg    	putintpositivo
                                negl  	%eax
                                movl  	$'-', Caract
				movl  	$Caract, %edx
                                pushl 	%edx
                                call  	_putchar
                                addl  	$4, %esp
putintpositivo:                 
				movl  	$0, %ecx
putintempilha:                  
				movl  	$0, %edx
                                idivl 	Dez
                                pushl 	%edx
                                incl  	%ecx
                                cmpl  	$0, %eax
                                jne   	putintempilha
putintdesempilha:               
				popl  	%edx
                                addl  	$0x0030, %edx
				movl  	%edx, Caract
				movl  	$Caract, %edx
                                pushl 	%edx
                                call  	_putchar
                                addl  	$4, %esp
                                decl  	%ecx
                                cmpl  	$0, %ecx
                                jne   	putintdesempilha
                                jmp   	putintfim
putintzero:                     
				movl  	$'0', Caract
				movl  	$Caract, %edx
                                pushl 	%edx
                                call  	_putchar
                                addl  	$4, %esp

putintfim:                      
				popl  	%edx
				popl  	%ecx
				popl  	%eax

				movl  	%ebp, %esp
                                popl  	%ebp
                                ret
