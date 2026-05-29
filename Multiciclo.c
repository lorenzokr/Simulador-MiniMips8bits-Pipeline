#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <stdint.h>
#define RESET       "\x1b[0m"
#define BOLD        "\x1b[1m"
#define UNDERLINE   "\x1b[4m"
#define BG_GRAY     "\x1b[100m"

#define CLR_RED     "\x1b[31m"
#define CLR_GREEN   "\x1b[32m"
#define CLR_YELLOW  "\x1b[33m"
#define CLR_BLUE    "\x1b[34m"
#define CLR_MAGENTA "\x1b[35m"
#define CLR_CYAN    "\x1b[36m"
#define CLR_GRAY    "\x1b[90m"


#define CLR_C       CLR_CYAN
#define CLR_G       CLR_GREEN
#define CLR_Y       CLR_YELLOW
#define CLR_R       CLR_RED
#define CLR_B       CLR_BLUE

#define CL_CYAN     CLR_CYAN
#define CL_YELLOW   CLR_YELLOW
#define CL_GREEN    CLR_GREEN
#define CL_MAGENT   CLR_MAGENTA
#define CL_GRAY     CLR_GRAY

//struct para busca
typedef struct instrucao {
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int8_t  imm;
    int  addr;
    int64_t dado;
} instrucao;
//struct para decodificação
typedef struct unidade_controle {
    int ALUSrc;
    int MemRead;
    int jump;
    //novos sinais
    int origPC;
    int ula_fonteA;
    int ula_fonteB;
    int RegWrite;
    int RegDst;
    int MemToReg;
    int Irwrite;
    int MemWrite;
    int PCwrite;
    int IouD;
    int ALUOp;
    int Branch;
}controle;

typedef struct metricas {
    int contInst;
    int contInstReg;
    int contInstImm;
    int contInstJump;
    int contClock;
    int clockTime;
} metricas;

typedef struct nodoPilha nodoPilha;

typedef struct nodoPilha {
    nodoPilha *ant;
    int registradores[8], pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT;
    char RegIR[17], memu[256][17];
    metricas metricas;
    int estado_atual;
    int etapa;
    instrucao instr;
    int funct;
} nodoPilha;

typedef struct descritorPilha {
    nodoPilha *topo;
} descritorPilha;


int registradores[8] = {0};
int memoria[256] = {0};
int oldreg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int oldmem[256] = {0};
int oldpc=0;
char memu[256][17];
FILE *mem = NULL;
char **mem_instr = NULL;

instrucao decodificar(char *bin);
void imprimir_ass (char*bin, char memu[256][17], int k);
void carregamem (char memu[256][17]);
void imprimir_mem_instr(char memu[256][17], int m, int n, char* bin);
void imprimir_reg();
void imprimir_instrucao(instrucao p);
instrucao busca (char *bin, char memu[256][17], int pc);
int ula(int op1, int op2, controle c, int *overflow,int *zero);//adicionei o zero na função da ula que vai ser utilizado para o beq
int sign_extend6to8(int imm);
void gerar_asm(instrucao p,int pc,char bin[]);
void mostrar_metricas(metricas);
void conversao(char bin[], int numero);
void complemento2(char bin[]);
void imprimir_memoria(char memu[256][17],int m,int n, char* bin);
int mux_IouD(controle c,int entrada1,int entrada2);
int mux_memtoreg(controle c,int entrada1,int entrada2);
int mux_RegDst(controle c,int entrada1,int entrada2);
int mux_operandoA_ULA(controle c,int entrada1,int entrada2);
int mux_operandoB_ULA(controle c,int entrada1,int entrada2,int entrada3);
int mux_fontePC(controle c,int entrada1,int entrada2,int entrada3);
controle sinais_controle_multiclo(int estado_atual, int funct,int opcode,int *proximo_estado,int *proxima_etapa);
void etapa_busca_multiciclo(int *estado_atual,char menu[256][17],int *pc,char RegIR[17],int *etapa, metricas *metricas);
instrucao etapa_decodificacao_multiciclo(int *estado_atual,char RegIR[17],int *Reg_aluout,int banco_reg[],int *reg_tempA,int *reg_tempB,int pc,int *etapa, metricas *metricas);
void terceiro_estagio_multiciclo(int *estado_atual,int *etapa,int Reg_tempA,int Reg_tempB,int *Reg_aluout,int extensao_sinal,int valor_jump,int *pc,int opcode, int funct, metricas *metricas);
void quarto_estagio_multiciclo(int *estado_atual,int *etapa,int *Reg_aluout,int Reg_tempB,int Reg_tempA,int RD,char mem[256][17],int registradores[8],int *MDR,int pc,int extensao_sinal,int opcode,int rt, metricas *metricas,int funct);
void quinto_estagio_multiciclo(int *estado_atual,int *etapa,int MDR,int Reg_aluout,int rd,int rt,int registradores[8], metricas *metricas,int funct,int opcode);
void push(descritorPilha *descritor, int registradores[8], char RegIR[17], int pc, int Reg_tempA, int Reg_tempB, int Reg_dados, int Reg_aluOUT, metricas metricas, int etapa, int estadoAtual, instrucao instrucao, int funct);
void pop(descritorPilha *descritor, int registradores[8], char RegIR[17], int *pc, int *Reg_tempA, int *Reg_tempB, int *Reg_dados, int *Reg_aluOUT, metricas *metricas, int *etapa, int *estadoAtual, instrucao *instrucao, int *funct);
void empty (descritorPilha *descritor);
void limparBuffer (); 
void resetar(int *pc, int *estado_atual, int *etapa, char memu[256][17], int registradores[8], metricas *metricas);

void saidamem(char memu[256][17]);
void diagrama(int mpr, int a, int b, int ulaout, int pc, char RegIR[17]);
char *extender_sinal_8_para_16(char origem[9]);


int main() {
    FILE *mem = NULL;
    int funct = 0;
    int m = 256;
    int n = 17;
    int escolha=1,pc=0,estado_atual=0,etapa=1;
    char bin[17];
    int Reg_tempA=0;
    int Reg_tempB=0;
    int Reg_dados=0;
    char RegIR[17];
    int Reg_aluOUT=0;
    instrucao i;
    controle c;
    metricas metricas = {0};
    int clockTime;
    descritorPilha Pilha;
    Pilha.topo = NULL;
    printf("\n\n -=-=-= SIMULADOR MINI-MIPS 8 BITS MULTICICLO =-=-=-\n\n\n Qual o tempo de clock do simulador (em ps)? ");
    scanf("%i", &metricas.clockTime);
    do { 
     printf("\nMenu de opcoes do programa");
     printf("\n[1] Carregar memoria de instrucao");
     printf("\n[2] Executar clocks no multiclo");
     printf("\n[3] Imprimir memoria");
     printf("\n[4] Imprimir banco de registradores");
     printf("\n[5] Imprimir todo simulador");
     printf("\n[6] Salvar .mem");
     printf("\n[7] Mostrar Estatísticas do programa");
     printf("\n[8] Executar programa(RUN)");
     printf("\n[9] Resetar o programa");
     printf("\n[10] Voltar clocks no multiciclo");
     printf("\n[0] Encerrar programa");
     printf("\nEscolha uma opção: ");
     scanf("%d",&escolha);
     switch (escolha) {
         case 1:
             printf("\nCarregando memoria\n");
             carregamem(memu);
             break;
         case 2: 
            int cont=0;
            int alvo=0;
            printf("Quantos clocks executar: ");
            scanf("%d", &alvo);
            while(cont <alvo && pc <=127){
            switch (etapa)
            {
            case 1:
                printf("\nETAPA DE BUSCA!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                etapa_busca_multiciclo(&estado_atual,memu,&pc,RegIR,&etapa, &metricas);
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;
                break;
            case 2:
                printf("\nETAPA DE DECODIFICAÇÂO!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                i=etapa_decodificacao_multiciclo(&estado_atual,RegIR,&Reg_aluOUT,registradores,&Reg_tempA,&Reg_tempB,pc,&etapa, &metricas);
                funct = i.funct;
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;
                break;

            case 3:
                printf("\nETAPA DE EXECUÇÂO!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                terceiro_estagio_multiciclo(&estado_atual,&etapa,Reg_tempA,Reg_tempB,&Reg_aluOUT,i.imm,i.addr,&pc,i.opcode, funct, &metricas);
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;
                break;
            case 4:
                printf("\nETAPA 4!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                quarto_estagio_multiciclo(&estado_atual,&etapa,&Reg_aluOUT,Reg_tempB,Reg_tempA,i.rd,memu,registradores,&Reg_dados,pc,i.imm,i.opcode,i.rt, &metricas,i.funct);
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;
                break;
            case 5:
                printf("\nETAPA 5!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                quinto_estagio_multiciclo(&estado_atual,&etapa,Reg_dados,Reg_aluOUT,i.rd,i.rt,registradores, &metricas,i.funct,i.opcode);
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;  
                break;
            default:
                break;
            }
          }
         break;
         case 3:
            imprimir_memoria(memu,m,n,bin);
         break;
         case 4: 
         imprimir_reg();
         break;
         case 5:
            imprimir_memoria(memu,m,n,bin);
            printf("\nImprimir banco de registradores");
            imprimir_reg();
            printf("\nvalor pc:%d",pc);
         break;
         case 6:
          saidamem(memu);
          printf("Arquivo de saida.mem gerado!\n");
         break;
         case 7:
         mostrar_metricas(metricas);
         break;
         case 8:
            while(pc <=127){
            switch (etapa)
            {
            case 1:
                printf("\nETAPA DE BUSCA!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                etapa_busca_multiciclo(&estado_atual,memu,&pc,RegIR,&etapa, &metricas);
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;
                break;
            case 2:
                printf("\nETAPA DE DECODIFICAÇÃO!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                i=etapa_decodificacao_multiciclo(&estado_atual,RegIR,&Reg_aluOUT,registradores,&Reg_tempA,&Reg_tempB,pc,&etapa, &metricas);
                funct = i.funct;
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;
                break;

            case 3:
                printf("\nETAPA 3 DE EXECUÇÃO!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                terceiro_estagio_multiciclo(&estado_atual,&etapa,Reg_tempA,Reg_tempB,&Reg_aluOUT,i.imm,i.addr,&pc,i.opcode, funct, &metricas);
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;
                break;
            case 4:
                printf("\nETAPA 4 DE EXECUÇÃO!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                quarto_estagio_multiciclo(&estado_atual,&etapa,&Reg_aluOUT,Reg_tempB,Reg_tempA,i.rd,memu,registradores,&Reg_dados,pc,i.imm,i.opcode,i.rt, &metricas,i.funct);
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;
                break;
            case 5:
                printf("\nETAPA 5 DE EXECUÇÃO!\n");
                push(&Pilha, registradores, RegIR, pc, Reg_tempA, Reg_tempB, Reg_dados, Reg_aluOUT, metricas, etapa, estado_atual, i, funct);
                quinto_estagio_multiciclo(&estado_atual,&etapa,Reg_dados,Reg_aluOUT,i.rd,i.rt,registradores, &metricas,i.funct,i.opcode);
                diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
                cont++;  
                break;
            default:
                break;
            }
          }
          printf("\n\n-----PROGRAMA EXECUTADO!-----\n");
         break;
         case 9: 
          resetar(&pc,&estado_atual,&etapa,memu, registradores, &metricas);
          printf("\nPrograma resetado!\n");
          break;
        case 10:
            int clock;
            cont = 0;
            printf("\n\n Quantos clocks voltar no simulador?");
            scanf("%i", &clock);
            while (cont < clock && Pilha.topo != NULL) {
                pop(&Pilha, registradores, RegIR, &pc, &Reg_tempA, &Reg_tempB, &Reg_dados, &Reg_aluOUT, &metricas, &etapa, &estado_atual, &i, &funct);
                cont++;
            }
            i = busca(bin, memu, pc);
            diagrama(Reg_dados, Reg_tempA, Reg_tempB, Reg_aluOUT, pc, RegIR);
            break;
        default:
             empty(&Pilha);
             return 0;
             break;
    }
} while (escolha !=0);

empty(&Pilha);
return 0;
}

void diagrama(int mdr, int a, int b, int ulaout, int pc, char RegIR[17])
{
    printf("\n %s%s VALORES DOS REGISTRADORES TEMPORARIOS %s\n",
           BOLD, BG_GRAY, RESET);
    printf(" %s┌──────────────────────────────────────┐%s\n",
           CLR_C, RESET);
    printf(" %s│ %-36s │%s\n",
           CLR_C, "", CLR_C);
    printf(" %s│ %sPC:%s %-32d │%s\n",
           CLR_C,
           CLR_Y, RESET, pc,
           CLR_C);
    printf(" %s│ %-36s │%s\n",
           CLR_C, "", CLR_C);
    printf(" %s├──────────────────────────────────────┤%s\n",
           CLR_C, RESET);
    printf(" %s│ %sMDR:%s %-31d │%s\n",
           CLR_C,
           CLR_Y, RESET, mdr,
           CLR_C);
    printf(" %s│ %sIR:%s  %-31s │%s\n",
           CLR_C,
           CLR_B, RESET, RegIR,
           CLR_C);
    printf(" %s├──────────────────────────────────────┤%s\n",
           CLR_C, RESET);
    printf(" %s│ %sA:%s   %-31d │%s\n",
           CLR_C,
           CLR_G, RESET, a,
           CLR_C);
    printf(" %s│ %sB:%s   %-31d │%s\n",
           CLR_C,
           CLR_G, RESET, b,
           CLR_C);
    printf(" %s├──────────────────────────────────────┤%s\n",
           CLR_C, RESET);
    printf(" %s│ %sALUOut:%s %-28d │%s\n",
           CLR_C,
           CLR_R, RESET, ulaout,
           CLR_C);
    printf(" %s└──────────────────────────────────────┘%s\n",
           CLR_C, RESET);
         imprimir_reg();
}


void carregamem(char memu[256][17]) {

    char arq[256];

    printf("Digite o nome do arquivo a ser lido: ");
    scanf("%s", arq);

    mem = fopen(arq, "r");
    if (mem == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    int addr_inst = 0;
    int addr_data = 128;
    int modo = 0;

    char aux[32];

    while (fgets(aux, sizeof(aux), mem) != NULL) {

        aux[strcspn(aux, "\n")] = '\0';

        // muda para seção de dados
        if (strcmp(aux, ".data") == 0) {
            modo = 1;
            continue;
        }

        if (strlen(aux) == 0) continue;

        if (modo == 0) {
            // instruções (0–127)
            if (addr_inst <128) {
                strncpy(memu[addr_inst], aux, 16);
                memu[addr_inst][16] = '\0';
                addr_inst++;
            }
        } else {
            // dados (128–255)
            if (addr_data < 256) {
                strcpy(memu[addr_data], aux);
                addr_data++;
            }
        }
    }

    fclose(mem);

    printf("Memoria carregada!\n");
}


instrucao decodificar(char *bin) {
    unsigned int valor = strtoul(bin, NULL, 2);
    instrucao i;
    i.opcode = (valor >> 12) & 0xF;
    if (i.opcode==0) {
        //Tipo R
    i.rs = (valor >> 9) & 0x7;
    i.rt = (valor >> 6) & 0x7;
    i.rd = (valor >> 3) & 0x7;
    i.funct = valor & 0x7; }
    else if (i.opcode==2) {
        //TIPO J
        i.addr=valor & 0xff;
        i.rs=0;
        i.rd=0;
        i.rt=0;
        i.imm=0;
        i.funct=0;
        }
        else {
            //TIPO I
            i.rs=(valor >> 9) & 0x7;
            i.rt=(valor >> 6) & 0x7;
            i.imm=valor & 0x3f;
        } return i;
}


void imprimir_ass (char*bin, char memu[256][17], int k){
    strcpy(bin, memu[k]);
    instrucao i = decodificar(bin);
    imprimir_instrucao(i); 
}

void imprimir_reg() {
    int i;
    
    // Título com fundo e negrito
    printf("\n  %s%s  BANCO DE REGISTRADORES MIPS  %s\n", BOLD, BG_GRAY, RESET);
    
    // Borda superior
    printf("  %s┌──────────────┬──────────────┐%s\n", CLR_C, RESET);

    for (i = 0; i < 4; i++) {
        // Imprime dois registradores por linha (R0 e R4, R1 e R5...)
        // %2d para o índice e %6d para o valor garante que a tabela não "quebre" com números grandes
        printf("  %s│%s %sR%d:%s %6d %s  │%s %sR%d:%s %6d %s  │%s\n", 
               CLR_C, RESET, 
               CLR_Y, i, CLR_G, registradores[i], 
               CLR_C, RESET, 
               CLR_Y, i + 4, CLR_G, registradores[i + 4], 
               CLR_C, RESET);

        // Linha divisória interna
        if (i < 3) {
            printf("  %s├──────────────┼──────────────┤%s\n", CLR_C, RESET);
        }
    }

    // Borda inferior
    printf("  %s└──────────────┴──────────────┘%s\n", CLR_C, RESET);
}

void imprimir_instrucao(instrucao p) {
  switch (p.opcode)
    {
    case 0:
        switch (p.funct)
        {
        case 0:
            printf("Assembly: add $%d,$%d,$%d",p.rd,p.rs,p.rt);
            break;

        case 2:
            printf("Assembly: sub $%d,$%d,$%d",p.rd,p.rs,p.rt);
            break;
        }
        break;

    case 2:
        printf("Assembly: jump %d",p.addr);
        break;

    case 4:
        p.imm=sign_extend6to8(p.imm);
        printf("Assembly: addi $%d,$%d,%d",p.rt,p.rs,p.imm);
        break;

    case 8:
        p.imm=sign_extend6to8(p.imm);
        printf("Assembly: beq $%d,$%d,%d",p.rs,p.rt,p.imm);
        break;

    case 11:
        p.imm=sign_extend6to8(p.imm);
        printf("Assembly: lw $%d,%d($%d)",p.rt,p.imm,p.rs);
        break;

    case 15:
        p.imm=sign_extend6to8(p.imm);
        printf("Assembly: sw $%d,%d($%d)",p.rt,p.imm,p.rs);
        break;

    default:
        printf(" instrucao desconhecida\n");
        break;
    }
}

void limparBuffer() {
    char c;
    while ((c = getchar()) != EOF && c !='\n');
    return;
}

//função que realiza a busca da instrução
instrucao busca (char *bin, char memu[256][17], int pc){
    strcpy(bin, memu[pc]);
    instrucao i = decodificar(bin);
    printf("\ninstrucao em binario:%s\n",bin);
    imprimir_instrucao(i); return i;
}



int ula(int op1, int op2, controle c, int *overflow,int *zero){
    *overflow = 0;
    *zero=0;
    int res=0;
    switch(c.ALUOp){
        case 0: { 
            // ADD
            res=op1 + op2;
            //eu arrumei aqui porque o overflow no caso de 8 bits tem a faixa de -128 ate 127
            if (res>127 || res<-128){
                *overflow=1;
            }
            if (res==0){
                *zero=1;
            }
            return res;
        }
        case 2: 
         // SUB
            res=op1-op2;
            printf("\nOP1:%d op2:%d resultado:%d",op1,op2,res);
            if (res>127 || res<-128){
                *overflow=1;
                
            }
            if (res==0){
                *zero=1;
            }
            return res;
        case 4:
            return op1 & op2;
        case 5:
            return op1 | op2;
        default:
            return 0;
    }
}


int sign_extend6to8(int imm)
{
    if (imm & 0x20)      // verifica bit de sinal (bit 5)
        return imm | 0xC0;  // adiciona 11 nos 2 bits superiores
    else
        return imm;
}


void gerar_asm(instrucao p,int pc,char bin[]){
    FILE *arquivo;
    arquivo = fopen("assembly.asm","a");
    if (!arquivo){
        printf("\nProblema ao gerar arquivo!");
        return;
    }
    switch (p.opcode){
    case 0:
        switch (p.funct){
        case 0:
            fprintf(arquivo,"add $%d,$%d,$%d\n",p.rd,p.rs,p.rt);
            break;

        case 2:
            fprintf(arquivo,"sub $%d,$%d,$%d\n",p.rd,p.rs,p.rt);
            break;
        }
        break;

    case 2:
        fprintf(arquivo,"jump %d\n",p.addr);
        break;

    case 4:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"addi $%d,$%d,%d\n",p.rt,p.rs,p.imm);
        break;

    case 8:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"$%d,$%d,%d\n",p.rs,p.rt,p.imm);
        break;

    case 11:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"lw $%d,%d($%d)\n",p.rt,p.imm,p.rs);
        break;

    case 15:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"sw $%d,%d($%d)\n",p.rt,p.imm,p.rs);
        break;

    default:
        fprintf(arquivo,"instrucao desconhecida\n");
        break;
    }

    fclose(arquivo);
}

char *extender_sinal_8_para_16(char bin[9]){
    static char convertido[17];
    int i;
    char sinal = bin[0];
    for(i = 0; i < 8; i++){
        convertido[i] = sinal;
    }
    for(i = 0; i < 8; i++){
        convertido[i + 8] = bin[i];
    }
    convertido[16] = '\0';
    return convertido;
}


void mostrar_metricas(metricas m) {
    float cpi = 0;
    cpi = (float)m.contClock/m.contInst;
    printf("\n\n---Métricas---"
    "\nInstruções executadas: %i"
    "\nInstruções tipo R executadas: %i"
    "\nInstruções tipo I executadas: %i"
    "\nInstruções tipo J executadas: %i"
    "\nNúmero de clocks: %i"
    "\nTempo de execução: %i ps",
    m.contInst, m.contInstReg, m.contInstImm, m.contInstJump, m.contClock, m.contClock * m.clockTime);
    if (m.contInst != 0) {
        printf("\nCPI: %.2f\n\n", cpi);
    }
    else {
        printf("\n Nenhuma instrução concluída ainda. Não é possível calcular CPI.\n\n");
    }
    
    return;
}
void conversao(char bin[], int numero)
{
    int atual = numero;
    int indice = 15;  
    int resto = 0;
    if (numero < 0) {
        atual = numero & 0xFF; 
    }
    while (atual > 0 && indice >= 0)
    {
        resto = atual % 2;

        if (resto == 0)
            bin[indice] = '0';
        else
            bin[indice] = '1';

        atual /= 2;
        indice--;
    }
    while (indice >= 0)
    {
        bin[indice] = '0';
        indice--;
    }

    bin[16] = '\0'; 
}
int binario_para_decimal_corrigido(char bin[])
{
    int valor = strtol(bin, NULL, 2);
    int tamanho = strlen(bin);

    if (tamanho == 16) {
        // Caso 1: Negativo padrão de 16 bits
        if (bin[0] == '1') {
            valor -= 65536; 
        }
        // Caso 2: Negativo de 8 bits estendido/guardado na memória de 16 bits
        else if (strncmp(bin, "00000000", 8) == 0 && bin[8] == '1') {
            valor -= 256;   
        }
    }
    return valor;
}

// --- Sua Função de Memória Atualizada ---
void imprimir_memoria(char memu[256][17], int m, int n, char* bin)
{
    int k;

    printf("\n%s╔══════════════════════════════════════════════════════════════╗%s\n", BOLD CL_CYAN, RESET);
    printf("%s║%s                %sMAPA DE MEMÓRIA (MIPS MULTICICLO)%s            %s║%s\n", BOLD CL_CYAN, RESET, BOLD CL_YELLOW, RESET, BOLD CL_CYAN, RESET);
    printf("%s╠══════════╦══════════════════════╦════════════════════════════╣%s\n", BOLD CL_CYAN, RESET);
    printf("%s║ %sEndereço %s║ %sBinário (16 bits)    %s║ %sConteúdo / Assembly        %s║%s\n", BOLD CL_CYAN, RESET, BOLD CL_CYAN, RESET, BOLD CL_CYAN, RESET, BOLD CL_CYAN, RESET);
    printf("%s╠══════════╬══════════════════════╬════════════════════════════╣%s\n", BOLD CL_CYAN, RESET);

    for (k = 0; k < m && k < 256; k++) {
        // Imprime o endereço em Amarelo
        printf("%s║ %s  %3d    %s║ ", BOLD CL_CYAN, CL_YELLOW, k, BOLD CL_CYAN);

        // Caso a posição esteja vazia (primeiro caractere nulo ou espaço)
        if (memu[k][0] == '\0' || memu[k][0] == ' ' || memu[k][0] == '\n') {
            printf("%s0000000000000000 %s║ %s", CL_GRAY, BOLD CL_CYAN, CL_GRAY);
            if (k < 128) printf("--- vazio (code) ---      ");
            else         printf("%26d", 0);
            printf(" %s║%s\n", BOLD CL_CYAN, RESET);
            continue;
        }

        // Copia para o buffer 'bin' passado por parâmetro
        strncpy(bin, memu[k], n);
        bin[n] = '\0'; 

        // Imprime o binário em Verde
        printf("%s%s %s║ ", CL_GREEN, bin, BOLD CL_CYAN);

        // Conteúdo: Instrução (0 a 127) ou Dado (128 para cima)?
        if (k < 128) {
            printf("%s", CL_MAGENT);
            
            // Decodifica para mostrar o assembly na tabela
            instrucao inst = decodificar(bin);
            
            // Exibe a instrução formatada para caber no espaço da tabela
            switch(inst.opcode) {
                case 0:  
                    if(inst.funct == 0) printf("add $%d, $%d, $%d", inst.rd, inst.rs, inst.rt);
                    else if(inst.funct == 2) printf("sub $%d, $%d, $%d", inst.rd, inst.rs, inst.rt);
                    else printf("R-type op:%d", inst.funct);
                    break;
                case 2:  printf("j %d", inst.addr); break;
                case 4:  printf("addi $%d, $%d, %d", inst.rt, inst.rs, sign_extend6to8(inst.imm)); break;
                case 8:  printf("beq $%d, $%d, %d", inst.rs, inst.rt, sign_extend6to8(inst.imm)); break;
                case 11: printf("lw $%d, %d($%d)", inst.rt, sign_extend6to8(inst.imm), inst.rs); break;
                case 15: printf("sw $%d, %d($%d)", inst.rt, sign_extend6to8(inst.imm), inst.rs); break;
                default: printf("unknown          ");
            }
            // Ajuste estático de espaçamento para as instruções
            printf("%*s", 4, ""); 
        } else {
            // Segmento de Dados: Aqui usamos a nova função para capturar o valor negativo real
            int valor = binario_para_decimal_corrigido(bin);
            
            // O modificador %26d garante que se o número ganhar um sinal de menos '-', 
            // ele empurra o texto para a esquerda mantendo o alinhamento perfeito da borda direita '║'
            printf("%s%26d", CL_YELLOW, valor);
        }

        printf(" %s║%s\n", BOLD CL_CYAN, RESET);
    }

    printf("%s╚══════════╩══════════════════════╩════════════════════════════╝%s\n", BOLD CL_CYAN, RESET);
}
controle sinais_controle_multiclo(int estado_atual, int funct,int opcode,int *proximo_estado,int *proxima_etapa)
{
    controle c;
    
    switch (estado_atual)
    {
    case 0:
        c.PCwrite=1;
        c.ALUOp=0;
        c.IouD=0;
        c.MemWrite=0;
        c.Irwrite=1;
        c.ula_fonteA=0;
        c.ula_fonteB=1;
        c.RegWrite=0;
        c.Branch=0;
        c.origPC=0;
        *proximo_estado=1;
        *proxima_etapa=2;
        return c;
        break;
    case 1:
        c.PCwrite=0;
        c.MemWrite=0;
        c.Irwrite=0;
        c.ALUOp=0;
        c.ula_fonteA=0;
        c.ula_fonteB=2;
        c.Branch=0;
        c.RegWrite=0;
        switch (opcode)
        {
            case 0:*proximo_estado=7,*proxima_etapa=3;break;//tipo R
            case 4:*proximo_estado=2,*proxima_etapa=3;break;//ADDI
            case 11:*proximo_estado=2,*proxima_etapa=3;break;//LW
            case 15:*proximo_estado=2,*proxima_etapa=3;break;//SW
            case 8:*proximo_estado=9,*proxima_etapa=3;break;
            case 2:*proximo_estado=10,*proxima_etapa=3;break;
            default:*proximo_estado=0,*proxima_etapa=0;break;
        }
        return c;
    case 2:
        c.ula_fonteA=1;
        c.ula_fonteB=2;
        c.ALUOp=0;
        c.PCwrite=0;
        c.Branch=0;
        switch (opcode)
        {
            case 4:*proximo_estado=6,*proxima_etapa=4;break;//ADDI
            case 11:*proximo_estado=3,*proxima_etapa=4;break;//LW
            case 15:*proximo_estado=5,*proxima_etapa=4;break;//SW
            default:*proximo_estado=0,*proxima_etapa=0;break;
        }
        return c;
    case 3:
        c.MemWrite=0;
        c.IouD=1;
        c.ula_fonteA=1;
        c.ula_fonteB=2;
        c.RegWrite=0;
        *proximo_estado=4;
        *proxima_etapa=5;
        return c;
    case 4:
        c.RegWrite=1;
        c.MemToReg=1;
        c.RegDst=0;
        c.ula_fonteA=1;
        c.ula_fonteB=2;
        *proximo_estado=0;//lw
        *proxima_etapa=1;//lw
        return c;
    case 5:
        c.MemWrite=1;
        c.IouD=1;
        c.ula_fonteA=1;
        c.ula_fonteB=2;
        c.RegWrite=0;
        *proximo_estado=0;//sw
        *proxima_etapa=1;//sw
        return c;
    case 6:
        c.MemWrite=0;
        c.RegWrite=1;
        c.RegDst=0;
        c.MemToReg=0;
        c.ula_fonteB=2;
        c.ula_fonteA=1;
        *proximo_estado=0;//addi
        *proxima_etapa=1;//addi
        return c;
    case 7:
        c.PCwrite=0;
        c.MemWrite=0;
        c.Irwrite=0;
        c.ALUOp=funct;
        c.ula_fonteA=1;
        c.ula_fonteB=0;
        c.RegWrite=0;
        c.Branch=0;
        *proximo_estado=8;//tipor
        *proxima_etapa=4;//tipor
        return c;
    case 8:
        c.RegDst=1;
        c.MemToReg=0;
        c.Branch=0;
        c.PCwrite=0;
        c.RegWrite=1;
        c.origPC=0;
        c.MemWrite=0;
        c.Irwrite=0;
        *proximo_estado=0;//tipor
        *proxima_etapa=1;//tipor
        return c;
    case 9:
        c.ula_fonteB=0;
        c.ula_fonteA=1;
        c.ALUOp=2;
        c.Branch=1;
        c.PCwrite=0;
        c.origPC=1;
        *proximo_estado=0;//beq
        *proxima_etapa=1;//beq
        return c;
    case 10:
        c.PCwrite=1;
        c.origPC=2;
        c.Branch = 0;
        *proximo_estado=0;//tipo j
        *proxima_etapa=1;//tipo j
        return c;
    default:
        break;
    }
}
int mux_operandoA_ULA(controle c,int entrada1,int entrada2)
{
    switch (c.ula_fonteA)
    {
    case 0:
        return entrada1;

        break;
    case 1:
        return entrada2;
        break;
    default:
        break;
    }
}
int mux_operandoB_ULA(controle c,int entrada1,int entrada2,int entrada3)
{
    switch (c.ula_fonteB)
    {
    case 0:
        return entrada1;
        break;
    case 1:
        return entrada2;
        break;
    case 2:
        return entrada3;
        break;
    default:
        break;
    }
}
int mux_fontePC(controle c,int entrada1,int entrada2,int entrada3)
{
    switch (c.origPC)
    {
    case 0:
        return entrada1;
        break;
    case 1:
        return entrada2;
        break;
    case 2:
        return entrada3;
        break;
    default:
        break;
    }
}
int mux_memtoreg(controle c,int entrada1,int entrada2)
{
    switch (c.MemToReg)
    {
    case 0:
        return entrada1;

        break;
    case 1:
        return entrada2;
        break;
    default:
        break;
    }
}
int mux_IouD(controle c,int entrada1,int entrada2)
{
    switch (c.IouD)
    {
    case 0:
        return entrada1;

        break;
    case 1:
        return entrada2;
        break;
    default:
        break;
    }
}
int mux_RegDst(controle c,int entrada1,int entrada2)
{
    switch (c.RegDst)
    {
    case 0:
        return entrada1;

        break;
    case 1:
        return entrada2;
        break;
    default:
        break;
    }
}
void etapa_busca_multiciclo(int *estado_atual,char menu[256][17],int *pc,char RegIR[17],int *etapa, metricas *metricas)
{
    //primeiro passo é os sinais de controle para o estado 0
    int op1=0;
    int op2=0;
    int resultado_ula=0;
    int overflow=0;
    int zero=0;
    int saida_muxORIGpc=0;
    int resultado_and=0;
    int resultado_or=0;
    int proximo_estado;
    int proxima_etapa;
    controle c;
    metricas->contClock ++;

    c=sinais_controle_multiclo(*estado_atual, 0,0,&proximo_estado,&proxima_etapa);
    *estado_atual=proximo_estado;
    *etapa=proxima_etapa;
    if (c.Irwrite==1)
    {
        //Se o sinal de escrita no registrador temporario de instrução de instrução
        //estiver ativado iremos escrever nele para isso iremos copiar a instruçao da memoria de instrucao para ele fazendo strcpy
        strcpy(RegIR, memu[*pc]);
        printf("\nInstrução salva em RI:%s\n",RegIR);
    }
    //agora iremos decidir os operandos fontes da ula
    op1=mux_operandoA_ULA(c,*pc,0);
    //observação a entrada 2 do mux para decidir o primeiro operando fonte é valor vindo dos registradores
    //todavia esse valor não foi feito a leitura dos registradores nessa etapa entao seria dont care entretando em c não tem como representar essa coisa
    op2=mux_operandoB_ULA(c,0,1,0);
    //O mesmo raciocinio vale para este mux visto que a primeira entrada vem do banco de regitradores e a 3 entrada
    //vem do sinal extendido do imediato todavia nesta etapa a instrução não foi decodificada portanto dont care
    resultado_ula=ula(op1,op2,c,&overflow,&zero);
    saida_muxORIGpc=mux_fontePC(c,resultado_ula,0,0);
    //as duas entradas estao sendo representa como 0 para representar um dont care
    //visto que nessa etapa não escrevemos no registrador temporario saida da ula e nem utilizamos o valor do jump
    //verificar se vai ter escrita no pc
    resultado_and=c.Branch & zero;
    resultado_or=resultado_and | c.PCwrite;
    if (resultado_or)
    {
        *pc=saida_muxORIGpc;
    }
}
instrucao etapa_decodificacao_multiciclo(int *estado_atual,char RegIR[17],int *Reg_aluout,int banco_reg[],int *reg_tempA,int *reg_tempB,int pc,int *etapa, metricas *metricas)
{
    controle c;
    instrucao i;
    int op1=0;
    int op2=0;
    int resultado_ula=0;
    int overflow=0;
    int zero=0;
    metricas->contClock ++;
    int proximo_estado;
    int proxima_etapa;
    i = decodificar(RegIR);//decodificação
    imprimir_instrucao(i);
    c=sinais_controle_multiclo(*estado_atual, i.funct,i.opcode,&proximo_estado,&proxima_etapa);
    *estado_atual=proximo_estado;
    *etapa=proxima_etapa;
    //agora iremos pegar os valores no registradores e guarda nos registradores temporario
    printf("\nRegistrador rs:%d valor:%d",i.rs,registradores[i.rs]);
    printf("\nRegistrador rt:%d valor:%d",i.rt,registradores[i.rt]);
    *reg_tempA=registradores[i.rs];
    *reg_tempB=registradores[i.rt];
    i.imm=sign_extend6to8(i.imm);
    op1=mux_operandoA_ULA(c,pc,*reg_tempA);
    op2=mux_operandoB_ULA(c,*reg_tempB,1,i.imm);
    resultado_ula=ula(op1,op2,c,&overflow,&zero);
    *Reg_aluout=resultado_ula;
    //agora iremos ver o proximo estado dependendo do opcode
    return i;
}
void terceiro_estagio_multiciclo(int *estado_atual,int *etapa,int Reg_tempA,int Reg_tempB,int *Reg_aluout,int extensao_sinal,int valor_jump,int *pc,int opcode, int funct, metricas *metricas)
{
    controle c;
    int saida_mux_ulafonteA;
    int saida_mux_ulafonteB;
    int saida_mux_fontepc;
    int zero=2,overflow;
    int resultado_ula;
    int verificação_escrita_pc;
    metricas->contClock ++;
    int proximo_estado;
    int proxima_etapa;
    if (*estado_atual == 9) {
        metricas->contInst++;
        metricas->contInstImm++;
    }
    else if (*estado_atual == 10) {
        metricas->contInst++;
        metricas->contInstJump++;
    }
    c=sinais_controle_multiclo(*estado_atual,funct,opcode,&proximo_estado,&proxima_etapa);
    *estado_atual=proximo_estado;
    *etapa=proxima_etapa;
    //primeira parte os mux
    saida_mux_ulafonteA=mux_operandoA_ULA(c,*pc,Reg_tempA);
    saida_mux_ulafonteB=mux_operandoB_ULA(c,Reg_tempB,1,extensao_sinal);
    resultado_ula=ula(saida_mux_ulafonteA,saida_mux_ulafonteB,c,&overflow,&zero);
    //parte da instrução beq ou jump
    saida_mux_fontepc=mux_fontePC(c,resultado_ula,*Reg_aluout,valor_jump);
    printf("\nPc write:%d  branch:%d zero:%d",c.PCwrite,c.Branch,zero);
    verificação_escrita_pc=c.PCwrite |(c.Branch & zero);
    if (verificação_escrita_pc)
    {
        *pc=saida_mux_fontepc;
    }

    *Reg_aluout=resultado_ula;
}
void quarto_estagio_multiciclo(int *estado_atual,int *etapa,int *Reg_aluout,int Reg_tempB,int Reg_tempA,int RD,char mem[256][17],int registradores[8],int *MDR,int pc,int extensao_sinal,int opcode,int rt, metricas *metricas,int funct)
{
    controle c;
    int saida_mux_ulafonteA;
    int saida_mux_ulafonteB;
    int saida_mux_memREG;
    int saida_mux_regDST;
    int saida_mux_Iod;
    int zero=0,overflow=0;
    int resultado_ula;
    char bin[17];
    int proximo_estado;
    int proxima_etapa;
    if (*estado_atual == 8) {
        metricas->contInst++;
        metricas->contInstReg++;
    }
    else if (*estado_atual == 6) {
        metricas->contInst++;
        metricas->contInstImm++;
    }
    c=sinais_controle_multiclo(*estado_atual,funct,opcode,&proximo_estado,&proxima_etapa);
    *estado_atual=proximo_estado;
    *etapa=proxima_etapa;
    metricas->contClock ++;
    saida_mux_ulafonteA=mux_operandoA_ULA(c,pc,Reg_tempA);
    saida_mux_ulafonteB=mux_operandoB_ULA(c,Reg_tempB,1,extensao_sinal);
    resultado_ula=ula(saida_mux_ulafonteA,saida_mux_ulafonteB,c,&overflow,&zero);
    printf("\nReg write:%d",c.RegWrite);
    if (c.MemWrite == 0 && c.RegWrite !=1)
    {
        saida_mux_Iod=mux_IouD(c,pc,*Reg_aluout);
        printf("\nEndereco de leitura:%d",saida_mux_Iod);
        *MDR=binario_para_decimal_corrigido(mem[saida_mux_Iod]);
    }
    else if (c.MemWrite)
    {
        saida_mux_Iod=mux_IouD(c,pc,*Reg_aluout);
        printf("\nEndereco de escrita:%d",saida_mux_Iod);
        conversao(bin,Reg_tempB);
        strcpy(mem[saida_mux_Iod],bin);
    }
    if (c.RegWrite)
    {
        saida_mux_memREG=mux_memtoreg(c,*Reg_aluout,*MDR);
        saida_mux_regDST=mux_RegDst(c,rt,RD);
        registradores[saida_mux_regDST]=saida_mux_memREG;
    }
}
void quinto_estagio_multiciclo(int *estado_atual,int *etapa,int MDR,int Reg_aluout,int rd,int rt,int registradores[8], metricas *metricas,int funct,int opcode)
{
    controle c;
    int saida_mux_memREG;
    int saida_mux_regDST;
    metricas->contClock ++;
    metricas->contInst ++;
    metricas->contInstImm ++;//aqui nao seria lw em vez de ser imediato?      Mas o lw não é um imm, para offset do endereço?
    int proximo_estado;
    int proxima_etapa;
    c=sinais_controle_multiclo(*estado_atual,funct,opcode,&proximo_estado,&proxima_etapa);
    *estado_atual=proximo_estado;
    *etapa=proxima_etapa;
    if (c.RegWrite)
    {
        saida_mux_memREG=mux_memtoreg(c,Reg_aluout,MDR);
        saida_mux_regDST=mux_RegDst(c,rt,rd);
        printf("\nSaida mux mentoreg:%d e muxRegDST:%d ",saida_mux_memREG,saida_mux_regDST);
        registradores[saida_mux_regDST]=saida_mux_memREG;
    }
}
void push(descritorPilha *descritor, int registradores[8], char RegIR[17], int pc, int Reg_tempA, int Reg_tempB, int Reg_dados, int Reg_aluOUT, metricas metricas, int etapa, int estadoAtual, instrucao instrucao, int funct) {

    nodoPilha *nodo = malloc(sizeof(nodoPilha));

    memcpy(nodo->memu, memu, sizeof(nodo->memu));
    memcpy(nodo->registradores, registradores, sizeof(nodo->registradores));
    memcpy(nodo->RegIR, RegIR, sizeof(nodo->RegIR));
    nodo->pc = pc;
    nodo->Reg_aluOUT = Reg_aluOUT;
    nodo->Reg_dados = Reg_dados;
    nodo->Reg_tempA = Reg_tempA;
    nodo->Reg_tempB = Reg_tempB;
    nodo->estado_atual = estadoAtual;
    nodo->etapa = etapa;
    nodo->instr = instrucao;
    nodo->funct = funct;

    nodo->metricas.contClock = metricas.contClock;
    nodo->metricas.contInst = metricas.contInst;
    nodo->metricas.contInstReg = metricas.contInstReg;
    nodo->metricas.contInstImm = metricas.contInstImm;
    nodo->metricas.contInstJump = metricas.contInstJump;

    nodo->ant = descritor->topo;
    descritor->topo = nodo;
    return;
}
void pop(descritorPilha *descritor, int registradores[8], char RegIR[17], int *pc, int *Reg_tempA, int *Reg_tempB, int *Reg_dados, int *Reg_aluOUT, metricas *metricas, int *etapa, int *estadoAtual, instrucao *instrucao, int *funct) {
    
    if(descritor->topo == NULL) {
        printf("\n\n Pilha vazia. ");
        return;
    }
    nodoPilha *nodo = descritor->topo;
    memcpy(memu, nodo->memu, sizeof(nodo->memu));
    memcpy(registradores, nodo->registradores, sizeof(nodo->registradores));
    memcpy(RegIR, nodo->RegIR, sizeof(nodo->RegIR));
    *pc = nodo->pc;
    *Reg_aluOUT = nodo->Reg_aluOUT;
    *Reg_dados = nodo->Reg_dados;
    *Reg_tempA = nodo->Reg_tempA;
    *Reg_tempB = nodo->Reg_tempB;
    *estadoAtual = nodo->estado_atual;
    *etapa = nodo->etapa;
    *instrucao = nodo->instr;
    *funct = nodo->funct;

    metricas->contClock = nodo->metricas.contClock;
    metricas->contInst = nodo->metricas.contInst;
    metricas->contInstReg = nodo->metricas.contInstReg;
    metricas->contInstImm = nodo->metricas.contInstImm;
    metricas->contInstJump = nodo->metricas.contInstJump;
    
    descritor->topo = nodo->ant;
    free(nodo);
}

void empty (descritorPilha *descritor) {
    nodoPilha *nodo = descritor->topo, *nodoAux;
    while (nodo != NULL) {
        nodoAux = nodo->ant;
        free(nodo);
        nodo = nodoAux;
    }
    descritor->topo = NULL;
    return;
}

void resetar(int *pc, int *estado_atual, int *etapa, char memu[256][17], int registradores[8], metricas *metricas){
  *pc=0;
  *estado_atual=0;
  *etapa=1;
  metricas->contInst = 0;
  metricas->contInstReg =0;
  metricas->contInstImm = 0;
  metricas->contInstJump = 0;
  metricas->contClock = 0;
  memset(registradores, 0, 8 * sizeof(int));
  int f;
  printf("Deseja apagar a meoria?\n1.Sim\n2.Não\n");
  scanf("%d", &f);
  switch(f){
    case 1:
    memset(memu, 0, 256*17*sizeof(char));
    printf("Memoria apagada!\n");
    break;
    case 2:
      printf("Memoria mantida!\n");
      break;
    default:
      printf("Opção inavlida, memoria mantida!\n");
    break;
  }
  return;
}


void saidamem(char memu[256][17]){
  int i, j;
  FILE*
  mem = fopen("saida.mem", "w");
    if (mem == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }
  for(i=0;i<256;i++){
    if(i==128){
      fprintf(mem, ".data\n");
    }
    for(j=0;j<16;j++){
      if(memu[i][j] == '\0'){
        fputc('0', mem);
      }
      else{
        fputc(memu[i][j], mem);
      }
    }
        fputc('\n', mem);
    }
  fclose(mem);
  return;
}
