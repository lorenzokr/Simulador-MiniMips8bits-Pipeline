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
    int contClock;
    int clockTime;
} metricas;
typedef struct nodoPilha nodoPilha;
typedef struct descritorPilha descritorPilha;
typedef struct nodoPilha
{
    nodoPilha *ant;
    controle controle;
    REG_pepiline_BI_ID PCInst;
    REG_pepiline_ID_EX IDEX;
    REG_pepiline_EX_MEM EXMEM;
    REG_pepiline_MEM_WB MEMWB;
    unidade_forwading forwarding;
    sinais_controle_forwading controleForwarding;
    metricas metricas;
    int pc;
    int registradores[8];
    int memoria[256];
} nodoPilha;

typedef struct descritorPilha
{
    nodoPilha *topo;
} descritorPilha;
typedef struct deteccao_hazard
{
    int ID_EX_READMEM;
    int ID_EX_registradorRT;
    int IF_ID_registradorRS;
    int IF_ID_registradorRT;
}entrada_unidade_hazard;
typedef struct unidade_hazard
{
    int IF_ID_escrita;
    int pc_escrita;
    int sinal_mux_controle;

}saida_unidade_hazard;




instrucao decodificar(char *bin);
void imprimir_ass (char*bin, char** mem_instr, int k);
void carregamem (char **mem_instr, int m, int n);
void imprimir_mem_instr(char **mem_instr, int m, int n, char* bin);
void imprimir_reg(int registradores[8]);
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
REG_pepiline_ID_EX estagio_ID(REG_pepiline_BI_ID r,int banco_registrador[8],entrada_unidade_hazard entrada_hazard_unidade,saida_unidade_hazard *saida_hazard_unidade);
REG_pepiline_EX_MEM estagio_ex(REG_pepiline_ID_EX id,int ex_mem,int mem_wb,sinais_controle_forwading sinal_forwading);
REG_pepiline_MEM_WB estagio_mem(REG_pepiline_EX_MEM ex,int memoria[],int *pc);
void estagio_wb(REG_pepiline_MEM_WB Mem,int banco_registrador[8]);
int mux_ula_fonte(int rt,int imediato,int sinal_ula_fonte);
sinais_controle_forwading forwading_unidade(unidade_forwading f);
int mux_forwadingA(int entrada1,int entrada2,int entrada3,int sinal_forwading);
int mux_forwadingB(int entrada1,int entrada2,int entrada3,int sinal_forwading);
int mux_regDST(int rt,int rd,int sinal_regdst);
int mux_memtoreg(int saida_mem,int saida_ula,int memtoreg);
REG_pepiline_ID_EX mux_sinais_controle(int sinal_unidade_hazard,REG_pepiline_ID_EX entrada1,REG_pepiline_ID_EX entrada2);
saida_unidade_hazard unidade_hazard(entrada_unidade_hazard hazard_unidade);
void pushStepback(descritorPilha *descritor, controle controle, REG_pepiline_BI_ID PCInst, REG_pepiline_ID_EX IDEX, REG_pepiline_EX_MEM EXMEM, REG_pepiline_MEM_WB MEMWB, unidade_forwading forwarding, sinais_controle_forwading controleForwarding, metricas metricas, int pc, int regitradores[8], int memoria[256]);
void popStepback(descritorPilha *descritor, controle *controle, REG_pepiline_BI_ID *PCInst, REG_pepiline_ID_EX *IDEX, REG_pepiline_EX_MEM *EXMEM, REG_pepiline_MEM_WB *MEMWB, unidade_forwading *forwarding, sinais_controle_forwading *controleForwarding, metricas *metricas, int *pc, int registradores[8], int memoria[256]);

int main() {
    FILE *mem = NULL;
    char **mem_instr = NULL;
    char ultimainst = 0;
    int m = 256;
    int n = 16;
    int pc = 0;
    int registradores[8]={0};
    int memoria[256] = {0};
    int escolha = 1;
    char bin[17];
    
    // 1. REGISTRADORES ÚNICOS (Sem atual/prox)
    REG_pepiline_BI_ID   reg_IfID  = {0};
    REG_pepiline_ID_EX   reg_IdEX  = {0};
    REG_pepiline_EX_MEM  reg_ExMem = {0};
    REG_pepiline_MEM_WB  reg_MemWb = {0};

    unidade_forwading         entradas_forwarding = {0}; // Dados que entram na unidade
    sinais_controle_forwading sinais_forwarding   = {0}; // Sinais (A e B) que saem da unidade

    entrada_unidade_hazard entrada_hazard_unidade= {0};

    REG_pepiline_MEM_WB reg_MemWb_antigo;

    saida_unidade_hazard saida_hazard_unidade;
    saida_hazard_unidade.IF_ID_escrita=1;
    saida_hazard_unidade.pc_escrita=1;
    saida_hazard_unidade.sinal_mux_controle=0;

    instrucao i;
    controle c = {0}; // Inicializado
    descritorPilha pilha;
    pilha.topo = NULL;
    int saida_mem_wb = 0;
    metricas metricas = {0};
    mem_instr = criameminstr(m, n);
    int temp_pc = 0;
    int pc_prox = 0;
    
    printf("\n\n -=-=-= SIMULADOR MINI-MIPS 8 BITS PIPELINE =-=-=-\n\n\n Qual o tempo de clock do simulador (em ps)? ");
    scanf("%i", &metricas.clockTime);
    
    do { 
        printf("\n\n[1] Carregar memoria de instrucao");
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
                imprimir_mem_instr(mem_instr, m, n, bin);
                imprimir_mem_dados(memoria);
                break;
            case 4: 
                printf("\nbanco de registradores\n");
                imprimir_reg(registradores);
                break;
            case 5: 
                printf("\nImprimindo banco de registradores e memoria de dados:\n");
                imprimir_mem_dados(memoria);
                imprimir_reg(registradores);
                printf("PC da proxima instrucao:%d", pc);
                break;
            case 6:
                printf("\nArquivo Assembly sendo gerado...");
                strcpy(bin, mem_instr[temp_pc]);
                while (strcmp(bin,"0000000000000000") != 0) {
                    instrucao p = decodificar(bin);
                    gerar_asm(p, temp_pc, bin);
                    temp_pc++;
                    strcpy(bin, mem_instr[temp_pc]);                
                }
                printf("\nArquivo gerado!");
                printf("\nArquivo de dados sendo gerado....");
                gerar_dat(memoria);
                printf("\nArquivo gerado!");
                break;
            case 7:
                printf("Estatisticas do programa: ");
                mostrar_metricas(metricas);
                break;
                
            case 8: // ================== RUN (Código unificado com o STEP) ==================
            case 9: // ================== STEP ==================
                if (escolha == 8) 
                {
                    printf("\n================ EXECUTANDO PROGRAMA (RUN) ================\n");
                }
                do 
                {
                    pushStepback(&pilha, c, reg_IfID, reg_IdEX, reg_ExMem, reg_MemWb, entradas_forwarding, sinais_forwarding, metricas, pc, registradores, memoria);
                    
                    if (escolha == 9) 
                    {
                        printf("\n\n================ CLOCK %d ================\n", metricas.contClock);
                    }
                    pc_prox = somador_pc(pc);

                    // ---------------------------------------------------------
                    // 5. ETAPA DE WRITE BACK (WB)
                    // ---------------------------------------------------------
                    estagio_wb(reg_MemWb, registradores);
                    
                    if (escolha == 9) {
                        printf("\n[5] ETAPA DE WRITE BACK (WB):");
                        imprimir_instrucao(reg_MemWb.instrucao);
                        printf("\n -> Gravando no Banco de Registradores (se RegWrite=1).");
                        imprimir_reg(registradores);  
                    }
                    
                    if (reg_MemWb.instrucao.opcode == 0) {
                        metricas.contInstReg++;
                    } else if(reg_MemWb.instrucao.opcode == 2) {
                        metricas.contInstJump++;
                    } else if(reg_MemWb.instrucao.opcode != -1) { // Ignora contagem se for bolha (-1)
                        metricas.contInstImm++;
                    }

                    // ---------------------------------------------------------
                    // 4. ETAPA DE ACESSO A MEMORIA (MEM)
                    // ---------------------------------------------------------
                    reg_MemWb_antigo = reg_MemWb;
                    reg_MemWb = estagio_mem(reg_ExMem, memoria, &pc_prox);
                    
                    if (escolha == 9) {
                        printf("\n[4] ETAPA DE ACESSO A MEMORIA (MEM):");
                        imprimir_instrucao(reg_MemWb.instrucao);
                        printf("\n -> [MEM/WB] Registrador destino: %d", reg_MemWb.registrador_destino);
                        printf("\n -> [MEM/WB] Resultado ULA: %d", reg_MemWb.resultado_ula);
                        printf("\n -> [MEM/WB] Saida da memoria: %d", reg_MemWb.saida_memoria);
                        printf("\n -> [MEM/WB] Sinais p/ WB  [Regwrite: %d | Memtoreg: %d]\n", reg_MemWb.sinais_wb.RegWrite, reg_MemWb.sinais_wb.MemToReg);
                    }

                    // ---------------------------------------------------------
                    // 3. ESTAGIO DE EXECUCAO E FORWARDING (EX)
                    // ---------------------------------------------------------
                    entradas_forwarding.id_ex_RegRS   = reg_IdEX.rs;
                    entradas_forwarding.id_ex_RegRT   = reg_IdEX.rt;
                    entradas_forwarding.ex_mem_writeREG = reg_ExMem.sinais_wb.RegWrite;
                    entradas_forwarding.ex_mem_RegRD  = reg_ExMem.registrador_destino;
                    entradas_forwarding.Mem_WB_WriteREG = reg_MemWb_antigo.sinais_wb.RegWrite;
                    entradas_forwarding.mem_wb_RegRD  = reg_MemWb_antigo.registrador_destino;

                    sinais_forwarding = forwading_unidade(entradas_forwarding);
                    saida_mem_wb = mux_memtoreg(reg_MemWb_antigo.saida_memoria, reg_MemWb_antigo.resultado_ula, reg_MemWb_antigo.sinais_wb.MemToReg);

                    reg_ExMem = estagio_ex(reg_IdEX, reg_ExMem.resultado_ula, saida_mem_wb, sinais_forwarding);

                    if (escolha == 9) {
                        printf("\n[3] ESTAGIO DE EXECUCAO E FORWARDING (EX):");
                        if(sinais_forwarding.forwadingA != 0 || sinais_forwarding.forwadingB != 0) {
                            printf("\n    [!] Forwarding Ativado [A: %d, B: %d]", sinais_forwarding.forwadingA, sinais_forwarding.forwadingB);
                        }
                        imprimir_instrucao(reg_ExMem.instrucao);
                        printf("\n -> [EX/MEM] Saida da ULA: %d | Zero ULA: %d", reg_ExMem.resultado_ula, reg_ExMem.zero_ula);
                        printf("\n -> [EX/MEM] Endereco de desvio (Branch): %d", reg_ExMem.endereco_desvio);
                        printf("\n -> [EX/MEM] Registrador destino (MUX RegDst): %d", reg_ExMem.registrador_destino);
                        printf("\n -> [EX/MEM] Dado p/ Escrita na Memoria (RT): %d", reg_ExMem.saida2_banco_registradores);
                        printf("\n -> [EX/MEM] Sinais p/ MEM [Branch: %d | MemWrite: %d | Jump: %d]", reg_ExMem.sinais_mem.Branch, reg_ExMem.sinais_mem.MemWrite, reg_ExMem.sinais_mem.jump);
                        printf("\n -> [EX/MEM] Sinais p/ WB  [RegWrite: %d | MemToReg: %d]\n", reg_ExMem.sinais_wb.RegWrite, reg_ExMem.sinais_wb.MemToReg);
                    }

                    // ---------------------------------------------------------
                    // 2. ESTAGIO DE DECODIFICACAO (ID) E HAZARD
                    // ---------------------------------------------------------
                    entrada_hazard_unidade.ID_EX_READMEM = (reg_IdEX.instrucao.opcode == 11) ? 1 : 0;
                    entrada_hazard_unidade.ID_EX_registradorRT = reg_IdEX.rt;
                    
                    reg_IdEX = estagio_ID(reg_IfID, registradores, entrada_hazard_unidade, &saida_hazard_unidade);
                    
                    if (escolha == 9) {
                        printf("\n[2] ESTAGIO DE DECODIFICACAO (ID):");
                        if(saida_hazard_unidade.sinal_mux_controle == 1) {
                            printf("\n    [!] Hazard Detectado! Inserindo Bolha (NOP)...");
                        }
                        imprimir_instrucao(reg_IdEX.instrucao);
                        printf("\n -> [ID/EX] Saida RS (Dado 1): %d | Saida RT (Dado 2): %d", reg_IdEX.saida1_banco_reg, reg_IdEX.saida2_banco_reg);
                        printf("\n -> [ID/EX] Imediato Estendido: %d", reg_IdEX.sinal_extendido);
                        printf("\n -> [ID/EX] Registradores [RS: %d | RT: %d | RD: %d]", reg_IdEX.rs, reg_IdEX.rt, reg_IdEX.rd);
                        printf("\n -> [ID/EX] Sinais p/ EX  [ALUOp: %d | ALUSrc: %d | RegDst: %d]", reg_IdEX.sinais_ex.ALUOp, reg_IdEX.sinais_ex.ALUSrc, reg_IdEX.sinais_ex.RegDst);
                        printf("\n -> [ID/EX] Sinais p/ MEM [Branch: %d | MemWrite: %d | Jump: %d]", reg_IdEX.sinais_mem.Branch, reg_IdEX.sinais_mem.MemWrite, reg_IdEX.sinais_mem.jump);
                        printf("\n -> [ID/EX] Sinais p/ WB  [RegWrite: %d | MemToReg: %d]\n", reg_IdEX.sinais_wb.RegWrite, reg_IdEX.sinais_wb.MemToReg);
                    }

                    // ---------------------------------------------------------
                    // 1. ETAPA DE BUSCA (IF)
                    // ---------------------------------------------------------
                    if (escolha == 9) printf("\n[1] ETAPA DE BUSCA (IF):");
                    
                    if (saida_hazard_unidade.IF_ID_escrita == 1) {
                        reg_IfID = estagio_busca(pc, mem_instr);
                        if (escolha == 9) {
                            printf("\n -> Buscou a instrucao do PC %d", pc);
                            printf("\n -> [IF/ID] Instrucao salva: %s", reg_IfID.instrucao);
                            printf("\n -> [IF/ID] Soma PC (PC+1): %d", reg_IfID.soma_pc);
                        }
                    } else {
                        if (escolha == 9) printf("\n    [!] STALL: Registrador IF/ID Congelado (mantendo instrucao anterior).");
                    }

                    // =========================================================
                    // ATUALIZAÇÃO DO PC 
                    // =========================================================
                    if (saida_hazard_unidade.pc_escrita == 1) {
                        pc = pc_prox; 
                    } else {
                        if (escolha == 9) printf("\n    [!] STALL: PC Congelado em %d!", pc);
                    }

                    if (escolha == 9) printf("\n\n>>> PC atualizado para o proximo ciclo: %d <<<\n", pc);
                    
                    metricas.contClock++;      
                } while (escolha == 8 && pc <= 255);
                
                if (escolha == 8) printf("\nPrograma Executado com sucesso!\n");
                break;
            case 10:
                popStepback(&pilha, &c, &reg_IfID, &reg_IdEX, &reg_ExMem, &reg_MemWb, &entradas_forwarding, &sinais_forwarding, &metricas, &pc, registradores, memoria);
                printf("\nPasso desfeito. PC retornou para: %d", pc);
                break;
                
            default:
                if (escolha != 0) printf("\nOpcao invalida!");
                break;
        }
    } while (escolha != 0);
    
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

void imprimir_reg(int registradores[8]) {
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
//função que realiza a busca da instrução
instrucao busca (char *bin, char **mem_instr, int pc){
    strcpy(bin, mem_instr[pc]);
    instrucao i = decodificar(bin);
    printf("\ninstrucao em binario:%s",bin);
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
    for (int i = 0; i < 256; i++) // linhas
    {
        printf("[%3d] =%4d |",i, mem[i]);
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
    float cpi = 0;
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
        cpi = (float)m.contClock/m.contInst;
    }
    else {
        printf("\n Nenhuma instrução concluída ainda. Não é possível calcular CPI.\n\n");
    }
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
saida_unidade_hazard unidade_hazard(entrada_unidade_hazard hazard_unidade)
{
    saida_unidade_hazard saida_hazard_unidade;
    if (hazard_unidade.ID_EX_READMEM &&((hazard_unidade.ID_EX_registradorRT == hazard_unidade.IF_ID_registradorRS) || (hazard_unidade.ID_EX_registradorRT == hazard_unidade.IF_ID_registradorRT)))
    {
        saida_hazard_unidade.IF_ID_escrita=0;
        saida_hazard_unidade.pc_escrita=0;
        saida_hazard_unidade.sinal_mux_controle=1;
        printf("\n||===================================||");
        printf("\n||INSERINDO NOP NO CAMINHO DE DADOS! ||");
        printf("\n||===================================||");
    }
    else
    {
        saida_hazard_unidade.IF_ID_escrita=1;
        saida_hazard_unidade.pc_escrita=1;
        saida_hazard_unidade.sinal_mux_controle=0;

    }
    return saida_hazard_unidade;
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
REG_pepiline_ID_EX mux_sinais_controle(int sinal_unidade_hazard,REG_pepiline_ID_EX entrada1,REG_pepiline_ID_EX entrada2)
{
    switch (sinal_unidade_hazard)
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
REG_pepiline_ID_EX estagio_ID(REG_pepiline_BI_ID r,int banco_registrador[8],entrada_unidade_hazard entrada_hazard_unidade,saida_unidade_hazard *saida_hazard_unidade)
{
    instrucao i;
    REG_pepiline_ID_EX id={0};
    REG_pepiline_ID_EX entrada2={0};
    REG_pepiline_ID_EX Registrador_id_ex={0};
    controle c;
    i=decodificar(r.instrucao);
    c=sinais_controle_pipeline(i);
    entrada_hazard_unidade.IF_ID_registradorRS=i.rs;
    entrada_hazard_unidade.IF_ID_registradorRT=i.rt;
    *saida_hazard_unidade=unidade_hazard(entrada_hazard_unidade);

    id.instrucao=i;
    id.saida1_banco_reg=banco_registrador[i.rs];
    id.saida2_banco_reg=banco_registrador[i.rt];
    id.sinal_extendido=sign_extend6to8(i.imm);
    id.soma_pc=r.soma_pc;
    id.valor_jump=i.addr;
    id.rd=i.rd;
    id.rt=i.rt;
    id.rs=i.rs;

    entrada2.instrucao=id.instrucao;
    entrada2.saida1_banco_reg=id.saida1_banco_reg;
    entrada2.saida2_banco_reg=id.saida2_banco_reg;
    entrada2.sinal_extendido=id.sinal_extendido;
    entrada2.soma_pc=id.soma_pc;
    entrada2.valor_jump=id.valor_jump;
    entrada2.rd=id.rd;
    entrada2.rs=id.rs;
    entrada2.rt=id.rt;

    id.sinais_ex.ALUOp=c.ALUOp;
    id.sinais_ex.ALUSrc=c.ALUSrc;
    id.sinais_ex.RegDst=c.RegDst;
    id.sinais_mem.Branch=c.Branch;
    id.sinais_mem.jump=c.jump;
    id.sinais_mem.MemWrite=c.MemWrite;
    id.sinais_wb.MemToReg=c.MemToReg;
    id.sinais_wb.RegWrite=c.RegWrite;
    Registrador_id_ex=mux_sinais_controle(saida_hazard_unidade->sinal_mux_controle,id,entrada2);
    return Registrador_id_ex;
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

    // Só alteramos o PC se o Branch for tomado  OU se for um Jump.
    // Se for uma instrução normal , NÃO tocamos no *pc.
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
void estagio_wb(REG_pepiline_MEM_WB Mem,int banco_registrador[8])
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

void pushStepback(descritorPilha *descritor, controle controle, REG_pepiline_BI_ID PCInst, REG_pepiline_ID_EX IDEX, REG_pepiline_EX_MEM EXMEM, REG_pepiline_MEM_WB MEMWB, unidade_forwading forwarding, sinais_controle_forwading controleForwarding, metricas metricas, int pc, int registradores[8], int memoria[256])
{
    if (descritor == NULL) {
        return;
    }
    nodoPilha *nodo = malloc(sizeof(nodoPilha));
    nodo->ant = descritor->topo;
    descritor->topo = nodo;

    nodo->controle = controle;
    nodo->controleForwarding = controleForwarding;
    nodo->EXMEM = EXMEM;
    nodo->forwarding = forwarding;
    nodo->IDEX = IDEX;
    nodo->MEMWB = MEMWB;
    nodo->metricas = metricas;
    nodo->PCInst = PCInst;
    nodo->pc = pc;
    memcpy(nodo->registradores, registradores, sizeof(nodo->registradores));
    memcpy(nodo->memoria, memoria, sizeof(nodo->memoria));
    return;
}

void popStepback(descritorPilha *descritor, controle *controle, REG_pepiline_BI_ID *PCInst, REG_pepiline_ID_EX *IDEX, REG_pepiline_EX_MEM *EXMEM, REG_pepiline_MEM_WB *MEMWB, unidade_forwading *forwarding, sinais_controle_forwading *controleForwarding, metricas *metricas, int *pc, int registradores[8], int memoria[256])
{
    if (descritor == NULL) {
        printf("\n\n Pilha não inicializada!");
        return;
    }
    if (descritor->topo == NULL) {
        printf("\n\n Pilha vazia!");
        return;
    }
    nodoPilha *nodo = descritor->topo;

    *controle = nodo->controle;
    *controleForwarding = nodo->controleForwarding;
    *EXMEM = nodo->EXMEM;
    *forwarding = nodo->forwarding;
    *IDEX = nodo->IDEX;
    *MEMWB = nodo->MEMWB;
    *metricas = nodo->metricas;
    *PCInst = nodo->PCInst;
    *pc = nodo->pc;
    memcpy(registradores, nodo->registradores, sizeof(nodo->registradores));
    memcpy(memoria, nodo->memoria, sizeof(nodo->memoria));
    descritor->topo = nodo->ant;
    free(nodo);
    return;
}
