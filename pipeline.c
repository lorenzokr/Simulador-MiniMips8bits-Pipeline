#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <stdint.h>
#include <ncurses.h>
#include <dirent.h>
#include <limits.h>

#define MAX_ARQUIVOS 20
#define MAX_NOMEARQUIVO 64

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
    int num_desvio_condicional;
    int lw;
    int sw;
    int contInstJump;
    int contClock;
    int clockTime;
    int contDataHazard;
    int contControlHazard;
    int num_stall;
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
int carregamem (char **mem_instr, int m, int n, char *arquivo);
int carregadat(int *mem_dados, char *arquivo);
char **criameminstr(int m, int n);
void desalocameminstr(char **mem_instr, int m, int n);
int mux_branch(int sinal_branch,int entrada1,int entrada2);
int mux_jump(int sinal_jump,int entrada1,int entrada2);
int somador_pc(int entrada1);
int sign_extend6to8(int imm);
void gerar_asm(instrucao p,int pc,char bin[]);
void gerar_dat(int mem[]);
REG_pepiline_BI_ID estagio_busca(int pc,char **mem_instr);
REG_pepiline_ID_EX estagio_ID(REG_pepiline_BI_ID r,int banco_registrador[8],entrada_unidade_hazard entrada_hazard_unidade,saida_unidade_hazard *saida_hazard_unidade,metricas *m);
REG_pepiline_EX_MEM estagio_ex(REG_pepiline_ID_EX id,int ex_mem,int mem_wb,sinais_controle_forwading sinal_forwading);
REG_pepiline_MEM_WB estagio_mem(REG_pepiline_EX_MEM ex,int memoria[],int *pc);
void estagio_wb(REG_pepiline_MEM_WB Mem,int banco_registrador[8]);
int mux_ula_fonte(int rt,int imediato,int sinal_ula_fonte);
sinais_controle_forwading forwading_unidade(unidade_forwading f,metricas *m);
int mux_forwadingA(int entrada1,int entrada2,int entrada3,int sinal_forwading);
int mux_forwadingB(int entrada1,int entrada2,int entrada3,int sinal_forwading);
int mux_regDST(int rt,int rd,int sinal_regdst);
int mux_memtoreg(int saida_mem,int saida_ula,int memtoreg);
REG_pepiline_ID_EX mux_sinais_controle(int sinal_unidade_hazard,REG_pepiline_ID_EX entrada1,REG_pepiline_ID_EX entrada2);
saida_unidade_hazard unidade_hazard(entrada_unidade_hazard hazard_unidade,metricas *m);
void pushStepback(descritorPilha *descritor, controle controle, REG_pepiline_BI_ID PCInst, REG_pepiline_ID_EX IDEX, REG_pepiline_EX_MEM EXMEM, REG_pepiline_MEM_WB MEMWB, unidade_forwading forwarding, sinais_controle_forwading controleForwarding, metricas metricas, int pc, int regitradores[8], int memoria[256]);
void popStepback(descritorPilha *descritor, controle *controle, REG_pepiline_BI_ID *PCInst, REG_pepiline_ID_EX *IDEX, REG_pepiline_EX_MEM *EXMEM, REG_pepiline_MEM_WB *MEMWB, unidade_forwading *forwarding, sinais_controle_forwading *controleForwarding, metricas *metricas, int *pc, int registradores[8], int memoria[256]);
void desenha_estatisticas(WINDOW *win, int largura, int altura, metricas m);
void desenha_menu(WINDOW *win, int largura, int altura);
void desenha_opcao(WINDOW *win, int largura, int altura);
void desenha_registradores_pipeline(WINDOW *win, int largura, int altura, int reg[8], REG_pepiline_BI_ID ifid, REG_pepiline_ID_EX idex, REG_pepiline_EX_MEM exmem, REG_pepiline_MEM_WB memwb,sinais_controle_forwading controleForwarding,saida_unidade_hazard unidade_hazard);
void exibir_memorias_pipeline_ncurses(char **mem_inst, int *mem_dados);
int listar_arquivos(const char *extensao, char arquivos[][MAX_NOMEARQUIVO]);
void menu_carregar_memoria(const char *extensao, int tipo, char **memoriaInstrucoes, int *memoriaDados);
void popup_msg(const char *msg, int flag);



int main() {
    FILE *mem = NULL;
    char **mem_instr = NULL;
    char ultimainst = 0;
    int m = 256;
    int n = 16;
    int nClocks;
    int pc = 0;
    int registradores[8]={0};
    int memoria[256] = {0};
    int escolha = 1;
    int contador;
    char bin[17];
    char arquivos[MAX_ARQUIVOS][MAX_NOMEARQUIVO];
    
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
    initscr();
    cbreak();
    noecho();
    refresh();

    if (has_colors() == FALSE) {
        endwin();
        printf("Seu terminal nao suporta cores!\n");
        return 1;
    }
    start_color();

    init_pair(1, COLOR_CYAN, COLOR_BLACK);   
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(3, COLOR_GREEN, COLOR_BLACK);  
    init_pair(4, COLOR_RED, COLOR_BLACK);    
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);    
    init_pair(6, COLOR_WHITE, COLOR_BLACK);

    int t_linhas, t_colunas;
    getmaxyx(stdscr, t_linhas, t_colunas);

    int margem_esquerda = 2; 
    int margem_direita = 1;
    
    int largura_janela_esq = 65; 

    // Mantido o limite de 31 linhas, que é o exato do seu terminal
    if (t_linhas < 31 || t_colunas < 110) {
        endwin();
        printf("Erro: Maximize o terminal! Necessario: 31x110 | Seu: %dx%d\n", t_linhas, t_colunas);
        return 1;
    }

    // =======================================================
    // CÁLCULO DE POSICIONAMENTO (NOVO LAYOUT)
    // =======================================================
    
    // COLUNA DA ESQUERDA (Empilhadas perfeitamente para dar 31 linhas no total)
    // 1. Estatísticas
    int janela_estY = 0;
    int janela_estX = margem_esquerda;
    int altura_janela_est = 15;

    // 2. Menu
    int janela_menuY = janela_estY + altura_janela_est;
    int janela_menuX = margem_esquerda;
    int altura_janela_menu = 12;

    // 3. Opções (No Canto Inferior Esquerdo)
    int janela_opY = janela_menuY + altura_janela_menu;
    int janela_opX = margem_esquerda;
    int altura_janela_opcao = t_linhas - janela_opY; // Pega o resto das linhas na base (aprox 4)

    // COLUNA DA DIREITA (Pipeline agora domina de cima a baixo!)
    int janela_pipeY = 0; 
    int janela_pipeX = janela_estX + largura_janela_esq + 1;
    int largura_janela_pipe = t_colunas - janela_pipeX - margem_direita;
    int altura_janela_pipe = t_linhas;
    clear();
    printw("\n\n -=-=-= SIMULADOR MINI-MIPS 8 BITS PIPELINE =-=-=-\n\n\n Qual o tempo de clock do simulador (em ps)? ");
    echo(); 
    scanw("%i", &metricas.clockTime); 
    noecho();
    clear(); 
    refresh();

    // Criação das janelas
    WINDOW *janela_est = newwin(altura_janela_est, largura_janela_esq, janela_estY, janela_estX);
    WINDOW *menu_win = newwin(altura_janela_menu, largura_janela_esq, janela_menuY, janela_menuX);
    
    // ATENÇÃO: A janela de opções agora usa a "largura_janela_esq" e não mais a do pipeline
    WINDOW *janela_op = newwin(altura_janela_opcao, largura_janela_esq, janela_opY, janela_opX);
    WINDOW *janela_pipe = newwin(altura_janela_pipe, largura_janela_pipe, janela_pipeY, janela_pipeX);
    do { 
        desenha_estatisticas(janela_est, largura_janela_esq, altura_janela_est,metricas);
        desenha_menu(menu_win, largura_janela_esq, altura_janela_menu);
        desenha_opcao(janela_op, largura_janela_esq, altura_janela_opcao);
        desenha_registradores_pipeline(janela_pipe, largura_janela_pipe, altura_janela_pipe, registradores, reg_IfID, reg_IdEX, reg_ExMem, reg_MemWb,sinais_forwarding,saida_hazard_unidade);
        // Atualização gráfica das janelas
        wrefresh(janela_est); 
        wrefresh(menu_win);
        wrefresh(janela_op);
        wrefresh(janela_pipe);

        echo();
        wmove(janela_op, altura_janela_opcao / 2 - 1, 23);
        wscanw(janela_op, "%d", &escolha);
        noecho();
        
        wclear(menu_win);
        box(menu_win, 0, 0);

        if (escolha == 5 || escolha == 7)
        {
            mvwprintw(janela_op, altura_janela_opcao / 2 + 1, 4, "Quantos clocks? ");
            wmove(janela_op, altura_janela_opcao / 2 + 1, 20);
            echo();
            wscanw(janela_op, "%d", &nClocks);
            noecho();
        }
        
        if (escolha > 0) 
        {
            def_prog_mode(); // Salva estado do ncurses
            endwin();        // Sai do ncurses temporariamente
        }
        switch (escolha) 
        {
            case 1:
                menu_carregar_memoria(".mem", 0, mem_instr, memoria);
                break;
            case 2:
                menu_carregar_memoria(".dat", 1, mem_instr, memoria);
                break;
            case 4:
                exibir_memorias_pipeline_ncurses(mem_instr,memoria);
                break;
            case 3:
                temp_pc = 0; // Previne erro caso rode a opção 3 mais de uma vez
                strcpy(bin, mem_instr[temp_pc]);
                while (strcmp(bin,"0000000000000000") != 0) 
                {
                    instrucao p = decodificar(bin);
                    gerar_asm(p, temp_pc, bin);
                    temp_pc++;
                    strcpy(bin, mem_instr[temp_pc]);                
                }
                gerar_dat(memoria);
                break;    
                case 6: // ================== RUN (Código unificado com o STEP) ==================
                case 5: // ================== STEP ==================
                while (pc <= 255 && (escolha == 6 || nClocks > 0))
                {

                    pushStepback(&pilha, c, reg_IfID, reg_IdEX, reg_ExMem, reg_MemWb, entradas_forwarding, sinais_forwarding, metricas, pc, registradores, memoria);
                    pc_prox = somador_pc(pc);
                    metricas.contClock++;
                    nClocks--;



                    // 5. ETAPA DE WRITE BACK (WB)
                    estagio_wb(reg_MemWb, registradores);

                    

                    // 4. ETAPA DE ACESSO A MEMORIA (MEM)
                    reg_MemWb_antigo = reg_MemWb;
                    reg_MemWb = estagio_mem(reg_ExMem, memoria, &pc_prox);
                
                    // === DETECÇÃO DE FLUSH (Controle de Hazard de Desvio) ===
                    int desvio_tomado = ((reg_ExMem.sinais_mem.Branch && reg_ExMem.zero_ula) || reg_ExMem.sinais_mem.jump);



                    // 3. ESTAGIO DE EXECUCAO E FORWARDING (EX)
                    entradas_forwarding.id_ex_RegRS   = reg_IdEX.rs;
                    entradas_forwarding.id_ex_RegRT   = reg_IdEX.rt;
                    entradas_forwarding.ex_mem_writeREG = reg_ExMem.sinais_wb.RegWrite;
                    entradas_forwarding.ex_mem_RegRD  = reg_ExMem.registrador_destino;
                    entradas_forwarding.Mem_WB_WriteREG = reg_MemWb_antigo.sinais_wb.RegWrite;
                    entradas_forwarding.mem_wb_RegRD  = reg_MemWb_antigo.registrador_destino;

                    sinais_forwarding = forwading_unidade(entradas_forwarding, &metricas);
                    saida_mem_wb = mux_memtoreg(reg_MemWb_antigo.saida_memoria, reg_MemWb_antigo.resultado_ula, reg_MemWb_antigo.sinais_wb.MemToReg);
                    reg_ExMem = estagio_ex(reg_IdEX, reg_ExMem.resultado_ula, saida_mem_wb, sinais_forwarding);                

                    // === FLUSH NO REGISTRADOR EX/MEM ===
                    if (desvio_tomado) {
                        memset(&reg_ExMem, 0, sizeof(reg_ExMem));
                        reg_ExMem.instrucao.opcode = 1; // Sinaliza bolha
                    }



                    // 2. ESTAGIO DE DECODIFICACAO (ID) E HAZARD
                    entrada_hazard_unidade.ID_EX_READMEM = (reg_IdEX.instrucao.opcode == 11) ? 1 : 0;
                    entrada_hazard_unidade.ID_EX_registradorRT = reg_IdEX.rt;                    
                    reg_IdEX = estagio_ID(reg_IfID, registradores, entrada_hazard_unidade, &saida_hazard_unidade, &metricas);

                    // === FLUSH NO REGISTRADOR ID/EX ===
                    if (desvio_tomado) 
                    {
                        metricas.contControlHazard+=1;
                        memset(&reg_IdEX, 0, sizeof(reg_IdEX));
                        reg_IdEX.instrucao.opcode = 1; // Sinaliza bolha
                    }



                    // 1. ETAPA DE BUSCA (IF)                    
                    if (saida_hazard_unidade.IF_ID_escrita == 1) {
                        reg_IfID = estagio_busca(pc, mem_instr);
                    }

                    // === FLUSH NO REGISTRADOR IF/ID ===
                    if (desvio_tomado) {
                        memset(&reg_IfID, 0, sizeof(reg_IfID));
                        strcpy(reg_IfID.instrucao, "0001000000000000"); // Define um Opcode inválido que zera sinais
                    }

                    // ATUALIZAÇÃO DO PC 
                    // Agora inclui a verificação de desvio_tomado para ignorar os stalls
                    if (desvio_tomado || saida_hazard_unidade.pc_escrita == 1) {
                        pc = pc_prox; 
                    }

                }
                break;
            case 7:
                contador = 0;
                while (contador < nClocks && pilha.topo != NULL) {
                    popStepback(&pilha, &c, &reg_IfID, &reg_IdEX, &reg_ExMem, &reg_MemWb, &entradas_forwarding, &sinais_forwarding, &metricas, &pc, registradores, memoria);
                    contador++;
                }
                break;
                
            default:
                break;
        }
    } while (escolha != 0);
    endwin();
    desalocameminstr(mem_instr, m, n);
    printf("\n\nSimulador encerrado com sucesso.\n");
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

int carregamem (char **mem_instr, int m, int n, char *arquivo)
{
    mem = fopen(arquivo, "r");
    if (mem == NULL){
        return 1;
    }
    int c;
    for(int i=0;i<m;i++){
        for(int j=0;j<n;j++) {
            c = fgetc(mem);
            if(c == '1'){
                mem_instr[i][j] = c;
            }
            else if(c=='0'){
                mem_instr[i][j] = '0';
            }
            else if( c == '\n'){
            j--;
            }
            else {
            mem_instr[i][j] = '0';
            }
        }
    }
    fclose(mem);
    return 0;
}

int carregadat(int *mem_dados, char *arquivo)
{
    mem = fopen(arquivo, "r");
    if (mem == NULL){
        return 1;
    }
    char c[6];
    int i = 0;
    while (i < 256 && fscanf(mem, "%5s", c) == 1){
        mem_dados[i] = atoi(c);
        i++;
    }

    fclose(mem);
    return 0;
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
    }
    
    return i;
}


int sign_extend6to8(int imm)
{
    if (imm & 0x20)      // verifica bit de sinal (bit 5)
        return imm | 0xC0;  // adiciona 11 nos 2 bits superiores
    else
        return imm;
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

void gerar_asm(instrucao p,int pc,char bin[])
{
    FILE *arquivo;
    arquivo = fopen("assembly.asm","a");

    if (!arquivo)
    {
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


saida_unidade_hazard unidade_hazard(entrada_unidade_hazard hazard_unidade,metricas *m)
{
    saida_unidade_hazard saida_hazard_unidade;
    if (hazard_unidade.ID_EX_READMEM &&((hazard_unidade.ID_EX_registradorRT == hazard_unidade.IF_ID_registradorRS) || (hazard_unidade.ID_EX_registradorRT == hazard_unidade.IF_ID_registradorRT)))
    {
        saida_hazard_unidade.IF_ID_escrita=0;
        saida_hazard_unidade.pc_escrita=0;
        saida_hazard_unidade.sinal_mux_controle=1;
        m->num_stall+=1;
    }
    else
    {
        saida_hazard_unidade.IF_ID_escrita=1;
        saida_hazard_unidade.pc_escrita=1;
        saida_hazard_unidade.sinal_mux_controle=0;

    }
    return saida_hazard_unidade;
}
sinais_controle_forwading forwading_unidade(unidade_forwading f,metricas *m)
{
    sinais_controle_forwading s = {0};
    if (f.ex_mem_writeREG && (f.ex_mem_RegRD != 0) && (f.ex_mem_RegRD == f.id_ex_RegRS)) 
    {
        s.forwadingA = 2;
        m->contDataHazard+=1;
    } 
    else if (f.Mem_WB_WriteREG && (f.mem_wb_RegRD != 0) && (f.mem_wb_RegRD == f.id_ex_RegRS))
    {
        s.forwadingA = 1;
        m->contDataHazard+=1;
        
    }
    if (f.ex_mem_writeREG && (f.ex_mem_RegRD != 0) && (f.ex_mem_RegRD == f.id_ex_RegRT)) {
        s.forwadingB = 2;
        m->contDataHazard+=1;
    } 
    else if (f.Mem_WB_WriteREG && (f.mem_wb_RegRD != 0) && (f.mem_wb_RegRD == f.id_ex_RegRT)) {
        s.forwadingB = 1;
        m->contDataHazard+=1;
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
controle sinais_controle_pipeline(instrucao i,metricas *m)
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
            c.ALUOp = i.funct;
            if(!(i.rd == 0 && i.rs == 0 && i.rt == 0))
            {
                m->contInstReg+=1;
            } // usa funct direto
            break;
        case 4:
            // ADDI
            m->contInstImm+=1;
            c.RegDst = 0;
            c.ALUSrc = 1;
            c.RegWrite = 1;
            c.ALUOp = 0;
            c.MemToReg=1;
            break;
        case 11:
            // LW
            m->contInstImm+=1;
            m->lw+=1;
            c.ALUSrc = 1;
            c.MemToReg = 0;
            c.RegWrite = 1;
            c.MemRead = 1;
            break;
        case 15:
            // SW
            m->contInstImm+=1;
            m->sw+=1;
            c.ALUSrc = 1;
            c.MemWrite = 1;
            break;
        case 8:
            // BEQ
            m->contInstImm+=1;
            m->num_desvio_condicional+=1;
            c.Branch = 1;
            c.ALUOp = 2;
            break;
        case 2:
            // JUMP
            m->contInstJump+=1;
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
REG_pepiline_ID_EX estagio_ID(REG_pepiline_BI_ID r,int banco_registrador[8],entrada_unidade_hazard entrada_hazard_unidade,saida_unidade_hazard *saida_hazard_unidade,metricas *m)
{
    instrucao i;
    REG_pepiline_ID_EX id={0};
    REG_pepiline_ID_EX entrada2={0};
    REG_pepiline_ID_EX Registrador_id_ex={0};
    controle c;
    i=decodificar(r.instrucao);
    c=sinais_controle_pipeline(i,m);
    entrada_hazard_unidade.IF_ID_registradorRS=i.rs;
    entrada_hazard_unidade.IF_ID_registradorRT=i.rt;
    *saida_hazard_unidade=unidade_hazard(entrada_hazard_unidade,m);

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
    
    entrada2.instrucao=id.instrucao;
    entrada2.saida1_banco_reg=id.saida1_banco_reg;
    entrada2.saida2_banco_reg=id.saida2_banco_reg;
    entrada2.sinal_extendido=id.sinal_extendido;
    entrada2.soma_pc=id.soma_pc;
    entrada2.valor_jump=id.valor_jump;
    entrada2.rd=id.rd;
    entrada2.rs=id.rs;
    entrada2.rt=id.rt;

    Registrador_id_ex=mux_sinais_controle(saida_hazard_unidade->sinal_mux_controle,id,entrada2);
    return Registrador_id_ex;
}
REG_pepiline_EX_MEM estagio_ex(REG_pepiline_ID_EX id, int ex_mem, int mem_wb, sinais_controle_forwading sinal_forwading)
{
    REG_pepiline_EX_MEM ex = {0};
    int overflow;
    int zero;
    int saida_mux_ula_fonte;
    int resultado_forwadingA;
    int resultado_forwadingB;
    resultado_forwadingA = mux_forwadingA(id.saida1_banco_reg, ex_mem, mem_wb, sinal_forwading.forwadingA);
    resultado_forwadingB = mux_forwadingB(id.saida2_banco_reg, ex_mem, mem_wb, sinal_forwading.forwadingB); // Usa a saida2_banco_reg!


    // 2. O MUX ALUSrc decide entre a Saída do Forwarding B ou o Imediato
    saida_mux_ula_fonte = mux_ula_fonte(resultado_forwadingB, id.sinal_extendido, id.sinais_ex.ALUSrc);

    // 3. A ULA recebe o Forwarding A e a Saída do MUX ALUSrc
    ex.resultado_ula = ula_pipeline(resultado_forwadingA, saida_mux_ula_fonte, id.sinais_ex.ALUOp, &overflow, &zero);

    ex.registrador_destino = mux_regDST(id.rt, id.rd, id.sinais_ex.RegDst);
    
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


    switch (ex.sinais_mem.MemWrite)
    {
    case 1:
        if (ex.resultado_ula >= 0 && ex.resultado_ula < 256) {
            memoria[ex.resultado_ula] = ex.saida2_banco_registradores;
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

    saida_mux_branch = mux_branch(ex.sinais_mem.Branch & ex.zero_ula, ex.soma_pc, ex.endereco_desvio);

    // Só alteramos o PC se o Branch for tomado  OU se for um Jump.
    // Se for uma instrução normal , NÃO tocamos no *pc.
    if ((ex.sinais_mem.Branch && ex.zero_ula) || ex.sinais_mem.jump) {
        *pc = mux_jump(ex.sinais_mem.jump, saida_mux_branch, ex.valor_jump);
    }
    Mem.resultado_ula = ex.resultado_ula;
    Mem.registrador_destino = ex.registrador_destino;
    Mem.sinais_wb = ex.sinais_wb;
    Mem.instrucao=ex.instrucao;
    
    return Mem;
}

void estagio_wb(REG_pepiline_MEM_WB Mem, int banco_registrador[8])
{
    if (Mem.sinais_wb.RegWrite)
    {
        banco_registrador[Mem.registrador_destino] = mux_memtoreg(Mem.saida_memoria,Mem.resultado_ula,Mem.sinais_wb.MemToReg);
        // MemToReg 0: Memória    1: ULA
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
        return;
    }
    if (descritor->topo == NULL) {
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
void desenha_menu(WINDOW *win, int largura, int altura) {
    wclear(win);
    wattron(win, COLOR_PAIR(5));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(5));

    wattron(win, COLOR_PAIR(1) | A_BOLD); 
    mvwprintw(win, 1, (largura / 2) - 14, "SIMULADOR MINI MIPS PIPELINE");
    wattroff(win, COLOR_PAIR(1) | A_BOLD);

    wattron(win, COLOR_PAIR(5));
    mvwaddch(win, 2, 0, ACS_LTEE);  
    mvwhline(win, 2, 1, 0, largura - 2);    
    mvwaddch(win, 2, largura - 1, ACS_RTEE); 
    wattroff(win, COLOR_PAIR(5)); 

    mvwprintw(win, 3,  4, "[1] Carregar memoria de instrucoes");
    mvwprintw(win, 4,  4, "[2] Carregar memoria de dados");
    mvwprintw(win, 5,  4, "[3] Salvar asm e dat");
    mvwprintw(win, 6,  4, "[4] Imprimir memoria de dados e instrucoes");
    mvwprintw(win, 7,  4, "[5] Executar um clock");
    mvwprintw(win, 8,  4, "[6] Executar um programa");
    mvwprintw(win, 9,  4, "[7] Voltar um clock");
    mvwprintw(win, 10,  4, "[0] Sair do programa");
}
void desenha_estatisticas(WINDOW *win, int largura, int altura, metricas m) 
{
    wclear(win);
    wattron(win, COLOR_PAIR(2));
    box(win, 0, 0);
    int meio = largura / 2;
    mvwaddch(win, 2, 0, ACS_LTEE);  
    mvwhline(win, 2, 1, ACS_HLINE, largura - 2);    
    mvwaddch(win, 2, largura - 1, ACS_RTEE); 
    
    mvwvline(win, 3, meio, ACS_VLINE, altura - 4);
    mvwaddch(win, 2, meio, ACS_TTEE);
    mvwaddch(win, altura - 1, meio, ACS_BTEE);
    wattroff(win, COLOR_PAIR(6));

    wattron(win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win, 1, (largura / 2) - 12, "ESTATISTICAS DO SIMULADOR");
    wattroff(win, COLOR_PAIR(3) | A_BOLD);

    int col1 = 2;
    int col2 = meio + 2;

    // Lado Esquerdo dinâmico
    wattron(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    mvwprintw(win, 3, 2, "DESEMPENHO GLOBAL");
    wattroff(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    mvwprintw(win, 5, 2, "Ciclos de clock: %d", m.contClock);
    mvwprintw(win, 6, 2, "Instruções Uteis: %d", (m.contInstReg + m.contInstImm + m.contInstJump));
    mvwprintw(win, 7, 2, "CPI real: %.2f", m.contClock > 0 ? (float)m.contClock / (m.contInstReg + m.contInstImm + m.contInstJump) : 0.0);
    mvwprintw(win, 8, 2, "Tempo de clock: %d ps", m.clockTime);
    wattron(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    mvwprintw(win, 10, 2, "EFICIENCIA E HAZARDS");
    wattroff(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    mvwprintw(win, 11, 2, "Bolhas (Data Hazard): %d", m.num_stall); // Assumindo que você tem essa métrica
    mvwprintw(win, 12, 2, "Flushes (Ctrl Hazard): %d", m.contControlHazard); // Assumindo que você tem essa métrica
    mvwprintw(win, 13, 2, "Forwardings: %d",m.contDataHazard);

    // Lado Direito dinâmico
    int col_direita = meio + 2;
    wattron(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    mvwprintw(win, 3,col_direita, "PERFIL DO PROGRAMA (MIX)");
    wattroff(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    mvwprintw(win, 5, col_direita, "TIPO R: %d", m.contInstReg);
    mvwprintw(win, 6, col_direita, "TIPO I (Imm): %d", m.contInstImm);
    mvwprintw(win, 7, col_direita, "Leitura memoria: %d",m.lw);
    mvwprintw(win, 8, col_direita, "Escrita memoria: %d",m.sw);
    mvwprintw(win, 9, col_direita, "Desvio condicional:%d",m.num_desvio_condicional); // Ajuste com sua contagem real
    mvwprintw(win, 10, col_direita, "Jump (Incond): %d", m.contInstJump);
}
void desenha_opcao(WINDOW *win, int largura, int altura) {
    wclear(win);
    wattron(win, COLOR_PAIR(3));
    box(win, 0, 0);
    // Centraliza o texto verticalmente dependendo da altura que sobrar
    mvwprintw(win, altura / 2 - 1, 4, "Escolha uma opcao: "); 
    wattroff(win, COLOR_PAIR(3));
}
void gerar_assembly_str(instrucao p, char *destino, sinais_controle_forwading f, saida_unidade_hazard h) 
{
    int imm_ext;
            sprintf(destino, "1 %d", p.addr);
    switch (p.opcode) {



            //testando
        case 1:
            sprintf(destino, "Bolha %d", p.addr);
            break;

        case 0:
            switch (p.funct) {
                case 0:
                    sprintf(destino, "add $%d,$%d,$%d", p.rd, p.rs, p.rt);
                    break;
                case 2:
                    sprintf(destino, "sub $%d,$%d,$%d", p.rd, p.rs, p.rt);
                    break;
                default:
                    sprintf(destino, "nop");
                    break;
            }
            break;

        case 2:
            sprintf(destino, "jump %d", p.addr);
            break;
        case 4:
            imm_ext = sign_extend6to8(p.imm);
            sprintf(destino, "addi $%d,$%d,%d", p.rt, p.rs, imm_ext);
            break;

        case 8:
            imm_ext = sign_extend6to8(p.imm);
            sprintf(destino, "beq $%d,$%d,%d", p.rs, p.rt, imm_ext);
            break;

        case 11:
            imm_ext = sign_extend6to8(p.imm);
            sprintf(destino, "lw $%d,%d($%d)", p.rt, imm_ext, p.rs);
            break;

        case 15:
            imm_ext = sign_extend6to8(p.imm);
            sprintf(destino, "sw $%d,%d($%d)", p.rt, imm_ext, p.rs);
            break;

        default:
            sprintf(destino, "unknown %d", p.opcode);
            break;
    }
}

void desenha_registradores_pipeline(WINDOW *win, int largura, int altura, int reg[8], REG_pepiline_BI_ID ifid, REG_pepiline_ID_EX idex, REG_pepiline_EX_MEM exmem, REG_pepiline_MEM_WB memwb, sinais_controle_forwading controleForwarding, saida_unidade_hazard unidade_hazard)
{
    wclear(win);
    wattron(win, COLOR_PAIR(4)); box(win, 0, 0); wattroff(win, COLOR_PAIR(4));

    int meio = largura / 2;
    int col1 = 2;          
    int col2 = meio + 2; 
    int y = 1; 
    char txt_buffer[128];
    char ass_str[64];
    
    // Criamos uma estrutura vazia de segurança para passar nos estágios que não usam hazard/forwarding
    sinais_controle_forwading fwd_vazio = {0};
    saida_unidade_hazard hazard_vazio = {0};

    // ==========================================
    // BANCO REGISTRADORES
    // ==========================================
    wattron(win, COLOR_PAIR(6) | A_BOLD | A_REVERSE);
    mvwprintw(win, y, 1, " BANCO DE REGISTRADORES %*s", largura - 26, ""); 
    wattroff(win, COLOR_PAIR(6) | A_BOLD | A_REVERSE); y++;
    
    mvwprintw(win, y, col1, "R0:[%d]", reg[0]); mvwprintw(win, y, col2, "R4:[%d]", reg[4]); y++;
    mvwprintw(win, y, col1, "R1:[%d]", reg[1]); mvwprintw(win, y, col2, "R5:[%d]", reg[5]); y++;
    mvwprintw(win, y, col1, "R2:[%d]", reg[2]); mvwprintw(win, y, col2, "R6:[%d]", reg[6]); y++;
    mvwprintw(win, y, col1, "R3:[%d]", reg[3]); mvwprintw(win, y, col2, "R7:[%d]", reg[7]); y++;
    
    wattron(win, COLOR_PAIR(4)); mvwaddch(win, y, 0, ACS_LTEE); mvwhline(win, y, 1, ACS_HLINE, largura - 2); mvwaddch(win, y, largura - 1, ACS_RTEE); wattroff(win, COLOR_PAIR(4)); y++;

    // ==========================================
    // IF/ID
    // ==========================================
    snprintf(txt_buffer, sizeof(txt_buffer), " IF/ID  ->  [ %s ]", ifid.instrucao);
    
    wattron(win, COLOR_PAIR(1) | A_BOLD | A_REVERSE);
    mvwprintw(win, y, 1, "%-*s", largura - 2, txt_buffer); 
    wattroff(win, COLOR_PAIR(1) | A_BOLD | A_REVERSE); y++;
    
    mvwprintw(win, y, col1, "Inst: %s", ifid.instrucao); y++;
    mvwprintw(win, y, col1, "PC+1: %d", ifid.soma_pc); y++;
    
    wattron(win, COLOR_PAIR(4)); mvwaddch(win, y, 0, ACS_LTEE); mvwhline(win, y, 1, ACS_HLINE, largura - 2); mvwaddch(win, y, largura - 1, ACS_RTEE); wattroff(win, COLOR_PAIR(4)); y++;

    // ==========================================
    // ID/EX -> COM DETECÇÃO DE BOLHA/NOP
    // ==========================================
    gerar_assembly_str(idex.instrucao, ass_str, controleForwarding, unidade_hazard);
    
    // Se a unidade de hazard indicar o mux de controle em 1, adiciona o aviso de NOP inserido
    if (unidade_hazard.sinal_mux_controle == 1) {
        snprintf(txt_buffer, sizeof(txt_buffer), " ID/EX  ->  [ %s ] << HAZARD: NOP INSERIDO >>", ass_str);
    } else {
        snprintf(txt_buffer, sizeof(txt_buffer), " ID/EX  ->  [ %s ]", ass_str);
    }

    wattron(win, COLOR_PAIR(2) | A_BOLD | A_REVERSE);
    mvwprintw(win, y, 1, "%-*s", largura - 2, txt_buffer); 
    wattroff(win, COLOR_PAIR(2) | A_BOLD | A_REVERSE); y++;
    
    mvwprintw(win, y, col1, "Saida RS: %d", idex.saida1_banco_reg); mvwprintw(win, y, col2, "Saida RT: %d", idex.saida2_banco_reg); y++;
    mvwprintw(win, y, col1, "Imed Estendido: %d", idex.sinal_extendido); mvwprintw(win, y, col2, "Regs [RS:%d RT:%d RD:%d]", idex.rs, idex.rt, idex.rd); y++;
    mvwprintw(win, y, col1, "Sinais EX  [ALUOp:%d ALUSrc:%d RegDst:%d]", idex.sinais_ex.ALUOp, idex.sinais_ex.ALUSrc, idex.sinais_ex.RegDst); y++;
    mvwprintw(win, y, col1, "Sinais MEM [Branch:%d MemWrt:%d Jump:%d]", idex.sinais_mem.Branch, idex.sinais_mem.MemWrite, idex.sinais_mem.jump); y++;
    mvwprintw(win, y, col1, "Sinais WB  [RegWrt:%d M2R:%d]", idex.sinais_wb.RegWrite, idex.sinais_wb.MemToReg); y++;
    
    wattron(win, COLOR_PAIR(4)); mvwaddch(win, y, 0, ACS_LTEE); mvwhline(win, y, 1, ACS_HLINE, largura - 2); mvwaddch(win, y, largura - 1, ACS_RTEE); wattroff(win, COLOR_PAIR(4)); y++;

    // ==========================================
    // EX/MEM -> COM DETECÇÃO DE FORWARDING
    // ==========================================
    gerar_assembly_str(exmem.instrucao, ass_str, controleForwarding, hazard_vazio);
    
    // Se qualquer um dos seletores de forwarding for diferente de 0, exibe o aviso
    if (controleForwarding.forwadingA != 0 || controleForwarding.forwadingB != 0) {
        snprintf(txt_buffer, sizeof(txt_buffer), " EX/MEM  ->  [ %s ] << FWD ATIVO (A:%d B:%d) >>", ass_str, controleForwarding.forwadingA, controleForwarding.forwadingB);
    } else {
        snprintf(txt_buffer, sizeof(txt_buffer), " EX/MEM  ->  [ %s ]", ass_str);
    }

    wattron(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    mvwprintw(win, y, 1, "%-*s", largura - 2, txt_buffer); 
    wattroff(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE); y++;
    
    mvwprintw(win, y, col1, "Res ULA: %d", exmem.resultado_ula); mvwprintw(win, y, col2, "Zero: %d", exmem.zero_ula); y++;
    mvwprintw(win, y, col1, "End Desvio: %d", exmem.endereco_desvio); mvwprintw(win, y, col2, "Dado p/ Mem: %d", exmem.saida2_banco_registradores); y++;
    mvwprintw(win, y, col1, "Reg Destino: %d", exmem.registrador_destino); y++;
    mvwprintw(win, y, col1, "Sinais MEM [Branch:%d MemWrt:%d Jump:%d]", exmem.sinais_mem.Branch, exmem.sinais_mem.MemWrite, exmem.sinais_mem.jump); y++;
    mvwprintw(win, y, col1, "Sinais WB  [RegWrt:%d M2R:%d]", exmem.sinais_wb.RegWrite, exmem.sinais_wb.MemToReg); y++;
    
    wattron(win, COLOR_PAIR(4)); mvwaddch(win, y, 0, ACS_LTEE); mvwhline(win, y, 1, ACS_HLINE, largura - 2); mvwaddch(win, y, largura - 1, ACS_RTEE); wattroff(win, COLOR_PAIR(4)); y++;

    // ==========================================
    // MEM/WB
    // ==========================================
    gerar_assembly_str(memwb.instrucao, ass_str, fwd_vazio, hazard_vazio);
    snprintf(txt_buffer, sizeof(txt_buffer), " MEM/WB  ->  [ %s ]", ass_str);

    wattron(win, COLOR_PAIR(5) | A_BOLD | A_REVERSE);
    mvwprintw(win, y, 1, "%-*s", largura - 2, txt_buffer); 
    wattroff(win, COLOR_PAIR(5) | A_BOLD | A_REVERSE); y++;
    
    mvwprintw(win, y, col1, "Res ULA: %d", memwb.resultado_ula); mvwprintw(win, y, col2, "Saida Mem: %d", memwb.saida_memoria); y++;
    mvwprintw(win, y, col1, "Reg Destino: %d", memwb.registrador_destino); y++;
    mvwprintw(win, y, col1, "Sinais WB  [RegWrite:%d MemToReg:%d]", memwb.sinais_wb.RegWrite, memwb.sinais_wb.MemToReg);
}
void exibir_memorias_pipeline_ncurses(char **mem_inst, int *mem_dados) 
{
    clear();
    refresh();
    int margem = 1;
    int alt = LINES - 3; 
    int larg = (COLS - 3) / 2; 
    sinais_controle_forwading fwd_vazio = {0};
    saida_unidade_hazard hazard_vazio = {0};
    // Criação das janelas separadas

    WINDOW *w_inst = newwin(alt, larg, margem, margem);
    WINDOW *w_data = newwin(alt, larg, margem, margem + larg + 1);
    keypad(stdscr, TRUE); 
    nodelay(stdscr, FALSE); 
    flushinp();

    int topo_inst = 0, topo_dados = 0;
    int max_visiveis = alt - 6;
    int rodando = 1;
    char bin[17];
    char ass_str[64];

    while(rodando) {
        werase(w_inst); box(w_inst, 0, 0);
        werase(w_data); box(w_data, 0, 0);

        // ====================================================
        // CABEÇALHO: MEMÓRIA DE INSTRUÇÕES (Esquerda)
        // ====================================================
        wattron(w_inst, A_REVERSE | COLOR_PAIR(2));
        mvwprintw(w_inst, 1, (larg - 23)/2, " MEMÓRIA DE INSTRUÇÕES ");
        wattroff(w_inst, A_REVERSE | COLOR_PAIR(2));
        mvwhline(w_inst, 2, 1, ACS_HLINE, larg - 2);
        
        wattron(w_inst, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(w_inst, 3, 2, "End");
        mvwprintw(w_inst, 3, 7, "Binário (16 bits)");
        mvwprintw(w_inst, 3, 26, "Assembly");
        wattroff(w_inst, COLOR_PAIR(1) | A_BOLD);
        mvwhline(w_inst, 4, 1, ACS_HLINE, larg - 2);

        // ====================================================
        // CABEÇALHO: MEMÓRIA DE DADOS (Direita)
        // ====================================================
        wattron(w_data, A_REVERSE | COLOR_PAIR(3));
        mvwprintw(w_data, 1, (larg - 18)/2, " MEMÓRIA DE DADOS ");
        wattroff(w_data, A_REVERSE | COLOR_PAIR(3));
        mvwhline(w_data, 2, 1, ACS_HLINE, larg - 2);
        
        wattron(w_data, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(w_data, 3, 2, "End");
        mvwprintw(w_data, 3, 7, "Valor Armazenado");
        wattroff(w_data, COLOR_PAIR(1) | A_BOLD);
        mvwhline(w_data, 4, 1, ACS_HLINE, larg - 2);

        // ====================================================
        // PREENCHENDO INSTRUÇÕES (char **)
        // ====================================================
        for (int i = 0; i < max_visiveis; i++) {
            int k = topo_inst + i;
            if (k >= 256) break;
            int y = 5 + i;

            mvwprintw(w_inst, y, 2, "%3d", k); mvwaddch(w_inst, y, 6, ACS_VLINE);

            // Verifica se a string existe e se não está vazia para não quebrar
            if (mem_inst == NULL || mem_inst[k] == NULL || mem_inst[k][0] == '\0' || mem_inst[k][0] == ' ' || mem_inst[k][0] == '\n') {
                wattron(w_inst, COLOR_PAIR(5));
                mvwprintw(w_inst, y, 8, "0000000000000000"); mvwaddch(w_inst, y, 25, ACS_VLINE);
                mvwprintw(w_inst, y, 27, "--- vazio ---");
                wattroff(w_inst, COLOR_PAIR(5));
            } else {
                strncpy(bin, mem_inst[k], 16); bin[16] = '\0';
                wattron(w_inst, COLOR_PAIR(3)); mvwprintw(w_inst, y, 8, "%s", bin); wattroff(w_inst, COLOR_PAIR(3));
                mvwaddch(w_inst, y, 25, ACS_VLINE);

                instrucao inst = decodificar(bin);
                gerar_assembly_str(inst, ass_str, fwd_vazio, hazard_vazio); 
                
                wattron(w_inst, COLOR_PAIR(4)); mvwprintw(w_inst, y, 27, "%s", ass_str); wattroff(w_inst, COLOR_PAIR(4));
            }
        }

        // ====================================================
        // PREENCHENDO DADOS (int *)
        // ====================================================
        for (int i = 0; i < max_visiveis; i++) {
            int k = topo_dados + i;
            if (k >= 256) break;
            int y = 5 + i;

            mvwprintw(w_data, y, 2, "%3d", k); mvwaddch(w_data, y, 6, ACS_VLINE);

            // Acesso direto ao valor inteiro
            int valor = mem_dados[k];

            if (valor == 0) {
                wattron(w_data, COLOR_PAIR(5));
                mvwprintw(w_data, y, 8, "%d (vazio)", valor);
                wattroff(w_data, COLOR_PAIR(5));
            } else {
                wattron(w_data, COLOR_PAIR(4)); 
                mvwprintw(w_data, y, 8, "%d", valor); 
                wattroff(w_data, COLOR_PAIR(4));
            }
        }

        wrefresh(w_inst);
        wrefresh(w_data);

        // ====================================================
        // MENU INFERIOR GERAL
        // ====================================================
        attron(COLOR_PAIR(5) | A_BOLD | A_REVERSE);
        mvprintw(LINES - 2, (COLS - 75)/2, " [SETAS]: Rolar Instrucoes | [W/S]: Rolar Dados | [ENTER]: Voltar ao Menu ");
        attroff(COLOR_PAIR(5) | A_BOLD | A_REVERSE);
        refresh();

        // ====================================================
        // CAPTURA DE TECLAS E LÓGICA DE ROLAGEM
        // ====================================================
        int tecla = getch();
        
        // Rola Instruções
        if (tecla == KEY_DOWN && topo_inst < 256 - max_visiveis) topo_inst++;
        else if (tecla == KEY_UP && topo_inst > 0) topo_inst--;
        
        // Rola Dados
        else if ((tecla == 's' || tecla == 'S') && topo_dados < 256 - max_visiveis) topo_dados++;
        else if ((tecla == 'w' || tecla == 'W') && topo_dados > 0) topo_dados--;
        
        // Sair da aba
        else if (tecla == 10 || tecla == 27 || tecla == 'q') rodando = 0; 
    }

    // Limpeza na hora de fechar e voltar ao menu
    delwin(w_inst);
    delwin(w_data);
    clear();
    refresh();
}
int listar_arquivos(const char *extensao, char arquivos[][MAX_NOMEARQUIVO])
{
    DIR *dir;
    struct dirent *ent;
    int qtd = 0;

    dir = opendir(".");

    if(dir == NULL)
        return 0;

    while((ent = readdir(dir)) != NULL)
    {
        char *ponto = strrchr(ent->d_name,'.');

        if(ponto && strcmp(ponto, extensao)==0)
        {
            strcpy(arquivos[qtd], ent->d_name);
            // REMOVIDO: printf("Lido: '%s' (%zu)\n", ...);
            
            qtd++;

            if(qtd >= MAX_ARQUIVOS)
                break;
        }
    }

    closedir(dir);

    return qtd;
}

void menu_carregar_memoria(const char *ext, int tipo, char **memoriaInstrucoes, int *memoriaDados)
{
    char arquivos[MAX_ARQUIVOS][MAX_NOMEARQUIVO];

    int qtd = listar_arquivos(ext, arquivos);
    
    // REMOVIDO: for com printf imprimindo os arquivos no terminal

    if(qtd == 0)
    {
        popup_msg("Nenhum arquivo encontrado", 0);
        return;
    }

    keypad(stdscr, TRUE);

    int sel = 0;

    int H = LINES * 3 / 4;
    int W = COLS / 2;

    int Y = (LINES - H) / 2;
    int X = (COLS - W) / 2;

    WINDOW *w = newwin(H, W, Y, X);
    WINDOW *lista = derwin(w, H-8, W-8, 4, 4);

    while(1)
    {
        werase(w);
        wattron(w, COLOR_PAIR(1));   
        box(w, 0, 0);
        wattroff(w, COLOR_PAIR(1));
        
        const char *titulo =
            (tipo == 0)
            ? " CARREGAR MEMORIA DE INSTRUCOES "
            : " CARREGAR MEMORIA DE DADOS ";

        wattron(w, A_REVERSE | COLOR_PAIR(1));
        mvwprintw(w, 1, (W - strlen(titulo))/2, "%s", titulo);
        wattroff(w, A_REVERSE | COLOR_PAIR(1));

        mvwhline(w, 2, 1, ACS_HLINE, W-2);

        // lista
        werase(lista);
        box(lista, 0, 0);

        mvwprintw(lista, 1, 2, "Arquivos (%s)", ext);

        int max = getmaxy(lista);
        for(int i = 0; i < qtd && (3+i) < max-1; i++)
        {
            if(i == sel)
            {
                wattron(lista, COLOR_PAIR(3) | A_BOLD);
                mvwprintw(lista, 3+i, 2, "> %s", arquivos[i]);
                wattroff(lista, COLOR_PAIR(3) | A_BOLD);
            }
            else
            {
                mvwprintw(lista, 3+i, 4, "%s", arquivos[i]);
            }
        }

        // footer
        wattron(w, COLOR_PAIR(5) | A_REVERSE);
        mvwprintw(w, H-2, (W - 60)/2,
                  " [SETAS]: Selecionar | [ENTER]: Carregar | [ESC]: Voltar ");
        wattroff(w, COLOR_PAIR(5) | A_REVERSE);

        // CORREÇÃO: Ordem de refresh. Primeiro o Pai (w), depois o Filho (lista).
        wrefresh(w);
        wrefresh(lista);

        int ch = getch();

        if(ch == KEY_UP && sel > 0)
            sel--;
        else if(ch == KEY_DOWN && sel < qtd-1)
            sel++;
        else if(ch == 10) // ENTER
        {
            int erro;

            if(tipo == 0)
                erro = carregamem(memoriaInstrucoes, 256, 16, arquivos[sel]);
            else
                erro = carregadat(memoriaDados, arquivos[sel]);

            // LIMPEZA VISUAL DA JANELA ANTES DE DESTRUIR
            werase(w);
            wrefresh(w);
            
            delwin(lista);
            delwin(w);

            if(!erro)
            {
                char buf[120];
                snprintf(buf, sizeof(buf), "Arquivo '%s' carregado!", arquivos[sel]);
                popup_msg(buf, 1);
            }
            else
            {
                popup_msg("Erro ao carregar arquivo", 0);
            }

            return;
        }
        else if(ch == 27) // ESC
        {
            // LIMPEZA VISUAL DA JANELA ANTES DE DESTRUIR
            werase(w);
            wrefresh(w);
            
            delwin(lista);
            delwin(w);
            return;
        }
    }
}

void popup_msg(const char *msg, int flag)
{
    int h = 7, w = 50;

    WINDOW *p = newwin(h, w, (LINES-h)/2, (COLS-w)/2);
    wattron(p, (flag == 1) ? COLOR_PAIR(3) | A_BOLD : COLOR_PAIR(4) | A_BOLD);
    box(p,0,0);
    wattroff(p, (flag == 1) ? COLOR_PAIR(3) | A_BOLD : COLOR_PAIR(4) | A_BOLD);

    wattron(p, (flag == 1) ? COLOR_PAIR(3) | A_BOLD : COLOR_PAIR(4) | A_BOLD);
    mvwprintw(p, 2, (w - strlen(msg))/2, "%s", msg);
    wattroff(p, (flag == 1) ? COLOR_PAIR(3) | A_BOLD : COLOR_PAIR(4) | A_BOLD);

    mvwprintw(p, 4, (w - 22)/2, "ENTER para continuar");

    wrefresh(p);

    while(getch() != 10);

    // LIMPEZA VISUAL DO POPUP ANTES DE DESTRUIR
    werase(p);
    wrefresh(p);
    delwin(p);
}