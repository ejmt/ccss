/*----------------------------------*/
/* Autor: Elias Jose Moreira Torres */
/* <ejmt_ce@hotmail.com>            */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/

/*---------------------------------*/
/* anasin.c - analisador sintatico */
/*---------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analex.h"
#include "anasin.h"
#include "anasem.h"

PRIVATE	int	Token;
PRIVATE	char 	Lexema[TAM_LEXEMA+1];
PRIVATE void 	EXPRESSAO();
PRIVATE void 	COMANDOS();

/*----------------------------------*/
/* mostrar erro sintatico e abortar */
/*----------------------------------*/
PRIVATE void Abortar(int N)
{
	char	Mensagem[TAM_MENSAGEM];

  	switch(N)
  	{
		case  0 : strcpy(Mensagem, "fim de arquivo"); 		break;
    		case  1 : strcpy(Mensagem, "identificador");        	break;
    		case  2 : strcpy(Mensagem, "tipo");          		break;
    		case  3 : strcpy(Mensagem, "';'");         		break;
    		case  4 : strcpy(Mensagem, "')'");         		break;
    		case  5 : strcpy(Mensagem, "'{'");         		break;
    		case  6 : strcpy(Mensagem, "'}'");         		break;
		case  7 : strcpy(Mensagem, "'=' ou '(' ou '['");	break;
		case  8 : strcpy(Mensagem, "expressao");		break;
		case  9 : strcpy(Mensagem, "'=' ou '['");		break;
		case 10 : strcpy(Mensagem, "']'"); 			break;
		case 11 : strcpy(Mensagem, "'='"); 			break;
		case 12 : strcpy(Mensagem, "numero"); 			break;
  	}
  	printf("Erro sintático (%02d): Linha %d ==> %s esperado(a)!\n", N, ObterLinha(), Mensagem);
  	exit(N);
}

/*-------------------------------------------*/
/* RELACAO: '<', "<=", '>', ">=", "==", "!=" */
/*-------------------------------------------*/
PRIVATE void RELACAO(char *Op)
{
	Token=ObterToken(Lexema);
}

/*---------------------------------------------------*/
/* LISTA_DE_EXPRESSOES = EXPRESSAO { ',' EXPRESSAO } */
/*---------------------------------------------------*/
PRIVATE void LISTA_DE_EXPRESSOES(int *NA, int *NB)
{
	int	T; // tipo da expressao
	int 	D; // deslocamento na pilha onde vai ser colocado o valor de cada EXPRESSAO
    
	D   = 0;
	*NA = 0;
    	EXPRESSAO(&T);
    	EmpilharExpressao(D);
	D   = D + NB_INTEIRO;
    	*NA = *NA + 1;
    	switch(T)
    	{
        	case INT    : *NB = *NB + NB_INTEIRO;   break;
		case CHAR   : *NB = *NB + NB_INTEIRO;	break;
    		//    case PONTEIRO: *NB = *NB + NB_PONTEIRO; break;
   		case CADEIA : *NB = *NB + NB_PONTEIRO; 	break;
    	}
	while (Token == ',')
  	{
    		Token=ObterToken(Lexema);
    		EXPRESSAO(&T);
        	EmpilharExpressao(D);
		D   = D + NB_INTEIRO;
        	*NA = *NA + 1;
        	switch(T)
        	{
            		case INT  : *NB = *NB + NB_INTEIRO;	break;
			case CHAR : *NB = *NB + NB_INTEIRO; 	break;
            		//case PONTEIRO: *NB = *NB + NB_PONTEIRO; break;
            		case CADEIA:   *NB = *NB + NB_PONTEIRO; break;
        	}

  	}
}

/*------------------------------------------------------------*/
/* LISTA_DE_INDICES = '[' EXPRESSAO ']' { '[' EXPRESSAO ']' } */
/*------------------------------------------------------------*/
PRIVATE void LISTA_DE_INDICES()
{
	EXPRESSAO();
	if (Token==']')
	{
		Token=ObterToken(Lexema);
		while (Token=='[')
		{
			Token=ObterToken(Lexema);
			EXPRESSAO();
			if (Token==']')
			{
				Token=ObterToken(Lexema);
			}
			else
			{
				Abortar(10);
			}
		}
	}
	else
	{
		Abortar(10);
	}
}

/*----------------------------------------------------------------------------------------------*/
/* FATOR = NUMERO | [ ( '*' | '&' ) ] IDENTIFICADOR [ LISTA_DE_EXPRESSOES ] | '(' EXPRESSAO ')' */
/*----------------------------------------------------------------------------------------------*/
PRIVATE void FATOR(int *R, int *T)
{
	int	TR;      	/* topo dos registradores */
    	int 	Cat;     	/* categoria */
    	int 	NA;      	/* numero de argumentos ou indices */
    	int 	NB;      	/* numero de bytes empilhados */
    	int 	TA;      	/* topo auxilar dos registradores */
    	int 	P;       	/* entrada para tipo estruturado */
    	char 	Alvo[TAM_NOME]; /* nome da funcao ou vetor */

  	switch(Token)
  	{
    		case IDENTIF : 	Cat = ObterCategoria(Lexema);
                   		/* funcao declarada ou externa */
                   		if ((Cat == CFUNC)||(Cat == NADA))
                   		{
                     			strcpy(Alvo, Lexema);
                     			Token = ObterToken(Lexema);
                     			IniciarFuncao(&NA, &NB, &TA);
                     			if (Token == '(')
                     			{
                        			Token = ObterToken(Lexema);
                        			if (Token == ')') Token = ObterToken(Lexema);
                        			else
                        			{
							AlocarAreaDosArgumentos(Alvo);
                          				LISTA_DE_EXPRESSOES(&NA, &NB);
                          				if (Token == ')') Token = ObterToken(Lexema);
                          				else Abortar(2);
                        			}
                     			}
                     			TerminarFuncao(R, T, NA, NB, TA, Alvo);
                   		}
                   //else
                   /* variavel vetor */
                   //if (Cat == CVET)
                   //{
                   //  strcpy(Alvo, Lexema);
                   //  Token = ObterToken(Lexema);
                   //  IniciarFatorVetor(&NA, &TA, R, T, &P, Alvo);
                   //  if (Token == '[')
                   //  {
                   //    Token = ObterToken(Lexema);
                   //    ListaDeIndices(&NA, P);
                   //    if (Token == ']') Token = ObterToken(Lexema);
                   //    else Abortar(21);
                   //  }
                   //  else  Abortar(20);
                   //  TerminarFatorVetor(NA, TA, *R, Alvo);
                   //}
                   /* variavel simples */
                   		else
				if ((Cat == CVARG) || 
				    (Cat == CVARL) ||
				    (Cat == CPAR)) 
                   		{
                     			GerarFator(R, T, Token, Lexema);
                     			Token = ObterToken(Lexema);
                   		}
                   		break;
    		case NUMERO  : 	GerarFator(R, T, Token, Lexema);
                   		Token = ObterToken(Lexema);
                   		break;
// VERIFICAR: acho que nunca passa por aqui
    		case CARACT  : 	GerarFator(R, T, Token, Lexema);
                   		Token = ObterToken(Lexema);
                   		break;
    		//case VALOR   : Token = ObterToken(Lexema);
    		//               if (Token == '(') Token = ObterToken(Lexema);
    		//               else Abortar(18);
    		//               if (Token == IDENTIF)
    		//               {
    		//                 GerarFatorValor(R, T, Lexema);
    		//                 Token = ObterToken(Lexema);
    		//               }
    		//               else Abortar(1);
    		//               if (Token == ')') Token = ObterToken(Lexema);
    		//               else Abortar(2);
    		//               break;
    		case '('     :	SalvarRegistradores(&TR);
                   		Token = ObterToken(Lexema);
                   		EXPRESSAO(T);
                   		if (Token == ')') Token = ObterToken(Lexema);
                   		else Abortar(2);
                   		RecuperarRegistradores(R, TR);
                   		break;
    		default      : 	Abortar(8);
  }
}

/*---------------------------------------*/
/* TERMO = FATOR { ( '*' | '/' ) FATOR } */
/*---------------------------------------*/
PRIVATE void TERMO(int *R, int *T)
{
  	int	R1, R2; /* registradores para os fatores */
  	int 	T1, T2; /* tipos dos fatores */
  	int 	Operador;

  	FATOR(&R1, &T1);
  	while ((Token == '*')||(Token == '/'))
  	{
    		Operador = Token;
    		Token    = ObterToken(Lexema);
    		FATOR(&R2, &T2);
    		Operar(Operador, R1, R2, T1, T2);
  	}
  	*R = R1;
  	*T = T1;
}

/*-----------------------------------------------------------*/
/* EXPRESSAO_SIMPLES = [ '-' ] TERMO { ( '+' | '-' ) TERMO } */
/*-----------------------------------------------------------*/
PRIVATE void EXPRESSAO_SIMPLES(int *R, int *T)
{
  	int 	R1, R2; 	/* registradores de cada termo */
  	int 	T1, T2; 	/* tipos de cada termo */
  	int 	Operador;
  	int 	Menos; 		/* setado se tem menos unario */

  	Menos = 0;
  	if (Token == '-')
  	{
    		Menos = 1;
    		Token = ObterToken(Lexema);
  	}
  	TERMO(&R1, &T1);
  	GerarMenos(Menos, R1);
  	while ((Token == '+')||(Token == '-'))
  	{
    		Operador = Token;
    		Token    = ObterToken(Lexema);
    		TERMO(&R2, &T2);
    		Operar(Operador, R1, R2, T1, T2);
  	}
  	*R = R1;
  	*T = T1;
}

/*-------------------------------------------------------------*/
/* EXPRESSAO = EXPRESSAO_SIMPLES [ RELACAO EXPRESSAO_SIMPLES ] */
/*-------------------------------------------------------------*/
PRIVATE void EXPRESSAO(int *T)
{
  	int 	R1, R2; /* registradores das expressoes simples */
  	int 	T1, T2;	/* tipos das expressoes simples */
  	char 	Op[3]; 	/* operador das expressoes simples */

  	if (Token == CADEIA)
  	{
// DEBUG: printf("Passou em EXPRESSAO ...\n");
  		GerarCadeia(Lexema, T);
  		Token = ObterToken(Lexema);
  	}
  	else
	if (Token == CARACT)
	{	
// DEBUG: printf("Passou em EXPRESSAO com Lexema = %s\n", Lexema);
		GerarCaract(Lexema, T);
		Token = ObterToken(Lexema);
	}
	else
  	//if (Token == ENDER)
  	//{
  	//  Token = ObterToken(Lexema);
  	//  if (Token == '(') Token = ObterToken(Lexema);
  	//  else Abortar(18);
  	//  if (Token == IDENTIF)
  	//  {
  	//    GerarEndereco(Lexema, T);
  	//    Token = ObterToken(Lexema);
  	//  }
  	//  else Abortar(1);
  	//  if (Token == ')') Token = ObterToken(Lexema);
  	//  else Abortar(2);
  	//}
  	//else
  	{
    		EXPRESSAO_SIMPLES(&R1, &T1);
    	//if (Relacao(Lexema))
    	//{
    	//  strcpy(Op, Lexema);
    	//  Token = ObterToken(Lexema);
    	//  ExpressaoSimples(&R2, &T2);
    	//  OperarRelacao(Op, R1, R2, T1, T2);
    	//}
    		*T = T1;
  	}
}

/*-----------------------------------*/
/* TIPO = INT [ '*' ] | CHAR [ '*' ] */
/*-----------------------------------*/
PRIVATE void TIPO(int *T)
{
	switch(Token)
  	{
		case INT	:	*T=INT;
					Token=ObterToken(Lexema); 
					break;
    		case CHAR 	: 	*T=CHAR;
					Token=ObterToken(Lexema);
					break;
    		default   	: 	Abortar(2);
  	}
}

/*--------------------------------------------------------------------------*/
/* COMANDO_DE_ATRIBUICAO = IDENTIFICADOR [ LISTA_DE_INDICES ] '=' EXPRESSAO */
/*--------------------------------------------------------------------------*/
PRIVATE void COMANDO_DE_ATRIBUICAO(char *Alvo)
{
  	int	T;       /* tipo base do alvo */
  	int 	P;       /* tipo estruturado do alvo */
  	int 	Cat;     /* categoria do alvo */
  	int 	NI;      /* numero de indices */

  	Cat   = ObterCategoria(Alvo);
  	//Token = ObterToken(Lexema);
  	//if (Cat == CVET) /* variavel vetor */
  	//{
  	//  IniciarAtribuicaoVetor(&NI, &T, &P, Alvo);
  	//  if (Token == '[')
  	//  {
  	//    Token = ObterToken(Lexema);
  	//    ListaDeIndices(&NI, P);
  	//    if (Token == ']') Token = ObterToken(Lexema);
  	//    else Abortar(21);
  	//  }
  	//  else  Abortar(20);
  	//  TerminarAtribuicaoVetor(NI, Alvo);
  	//  if (Token == '=') Token = ObterToken(Lexema);
  	//  else Abortar(9);
  	//  Expressao(&T);
  	//  GerarAtribuicaoVetor(Alvo, T);
  	//}
  	//else             /* variavel simples */
// VERIFICAR: mudança recente 28 dez 2021
	if ((Cat == CVARG) ||
	    (Cat == CVARL) ||
            (Cat == CPAR))
  	{
    		if (Token == '=') Token = ObterToken(Lexema);
    		else Abortar(9);
    		EXPRESSAO(&T);
// DEBUG: printf("Em COMANDO_DE_ATRIBUICAO com Alvo=%s e tipo de expressao = %d\n", Alvo, T);
    		GerarAtribuicao(Alvo, T);
  	}
}

/*----------------------------------------------------------------*/
/* COMANDO_DE_CHAMADA = IDENTIFICADOR '(' LISTA_DE_EXPRESSOES ')' */
/*----------------------------------------------------------------*/
PRIVATE void COMANDO_DE_CHAMADA(char *Alvo)
{
  	int	NA;             	/* numero de argumentos */
	int	NA_PF;			/* numero de argumento na cadeia de formatação de printf */
  	int  	NB;             	/* numero de bytes empilhados */
	char	Formato[TAM_CADEIA];	/* cadeia de caracteres do formato usado em printf */

// DEBUG: printf("Passou por aqui, COMANDO_DE_CHAMADA, com Lexema=<%s>\n", Lexema);
  	NA    = 0;
	NA_PF = 0;
  	NB    = 0;
  	//Token = ObterToken(Lexema);
  	if (Token == '(')
  	{
    		Token = ObterToken(Lexema);
    		if (Token == ')') Token = ObterToken(Lexema);
    		else
    		{
// DEBUG: printf("Passou por aqui, COMANDO_DE_CHAMADA, com Lexema=<%s>\n", Lexema);
// VERIFICAR: tentar juntar todas a funçoes aqui dentro em uma rotina somente para elas
			// se o Alvo for printf, vamos ver quantos parametros existem na cadeia de caracteres que define o formato
			if (!strcmp(Alvo, "printf"))
			{
				strcpy(Formato, Lexema);
				NA_PF = ObterNumeroDeArgumentosDe_printf(Formato);
				AlocarAreaDosArgumentosDe_printf(NA_PF*NB_INTEIRO);
      				LISTA_DE_EXPRESSOES(&NA, &NB);
      				if (Token == ')') Token = ObterToken(Lexema);
      				else Abortar(2);
  				GerarChamada(Alvo, NA_PF, NA_PF*NB_INTEIRO);
			}
			else
			{
				AlocarAreaDosArgumentos(Alvo);
      				LISTA_DE_EXPRESSOES(&NA, &NB);
      				if (Token == ')') Token = ObterToken(Lexema);
      				else Abortar(2);
  				GerarChamada(Alvo, NA, NB);
			}
    		}
  	}
}

/*-----------------------------------*/
/* COMANDO_RETURN = RETURN EXPRESSAO */
/*-----------------------------------*/
PRIVATE void COMANDO_RETURN()
{
	int	T; /* tipo da expressao */

	Token = ObterToken(Lexema);
	EXPRESSAO(&T);
	GerarRetorne();
}

/*-----------------------------------------------*/
/* COMANDO_DO = DO CORPO WHILE '(' EXPRESSAO ')' */
/*-----------------------------------------------*/
PRIVATE void COMANDO_DO()
{
}

/*-----------------------------------------------*/
/* COMANDO_WHILE = WHILE '(' EXPRESSAO ')' CORPO */
/*-----------------------------------------------*/
PRIVATE void COMANDO_WHILE()
{
}

/*--------------------------------------------------------*/
/* COMANDO_IF = IF '(' EXPRESSAO ')' CORPO [ ELSE CORPO ] */ 
/*--------------------------------------------------------*/
PRIVATE void COMANDO_IF()
{
}

/*---------------------------------------------*/
/* COMANDO direciona para o comando apropriado */
/*---------------------------------------------*/
PRIVATE void COMANDO()
{
	char	Id[TAM_LEXEMA + 1]; // identificador

	switch(Token)
  	{
		case IF		: 	COMANDO_IF(); 
					break;
		case WHILE	: 	COMANDO_WHILE();	
					break;
		case DO		: 	COMANDO_DO();
					break;
		case RETURN	: 	COMANDO_RETURN();
					break;
		case IDENTIF	:	strcpy(Id, Lexema);
					Token=ObterToken(Lexema);
			  		switch(Token)
			  		{
						case '='	:
						case '[' 	:	COMANDO_DE_ATRIBUICAO(Id);
									break;
						case '(' 	: 	COMANDO_DE_CHAMADA(Id);
									break;
						default  	: 	Abortar(7);
			  		}
					break;
  	}
}

/*----------------------------*/
/* COMANDOS = { COMANDO ';' } */
/*----------------------------*/
PRIVATE void COMANDOS()
{
	while ((Token==IF) || (Token==WHILE) || (Token==DO) || (Token==RETURN) || (Token==IDENTIF))
	{
		COMANDO();
		if (Token==';')
		{
			Token=ObterToken(Lexema);
		}
		else
		{
			Abortar(3);
		}
	}
}

/*----------------------------------------*/
/* CORPO = COMANDO ';' | '{' COMANDOS '}' */
/*----------------------------------------*/
PRIVATE void CORPO()
{
	if (Token=='{')
	{
		COMANDOS();
		if (Token=='}')
		{
			Token=ObterToken(Lexema);
		}
		else
		{
			Abortar(6);
		}
	}
	else
	{
		COMANDO();
		if (Token==';')
		{
			Token=ObterToken(Lexema);
		}
		else
		{
			Abortar(3);
		}
	}
}

/*------------------------------------------------------*/
/* LISTA_DE_DIMENSOES = '[' NUMERO ']' { '[' NUMERO ']' } */
/*------------------------------------------------------*/
PRIVATE void LISTA_DE_DIMENSOES()
{
	Token=ObterToken(Lexema);
	if (Token==NUMERO)
	{
		Token=ObterToken(Lexema);
		if (Token==']')
		{	
			Token=ObterToken(Lexema);
			while (Token=='[')
			{
				Token=ObterToken(Lexema);
				if (Token==NUMERO)
				{
					Token=ObterToken(Lexema);
					if (Token==']')
					{
						Token=ObterToken(Lexema);
					}
					else
					{
						Abortar(10);
					}
				}
				else
				{
					Abortar(12);
				}
			}
		}
		else
		{
			Abortar(10);
		}
	}
	else
	{
		Abortar(12);
	}
}

/*---------------------------------------------------------------*/
/* VARIAVEL_LOCAL = [ '*' ] IDENTIFICADOR [ LISTA_DE_DIMENSOES ] */
/*---------------------------------------------------------------*/
PRIVATE void VARIAVEL_LOCAL(int T, int N)
{
// DEBUG: printf("Dentro de VARIAVEL_LOCAL: N=%d\n", N);
	if (Token=='*')
	{
		Token=ObterToken(Lexema);
	}
	if (Token==IDENTIF)
  	{
        	DefinirVariavelLocal(Lexema, T, N);
    		Token=ObterToken(Lexema);
		if (Token=='[')
		{
			LISTA_DE_DIMENSOES();
		}
		else
		{
			// nao eh vetor
		}
  	}
  	else
  	{
    		Abortar(1);
  	}
}

/*----------------------------------------------------------*/
/* DECLARACAO_LOCAL = VARIAVEL_LOCAL { ',' VARIAVEL_LOCAL } */
/*----------------------------------------------------------*/
PRIVATE void DECLARACAO_LOCAL(int T, int *N)
{
	VARIAVEL_LOCAL(T, *N);
	while (Token==',')
	{
		Token=ObterToken(Lexema);
		*N=*N + 1;
		VARIAVEL_LOCAL(T, *N);
	}
	if (Token==';')
	{
		Token=ObterToken(Lexema);
	}
	else
	{
		Abortar(3);
	}
}

/*------------------------------------------------*/
/* DECLARACOES_LOCAIS = { TIPO DECLARACAO_LOCAL } */
/*------------------------------------------------*/
PRIVATE void DECLARACOES_LOCAIS()
{
	int	T; // tipo
    	int 	N; // numero de variaveis locais em cada declaracao local

	N=0;
	while ((Token==INT) || (Token==CHAR))
	{
		TIPO(&T);
		N++;
		DECLARACAO_LOCAL(T, &N);
	}
	AlocarVariaveisLocaisNaPilha(N*NB_INTEIRO);
}

/*-----------------------------------------------------*/
/* BLOCO = '{' [ DECLARACOES_LOCAIS ] [ COMANDOS ] '}' */
/*-----------------------------------------------------*/
PRIVATE void BLOCO()
{
  	if (Token=='{')
  	{
    		Token=ObterToken(Lexema);
    		DECLARACOES_LOCAIS();
    		COMANDOS();
    		if (Token=='}')
    		{
      			Token=ObterToken(Lexema);
    		}
    		else
    		{
      			Abortar(6);
    		}
  	}
  	else
  	{
    		Abortar(5);
  	} 
}

/*-----------------------------------*/
/* PARAMETRO = [ '*' ] IDENTIFICADOR */
/*-----------------------------------*/
PRIVATE void PARAMETRO(int Tipo, int NP)
{
	if (Token=='*')
	{
		Token=ObterToken(Lexema);
	}
	if (Token==IDENTIF)
	{
		DefinirParametro(Lexema, Tipo, NP);
		Token=ObterToken(Lexema);
	}
	else
	{
		Abortar(1);
	}
}

/*------------------------------------------*/
/* DECLARACAO_DE_PARAMETRO = TIPO PARAMETRO */
/*------------------------------------------*/
PRIVATE void DECLARACAO_DE_PARAMETRO(int NP)
{
	int	T; // tipo

	TIPO(&T);
	PARAMETRO(T, NP);
}

/*-------------------------------------------------------------------------------------*/
/* DECLARACOES_DE_PARAMETROS = DECLARACAO_DE_PARAMETRO { ',' DECLARACAO_DE_PARAMETRO } */
/*-------------------------------------------------------------------------------------*/
PRIVATE void DECLARACOES_DE_PARAMETROS(int *NP)
{
	*NP = 0; // numero de parametros inicializado com 0
	if (Token==')')
	{
		/* sem parametros */
	}
	else
	{
		*NP = *NP + 1;
		DECLARACAO_DE_PARAMETRO(*NP);
		while (Token==',')
		{
			Token=ObterToken(Lexema);
			*NP = *NP + 1;
			DECLARACAO_DE_PARAMETRO(*NP);
		}
	}
}

/*----------------------------------------------------------------*/
/* VARIAVEL_GLOBAL = [ '*' ] Identificador [ LISTA_DE_DIMENSOES ] */
/*----------------------------------------------------------------*/
PRIVATE void VARIAVEL_GLOBAL(char *Id)
{
	if (Token=='*')
	{
		Token=ObterToken(Lexema);
	}
	if (Token==IDENTIF)
  	{
		strcpy(Id, Lexema);
    		Token=ObterToken(Lexema);
		if (Token=='[')
		{
			LISTA_DE_DIMENSOES();
		}
  	}
  	else
  	{
    		Abortar(1);
  	}
}

/*------------------------------------------------------------------------------------------------------------------------*/
/* DECLARACAO_GLOBAL = TIPO VARIAVEL_GLOBAL ( { ',' VARIAVEL_GLOBAL } ';' | '(' [ DECLARACOES_DE_PARAMETROS ] ')' BLOCO ) */
/*------------------------------------------------------------------------------------------------------------------------*/
PRIVATE void DECLARACAO_GLOBAL()
{
	char	Id[TAM_LEXEMA + 1]; // identificador
	int  	T;                  // tipo  
	int  	NP;                 // N. de parametros (se Id for função)

	TIPO(&T);
  	VARIAVEL_GLOBAL(Id);
	switch(Token)
	{
		case ';': 	IniciarSegmentoDeDados();
				DefinirVariavelGlobal(Id, T);
				Token=ObterToken(Lexema); 
				break;
		case ',': 	IniciarSegmentoDeDados();
				DefinirVariavelGlobal(Id, T);
				while (Token==',') 
    			   	{
      					Token=ObterToken(Lexema);
      					VARIAVEL_GLOBAL(Id);
					DefinirVariavelGlobal(Id, T);
    				}
    				if (Token==';')
    				{
      					Token=ObterToken(Lexema);
    				}
				else
				{
		  			Abortar(3);
				}
				break;
		case '(': 	IniciarSegmentoDeCodigo();
				DefinirNomeDaSubRotina(Id);
				IniciarSubRotina(Id);
				Token=ObterToken(Lexema);
    				DECLARACOES_DE_PARAMETROS(&NP);
				DefinirSubRotina(ObterNomeDaSubRotina(), T, NP);
    				if (Token==')')
    				{
      					Token=ObterToken(Lexema);
      					BLOCO();
					TerminarSubRotina(ObterNomeDaSubRotina());
    				}
				else
				{
					Abortar(4);
				}
				break;
		default :	Abortar(3);
	}
}

/*---------------------------------------------------------------*/
/* DECLARACOES_GLOBAIS = DECLARACAO_GLOBAL { DECLARACAO_GLOBAL } */
/*---------------------------------------------------------------*/
PRIVATE void DECLARACOES_GLOBAIS()
{
	DECLARACAO_GLOBAL();
  	while ((Token==INT) || (Token==CHAR))
  	{
		DECLARACAO_GLOBAL();
  	} 
}

/*--------------------------------------------*/
/* MODULO = [ INCLUSOES ] DECLARACOES_GLOBAIS */
/*--------------------------------------------*/
PRIVATE void MODULO()
{
  	/* Inclusoes(); */
	DECLARACOES_GLOBAIS();
}

/*-------------------------------------------------*/
/* Associar um analisador sintatico com um arquivo */
/* fonte e com um arquivo objeto (em assembly)     */
/*-------------------------------------------------*/
PUBLIC void IniciarAnalisadorSintatico(char *NomeFonte, char *NomeMonta)
{
  	IniciarAnalisadorLexico(NomeFonte);
  	IniciarAnalisadorSemantico(NomeMonta);

  	Token=ObterToken(Lexema);
  	MODULO();
	if (Token==FIM_ARQ)
	{	
		TerminarMontagem("main");
	}	
	else Abortar(0);
}

/*------------------------------------*/
/* Desassociar o analisador sintatico */
/*------------------------------------*/
PUBLIC void TerminarAnalisadorSintatico()
{
	TerminarAnalisadorSemantico();
  	TerminarAnalisadorLexico();
}
