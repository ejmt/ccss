/*----------------------------------*/
/* <ejmt_ce@yahoo.com.br>           */
/* Este software esta disponivel    */
/* via licenca GPL                  */
/*----------------------------------*/

/*---------------------------------*/
/* anasem.c - analisador semantico */
/*---------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analex.h"
#include "anasin.h"
#include "anasem.h"

PRIVATE FILE          *ArqMonta;           /* fluxo do arquivo destino */
PRIVATE TSimbolo      *Tabela[TAM_TABELA]; /* tabela de simbolos */
PRIVATE int           TopoTab;             /* topo da tab. de simb. */
PRIVATE char          *Reg32[] = {"%ebx", 
	                          "%ecx", 
			          "%esi", 
			          "%edi"}; /* regs de 32 bits */
PRIVATE int           TopoReg;             /* topo dos registradores */
PRIVATE int           Rotulo;              /* numero do rotulo atual */
PRIVATE int           Nivel;               /* nivel (global/local) */
PRIVATE unsigned long EspacoGlobal;        /* espaco global */
PRIVATE unsigned long EspacoLocal;         /* espaco na pilha */
PRIVATE long          DeslocLocal;         /* deslocamento na pilha */
PRIVATE char          NomeProg[TAM_NOME];  /* nome do programa compilado */
PRIVATE char          NomeSubRot[TAM_NOME];/* nome da sub rotina */
PRIVATE char          NomeTipo[TAM_NOME];  /* nome do tipo estruturado */

/*-------------------------*/
/* mostrar erro e terminar */
/*-------------------------*/
PRIVATE void Abortar(int N)
{
  	char Mensagem[TAM_MENSAGEM];

  	switch(N)
  	{
    		case  1 : strcpy(Mensagem, "Tabela Cheia");                break;
    		case  2 : strcpy(Mensagem, "Identificador Redeclarado");   break;
    		case  3 : strcpy(Mensagem, "Identificador Nao Declarado"); break;
    		case  4 : strcpy(Mensagem, "Mistura De Tipos");            break;
    		case  5 : strcpy(Mensagem, "Operacao Invalida");           break;
    		case  6 : strcpy(Mensagem, "Categoria Invalida");          break;
    		case  7 : strcpy(Mensagem, "Segmento Cheio");              break;
    		case  8 : strcpy(Mensagem, "Muitos Argumentos");           break;
    		case  9 : strcpy(Mensagem, "Poucos Argumentos");           break;
    		case 10 : strcpy(Mensagem, "Muitos Indices");              break;
    		case 11 : strcpy(Mensagem, "Poucos Indices");              break;
    		case 12 : strcpy(Mensagem, "Tipo de Retorno Invalido");    break;
    		case 13 : strcpy(Mensagem, "Tipo do Parametro Invalido");  break;
    		case 14 : strcpy(Mensagem, "Sem Memoria");                 break;
  	}
  	printf("Erro semântico (%02d): Linha %d ==> %s!\n", N, ObterLinha(), Mensagem);
  	exit(N);
}

/*-----------------------------------------------*/
/* iniciar o espaco alocado no segmento de dados */
/*-----------------------------------------------*/
PUBLIC void IniciarEspacoGlobal()
{
  	EspacoGlobal = 0;
}

/*--------------------------------*/
/* definir nivel lexico igual a N */
/*--------------------------------*/
PUBLIC void DefinirNivel(int N)
{
  	Nivel = N;
}

/*----------------------------------*/
/* definir nome do tipo estruturado */
/*----------------------------------*/
PUBLIC void DefinirNomeDoTipo(char *Id)
{
  	strcpy(NomeTipo, Id);
}

/*--------------------------------*/
/* obter nome do tipo estruturado */
/*--------------------------------*/
PUBLIC char *ObterNomeDoTipo()
{
  	return NomeTipo;
}

/*---------------------------*/
/* procurar um identificador */
/*---------------------------*/
PRIVATE int Procurar(char *Id)
{
  	int	Ind;

  	for (Ind=TopoTab-1; Ind>=0; Ind--)
    		if (! strcmp(Tabela[Ind]->Nome, Id)) return Ind;
  	return -1;
}

/*-------------------------------------*/
/* inserir um item na tab. de simbolos */
/*-------------------------------------*/
PRIVATE void Inserir(char *Id, int Cat, int Niv, long Desl, int Tipo)
{
  	TSimbolo	*PtrTopoTab;
  	int       	Indice;

  	Indice = Procurar(Id);
  	if (Indice != -1)
    		if (Tabela[Indice]->Nivel == Niv) Abortar(2);
  
  	if (TopoTab < TAM_TABELA)
  	{
    		PtrTopoTab = malloc(sizeof(TSimbolo));
    		if (PtrTopoTab == NULL) Abortar(14);
    		strcpy(PtrTopoTab->Nome, Id);
    		PtrTopoTab->Categ = Cat;
    		PtrTopoTab->Nivel = Niv;
    		PtrTopoTab->Desl  = Desl;
    		PtrTopoTab->Tipo  = Tipo;
    		Tabela[TopoTab] = PtrTopoTab;
    		TopoTab++;
  	}
  	else Abortar(1);
}

/*--------------------------*/
/* definir nome do programa */
/*--------------------------*/
PUBLIC void DefinirNomeDoPrograma(char *Id)
{
  	strcpy(NomeProg, Id);
  	Inserir(Id, CPROG, NGLOBAL, NADA, NADA);
}

/*------------------------*/
/* obter nome do programa */
/*------------------------*/
PUBLIC char *ObterNomeDoPrograma()
{
  	return NomeProg;
}

/*----------------------------*/
/* definir nome da sub rotina */
/*----------------------------*/
PUBLIC void DefinirNomeDaSubRotina(char *Id)
{
  	strcpy(NomeSubRot, Id);
}

/*--------------------------*/
/* obter nome da sub rotina */
/*--------------------------*/
PUBLIC char *ObterNomeDaSubRotina()
{
  	return NomeSubRot;
}

/*--------------------*/
/* alocar registrador */
/*--------------------*/
PRIVATE int AlocarRegistrador()
{
  	return TopoReg++;
}

/*-----------------------*/
/* desalocar registrador */
/*-----------------------*/
PRIVATE void DesalocarRegistrador()
{
  	--TopoReg;
}

/*----------------------*/
/* obter proximo rotulo */
/*----------------------*/
PRIVATE int ObterRotulo()
{
  	return Rotulo++;
}

/*------------------------------------------------*/
/* salvar registradores antes do abre parenteses  */
/*------------------------------------------------*/
PUBLIC void SalvarRegistradores(int *T)
{
  	int	I;

  	*T = TopoReg;
  	for (I=0; I<TopoReg; I++)
    		fprintf(ArqMonta, " %30s pushl %s\n", BRANCO, Reg32[I]);
  	TopoReg = 0;
}

/*----------------------------------------------------*/
/* recuperar registradores depois do fecha parenteses */
/*----------------------------------------------------*/
PUBLIC void RecuperarRegistradores(int *R, int T)
{
  	int	I;
	
  	TopoReg = T;
  	*R      = AlocarRegistrador();
  	if (*R > 0) /* se Reg32[*R] > bx */
    		fprintf(ArqMonta, " %30s movl  %%ebx, %s\n", BRANCO, Reg32[*R]);
  	for (I=T-1; I>=0; I--)
    		fprintf(ArqMonta, " %30s popl  %s\n", BRANCO, Reg32[I]);
}

/*-----------------------------------------------*/
/* associar um arquivo a um analisador semantico */
/*-----------------------------------------------*/
PUBLIC void IniciarAnalisadorSemantico(char *NomeAsm)
{
  	int	Ind;

  	if ((ArqMonta = fopen(NomeAsm, "w")) == (FILE *)0)
  	{
    		printf("Erro de criacao do arquivo %s\n", NomeAsm);
    		exit(1);
  	}
  	Rotulo = TopoReg = TopoTab = 0;
  	for (Ind=0; Ind<TAM_TABELA; Ind++) Tabela[Ind] = (TSimbolo *)0;
}

/*----------------------------------------------*/
/* remover todos simbolos do nivel especificado */
/*----------------------------------------------*/
PRIVATE void Remover(int Niv)
{
  	TopoTab--;
  	while ((TopoTab >= 0) &&
              (Tabela[TopoTab]->Nivel == Niv))
  	{
    		free(Tabela[TopoTab]);
    		TopoTab--;
  	}
  	TopoTab++;
}

/*----------------------------------------*/
/* definir N-esima dimensao do tipo vetor */
/*----------------------------------------*/
PUBLIC void DefinirN_ezimaDimensao(char *Id, char *D)
{
  	/* D = tamanho da N-esima dimensao */
  	TSimbolo	*PtrTopoTab;

  	if (TopoTab < TAM_TABELA)
  	{
    		PtrTopoTab = malloc(sizeof(TSimbolo));
    		if (PtrTopoTab == NULL) Abortar(14);
    		strcpy(PtrTopoTab->Nome, Id);
    		PtrTopoTab->Categ = CTIPO;
    		PtrTopoTab->Nivel = Nivel;
    		PtrTopoTab->Desl  = atol(D);
    		PtrTopoTab->Tipo  = NADA;
    		Tabela[TopoTab] = PtrTopoTab;
    		TopoTab++;
  	}
  	else Abortar(1);
}

/*---------------------------------------*/
/* definir tipo base do tipo estruturado */
/*---------------------------------------*/
PUBLIC void DefinirTipoBase(int T, int NDim)
{
  	while (NDim)
  	{
    		Tabela[TopoTab-NDim]->Tipo = T;
    		NDim--;
  	}
}

/*-----------------------------------*/
/* obter entrada da tabela para tipo */
/*-----------------------------------*/
PUBLIC int ObterTipo(char *Id)
{
	int	T; /* entrada de Id */
  	int 	C; /* categoria de Id */

  	T = Procurar(Id);
  	if (T == -1) Abortar(3);    /* nao foi declarado */
  	C = Tabela[T]->Categ;
  	if (C != CTIPO) Abortar(6); /* nao e um id. de tipo */
  	while ((T-1 >= 0) &&
              (! strcmp(Tabela[T]->Nome, Tabela[T-1]->Nome))) T--;
  	return T;
}

/*---------------------------------------*/
/* obter numero de dimensoes de um vetor */
/*---------------------------------------*/
PRIVATE int ObterNumeroDeDimensoes(char *Id)
{
  	int	Pos;
  	int 	NumDim;

  	Pos = Procurar(Id);
  	Pos = Tabela[Pos]->Tipo;
  	NumDim = 1;
  	while ((Pos+1 < TopoTab) &&
              (! strcmp(Tabela[Pos]->Nome, Tabela[Pos+1]->Nome)))
  	{
    		Pos++;
    		NumDim++;
  	}
  	return NumDim;
}

/*-------------------------------------*/
/* obter categoria de um identificador */
/*-------------------------------------*/
PUBLIC int ObterCategoria(char *Id)
{
  	int	Ind;

  	if ((Ind = Procurar(Id)) == -1) return NADA;
  	else                            return Tabela[Ind]->Categ;
}

/*----------------------------*/
/* iniciar segmento de codigo */
/*----------------------------*/
PUBLIC void IniciarSegmentoDeCodigo()
{
  fprintf(ArqMonta, "\n.text\n");
}

/*-----------------------------*/
/* terminar segmento de codigo */
/*-----------------------------*/
PUBLIC void TerminarSegmentoDeCodigo()
{
}

/*---------------------------*/
/* definir segmento de pilha */
/*---------------------------*/
PUBLIC void DefinirSegmentoDePilha()
{
}

/*---------------------------*/
/* iniciar segmento de dados */
/*---------------------------*/
PUBLIC void IniciarSegmentoDeDados()
{
  fprintf(ArqMonta, ".data\n");
}

/*----------------------------*/
/* terminar segmento de dados */
/*----------------------------*/
PUBLIC void TerminarSegmentoDeDados()
{
}

/*--------------------------*/
/* gerar diretivas iniciais */
/*--------------------------*/
PUBLIC void IniciarMontagem()
{
}

/*--------------------------------------*/
/* gerar ponto de entrada se for o caso */
/*--------------------------------------*/
PUBLIC void TerminarMontagem(char *Inicio)
{
	if (Procurar(Inicio) != -1) 
  	{
    		fprintf(ArqMonta, ".global _start\n");
    		fprintf(ArqMonta, "_start:\n");
    		fprintf(ArqMonta, " %30s call  _%s\n", BRANCO, Inicio);
		fprintf(ArqMonta, " %30s movl  %%eax, %%ebx\n", BRANCO);
  		fprintf(ArqMonta, " %30s movl  $1, %%eax\n",    BRANCO);
  		fprintf(ArqMonta, " %30s int   $0x80\n",        BRANCO);
	}
  	Remover(NGLOBAL);
}

/*-----------------------------------------*/
/* devolver o numero de elementos do vetor */
/*-----------------------------------------*/
PRIVATE long ObterNumeroDeElementos(int T)
{
  	long	NE;

  	NE = Tabela[T]->Desl;
  	while ((T+1 < TopoTab) &&
              (! strcmp(Tabela[T]->Nome, Tabela[T+1]->Nome)))
  	{
   		 NE *= Tabela[T+1]->Desl;
    		T++;
  	}
  	return NE;
}

/*--------------------------------*/
/* definir variavel global vetor */
/*--------------------------------*/
PRIVATE long DefinirVariavelGlobalVetor(char *Id, int Tipo)
{
  	int	TB; /* tipo base */
  	long 	NE; /* numero de elementos */
  	long 	NB; /* numero de bytes */

  	TB = Tabela[Tipo]->Tipo;
  	NE = ObterNumeroDeElementos(Tipo);
  	NB = NB_INTEIRO;
  	Inserir(Id, CVETG, NGLOBAL, NADA, Tipo);
  	switch (TB) /* se tipo basico for ... */
  	{
    		case INT	:	NB = NB_INTEIRO*NE;
	              			fprintf(ArqMonta, ".comm _%s, %ld, %d\n", Id, NB, ALIN_INT);
                    			break;
    		case CADEIA   	: 	NB = (NB_CADEIA + 1)*NE;
	              			fprintf(ArqMonta, ".comm _%s, %ld, %d\n", Id, NB, ALIN_CAD);
                    			break;
  	}
  	return NB;
}

/*-------------------------------*/
/* definir variavel local vetor */
/*-------------------------------*/
PRIVATE long DefinirVariavelLocalVetor(int Tipo)
{
  	int	TB; /* tipo base */
  	long 	NE; /* numero de elementos */
  	long 	NB; /* numero de bytes */

  	TB = Tabela[Tipo]->Tipo;
  	NE = ObterNumeroDeElementos(Tipo);
  	NB = NB_INTEIRO;
  	switch (TB) /* se tipo basico for ... */
  	{
    		case INT      : NB = NB_INTEIRO*NE;
                    		break;
    		case CADEIA   : NB = (NB_CADEIA + 1)*NE;
                    		break;
  	}
  	return NB;
}

/*---------------------------------------------*/
/* definir e gerar codigo para variavel global */
/*---------------------------------------------*/
PUBLIC void DefinirVariavelGlobal(char *Id, int Tipo)
{
	if (Tipo == INT)
    	{
      		Inserir(Id, CVARG, NGLOBAL, NADA, INT);
      		fprintf(ArqMonta, ".global _%s\n", Id);
      		fprintf(ArqMonta, "_%s:\n", Id);
      		fprintf(ArqMonta, ".long 0\n");
      		EspacoGlobal += NB_INTEIRO;
    	}
    	else
    	if (Tipo == CHAR)
    	{
      		Inserir(Id, CVARG, NGLOBAL, NADA, CHAR);
      		fprintf(ArqMonta, ".global _%s\n", Id);
      		fprintf(ArqMonta, "_%s:\n", Id);
      		fprintf(ArqMonta, ".byte 0\n");
      		EspacoGlobal += NB_CARACTERE;
    	}
    	else /* variavel global estruturada */
    	{
      		EspacoGlobal += DefinirVariavelGlobalVetor(Id, Tipo);
    	}
    	if (EspacoGlobal > TAM_DADOS) Abortar(7);
}

/*--------------------------------------------*/
/* definir e gerar codigo para variavel local */
/*--------------------------------------------*/
PUBLIC void DefinirVariavelLocal(char *Id, int T, int N)
{
// DEBUG: printf("Definindo uma variavel local: %s, %d, %d\n", Id, T, N);
    	if (T == INT)
    	{
      		Inserir(Id, CVARL, NLOCAL, N*NB_INTEIRO, INT);
      		EspacoLocal += NB_INTEIRO;
      		DeslocLocal += NB_INTEIRO;
    	}
    	if (T == CHAR)
    	{
      		Inserir(Id, CVARL, NLOCAL, N*NB_INTEIRO, CHAR);
      		EspacoLocal += NB_INTEIRO;
      		DeslocLocal += NB_INTEIRO;
    	}
    	//else
    	//if (Tipo == CADEIA)
    	//{
      	//	Inserir(Id, CVARL, NLOCAL, DeslocLocal+NB_CADEIA+1, CADEIA);
      	//	EspacoLocal += NB_CADEIA+1;
      	//	DeslocLocal += NB_CADEIA+1;
      	//	*N          += NB_CADEIA+1;
    	//}
    	//else /* variavel local estruturada */
    	//{
      	//	*N = DefinirVariavelLocalVetor(Tipo);
      	//	Inserir(Id, CVETL, NLOCAL, DeslocLocal+(*N), Tipo);
      	//	EspacoLocal += *N;
      	//	DeslocLocal += *N;
    	//}
   	if (EspacoLocal > TAM_PILHA)  Abortar(7);
}

/*----------------------------------------*/
/* definir e gerar codigo para parametro */
/*----------------------------------------*/
PUBLIC void DefinirParametro(char *Id, int Tipo, int NP)
{
  	if (Tipo == INT)
  	{
    		Inserir(Id, CPAR, NLOCAL, DESL_INIC + (NP - 1)*NB_INTEIRO, INT);
    		EspacoLocal += NB_INTEIRO;
		DeslocLocal += NB_INTEIRO;
  	}
  	else
  	if (Tipo == CHAR)
  	{
    		Inserir(Id, CPAR, NLOCAL, DESL_INIC + (NP - 1)*NB_INTEIRO, CHAR);
    		EspacoLocal += NB_INTEIRO; // mesmo sendo char deve ser tratado com int na pilha
      		DeslocLocal += NB_INTEIRO;
  	}
}

/*----------------------*/
/* iniciar sub-rotina   */
/*----------------------*/
PUBLIC void IniciarSubRotina(char *Id)
{
  	Inserir(Id, CFUNC, NGLOBAL, 0, INT);
  	/* publico por default */
  	fprintf(ArqMonta, "_%s:\n", Id);
  	fprintf(ArqMonta, " %-30s pushl %%ebp\n",        BRANCO);
  	fprintf(ArqMonta, " %-30s movl  %%esp, %%ebp\n", BRANCO);
}

/*-----------------------*/
/* terminar sub-rotina   */
/*-----------------------*/
PUBLIC void TerminarSubRotina(char *Id)
{
  	fprintf(ArqMonta, " %30s movl  %%ebp, %%esp\n", BRANCO);
  	fprintf(ArqMonta, " %30s popl  %%ebp\n",        BRANCO);
  	fprintf(ArqMonta, " %30s ret\n",                BRANCO);
	//TerminarBloco();
// DEBUG: printf("Antes de sair da rotinha %s ...\n", Id);
// DEBUG: Listar();
 	Remover(NLOCAL);
  	DefinirNivel(NGLOBAL);
}

/*--------------------------------------------------------*/
/* ajustar o tipo e numero de parametros de uma subrotina */
/*--------------------------------------------------------*/
PRIVATE void AjustarSubRotina(char *Id, int Tipo, int NP)
{
	int	Ind;

	Ind = Procurar(Id);
	Tabela[Ind]->Tipo = Tipo;
	Tabela[Ind]->Desl = NP;		// O numero de parametros fica em Deslocamento
}

/*-----------------------*/
/* definir uma subrotina */
/*-----------------------*/
PUBLIC void DefinirSubRotina(char *Id, int Tipo, int NP)
{
  	/* NP = numero de parametros (na posicao de tipo)*/
  	//Inserir(Id, CFUNC, NGLOBAL, NP, Tipo);
	AjustarSubRotina(Id, Tipo, NP);
  	DefinirNivel(NLOCAL);
  	EspacoLocal = 0;
	//IniciarBloco();
}

/*------------------------------------------------------------*/
/* alocar variaveis locais na pilha, abrindo espaço para elas */
/*------------------------------------------------------------*/
PUBLIC void AlocarVariaveisLocaisNaPilha(int N)
{
	if (N>0)
		fprintf(ArqMonta, " %30s subl  $%d, %%esp\n", BRANCO, N);
}

/*---------------*/
/* iniciar bloco */
/*---------------*/
PUBLIC void IniciarBloco(char *Id, int TipoSub)
{
  	/* NP = numero de parametros (na posicao de tipo)*/
  	Inserir(Id, CFUNC, NGLOBAL, NADA, 0);
  	DefinirNivel(NLOCAL);
  	EspacoLocal = 0;
}

/*----------------*/
/* terminar bloco */
/*----------------*/
PUBLIC void TerminarBloco()
{
 	Remover(NLOCAL);
  	DefinirNivel(NGLOBAL);
}

/*-------------------------------------*/
/* ajustar o tipo de retorno da funcao */
/*-------------------------------------*/
PUBLIC void AjustarTipoDaFuncao(char *NomeFunc, int TipoRet)
{
	int	Indice;

  	Indice = Procurar(NomeFunc);
  	if (Indice != -1)
    		Tabela[Indice]->Desl = TipoRet;
  	else Abortar(3);
}

/*--------------------------*/
/* iniciar chamada a funcao */
/*--------------------------*/
PUBLIC void IniciarFuncao(int *NA, int *NB, int *TA)
{
  	int	I;

  	*NA = 0;       /* inicializa numero de argumentos */
  	*NB = 0;       /* inicializa numero de bytes empilhado */
  	*TA = TopoReg; /* salva topo auxiliar */
  	for (I=0; I<TopoReg; I++)
    		fprintf(ArqMonta, " %30s pushl %s\n", BRANCO, Reg32[I]);
  	TopoReg = 0;
}

/*---------------------------*/
/* terminar chamada a funcao */
/*---------------------------*/
PUBLIC void TerminarFuncao(int *R, int *T, int NA, int NB, int TA, char *Id)
{
	int	NP;  /* numero de parametros */
  	int 	Ind; /* indice na tabela */
  	int 	I;

  	if ((Ind = Procurar(Id)) == -1)
  	{
    		*T = INT; /* default caso funcao seja externa */
  	}
  	else
  	{
    		*T = (int)Tabela[Ind]->Tipo; /* tipo de retorno */
    		NP = Tabela[Ind]->Desl;      /* n. de parametros (em deslocamento) */
    		if (NA < NP) Abortar(9);
    		if (NA > NP) Abortar(8);
  	}
  	fprintf(ArqMonta, " %30s call  _%s\n", BRANCO, Id);
  	if (NB > 0)
    		fprintf(ArqMonta, " %30s addl  $%d, %%esp\n", BRANCO, NB);
  	TopoReg = TA;
  	*R      = AlocarRegistrador();
  	fprintf(ArqMonta, " %30s movl  %%eax, %s\n", BRANCO, Reg32[*R]);
  	for (I=TA-1; I>=0; I--)
		fprintf(ArqMonta, " %30s popl  %s\n", BRANCO, Reg32[I]);
}

/*---------------------*/
/* iniciar fator vetor */
/*---------------------*/
PUBLIC void IniciarFatorVetor(int  *NI, int *TA, int *R, int *T, int *P,
                              char *Id)
{
  	int	I;

  	*P = Tabela[Procurar(Id)]->Tipo; /* encontra entrada do tipo do vetor */
  	*T = Tabela[*P]->Tipo;           /* encontra tipo base do vetor */
  	*R = AlocarRegistrador();
  	*NI = 0;              /* inicializa numero de indices */
  	*TA = TopoReg;        /* salva topo auxiliar */
  	for (I=0; I<TopoReg; I++)
    	fprintf(ArqMonta, " %30s pushl %s\n", BRANCO, Reg32[I]);
  	TopoReg = 0;
  	fprintf(ArqMonta, " %30s movl  $0, %%eax\n", BRANCO);
}

/*----------------------*/
/* terminar fator vetor */
/*----------------------*/
PUBLIC void TerminarFatorVetor(int NI, int TA, int R, char *Id)
{
  	int	ND;  /* numero de dimensoes */
  	int 	I;
  	int 	Pos;
  	int 	Niv;
  	int 	Tip;
  	long    Des;

  	ND = ObterNumeroDeDimensoes(Id);
  	if (NI > ND) Abortar(10);
  	if (NI < ND) Abortar(11);
  	Pos = Procurar(Id);
  	Niv = Tabela[Pos]->Nivel;
  	Des = Tabela[Pos]->Desl;
  	Tip = Tabela[Pos]->Tipo;
  	Tip = Tabela[Tip]->Tipo;
  	TopoReg = TA;
  	for (I=TA-1; I>=0; I--)
    		fprintf(ArqMonta, " %30s popl  %s\n", BRANCO, Reg32[I]);
  	if (Tip == INT)
  	{
    		fprintf(ArqMonta, " %30s addl  %%eax, %%eax\n", BRANCO);
    		fprintf(ArqMonta, " %30s addl  %%eax, %%eax\n", BRANCO);
    		if (Niv == NGLOBAL)
      			fprintf(ArqMonta, " %30s leal  _%s, %s\n", BRANCO, Id, Reg32[R]);
    		else
      			fprintf(ArqMonta, " %30s leal  -%ld(%%ebp), %s\n", BRANCO, Des, Reg32[R]);
    		fprintf(ArqMonta, " %30s addl  %%eax, %s\n", BRANCO, Reg32[R]);
    		if (R == 0) /* ebx ? */
    		{
      			fprintf(ArqMonta, " %30s movl  (%%ebx), %%ebx\n", BRANCO);
    		}
    		else
    		{
      			fprintf(ArqMonta, " %30s pushl %%ebx\n",       BRANCO);
      			fprintf(ArqMonta, " %30s movl  %s, %%ebx\n",   BRANCO, Reg32[R]);
      			fprintf(ArqMonta, " %30s movl  (%%ebx), %s\n", BRANCO, Reg32[R]);
      			fprintf(ArqMonta, " %30s popl  %%ebx\n",       BRANCO);
    		}
  	}
  	else
  	if (Tip == CADEIA)
  	{
    		fprintf(ArqMonta, " %30s movl  $%d, %s\n", BRANCO, NB_CADEIA+1, Reg32[R]);
    		fprintf(ArqMonta, " %30s mull  %s\n",      BRANCO, Reg32[R]);
    		if (Niv == NGLOBAL)
      			fprintf(ArqMonta, " %30s leal  _%s, %s\n", BRANCO, Id, Reg32[R]);
    		else
      			fprintf(ArqMonta, " %30s leal  -%ld(%%ebp), %s\n", BRANCO, Des, Reg32[R]);
    		fprintf(ArqMonta, " %30s addl  %%eax, %s\n", BRANCO, Reg32[R]);
  	}
}

/*-----------------------------*/
/* iniciar atribuicao de vetor */
/*-----------------------------*/
PUBLIC void IniciarAtribuicaoVetor(int *NI, int *T, int *P, char *Id)
{
  	*P = Tabela[Procurar(Id)]->Tipo; /* encontra entrada do tipo do vetor */
  	*T = Tabela[*P]->Tipo;           /* encontra tipo base do vetor */
  	/* inicializa numero de indices */
  	*NI = 0;
  	fprintf(ArqMonta, " %30s movl  $0, %%eax\n", BRANCO);
}

/*------------------------------*/
/* terminar atribuicao de vetor */
/*------------------------------*/
PUBLIC void TerminarAtribuicaoVetor(int NI, char *Id)
{
  	int	ND;  /* numero de dimensoes */
  	int 	Pos;
  	int 	Niv;
  	int 	Tip;
  	long    Des;

  	ND = ObterNumeroDeDimensoes(Id);
  	if (NI > ND) Abortar(10);
  	if (NI < ND) Abortar(11);
  	Pos = Procurar(Id);
  	Niv = Tabela[Pos]->Nivel;
  	Des = Tabela[Pos]->Desl;
  	Tip = Tabela[Pos]->Tipo;
  	Tip = Tabela[Tip]->Tipo;
  	if (Tip == INT)
  	{
    		fprintf(ArqMonta, " %30s addl  %%eax, %%eax\n", BRANCO);
    		fprintf(ArqMonta, " %30s addl  %%eax, %%eax\n", BRANCO);
    		if (Niv == NGLOBAL)
      			fprintf(ArqMonta, " %30s leal  _%s, %%ebx\n", BRANCO, Id);
    		else
      			fprintf(ArqMonta, " %30s leal  -%ld(%%ebp), %%ebx\n", BRANCO, Des);
    		fprintf(ArqMonta, " %30s addl  %%ebx, %%eax\n", BRANCO);
  	}
  	else
  	if (Tip == CADEIA)
  	{
    		fprintf(ArqMonta, " %30s movl  $%d, %%ebx\n", BRANCO, NB_CADEIA+1);
    		fprintf(ArqMonta, " %30s mull  %%ebx\n",     BRANCO);
    		if (Niv == NGLOBAL)
      			fprintf(ArqMonta, " %30s leal  _%s, %%ebx\n", BRANCO, Id);
    		else
      			fprintf(ArqMonta, " %30s leal  -%ld(%%ebp), %%ebx\n", BRANCO, Des);
    		fprintf(ArqMonta, " %30s addl  %%ebx, %%eax\n", BRANCO);
  	}
  	fprintf(ArqMonta, " %30s pushl %%eax\n", BRANCO);
}

/*---------------------------------*/
/* operar sobre dois registradores */
/*---------------------------------*/
PUBLIC void Operar(int Op, int R1, int R2, int T1, int T2)
{
  	if (T1 != INT) Abortar(5);
  	if (T2 != INT) Abortar(5);

  	switch(Op)
  	{
    		case '+' : 	fprintf(ArqMonta, " %30s addl  %s, %s\n", BRANCO, Reg32[R2], Reg32[R1]);
               			break;
    		case '-' : 	fprintf(ArqMonta, " %30s subl  %s, %s\n", BRANCO, Reg32[R2], Reg32[R1]);
               			break;
    		case '*' : 	fprintf(ArqMonta, " %30s xchgl %%eax, %s\n", BRANCO, Reg32[R1]);
               			fprintf(ArqMonta, " %30s imull %s\n",        BRANCO, Reg32[R2]);
               			fprintf(ArqMonta, " %30s xchgl %%eax, %s\n", BRANCO, Reg32[R1]);
               			break;
    		case '/' : 	fprintf(ArqMonta, " %30s movl  $0, %%edx\n", BRANCO);
               			fprintf(ArqMonta, " %30s xchgl %%eax, %s\n", BRANCO, Reg32[R1]);
               			fprintf(ArqMonta, " %30s idivl %s\n",        BRANCO, Reg32[R2]);
               			fprintf(ArqMonta, " %30s xchgl %%eax, %s\n", BRANCO, Reg32[R1]);
               			break;
  	}
  	DesalocarRegistrador();
}

/*---------------------------------*/
/* operar sobre dois registradores */
/*---------------------------------*/
PUBLIC void OperarRelacao(char *Op, int R1, int R2, int T1, int T2)
{
  	int	Rot1;
  	int 	Rot2;
  	char 	Instr[4]; /* instrucao de salto condicional */

  	if (T1 != T2)  Abortar(4);
  	if (T1 != INT) Abortar(5);
  	if (!strcmp(Op, "<"))  strcpy(Instr, "jl ");
  	else
  	if (!strcmp(Op, "<=")) strcpy(Instr, "jle");
  	else
  	if (!strcmp(Op, "<>")) strcpy(Instr, "jne");
  	else
  	if (!strcmp(Op, ">"))  strcpy(Instr, "jg ");
  	else
  	if (!strcmp(Op, ">=")) strcpy(Instr, "jge");
  	else
  	if (!strcmp(Op, "="))  strcpy(Instr, "je ");
  	Rot1 = ObterRotulo();
  	Rot2 = ObterRotulo();
  	fprintf(ArqMonta, " %30s cmpl  %s, %s\n", BRANCO, Reg32[R2], Reg32[R1]);
  	fprintf(ArqMonta, " %30s %s   L%d\n",     BRANCO, Instr,   Rot1);
  	fprintf(ArqMonta, " %30s movl  $0, %s\n", BRANCO, Reg32[R1]);
  	fprintf(ArqMonta, " %30s jmp   L%d\n",    BRANCO, Rot2);
  	fprintf(ArqMonta, "L%d:\n",               Rot1);
  	fprintf(ArqMonta, " %30s movl  $1, %s\n", BRANCO, Reg32[R1]);
  	fprintf(ArqMonta, "L%d:\n", Rot2);
  	DesalocarRegistrador();
}

/*----------------------------*/
/* alocar area dos argumentos */
/*----------------------------*/
PUBLIC void AlocarAreaDosArgumentos(char *Id)
{
	int Indice;
	int NP; // numero de parametros do procedimento/função Id
	int NB; // numero de bytes a serem alocados para os argumentos do procedimento/função ID

//DEBUG: printf("Dentro de AlocarAreaDosArgumentos com Id = %s\n", Id);
  	Indice = Procurar(Id);
  	if (Indice != -1) // Id esta na tabela de simbolos
	{
		// se Id não for procedimento nem função ... erro!
    		if ((Tabela[Indice]->Categ != CPROC) && (Tabela[Indice]->Categ != CFUNC)) Abortar(6);
		else
		{
			NP = Tabela[Indice]->Desl; // NP eh o numero de parametros 
			NB = NP*NB_INTEIRO;
			fprintf(ArqMonta, " %30s subl  $%d, %%esp\n", BRANCO, NB);
		}
	}
	else // Id deve ser um procedimento ou função da biblioteca
	{
		// por enquanto vamos supor que somente existe 1 (um) parametro
		fprintf(ArqMonta, " %30s subl  $%d, %%esp\n", BRANCO, NB_INTEIRO);
	}
}

/*-------------------------------------*/
/* alocar area dos argumento de printf */
/*-------------------------------------*/
PUBLIC void AlocarAreaDosArgumentosDe_printf(int N)
{
	fprintf(ArqMonta, " %30s subl  $%d, %%esp\n", BRANCO, N);
}

/*--------------------------------------*/
/* obter numero de argumentos de printf */
/*--------------------------------------*/
PUBLIC int ObterNumeroDeArgumentosDe_printf(char *Formato)
{
	char	*Ptr;
	int 	NA = 1;	// a cadeia de formatação eh o argumento-1

	Ptr = Formato;
	while (*Ptr) // enquanto não chegar ao fim de Formato
	{
		if (*Ptr++ == '%') NA++; // encontrou mais um parametro 
	}

	return NA;
}

/*--------------------*/
/* empilhar expressao */
/*--------------------*/
PUBLIC void EmpilharExpressao(long D)
{
  	fprintf(ArqMonta, " %30s movl  %%ebx, %ld(%%esp)\n", BRANCO, D);
  	DesalocarRegistrador();
}

/*----------------------------------------*/
/* gerar codigo para acessar fator (vetor)*/
/*----------------------------------------*/
PUBLIC void GerarIndice1()
{
  	fprintf(ArqMonta, " %30s addl  %%ebx, %%eax\n", BRANCO);
  	DesalocarRegistrador();
}

/*---------------------------------------------------*/
/* gerar codigo para acessar fator (vetor) - parte 2 */
/*---------------------------------------------------*/
PUBLIC void GerarIndiceN(int D)
{
  	/* D aponta para entrada que descreve dimensao a utilizar */

  	AlocarRegistrador();
  	fprintf(ArqMonta, " %30s movl  $%ld, %%ecx\n", BRANCO, Tabela[D]->Desl);
  	fprintf(ArqMonta, " %30s mull  %%ecx\n",       BRANCO);
  	DesalocarRegistrador();
  	fprintf(ArqMonta, " %30s addl  %%ebx, %%eax\n", BRANCO);
 	DesalocarRegistrador();
}

/*----------------------------------*/
/* gerar expressao do tipo endereco */
/*----------------------------------*/
PUBLIC void GerarEndereco(char *Id, int *T)
{
  	int	Ind;  /* indice da tab. de simb. */
  	int 	Catg; /* categoria */
  	int 	Nivl; /* nivel */
  	int 	Tipo; /* tipo */
  	long    Desl; /* deslocamento */

  	if ((Ind = Procurar(Id)) == -1) Abortar(3);
  	if ((Tabela[Ind]->Categ != CVARG) &&
            (Tabela[Ind]->Categ != CVARL) &&
            (Tabela[Ind]->Categ != CPAR)  &&
            (Tabela[Ind]->Categ != CVETG) &&
            (Tabela[Ind]->Categ != CVETL)) Abortar(6);
  	Desl = Tabela[Ind]->Desl;
  	Nivl = Tabela[Ind]->Nivel;
  	Tipo = Tabela[Ind]->Tipo;
  	Catg = Tabela[Ind]->Categ;
  	AlocarRegistrador(); /* ebx */
}

/*--------------------------------*/
/* gerar expressao do tipo cadeia */
/*--------------------------------*/
PUBLIC void GerarCadeia(char *Id, int *T)
{
	char	*PtrId;
  	int   	Cont;
  	int   	Rot1;
  	int   	Rot2;

  	PtrId = Id;
  	Rot1 = ObterRotulo();
  	Rot2 = ObterRotulo();
  	*T = CADEIA;
  	fprintf(ArqMonta, " %-30s jmp   L%d\n", BRANCO, Rot2);
  	TerminarSegmentoDeCodigo();
  	IniciarSegmentoDeDados();
  	fprintf(ArqMonta, "L%d:\n", Rot1);
  	Cont = 0;
  	if (*PtrId) 
  	{
    		fprintf(ArqMonta, ".byte '%c'", *PtrId++);
    		Cont++;
  	}
  	while (*PtrId)
  	{
    		if (Cont == NBYTE_LIN)
    		{
      			fprintf(ArqMonta, "\n.byte '%c'", *PtrId++);
      			Cont++;
    		}
    		else
    		{
      			fprintf(ArqMonta, ",'%c'", *PtrId++);
      			Cont++;
    		}
  	}
  	if (Cont == 0) fprintf(ArqMonta, ".byte 0\n");
  	else           fprintf(ArqMonta, "\n.byte 0\n");
  	TerminarSegmentoDeDados();
  	IniciarSegmentoDeCodigo();
  	fprintf(ArqMonta, "L%d:\n", Rot2);
  	fprintf(ArqMonta, " %30s leal  L%d, %s\n", BRANCO, Rot1, Reg32[AlocarRegistrador()]);
}

/*-----------------------------------*/
/* gerar expressao do tipo caractere */
/*-----------------------------------*/
PUBLIC void GerarCaract(char *Id, int *T)
{
  	int   	Cont;
  	int   	Rot1;
  	int   	Rot2;
	char	Caract;

	Caract = Id[1];
  	Rot1 = ObterRotulo();
  	Rot2 = ObterRotulo();
  	*T = CHAR;
  	fprintf(ArqMonta, " %-30s jmp   L%d\n", BRANCO, Rot2);
  	TerminarSegmentoDeCodigo();
  	IniciarSegmentoDeDados();
  	fprintf(ArqMonta, "L%d:\n", Rot1);
    	fprintf(ArqMonta, ".byte '%c'\n", Caract);
	fprintf(ArqMonta, ".byte 0\n");
  	TerminarSegmentoDeDados();
  	IniciarSegmentoDeCodigo();
  	fprintf(ArqMonta, "L%d:\n", Rot2);
  	fprintf(ArqMonta, " %30s leal  L%d, %s\n", BRANCO, Rot1, Reg32[AlocarRegistrador()]);
}
/*------------------------------------*/
/* atribuir um valor a um registrador */
/*------------------------------------*/
PUBLIC void GerarFator(int *R, int  *T, int Tk, char *Id)
{
  	int	Ind; /* indice da tab. de simb. */
  	int 	C;   /* categoria */
  	int 	N;   /* nivel */
  	long    D;   /* deslocamento */

  	//C = CVAR;
  	N  = NGLOBAL;
  	D  = 0;
  	*R = AlocarRegistrador();
  	if (Tk == NUMERO)
  	{
    		*T = INT;
    		fprintf(ArqMonta, " %30s movl  $%s, %s\n", BRANCO, Id, Reg32[*R]);
  	}
	else
  	if (Tk == CHAR)
  	{
    		*T = CHAR;
    		fprintf(ArqMonta, " %30s movl  $%s, %s\n", BRANCO, Id, Reg32[*R]);
  	}
	else
  	if (Tk == IDENTIF)
  	{
    		if ((Ind = Procurar(Id)) == -1) Abortar(3);
    		else
    		{
      			*T = Tabela[Ind]->Tipo;
      			D  = Tabela[Ind]->Desl;
      			N  = Tabela[Ind]->Nivel;
      			C  = Tabela[Ind]->Categ;
    		}
    		if ((C != CVARG) &&
		    (C != CVARL) &&
                    (C != CPAR)) Abortar(6);
    		/* variavel global */
    		if (N == NGLOBAL)
    		{
      			if (*T == INT)
        			fprintf(ArqMonta, " %30s movl  _%s, %s\n", BRANCO, Id, Reg32[*R]);
      			else
			if (*T == CHAR)
        			fprintf(ArqMonta, " %30s movl  _%s, %s\n", BRANCO, Id, Reg32[*R]);
			else
      			if (*T == CADEIA)
        			fprintf(ArqMonta, " %30s leal  _%s, %s\n",  BRANCO, Id, Reg32[*R]);
    		}
    		else
    		{
    			/* variavel local */
      			if (C == CVARL)
      			{
        			if (*T == INT)
          				fprintf(ArqMonta, " %30s movl  -%ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
        			else
				if (*T == CHAR)
          				fprintf(ArqMonta, " %30s movl  -%ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
				else
        			if (*T == CADEIA)
          				fprintf(ArqMonta, " %30s leal  -%ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
      			}
      			else
      			{
      				/* parametro */
        			if (*T == INT)
          				fprintf(ArqMonta, " %30s movl  %ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
        			else
				if (*T == CHAR)
          				fprintf(ArqMonta, " %30s movl  %ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
				else
        			if (*T == CADEIA)
          				fprintf(ArqMonta, " %30s movl  %ld(%%ebp), %s\n", BRANCO, D, Reg32[*R]);
      			}
    		}
	}
}

/*--------------------------*/
/* gerar Fator -> valor(Id) */
/*--------------------------*/
//PUBLIC void GerarFatorValor(int *R, int *T, char *Id)
//{
//  	int Ind; /* indice na tabela */
//  	int N;   /* nivel */
//  	int C;   /* categoria */
//  	long         D;   /* deslocamento */
//
//}

/*----------------------*/
/* gerar sinal de menos */
/*----------------------*/
PUBLIC void GerarMenos(int M, int R)
{
  	if (M) fprintf(ArqMonta, " %30s negl  %s\n", BRANCO, Reg32[R]);
}

/*------------------------------------------------------------*/
/* gerar codigo para valor(Id) do lado esquerdo da atribuicao */
/*------------------------------------------------------------*/
PUBLIC void GerarValor(int T, char *Id)
{
  	int	Ind;  /* indice na tabela */
  	int 	Niv;  /* nivel */
  	int 	Cat;  /* categoria */
  	int 	TId;  /* tipo do Id */
  	long    Desl; /* deslocamento */

  	Niv  = NGLOBAL;
  	//Cat  = CVAR;
  	TId  = INT;
  	Desl = 0;
  	if ((Ind = Procurar(Id)) == -1) Abortar(3);
  	else
  	{
    		TId  = Tabela[Ind]->Tipo;
    		Niv  = Tabela[Ind]->Nivel;
    		Cat  = Tabela[Ind]->Categ;
    		Desl = Tabela[Ind]->Desl;
  	}
  	DesalocarRegistrador();
}

/*------------------------------*/
/* gerar atribuicao mov id, reg */
/*------------------------------*/
PUBLIC void GerarAtribuicao(char *Id, int T)
{
	int	Ind;
  	int 	Tipo;
  	int 	Catg;
  	int 	Nivl;
  	long    Desl;

  	Tipo = T;
  	//Catg = CVAR;
  	Nivl = NGLOBAL;
  	Desl = 0;
  	if ((Ind = Procurar(Id)) == -1) Abortar(3);
  	else
  	{
    		Tipo = Tabela[Ind]->Tipo;
    		Catg = Tabela[Ind]->Categ;
    		Desl = Tabela[Ind]->Desl;
    		Nivl = Tabela[Ind]->Nivel;
  	}
  	if ((Catg != CVARL) &&
            (Catg != CVARG) &&
            (Catg != CPAR)) Abortar(6);
// DEBUG: printf("Id=%s com tipo %d e expressao com tipo T=%d\n", Id, Tipo, T);
  	if (Tipo != T) Abortar(4);
// DEBUG: printf("Id=%s e T=%d\n", Id, T);
  	/* variavel global */
  	if (Nivl == NGLOBAL)
  	{
    		if (Tipo == INT)
      			fprintf(ArqMonta, " %30s movl  %%ebx, _%s\n", BRANCO, Id);
    		else
    		if (Tipo == CHAR)
      			fprintf(ArqMonta, " %30s movl  %%ebx, _%s\n", BRANCO, Id);
		else
    		if (Tipo == CADEIA)
    		{
      			fprintf(ArqMonta, " %30s leal  _%s, %%ecx\n", BRANCO, Id);
      			fprintf(ArqMonta, " %30s pushl %%ecx\n",      BRANCO);
      			fprintf(ArqMonta, " %30s pushl %%ebx\n",      BRANCO);
      			fprintf(ArqMonta, " %30s call  _copcad\n",    BRANCO);
      			fprintf(ArqMonta, " %30s addl  $8, %%esp\n",  BRANCO);
    		}
  	}
  	else
  	{
    		/* variavel local */
    		if (Catg == CVARL)
    		{
      			if (Tipo == INT)
        			fprintf(ArqMonta, " %30s movl  %%ebx, -%ld(%%ebp)\n", BRANCO, Desl);
      			else
      			if (Tipo == CHAR)
        			fprintf(ArqMonta, " %30s movl  %%ebx, -%ld(%%ebp)\n", BRANCO, Desl);
			else
      			if (Tipo == CADEIA)
      			{
        			fprintf(ArqMonta, " %30s leal  -%ld(%%ebp), %%ecx\n", BRANCO, Desl);
        			fprintf(ArqMonta, " %30s pushl %%ecx\n",              BRANCO);
        			fprintf(ArqMonta, " %30s pushl %%ebx\n",              BRANCO);
        			fprintf(ArqMonta, " %30s call  _copcad\n",            BRANCO);
        			fprintf(ArqMonta, " %30s addl  $8, %%esp\n",          BRANCO);
      			}
    		}
    		else
    		{
    			/* parametro */
      			if (Tipo == INT)
        			fprintf(ArqMonta, " %30s movl  %%ebx, %ld(%%ebp)\n", BRANCO, Desl);
      			else
      			if (Tipo == CHAR)
        			fprintf(ArqMonta, " %30s movl  %%ebx, %ld(%%ebp)\n", BRANCO, Desl);
			else
      			if (Tipo == CADEIA)
      			{
        			fprintf(ArqMonta, " %30s movl  -%ld(%%ebp), %%ecx\n", BRANCO, Desl);
        			fprintf(ArqMonta, " %30s pushl %%ecx\n",              BRANCO);
        			fprintf(ArqMonta, " %30s pushl %%ebx\n",              BRANCO);
        			fprintf(ArqMonta, " %30s call  _copcad\n",            BRANCO);
        			fprintf(ArqMonta, " %30s add  $8, %%esp\n",           BRANCO);
      			}
    		}
	}
  	DesalocarRegistrador();
}

/*------------------------------*/
/* gerar atribuicao para vetor  */
/*------------------------------*/
PUBLIC void GerarAtribuicaoVetor(char *Id, int T)
{
  	int	Ind;
  	int 	Tipo;
  	int 	Catg;

  	Tipo = INT;
  	//Catg = CVAR;	  
  	if ((Ind = Procurar(Id)) == -1) Abortar(3);
  	else
  	{
    		Tipo = Tabela[Ind]->Tipo;
    		Tipo = Tabela[Tipo]->Tipo;
    		Catg = Tabela[Ind]->Categ;
  	}
  	if ((Catg != CVETG) && 
      	    (Catg != CVETL)) Abortar(6);
  	if (Tipo != T)    Abortar(4);
  	fprintf(ArqMonta, " %30s popl  %%eax\n", BRANCO);
  	if (Tipo == INT)
  	{
    		fprintf(ArqMonta, " %30s xchgl %%ebx, %%eax\n",   BRANCO);
    		fprintf(ArqMonta, " %30s movl  %%eax, (%%ebx)\n", BRANCO);
  	}
  	else
  	if (Tipo == CHAR)
  	{
    		fprintf(ArqMonta, " %30s xchgl %%ebx, %%eax\n",   BRANCO);
    		fprintf(ArqMonta, " %30s movl  %%eax, (%%ebx)\n", BRANCO);
  	}
  	else
  	if (Tipo == CADEIA)
  	{
    		fprintf(ArqMonta, " %30s pushl %%eax\n",     BRANCO);
    		fprintf(ArqMonta, " %30s pushl %%ebx\n",     BRANCO);
	  //    fprintf(ArqMonta, "extrn _copcad:near\n");
    		fprintf(ArqMonta, " %30s call  _copcad\n", BRANCO);
    		fprintf(ArqMonta, " %30s addl  $8, %%esp\n",  BRANCO);
  	}
  	DesalocarRegistrador();
}

/*-------------------------------*/
/* gerar chamada de procedimento */
/*-------------------------------*/
PUBLIC void GerarChamada(char *Id, int NA, int NB)
{
	int	Ind; /* indice na tabela */
  	int 	Cat; /* categoria */
  	int 	NP;  /* numero de parametros */

//DEBUG: printf("Id=%s, NA=%d, NB=%d\n", Id, NA, NB);
  	if ((Ind = Procurar(Id)) == -1)
  	{
//DEBUG: printf("Não encontrou %s na tabela de simbolos!\n", Id);
    		Cat = CPROC;
    		//if (Cat != CPROC) Abortar(6);
		//if (Cat != CFUNC) Abortar(6);
    		//NP  = Tabela[Ind]->Tipo;      /* contem o numero de parametros */
    		//if (NA > NP) Abortar(8);
    		//if (NA < NP) Abortar(9);
  	}
  	else
  	{
// DEBUG: printf("Sim encontrou %s na tabela de simbolos!\n", Id);
    		Cat = Tabela[Ind]->Categ;     /* id deve ser proc interna */
    		if ((Cat != CPROC) && (Cat != CFUNC)) Abortar(6);
    		NP  = Tabela[Ind]->Tipo;      /* contem o numero de parametros */
    		if (NA > NP) Abortar(8);
    		if (NA < NP) Abortar(9);
  	}
// DEBUG: printf("Chegou no fim de GerarChamada.\n");
  	fprintf(ArqMonta, " %30s call  _%s\n", BRANCO, Id);
  	if (NB > 0)
    		fprintf(ArqMonta, " %30s addl  $%d, %%esp\n", BRANCO, NB);
}

/*----------------------------------*/
/* gerar comando retorne expressao */
/*----------------------------------*/
PUBLIC void GerarRetorne()
{
  	fprintf(ArqMonta, " %30s movl  %%ebx, %%eax\n", BRANCO);
  	DesalocarRegistrador();
}

/*--------------------------*/
/* gerar comando se parte 1 */
/*--------------------------*/
PUBLIC void GerarSe1(int *Rot1, int *Rot2)
{
  	*Rot1 = ObterRotulo();
  	*Rot2 = ObterRotulo();
  	fprintf(ArqMonta, " %30s cmpl  $0, %%ebx\n", BRANCO);
  	fprintf(ArqMonta, " %30s jne   L%d\n",       BRANCO, *Rot1);
  	fprintf(ArqMonta, " %30s jmp   L%d\n",       BRANCO, *Rot2);
  	fprintf(ArqMonta, "L%d:\n",                  *Rot1);
  	DesalocarRegistrador();
}

/*--------------------------*/
/* gerar comando se parte 2 */
/*--------------------------*/
PUBLIC void GerarSe2(int Rot2, int *Rot3)
{
  	*Rot3 = ObterRotulo();
  	fprintf(ArqMonta, " %30s jmp   L%d\n", BRANCO, *Rot3);
  	fprintf(ArqMonta, "L%d:\n",           Rot2);
}

/*--------------------------*/
/* gerar comando se parte 3 */
/*--------------------------*/
PUBLIC void GerarSe3(int Rot3)
{
  	fprintf(ArqMonta, "L%d:\n", Rot3);
}

/*--------------------------------*/
/* gerar comando enquanto parte 1 */
/*--------------------------------*/
PUBLIC void GerarEnquanto1(int *Rot1)
{
  	*Rot1 = ObterRotulo();
  	fprintf(ArqMonta, "L%d:\n", *Rot1);
}

/*--------------------------------*/
/* gerar comando enquanto parte 2 */
/*--------------------------------*/
PUBLIC void GerarEnquanto2(int *Rot3)
{
  	int	Rot2;

  	Rot2  = ObterRotulo();
  	*Rot3 = ObterRotulo();
  	fprintf(ArqMonta, " %30s cmpl  $0, %%ebx\n", BRANCO);
  	fprintf(ArqMonta, " %30s jne   L%d\n",       BRANCO, Rot2);
  	fprintf(ArqMonta, " %30s jmp   L%d\n",       BRANCO, *Rot3);
  	fprintf(ArqMonta, "L%d:\n",                  Rot2);
  	DesalocarRegistrador();
}

/*--------------------------------*/
/* gerar comando enquanto parte 3 */
/*--------------------------------*/
PUBLIC void GerarEnquanto3(int Rot1, int Rot3)
{
  	fprintf(ArqMonta, " %30s jmp   L%d\n", BRANCO, Rot1);
  	fprintf(ArqMonta, "L%d:\n",           Rot3);
}

/*--------------------------------*/
/* gerar comando enquanto parte 1 */
/*--------------------------------*/
PUBLIC void GerarRepeticao1(int *Rot1)
{
  	*Rot1 = ObterRotulo();
  	fprintf(ArqMonta, "L%d:\n", *Rot1);
}

/*--------------------------------*/
/* gerar comando enquanto parte 2 */
/*--------------------------------*/
PUBLIC void GerarRepeticao2(int Rot1)
{
	int	Rot2;

  	Rot2 = ObterRotulo();
  	fprintf(ArqMonta, " %30s cmpl  $0, %%ebx\n", BRANCO);
  	fprintf(ArqMonta, " %30s jne   L%d\n",       BRANCO, Rot2);
  	fprintf(ArqMonta, " %30s jmp   L%d\n",       BRANCO, Rot1);
  	fprintf(ArqMonta, "L%d:\n", Rot2);
  	DesalocarRegistrador();
}

/*--------------------------------------------------*/
/* desassociar o arquivo com o analisador semantico */
/*--------------------------------------------------*/
PUBLIC void TerminarAnalisadorSemantico()
{
	fclose(ArqMonta);
}

/*----------------------------*/
/* listar a tabela de simbolos */
/*----------------------------*/
PUBLIC void Listar()
{
	TSimbolo	*P;
  	int Ind;

  	for (Ind=0; Ind<TopoTab; Ind++)
  	{
    		P = Tabela[Ind];
    		printf("%d> Nome=%30s, Categoria=%5d, Nivel=%1d, Desloc=%5ld, Tipo=%5d\n", 
		        Ind, P->Nome,   P->Categ,      P->Nivel,  P->Desl,     P->Tipo);
  	}
}
