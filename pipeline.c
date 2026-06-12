#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <stdint.h>

FILE *mem = NULL;
char **mem_instr = NULL;
int m = 256;
int n = 16;

typedef struct instrucao {
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int8_t  imm;
    int  addr;
} instrucao;

typedef struct unidade_controle {
    int RegDst;
    int ALUSrc;
    int MemToReg;
    int RegWrite;
    int MemRead;
    int MemWrite;
    int Branch;
    int ALUOp;
    int jump;
}controle;
typedef struct reg_pipeline1
{
    int soma_pc;
    char instrucao[17];
}REG_pepiline_BI_ID;
typedef struct sinais_controle_ex
{
    int ALUOp;
    int RegDst;
    int ALUSrc;

}EX_SINAIS;
typedef struct sinais_controle_mem
{
    int MemWrite;
    int Branch;
    int jump;

}MEM_SINAIS;
typedef struct sinais_controle_wb
{
    int RegWrite;
    int MemToReg;
}WB_SINAIS;
typedef struct reg_pipeline2
{
    int rt;
    int rd;
    int rs;
    int soma_pc;
    int valor_jump;
    instrucao instrucao;
    int8_t  sinal_extendido;
    int saida1_banco_reg;
    int saida2_banco_reg;
    EX_SINAIS sinais_ex;
    MEM_SINAIS sinais_mem;
    WB_SINAIS sinais_wb;
}REG_pepiline_ID_EX;
typedef struct reg_pipeline3
{
    int resultado_ula;
    instrucao instrucao;
    int zero_ula;
    int registrador_destino;
    int soma_pc;
    int valor_jump;
    int endereco_desvio;
    int saida2_banco_registradores;
    MEM_SINAIS sinais_mem;
    WB_SINAIS sinais_wb;
}REG_pepiline_EX_MEM;
typedef struct reg_pipeline4
{
    instrucao instrucao;
    int resultado_ula;
    int saida_memoria;
    int registrador_destino;
    WB_SINAIS sinais_wb;
}REG_pepiline_MEM_WB;
typedef struct forwading
{
    int ex_mem_writeREG;
    int ex_mem_RegRD;
    int id_ex_RegRT;
    int id_ex_RegRS;
    int Mem_WB_WriteREG;
    int mem_wb_RegRD;
}unidade_forwading;
typedef struct sinais_controle
{
    int forwadingA;
    int forwadingB;
}sinais_controle_forwading;
typedef struct metricas {
    int contInst;
    int contInstReg;
    int contInstImm;
    int contInstJump;
} metricas;


int registradores[8]={0, 1, 2, 0, 0, 10, 0, 0};
int memoria[256] = {0};
int oldreg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int oldmem[256] = {0};
int oldpc=0;

instrucao decodificar(char *bin);
void imprimir_ass (char*bin, char** mem_instr, int k);
void carregamem (char **mem_instr, int m, int n);
void imprimir_mem_instr(char **mem_instr, int m, int n, char* bin);
void imprimir_reg();
void imprimir_instrucao(instrucao p);
char **criameminstr(int m, int n);
void desalocameminstr(char **mem_instr, int m, int n);
int mux1(controle c, instrucao i);
int mux_branch(int sinal_branch,int entrada1,int entrada2);
int mux_jump(int sinal_jump,int entrada1,int entrada2);
int somador(int entrada1,int entrada2);
int somador_pc(int entrada1);
instrucao busca (char *bin, char **mem_instr, int pc);
controle sinais_controle(instrucao i, metricas *m, char *ultimaInst);
void executar(instrucao i, controle c, int *pc);
int ula(int op1, int op2, controle c, int *overflow,int *zero);//adicionei o zero na função da ula que vai ser utilizado para o beq
int lwsw(int operacao, int endereco, int dado);
int sign_extend6to8(int imm);
void imprimir_mem_dados(int mem[]);
void gerar_asm(instrucao p,int pc,char bin[]);
void gerar_dat(int mem[]);
void mostrar_metricas(metricas m);
void carregadat (int *mem_dados);
void reduzir_metricas(metricas *m, char ultimaInst);
REG_pepiline_BI_ID estagio_busca(int pc,char **mem_instr);
REG_pepiline_ID_EX estagio_ID(REG_pepiline_BI_ID r,int banco_registrador[7]);
REG_pepiline_EX_MEM estagio_ex(REG_pepiline_ID_EX id,int ex_mem,int mem_wb,sinais_controle_forwading sinal_forwading);
REG_pepiline_MEM_WB estagio_mem(REG_pepiline_EX_MEM ex,int memoria[],int *pc);
void estagio_wb(REG_pepiline_MEM_WB Mem,int banco_registrador[7]);
int mux_ula_fonte(int rt,int imediato,int sinal_ula_fonte);
sinais_controle_forwading forwading_unidade(unidade_forwading f);
int mux_forwadingA(int entrada1,int entrada2,int entrada3,int sinal_forwading);
int mux_forwadingB(int entrada1,int entrada2,int entrada3,int sinal_forwading);
int mux_regDST(int rt,int rd,int sinal_regdst);
int mux_memtoreg(int saida_mem,int saida_ula,int memtoreg);


int main() {
    FILE *mem = NULL;
    char **mem_instr = NULL;
    char ultimainst = 0;
    int m = 256;
    int n = 16;
    int pc=0;
    int memoria[256];
    int escolha=1;
    char bin[17];
    REG_pepiline_BI_ID   reg_IfID_atual  = {0};
    REG_pepiline_ID_EX   reg_IdEX_atual  = {0};
    REG_pepiline_EX_MEM  reg_ExMem_atual = {0};
    REG_pepiline_MEM_WB  reg_MemWb_atual = {0};
    REG_pepiline_BI_ID reg_IfID_prox={0};
    REG_pepiline_ID_EX reg_IdEX_prox={0};
    REG_pepiline_EX_MEM reg_ExMem_prox={0};
    REG_pepiline_MEM_WB reg_MemWb_prox={0};
    unidade_forwading Forwading;
    instrucao i;
    controle c;
    sinais_controle_forwading Forwading_atual={0};
    sinais_controle_forwading Forwading_prox={0};
    int saida_mem_wb=0;
    metricas metricas = {0};
    mem_instr = criameminstr(m, n);
    int temp_pc=0;
    int pc_prox=0;
    printf("\n\nMenu de opcoes do programa");
    do { printf("\n\n[1] Carregar memoria de instrucao");
     printf("\n[2] Carregar memoria de dados");
     printf("\n[3] Imprimir memoria de instrucoes e dados");
     printf("\n[4] Imprimir banco de registradores");
     printf("\n[5] Imprimir todo simulador");
     printf("\n[6] Salvar .asm e .dat");
     printf("\n[7] Mostrar Estatisticas do programa");
     printf("\n[8] Executar programa(RUN)");
     printf("\n[9] Executar um clock (STEP)");
     printf("\n[10] Voltar uma instrucao");
     printf("\n[0] Encerrar programa");
     printf("\nescolha uma opcao: ");
     scanf("%d",&escolha);
     switch (escolha) {
         case 1:
             printf("\nCarregando memoria\n");
             carregamem(mem_instr, m, n);
             break;
         case 2: 
            carregadat(memoria);
         break;
         case 3:
            imprimir_mem_instr(mem_instr,m,n, bin);
            imprimir_mem_dados(memoria);
         break;
         case 4: printf("\nbanco de registradores\n");
         imprimir_reg();
         break;
         case 5: printf("\nImprimindo banco de registradores e memória de dados:");
            imprimir_mem_dados(memoria);
            imprimir_reg();
            printf("Instrução executada em ");
            imprimir_instrucao(i);
            printf("\nPC da proxima instrucao:%d",pc);
         break;
         case 6:
            printf("\nArquivo  Assembly sendo gerado...");
            strcpy(bin, mem_instr[temp_pc]);
            while (strcmp(bin,"0000000000000000") !=0)
            {
                instrucao p=decodificar(bin);
                gerar_asm(p,temp_pc,bin);
                temp_pc++;
                strcpy(bin, mem_instr[temp_pc]);                
            }
            printf("\nArquivo gerado!");
            printf("\nArquivo sendo de dados sendo gerado....");
            gerar_dat(memoria);
            printf("\nArquivo gerado!");

         break;
         case 7:
            printf("Estatisticas do programa: ");
            mostrar_metricas(metricas);
         break;
         case 8:
          do{
            printf("\n================ CLOCK STEP ================\n");

            // 1. ETAPA DE BUSCA (IF)
            printf("\nETAPA DE BUSCA (IF):");
            printf("\npc:%d",pc);
            reg_IfID_prox = estagio_busca(pc, mem_instr);
            printf("\nConteudo registrador pipeline IF/ID gerado:");
            printf("\nValor do somador do pc:%d", reg_IfID_prox.soma_pc);
            printf("\nInstrucao:%s", reg_IfID_prox.instrucao);
            pc_prox=reg_IfID_prox.soma_pc;
            printf("\npc prox:%d",pc_prox);


            // 2. ESTÁGIO DE DECODIFICAÇÃO (ID)
            printf("\n\nESTAGIO DE DECODIFICACAO (ID):");
            // Correção: Lê do ciclo passado (atual)
            reg_IdEX_prox = estagio_ID(reg_IfID_atual, registradores);
            imprimir_instrucao(reg_ExMem_prox.instrucao);
            printf("\nConteudo registrador pipeline ID/EX gerado:");
            printf("\nSaida 1 do banco de registradores:%d", reg_IdEX_prox.saida1_banco_reg);
            printf("\nSaida 2 do banco de registradores:%d", reg_IdEX_prox.saida2_banco_reg);
            printf("\nValor do jump:%d", reg_IdEX_prox.valor_jump);
            printf("\nValor da soma pc:%d", reg_IdEX_prox.soma_pc);
            printf("\nValor do rd:%d", reg_IdEX_prox.rd);
            printf("\nValor do rt:%d", reg_IdEX_prox.rt);
            printf("\nSinais de controle EX -> ALUOP:%d | RegDST:%d | ALUscr:%d", reg_IdEX_prox.sinais_ex.ALUOp, reg_IdEX_prox.sinais_ex.RegDst, reg_IdEX_prox.sinais_ex.ALUSrc);
            printf("\nSinais de controle MEM -> Memwrite:%d | Branch:%d | Jump:%d", reg_IdEX_prox.sinais_mem.MemWrite, reg_IdEX_prox.sinais_mem.Branch, reg_IdEX_prox.sinais_mem.jump);
            printf("\nSinais de controle WB -> Regwrite:%d | Memtoreg:%d", reg_IdEX_prox.sinais_wb.RegWrite, reg_IdEX_prox.sinais_wb.MemToReg);


            // 3. ESTÁGIO DE EXECUÇÃO (EX) 
            printf("\n\nESTAGIO DE EXECUCAO (EX):");
            saida_mem_wb=mux_memtoreg(reg_MemWb_atual.saida_memoria,reg_MemWb_atual.resultado_ula,reg_MemWb_atual.sinais_wb.MemToReg);
            reg_ExMem_prox = estagio_ex(reg_IdEX_atual,reg_ExMem_atual.resultado_ula,saida_mem_wb,Forwading_atual);
            imprimir_instrucao(reg_ExMem_prox.instrucao);
            printf("\nValores do registrador pipeline EX/MEM gerado:");
            printf("\nSaida da ula:%d", reg_ExMem_prox.resultado_ula);
            printf("\nSaida zero da ula:%d", reg_ExMem_prox.zero_ula);
            printf("\nValor do jump:%d", reg_ExMem_prox.valor_jump);
            printf("\nSoma do pc:%d", reg_ExMem_prox.soma_pc);
            printf("\nRegistrador de destino:%d", reg_ExMem_prox.registrador_destino);
            printf("\nEndereco de desvio:%d", reg_ExMem_prox.endereco_desvio);
            printf("\nSaida 2 do banco de registradores:%d", reg_ExMem_prox.saida2_banco_registradores);
            printf("\nSinais de controle MEM -> Memwrite:%d | Branch:%d | Jump:%d", reg_ExMem_prox.sinais_mem.MemWrite, reg_ExMem_prox.sinais_mem.Branch, reg_ExMem_prox.sinais_mem.jump);
            printf("\nSinais de controle WB -> Regwrite:%d | Memtoreg:%d", reg_ExMem_prox.sinais_wb.RegWrite, reg_ExMem_prox.sinais_wb.MemToReg);


            // 4. ETAPA DE ACESSO À MEMÓRIA (MEM)
            printf("\n\nETAPA DE ACESSO A MEMORIA (MEM):");
            Forwading.id_ex_RegRS=reg_IdEX_atual.rs;
            Forwading.id_ex_RegRT=reg_IdEX_atual.rt;
            Forwading.ex_mem_writeREG=reg_ExMem_atual.sinais_wb.RegWrite;
            Forwading.ex_mem_RegRD=reg_ExMem_atual.registrador_destino;
            Forwading.mem_wb_RegRD=reg_MemWb_atual.registrador_destino;
            Forwading.Mem_WB_WriteREG=reg_MemWb_atual.sinais_wb.RegWrite;
            Forwading_prox=forwading_unidade(Forwading);
            reg_MemWb_prox = estagio_mem(reg_ExMem_atual, memoria, &pc_prox);
            imprimir_instrucao(reg_MemWb_prox.instrucao);
            printf("\npc proximo:%d",pc_prox);
            printf("\nValores do registrador pipeline MEM/WB gerado:");
            printf("\nRegistrador destino:%d", reg_MemWb_prox.registrador_destino);
            printf("\nSaida da ula:%d", reg_MemWb_prox.resultado_ula);
            printf("\nSaida da memoria:%d", reg_MemWb_prox.saida_memoria);
            printf("\nSinais de controle WB -> Regwrite:%d | Memtoreg:%d", reg_MemWb_prox.sinais_wb.RegWrite, reg_MemWb_prox.sinais_wb.MemToReg);
            
            if(Forwading_prox.forwadingA != 0 || Forwading_prox.forwadingB != 0)
            {
                printf("\n==========================================================");
                printf("\nDesvio da unidade de forwading feito novos valores do estagio EX:");
                saida_mem_wb=mux_memtoreg(reg_MemWb_atual.saida_memoria,reg_MemWb_atual.resultado_ula,reg_MemWb_atual.sinais_wb.MemToReg);
                reg_ExMem_prox = estagio_ex(reg_IdEX_atual,reg_ExMem_atual.resultado_ula,saida_mem_wb,Forwading_prox);
                imprimir_instrucao(reg_ExMem_prox.instrucao);
                printf("\nValores do registrador pipeline EX/MEM gerado:");
                printf("\nSaida da ula:%d", reg_ExMem_prox.resultado_ula);
                printf("\nSaida zero da ula:%d", reg_ExMem_prox.zero_ula);
                printf("\nValor do jump:%d", reg_ExMem_prox.valor_jump);
                printf("\nSoma do pc:%d", reg_ExMem_prox.soma_pc);
                printf("\nRegistrador de destino:%d", reg_ExMem_prox.registrador_destino);
                printf("\nEndereco de desvio:%d", reg_ExMem_prox.endereco_desvio);
                printf("\nSaida 2 do banco de registradores:%d", reg_ExMem_prox.saida2_banco_registradores);
                printf("\nSinais de controle MEM -> Memwrite:%d | Branch:%d | Jump:%d", reg_ExMem_prox.sinais_mem.MemWrite, reg_ExMem_prox.sinais_mem.Branch, reg_ExMem_prox.sinais_mem.jump);
                printf("\nSinais de controle WB -> Regwrite:%d | Memtoreg:%d", reg_ExMem_prox.sinais_wb.RegWrite, reg_ExMem_prox.sinais_wb.MemToReg);
                printf("\n===============================================================");
            }
            // 5. ETAPA DE WRITE BACK (WB)
            printf("\n\nETAPA DE WRITE BACK (WB):");
            estagio_wb(reg_MemWb_atual, registradores);
            imprimir_instrucao(reg_MemWb_atual.instrucao);
            imprimir_reg();  


            reg_IfID_atual  = reg_IfID_prox;
            reg_IdEX_atual  = reg_IdEX_prox;
            reg_ExMem_atual = reg_ExMem_prox;
            reg_MemWb_atual = reg_MemWb_prox;
            Forwading_atual=Forwading_prox;
            pc=pc_prox;
            printf("\nPC atualizado para o proximo ciclo: %d\n", pc);


        
        }while(pc<=255);
        printf("Programa Executado!\n");
        break;
        case 9:
            printf("\n================ CLOCK STEP ================\n");

            // 1. ETAPA DE BUSCA (IF)
            printf("\nETAPA DE BUSCA (IF):");
            printf("\npc:%d",pc);
            reg_IfID_prox = estagio_busca(pc, mem_instr);
            printf("\nConteudo registrador pipeline IF/ID gerado:");
            printf("\nValor do somador do pc:%d", reg_IfID_prox.soma_pc);
            printf("\nInstrucao:%s", reg_IfID_prox.instrucao);
            pc_prox=reg_IfID_prox.soma_pc;
            printf("\npc prox:%d",pc_prox);


            // 2. ESTÁGIO DE DECODIFICAÇÃO (ID)
            printf("\n\nESTAGIO DE DECODIFICACAO (ID):");
            // Correção: Lê do ciclo passado (atual)
            reg_IdEX_prox = estagio_ID(reg_IfID_atual, registradores);
            imprimir_instrucao(reg_ExMem_prox.instrucao);
            printf("\nConteudo registrador pipeline ID/EX gerado:");
            printf("\nSaida 1 do banco de registradores:%d", reg_IdEX_prox.saida1_banco_reg);
            printf("\nSaida 2 do banco de registradores:%d", reg_IdEX_prox.saida2_banco_reg);
            printf("\nValor do jump:%d", reg_IdEX_prox.valor_jump);
            printf("\nValor da soma pc:%d", reg_IdEX_prox.soma_pc);
            printf("\nValor do rd:%d", reg_IdEX_prox.rd);
            printf("\nValor do rt:%d", reg_IdEX_prox.rt);
            printf("\nSinais de controle EX -> ALUOP:%d | RegDST:%d | ALUscr:%d", reg_IdEX_prox.sinais_ex.ALUOp, reg_IdEX_prox.sinais_ex.RegDst, reg_IdEX_prox.sinais_ex.ALUSrc);
            printf("\nSinais de controle MEM -> Memwrite:%d | Branch:%d | Jump:%d", reg_IdEX_prox.sinais_mem.MemWrite, reg_IdEX_prox.sinais_mem.Branch, reg_IdEX_prox.sinais_mem.jump);
            printf("\nSinais de controle WB -> Regwrite:%d | Memtoreg:%d", reg_IdEX_prox.sinais_wb.RegWrite, reg_IdEX_prox.sinais_wb.MemToReg);


            // 3. ESTÁGIO DE EXECUÇÃO (EX) 
            printf("\n\nESTAGIO DE EXECUCAO (EX):");
            saida_mem_wb=mux_memtoreg(reg_MemWb_atual.saida_memoria,reg_MemWb_atual.resultado_ula,reg_MemWb_atual.sinais_wb.MemToReg);
            reg_ExMem_prox = estagio_ex(reg_IdEX_atual,reg_ExMem_atual.resultado_ula,saida_mem_wb,Forwading_atual);
            imprimir_instrucao(reg_ExMem_prox.instrucao);
            printf("\nValores do registrador pipeline EX/MEM gerado:");
            printf("\nSaida da ula:%d", reg_ExMem_prox.resultado_ula);
            printf("\nSaida zero da ula:%d", reg_ExMem_prox.zero_ula);
            printf("\nValor do jump:%d", reg_ExMem_prox.valor_jump);
            printf("\nSoma do pc:%d", reg_ExMem_prox.soma_pc);
            printf("\nRegistrador de destino:%d", reg_ExMem_prox.registrador_destino);
            printf("\nEndereco de desvio:%d", reg_ExMem_prox.endereco_desvio);
            printf("\nSaida 2 do banco de registradores:%d", reg_ExMem_prox.saida2_banco_registradores);
            printf("\nSinais de controle MEM -> Memwrite:%d | Branch:%d | Jump:%d", reg_ExMem_prox.sinais_mem.MemWrite, reg_ExMem_prox.sinais_mem.Branch, reg_ExMem_prox.sinais_mem.jump);
            printf("\nSinais de controle WB -> Regwrite:%d | Memtoreg:%d", reg_ExMem_prox.sinais_wb.RegWrite, reg_ExMem_prox.sinais_wb.MemToReg);


            // 4. ETAPA DE ACESSO À MEMÓRIA (MEM)
            printf("\n\nETAPA DE ACESSO A MEMORIA (MEM):");
            Forwading.id_ex_RegRS=reg_IdEX_atual.rs;
            Forwading.id_ex_RegRT=reg_IdEX_atual.rt;
            Forwading.ex_mem_writeREG=reg_ExMem_atual.sinais_wb.RegWrite;
            Forwading.ex_mem_RegRD=reg_ExMem_atual.registrador_destino;
            Forwading.mem_wb_RegRD=reg_MemWb_atual.registrador_destino;
            Forwading.Mem_WB_WriteREG=reg_MemWb_atual.sinais_wb.RegWrite;
            Forwading_prox=forwading_unidade(Forwading);
            reg_MemWb_prox = estagio_mem(reg_ExMem_atual, memoria, &pc_prox);
            imprimir_instrucao(reg_MemWb_prox.instrucao);
            printf("\npc proximo:%d",pc_prox);
            printf("\nValores do registrador pipeline MEM/WB gerado:");
            printf("\nRegistrador destino:%d", reg_MemWb_prox.registrador_destino);
            printf("\nSaida da ula:%d", reg_MemWb_prox.resultado_ula);
            printf("\nSaida da memoria:%d", reg_MemWb_prox.saida_memoria);
            printf("\nSinais de controle WB -> Regwrite:%d | Memtoreg:%d", reg_MemWb_prox.sinais_wb.RegWrite, reg_MemWb_prox.sinais_wb.MemToReg);
            
            if(Forwading_prox.forwadingA != 0 || Forwading_prox.forwadingB != 0)
            {
                printf("\n==========================================================");
                printf("\nDesvio da unidade de forwading feito novos valores do estagio EX:");
                saida_mem_wb=mux_memtoreg(reg_MemWb_atual.saida_memoria,reg_MemWb_atual.resultado_ula,reg_MemWb_atual.sinais_wb.MemToReg);
                reg_ExMem_prox = estagio_ex(reg_IdEX_atual,reg_ExMem_atual.resultado_ula,saida_mem_wb,Forwading_prox);
                imprimir_instrucao(reg_ExMem_prox.instrucao);
                printf("\nValores do registrador pipeline EX/MEM gerado:");
                printf("\nSaida da ula:%d", reg_ExMem_prox.resultado_ula);
                printf("\nSaida zero da ula:%d", reg_ExMem_prox.zero_ula);
                printf("\nValor do jump:%d", reg_ExMem_prox.valor_jump);
                printf("\nSoma do pc:%d", reg_ExMem_prox.soma_pc);
                printf("\nRegistrador de destino:%d", reg_ExMem_prox.registrador_destino);
                printf("\nEndereco de desvio:%d", reg_ExMem_prox.endereco_desvio);
                printf("\nSaida 2 do banco de registradores:%d", reg_ExMem_prox.saida2_banco_registradores);
                printf("\nSinais de controle MEM -> Memwrite:%d | Branch:%d | Jump:%d", reg_ExMem_prox.sinais_mem.MemWrite, reg_ExMem_prox.sinais_mem.Branch, reg_ExMem_prox.sinais_mem.jump);
                printf("\nSinais de controle WB -> Regwrite:%d | Memtoreg:%d", reg_ExMem_prox.sinais_wb.RegWrite, reg_ExMem_prox.sinais_wb.MemToReg);
                printf("\n===============================================================");
            }
            // 5. ETAPA DE WRITE BACK (WB)
            printf("\n\nETAPA DE WRITE BACK (WB):");
            estagio_wb(reg_MemWb_atual, registradores);
            imprimir_instrucao(reg_MemWb_atual.instrucao);
            imprimir_reg();  


            reg_IfID_atual  = reg_IfID_prox;
            reg_IdEX_atual  = reg_IdEX_prox;
            reg_ExMem_atual = reg_ExMem_prox;
            reg_MemWb_atual = reg_MemWb_prox;
            Forwading_atual=Forwading_prox;
            pc=pc_prox;
            printf("\nPC atualizado para o proximo ciclo: %d\n", pc);
            break;
         case 10:
            pc = oldpc;
             for(int j=0;j<256;j++){
            memoria[j] = oldmem[j];}
            for(int k=0;k<8;k++){
            registradores[k] = oldreg[k];}
            i = busca(bin, mem_instr, pc);
            reduzir_metricas(&metricas, ultimainst);
           printf("\nPC da proxima instrucao:%d",pc);
           break;
         default:
             return 0;
             break;
    }
} while (escolha !=0);
desalocameminstr(mem_instr, m, n);
return 0;
}


char **criameminstr(int m, int n){
    int i;
    char **mem_instr = NULL;
    mem_instr = (char**) malloc(m*sizeof(char*));
    for(i=0;i<m;i++){
        mem_instr[i] = (char*) malloc((n+1)*sizeof(char));
        for (int j = 0; j < 16; j++){
            mem_instr[i][j] = '0';
        }
        mem_instr[i][n] = '\0';
    } return mem_instr;
}

void desalocameminstr(char **mem_instr, int m, int n){
    int i;
    for(i=0;i<m;i++){
        free(mem_instr[i]);
    }
    free(mem_instr);
    return;
}

void carregamem (char **mem_instr, int m, int n)
{
char nome_arq[256];
printf("\nDigite o nome do arquivo que você quer acessar: ");
scanf("%s",nome_arq);
mem = fopen(nome_arq, "r");
if (mem == NULL){
    printf("Erro ao abrir o arquivo!\n");
    return; }
    int c;
    for(int i=0;i<m;i++){
        for(int j=0;j<n;j++) {
            c = fgetc(mem);
            if(c == '1'){
                mem_instr[i][j] = c; }
                else if(c=='0'){
                    mem_instr[i][j] = '0';
                }
                else if( c == '\n'){
                j--;
                }else {
                mem_instr[i][j] = '0';
                }
        }
    } fclose(mem);
    printf("Memoria carregada!\n");
    return;
}

void carregadat(int *mem_dados)
{
    char nome_dat[256];
    printf("\nDigite o nome do arquivo da memoria de dados que voce quer:");
    scanf("%s",nome_dat);
    mem = fopen(nome_dat, "r");
    if (mem == NULL){
        printf("Erro ao abrir o arquivo!\n");
        return;
    }
    char c[6];
    int i = 0;
    while (i < 256 && fscanf(mem, "%5s", c) == 1){
        mem_dados[i] = atoi(c);
        i++;
    }

    fclose(mem);
    printf("\nMemória de dados carregada");
}

instrucao decodificar(char *bin) {
    unsigned int valor = strtoul(bin, NULL, 2);
    instrucao i;
    i.opcode = (valor >> 12) & 0xF;
    if (i.opcode==0) 
    {
    i.rs = (valor >> 9) & 0x7;
    i.rt = (valor >> 6) & 0x7;
    i.rd = (valor >> 3) & 0x7;
    i.funct = valor & 0x7; 
    i.imm=valor & 0x3f;
    i.addr=valor & 0xff;
    }
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
            i.rd = (valor >> 3) & 0x7;
            i.imm=valor & 0x3f;
        } return i;
}

void imprimir_mem_instr(char **mem_instr, int m, int n, char* bin) {
    int k=0,j=0;
    printf("\n=======MEMORIA DE INSTRUCAO======\n");
    for ( k = 0; k < m; k++) {
      printf("Instrução %d: ", k+1);
        for ( j = 0; j < n; j++) {
            printf("%c",mem_instr[k][j]);
        }
        imprimir_ass(bin,mem_instr, k);
        printf("\n");
    }
}
void imprimir_ass (char*bin, char** mem_instr, int k){
    strcpy(bin, mem_instr[k]);
    instrucao i = decodificar(bin);
    imprimir_instrucao(i); 
}

void imprimir_reg() {
    int i;
    printf("\n=====BANCO DE REGISTRADORES=====\n");
    for ( i = 0; i < 8; i++) {
        printf("[R%d] = %d ",i, registradores[i]);
    }
    printf("\n\n");
}

void imprimir_instrucao(instrucao p) {
  switch (p.opcode)
    {
    case 0:
        switch (p.funct)
        {
        case 0:
            printf("|Assembly: add $%d,$%d,$%d\n",p.rd,p.rs,p.rt);
            break;

        case 2:
            printf("|Assembly: sub $%d,$%d,$%d\n",p.rd,p.rs,p.rt);
            break;
        }
        break;

    case 2:
        printf("|Assembly: jump %d\n",p.addr);
        break;

    case 4:
        p.imm=sign_extend6to8(p.imm);
        printf("|Assembly: addi $%d,$%d,%d\n",p.rt,p.rs,p.imm);
        break;

    case 8:
        p.imm=sign_extend6to8(p.imm);
        printf("|Assembly: beq $%d,$%d,%d\n",p.rs,p.rt,p.imm);
        break;

    case 11:
        p.imm=sign_extend6to8(p.imm);
        printf("|Assembly: lw $%d,%d($%d)\n",p.rt,p.imm,p.rs);
        break;

    case 15:
        p.imm=sign_extend6to8(p.imm);
        printf("|Assembly: sw $%d,%d($%d)\n",p.rt,p.imm,p.rs);
        break;

    default:
        printf("| instrucao desconhecida\n");
        break;
    }
}

//Função que recebe a instrução convertida e decodifica os sinais de controle
controle sinais_controle(instrucao i, metricas *m, char *ultimaInst){
    controle c;
    // Inicializa tudo com 0
    c.RegDst = 0;
    c.ALUSrc = 0;
    c.MemToReg = 0;
    c.RegWrite = 0;
    c.MemRead = 0;
    c.MemWrite = 0;
    c.Branch = 0;
    c.ALUOp = 0;
    c.jump = 0;
    m->contInst ++;
    switch(i.opcode){
        case 0:
            // Tipo R
            c.RegDst = 1;
            c.ALUSrc = 0;
            c.MemToReg = 0;
            c.RegWrite = 1;
            c.ALUOp = i.funct; // usa funct direto
            m->contInstReg ++;
            *ultimaInst = 'R';
            break;
        case 4:
            // ADDI
            c.RegDst = 0;
            c.ALUSrc = 1;
            c.RegWrite = 1;
            c.ALUOp = 0;
            m->contInstImm ++;
            *ultimaInst = 'I';
            break;
        case 11:
            // LW
            c.ALUSrc = 1;
            c.MemToReg = 1;
            c.RegWrite = 1;
            c.MemRead = 1;
            m->contInstImm ++;
            *ultimaInst = 'I';
            break;
        case 15:
            // SW
            c.ALUSrc = 1;
            c.MemWrite = 1;
            m->contInstImm ++;
            *ultimaInst = 'I';
            break;
        case 8:
            // BEQ
            c.Branch = 1;
            c.ALUOp = 2;
            m->contInstImm ++;
            *ultimaInst = 'I';
            break;
        case 2:
            // JUMP
            c.jump = 1;
            m->contInstJump ++;
            *ultimaInst = 'J';
            break;

    } return c;

}

//função que realiza a busca da instrução
instrucao busca (char *bin, char **mem_instr, int pc){
    strcpy(bin, mem_instr[pc]);
    instrucao i = decodificar(bin);
    printf("\ninstrucao em binario:%s",bin);
    imprimir_instrucao(i); return i;
}

int mux1(controle c, instrucao i){
    if(c.RegDst == 0){
        return i.rt; }
    else if(c.RegDst == 1){
        return i.rd;
    } return 0;
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

void executar(instrucao i, controle c, int *pc){
    int op1;
    int op2;
    int resultado;
    int destino;
    int saida_muxBranch=0;
    int resultado_soma_branch=0;
    int overflow;
    int zero;
    //Extensão de sinal:
    op1=registradores[i.rs];
    i.imm=sign_extend6to8(i.imm);
    // MUX da segunda entrada da ULA (ALUSrc)
    if(c.ALUSrc == 1){
        op2 = i.imm;
    } else {
        op2 = registradores[i.rt];
    }
    // Executa na ULA
    resultado = ula(op1, op2, c, &overflow,&zero);
    if(overflow){
    printf("Overflow!\n");
    return ;
  }
    // MUX do registrador destino
    destino = mux1(c, i);
    // Escreve no banco de registradores
    if(c.RegWrite){
        registradores[destino] = resultado;
    }
    // Branch
    //primeiro a gente soma o valor do imediato com o valor do pc + 1 para saber valor da entrada 1 do mux branch
    resultado_soma_branch=somador(i.imm,*pc+1);
    //agora a gente vai selecionar atraves de um mux qual vai ser caminho selecionado o da soma do branch ou do pc
    saida_muxBranch=mux_branch(c.Branch & zero,*pc+1,resultado_soma_branch);
    //agora temos o resultado selecionado pelo mux do branch
    //agora iremos fazer o mesmo com jump somar com pc +1
    //agora iremos selecionar o resultado do mux do branch com o resultado da soma do jump e o item selecionado vai virar o pc
    *pc=mux_jump(c.jump,saida_muxBranch,i.addr);
    // Sw
    if(c.MemWrite) {
        lwsw(1, resultado, i.rt);
    }
    // Lw
    if(c.MemRead) {
        registradores[destino] = lwsw(2, resultado, i.rt);
    }
}

int lwsw(int op, int endereco, int dado) {
    if (op == 1)
    {
        memoria[endereco] = registradores[dado];
        return 0;
    }
    else if (op == 2) {
        return memoria[endereco];
    }
}
int sign_extend6to8(int imm)
{
    if (imm & 0x20)      // verifica bit de sinal (bit 5)
        return imm | 0xC0;  // adiciona 11 nos 2 bits superiores
    else
        return imm;
}
int somador(int entrada1,int entrada2)
{
    int resultado=0;
    resultado=entrada1+entrada2;
    return resultado;
}
int mux_branch(int sinal_branch,int entrada1,int entrada2)
{
    switch (sinal_branch)
    {
    case 0:
        return entrada1;
        break;
    case 1: 
        return entrada2;
    default:
        break;
    }
    return 0;
}
int mux_jump(int sinal_jump,int entrada1,int entrada2)
{
    switch (sinal_jump)
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
    return 0;
}
void imprimir_mem_dados(int mem[]){
    printf("\n======memoria de dados======\n");
    for (int i = 0; i < 16; i++) // linhas
    {
        for (int j = 0; j < 16; j++) // colunas
        {
            int idx = i * 16 + j;
            printf("[%3d] =%4d |",idx, mem[idx]);
        }

        printf("\n");
    }
}


void gerar_asm(instrucao p,int pc,char bin[])
{
    FILE *arquivo;
    arquivo = fopen("assembly.asm","a");

    if (!arquivo)
    {
        printf("\nProblema ao gerar arquivo!");
        return;
    }

    switch (p.opcode)
    {
    case 0:
        switch (p.funct)
        {
        case 0:
            fprintf(arquivo,"instrucao:%d |Binario:%s |Assembly: add $%d,$%d,$%d\n",pc,bin,p.rd,p.rs,p.rt);
            break;

        case 2:
            fprintf(arquivo,"instrucao:%d |Binario:%s |Assembly: sub $%d,$%d,$%d\n",pc,bin,p.rd,p.rs,p.rt);
            break;
        }
        break;

    case 2:
        fprintf(arquivo,"Instrução:%d |Binario:%s |Assembly: jump %d\n",pc,bin,p.addr);
        break;

    case 4:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"Instruçao:%d |Binario:%s |Assembly: addi $%d,$%d,%d\n",pc,bin,p.rt,p.rs,p.imm);
        break;

    case 8:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"Instrucao:%d |Binario:%s |Assembly: beq $%d,$%d,%d\n",pc,bin,p.rs,p.rt,p.imm);
        break;

    case 11:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"Instrucao:%d |Binario:%s |Assembly: lw $%d,%d($%d)\n",pc,bin,p.rt,p.imm,p.rs);
        break;

    case 15:
        p.imm=sign_extend6to8(p.imm);
        fprintf(arquivo,"Instrucao:%d |Binario:%s |Assembly: sw $%d,%d($%d)\n",pc,bin,p.rt,p.imm,p.rs);
        break;

    default:
        fprintf(arquivo,"Instrucao:%d |Binario:%s | instrucao desconhecida\n",pc,bin);
        break;
    }

    fclose(arquivo);
}
void gerar_dat(int memoria[])
{
    FILE *arq=fopen("arquivo_dados.txt","w");
    for (int  i = 0; i < 256; i++)
    {
        fprintf(arq,"%d\t",memoria[i]);
    }
    fclose(arq);
}

void mostrar_metricas(metricas m) {
    printf("\n\n---Métricas---"
    "\nInstruções executadas: %i"
    "\nInstruções tipo R executadas: %i"
    "\nInstruções tipo I executadas: %i"
    "\nInstruções tipo J executadas: %i",
    m.contInst, m.contInstReg, m.contInstImm, m.contInstJump);
    return;
}


void reduzir_metricas(metricas *m, char ultimaInst) {
    m->contInst --;
    switch (ultimaInst)
    {
    case 'R':
        m->contInstReg --;
        break;
    case 'I':
        m->contInstImm --;
        break;
    case 'J':
        m->contInstJump --;
        break;
    default:
        break;
    }
    return;
}
sinais_controle_forwading forwading_unidade(unidade_forwading f)
{
    sinais_controle_forwading s = {0};
    if (f.ex_mem_writeREG && (f.ex_mem_RegRD != 0) && (f.ex_mem_RegRD == f.id_ex_RegRS)) 
    {
        s.forwadingA = 2;
        printf("\nDependecia detectada!");
        printf("\nEntre o registrador %d e %d",f.ex_mem_RegRD,f.id_ex_RegRS);
        printf("\nAtivando unidade de forwading A");
    } 
    else if (f.Mem_WB_WriteREG && (f.mem_wb_RegRD != 0) && (f.mem_wb_RegRD == f.id_ex_RegRS))
    {
        s.forwadingA = 1;
        printf("\nDependecia detectada!");
        printf("\nEntre o registrador %d e %d",f.mem_wb_RegRD,f.id_ex_RegRS);
        printf("\nAtivando unidade de forwading A");
        
    }
    if (f.ex_mem_writeREG && (f.ex_mem_RegRD != 0) && (f.ex_mem_RegRD == f.id_ex_RegRT)) {
        s.forwadingB = 2;
        printf("\nDependecia detectada!");
        printf("\nEntre o registrador %d e %d",f.ex_mem_RegRD,f.id_ex_RegRT);
        printf("\nAtivando unidade de forwading B");
    } 
    else if (f.Mem_WB_WriteREG && (f.mem_wb_RegRD != 0) && (f.mem_wb_RegRD == f.id_ex_RegRT)) {
        s.forwadingB = 1;
         printf("\nDependecia detectada!");
        printf("\nEntre o registrador %d e %d",f.mem_wb_RegRD,f.id_ex_RegRT);
        printf("\nAtivando unidade de forwading B");
    }

    return s; 
}
int mux_ula_fonte(int rt,int imediato,int sinal_ula_fonte)
{
    switch (sinal_ula_fonte)
    {
    case 0:
        return rt;
        break;
    case 1:
        return imediato;
        break;
    default:
        break;
    }
}
int mux_forwadingA(int entrada1,int entrada2,int entrada3,int sinal_forwading)
{
    switch (sinal_forwading)
    {
    case 0:
        return entrada1;
        break;
    case 1:
        return entrada3;
        break;
    case 2:
        return entrada2;
        break;
    default:
        break;
    }
}
int mux_forwadingB(int entrada1,int entrada2,int entrada3,int sinal_forwading)
{
    switch (sinal_forwading)
    {
    case 0:
        return entrada1;
        break;
    case 1:
        return entrada3;
        break;
    case 2:
        return entrada2;
        break;
    default:
        break;
    }
}
int mux_regDST(int rt,int rd,int sinal_regdst)
{
    switch (sinal_regdst)
    {
    case 0:
        return rt;
        break;
    case 1:
        return rd;
        break;
    default:
        break;
    }
}
int mux_memtoreg(int saida_mem,int saida_ula,int memtoreg)
{
    switch (memtoreg)
    {
    case 0:
        return saida_mem;
        break;
    case 1:
        return saida_ula;
        break;
    default:
        break;
    }
}
controle sinais_controle_pipeline(instrucao i)
{
    controle c;
    // Inicializa tudo com 0
    c.RegDst = 0;
    c.ALUSrc = 0;
    c.MemToReg = 0;
    c.RegWrite = 0;
    c.MemRead = 0;
    c.MemWrite = 0;
    c.Branch = 0;
    c.ALUOp = 0;
    c.jump = 0;
    switch(i.opcode){
        case 0:
            // Tipo R
            c.RegDst = 1;
            c.ALUSrc = 0;
            c.MemToReg = 1;
            c.RegWrite = 1;
            c.ALUOp = i.funct; // usa funct direto
            break;
        case 4:
            // ADDI
            c.RegDst = 0;
            c.ALUSrc = 1;
            c.RegWrite = 1;
            c.ALUOp = 0;
            c.MemToReg=1;
            break;
        case 11:
            // LW
            c.ALUSrc = 1;
            c.MemToReg = 0;
            c.RegWrite = 1;
            c.MemRead = 1;
            break;
        case 15:
            // SW
            c.ALUSrc = 1;
            c.MemWrite = 1;
            break;
        case 8:
            // BEQ
            c.Branch = 1;
            c.ALUOp = 2;
            break;
        case 2:
            // JUMP
            c.jump = 1;
            break;

    } return c;

}
int somador_pc(int entrada1)
{
    int resultado=0;
    resultado=entrada1+1;
    return resultado;
}
int ula_pipeline(int op1, int op2,int aluop, int *overflow,int *zero){
    *overflow = 0;
    *zero=0;
    int res=0;
    switch(aluop){
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
int calculo_endereco_desvio(int valor_pc,int imediato)
{
    int resultado=0;
    resultado=valor_pc+imediato;
    return resultado;
}
REG_pepiline_BI_ID estagio_busca(int pc,char **mem_instr)
{
    REG_pepiline_BI_ID r={0};
    strcpy(r.instrucao, mem_instr[pc]);
    r.soma_pc=somador_pc(pc);
    return r;
    
}
REG_pepiline_ID_EX estagio_ID(REG_pepiline_BI_ID r,int banco_registrador[7])
{
    instrucao i;
    REG_pepiline_ID_EX id={0};
    controle c;
    i=decodificar(r.instrucao);
    c=sinais_controle_pipeline(i);
    id.instrucao=i;
    id.saida1_banco_reg=banco_registrador[i.rs];
    id.saida2_banco_reg=banco_registrador[i.rt];
    id.sinal_extendido=sign_extend6to8(i.imm);
    id.soma_pc=r.soma_pc;
    id.valor_jump=i.addr;
    id.rd=i.rd;
    id.rt=i.rt;
    id.rs=i.rs;
    id.sinais_ex.ALUOp=c.ALUOp;
    id.sinais_ex.ALUSrc=c.ALUSrc;
    id.sinais_ex.RegDst=c.RegDst;
    id.sinais_mem.Branch=c.Branch;
    id.sinais_mem.jump=c.jump;
    id.sinais_mem.MemWrite=c.MemWrite;
    id.sinais_wb.MemToReg=c.MemToReg;
    id.sinais_wb.RegWrite=c.RegWrite;
    return id;
}
REG_pepiline_EX_MEM estagio_ex(REG_pepiline_ID_EX id, int ex_mem, int mem_wb, sinais_controle_forwading sinal_forwading)
{
    REG_pepiline_EX_MEM ex = {0};
    int overflow;
    int zero;
    int saida_mux_ula_fonte;
    int resultado_forwadingA;
    int resultado_forwadingB;
    printf("\nSinal forwading A:%d",sinal_forwading.forwadingA);
    printf("\nSinal forwading B:%d",sinal_forwading.forwadingB);
    printf("\nSaida banco de registradores:%d",id.saida1_banco_reg);
    resultado_forwadingA = mux_forwadingA(id.saida1_banco_reg, ex_mem, mem_wb, sinal_forwading.forwadingA);
    resultado_forwadingB = mux_forwadingB(id.saida2_banco_reg, ex_mem, mem_wb, sinal_forwading.forwadingB); // Usa a saida2_banco_reg!

    printf("\nResultado FORWADING A:%d", resultado_forwadingA);
    printf("\nResultado FORWADING B:%d", resultado_forwadingB);

    // 2. O MUX ALUSrc decide entre a Saída do Forwarding B ou o Imediato
    saida_mux_ula_fonte = mux_ula_fonte(resultado_forwadingB, id.sinal_extendido, id.sinais_ex.ALUSrc);

    // 3. A ULA recebe o Forwarding A e a Saída do MUX ALUSrc
    ex.resultado_ula = ula_pipeline(resultado_forwadingA, saida_mux_ula_fonte, id.sinais_ex.ALUOp, &overflow, &zero);

    ex.registrador_destino = mux_regDST(id.rt, id.rd, id.sinais_ex.RegDst);
    printf("\nEX registador destino:%d", ex.registrador_destino);
    
    ex.endereco_desvio = calculo_endereco_desvio(id.soma_pc, id.sinal_extendido);
    ex.valor_jump = id.valor_jump;
    
    // CORREÇÃO: Salvar o valor atualizado (com forwarding) para um possível Store na memória!
    ex.saida2_banco_registradores = resultado_forwadingB; 
    
    ex.sinais_mem = id.sinais_mem;
    ex.sinais_wb = id.sinais_wb;
    ex.zero_ula = zero;
    ex.instrucao = id.instrucao;
    
    return ex;
}
REG_pepiline_MEM_WB estagio_mem(REG_pepiline_EX_MEM ex, int memoria[], int *pc)
{
    REG_pepiline_MEM_WB Mem={0};
    int saida_mux_branch;

    // Seu controle de leitura/escrita na memória
    printf("\nFunção estagio mem");

    switch (ex.sinais_mem.MemWrite)
    {
    case 1:
        // Cuidado: certifique-se de que ex.resultado_ula é um endereço válido (>=0 e <256)
        if (ex.resultado_ula >= 0 && ex.resultado_ula < 256) {
            memoria[ex.resultado_ula] = ex.saida2_banco_registradores;
            printf("\nResultado da ula:%d",ex.resultado_ula);
            printf("\nDado que vai ser escrito na memoria:%d",ex.saida2_banco_registradores);
        }
        break;
    case 0:
        if (ex.resultado_ula >= 0 && ex.resultado_ula < 256) {
            Mem.saida_memoria = memoria[ex.resultado_ula];
        }
        break;
    default:
        break;
    }

    // Calcula as saídas dos MUX normalmente para os prints
    saida_mux_branch = mux_branch(ex.sinais_mem.Branch & ex.zero_ula, ex.soma_pc, ex.endereco_desvio);
    
    printf("\nSaida mux branch: %d", saida_mux_branch);
    printf("\nSoma pc: %d", ex.soma_pc);
    printf("\nENDERECO DE DESVIO: %d", ex.endereco_desvio);

    // =================================================================
    // CORREÇÃO CRÍTICA: PROTEÇÃO DO PC
    // =================================================================
    // Só alteramos o PC se o Branch for tomado (Branch AND Zero) OU se for um Jump.
    // Se for uma instrução normal (ou pipeline vazio), NÃO tocamos no *pc.
    if ((ex.sinais_mem.Branch && ex.zero_ula) || ex.sinais_mem.jump) {
        *pc = mux_jump(ex.sinais_mem.jump, saida_mux_branch, ex.valor_jump);
        printf("\n[MEM] Desvio detectado! PC atualizado para: %d", *pc);
    } else {
        printf("\n[MEM] Nao houve desvio neste estagio. Mantendo o PC da busca.");
    }
    // =================================================================

    Mem.resultado_ula = ex.resultado_ula;
    Mem.registrador_destino = ex.registrador_destino;
    Mem.sinais_wb = ex.sinais_wb;
    Mem.instrucao=ex.instrucao;
    
    return Mem;
}
void estagio_wb(REG_pepiline_MEM_WB Mem,int banco_registrador[7])
{
    printf("\nEstagio WB");
    int saida_mux_memtoreg;
    printf("\nRegistrador destino:%d",Mem.registrador_destino);
    printf("\nResultado ula:%d",Mem.resultado_ula);
    printf("\nSaida da memoria:%d",Mem.saida_memoria);
    printf("\nSinal mem to reg:%d",Mem.sinais_wb.MemToReg);
    saida_mux_memtoreg=mux_memtoreg(Mem.saida_memoria,Mem.resultado_ula,Mem.sinais_wb.MemToReg);
    printf("\nDado:%d",saida_mux_memtoreg);
    if(Mem.sinais_wb.RegWrite)
    {
        banco_registrador[Mem.registrador_destino]=saida_mux_memtoreg;
        return;
    }
}
