#include <ncurses.h>
#include <stdio.h>
void tela_registradores(WINDOW *win, int regs[]);
int registradores[8] = {0, 1, 2, 0, 0, 10, 0, 0}; 
int main() 
{
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

    int t_linhas, t_colunas;
    getmaxyx(stdscr, t_linhas, t_colunas);

    int altura_janela = 17;
    int largura_janela = 50;
    int largura_janela_opcao = 50;
    int altura_janela_opcao = 5;
    int altura_total_bloco = altura_janela + altura_janela_opcao + 1;

    if (t_linhas < altura_total_bloco || t_colunas < largura_janela) {
        endwin();
        printf("Erro: Seu terminal e muito pequeno para as duas janelas!\n");
        printf("Necessario: %dx%d | Seu terminal: %dx%d\n", altura_total_bloco, largura_janela, t_linhas, t_colunas);
        return 1;
    }

    int janela_mainY = (t_linhas - altura_total_bloco) / 2;
    int janela_mainX = (t_colunas - largura_janela) / 2;

    WINDOW *menu_win = newwin(altura_janela, largura_janela, janela_mainY, janela_mainX);
    
    int caixa_opcaoY = janela_mainY + altura_janela + 1;
    int caixa_opcaoX = janela_mainX;
    
    WINDOW *janela_op = newwin(altura_janela_opcao, largura_janela_opcao, caixa_opcaoY, caixa_opcaoX);
    int altura_janela_reg = 10;     
    int largura_janela_reg = 50;     
    int janela_regY = janela_mainY;  
    int janela_regX = janela_mainX;  
    WINDOW *janela_reg = newwin(altura_janela_reg, largura_janela_reg, janela_regY, janela_regX);    
    int escolha = -1;
    
    do {
        wclear(menu_win);
        wclear(janela_op);

        wattron(menu_win, COLOR_PAIR(5));
        box(menu_win, 0, 0);
        wattroff(menu_win, COLOR_PAIR(5));

        wattron(menu_win, COLOR_PAIR(1) | A_BOLD); 
        mvwprintw(menu_win, 1, 11, "SIMULADOR MINI MIPS PIPELINE");
        wattroff(menu_win, COLOR_PAIR(1) | A_BOLD);

        wattron(menu_win, COLOR_PAIR(5));
        mvwaddch(menu_win, 2, 0, ACS_LTEE);  
        mvwhline(menu_win, 2, 1, 0, 48);    
        mvwaddch(menu_win, 2, 49, ACS_RTEE); 
        wattroff(menu_win, COLOR_PAIR(5)); 

        mvwprintw(menu_win, 3,  4, "[1] Carregar memoria de instrucao");
        mvwprintw(menu_win, 4,  4, "[2] Carregar memoria de dados");
        mvwprintw(menu_win, 5,  4, "[3] Imprimir instrucoes e dados");
        mvwprintw(menu_win, 6,  4, "[4] Imprimir banco registradores");
        mvwprintw(menu_win, 7,  4, "[5] Imprimir todo simulador");
        mvwprintw(menu_win, 8,  4, "[6] Salvar .asm e .dat");
        mvwprintw(menu_win, 9,  4, "[7] Mostrar Estatisticas");
        mvwprintw(menu_win, 10, 4, "[8] Executar programa (RUN)");
        mvwprintw(menu_win, 11, 4, "[9] Executar um clock (STEP)");
        mvwprintw(menu_win, 12, 4, "[s] Voltar uma instrucao");
        mvwprintw(menu_win, 13, 4, "[0] Encerrar programa");
        
        wattron(janela_op, COLOR_PAIR(3));
        box(janela_op, 0, 0);
        mvwprintw(janela_op, 2, 4, "Escolha uma opcao: "); 
        wattroff(janela_op, COLOR_PAIR(3));

        wrefresh(menu_win);
        wrefresh(janela_op);

        int tecla = wgetch(menu_win);

        if (tecla >= '0' && tecla <= '9') {
            escolha = tecla - '0'; 
        } else if (tecla == 's' || tecla == 'S') {
            escolha = 10; 
        } else {
            escolha = -1; 
        }

        if (escolha != 0) 
        {
            wclear(menu_win);
            box(menu_win, 0, 0);

            if (escolha == -1) {
                wattron(menu_win, COLOR_PAIR(4) | A_BOLD);
                mvwprintw(menu_win, 2, 4, ">> ERRO <<");
                mvwprintw(menu_win, 5, 4, "Opcao Invalida!");
                wattroff(menu_win, COLOR_PAIR(4) | A_BOLD);
                
                mvwprintw(menu_win, 12, 4, "Pressione qualquer tecla para voltar...");
                wrefresh(menu_win);
                wgetch(menu_win);
            } 
            else {
                switch (escolha) {
                    case 1:
                        mvwprintw(menu_win, 5, 4, "Chamar: Carregar memoria de instrucao");
                        break;
                    case 2:
                        mvwprintw(menu_win, 5, 4, "Chamar: Carregar memoria de dados");
                        break;
                    case 3:
                        mvwprintw(menu_win, 5, 4, "Chamar: Imprimir instrucoes e dados");
                        break;
                    
                    case 4:
                        // CORREÇÃO 4: Para esconder o menu e a opção antes de abrir a janela flutuante
                        wclear(janela_op);
                        wclear(menu_win);
                        wrefresh(janela_op); // Força a tela a apagar
                        wrefresh(menu_win);  // Força a tela a apagar
                        
                        tela_registradores(janela_reg, registradores); 
                        break;
                        
                    case 5:
                        mvwprintw(menu_win, 5, 4, "Chamar: Imprimir todo simulador");
                        break;
                    case 6:
                        mvwprintw(menu_win, 5, 4, "Chamar: Salvar .asm e .dat");
                        break;
                    case 7:
                        mvwprintw(menu_win, 5, 4, "Chamar: Mostrar Estatisticas");
                        break;
                    case 8:
                        mvwprintw(menu_win, 5, 4, "Chamar: Executar programa (RUN)");
                        break;
                    case 9:
                        mvwprintw(menu_win, 5, 4, "Chamar: Executar um clock (STEP)");
                        break;
                    case 10: 
                        mvwprintw(menu_win, 5, 4, "Chamar: Voltar uma instrucao");
                        break;
                    default:
                        break;
                }

                if (escolha != 4) { 
                    wattron(menu_win, COLOR_PAIR(1) | A_BOLD);
                    mvwprintw(menu_win, 2, 4, ">> TELA DE EXECUCAO <<");
                    wattroff(menu_win, COLOR_PAIR(1) | A_BOLD);
                    
                    mvwprintw(menu_win, 12, 4, "Pressione qualquer tecla para voltar...");
                    wrefresh(menu_win);
                    wgetch(menu_win);
                }
            }
            wrefresh(janela_op); 
        }

    } while (escolha != 0); 

    endwin();
    return 0;
}

void tela_registradores(WINDOW *win, int regs[])
{
    wclear(win);

    wattron(win, COLOR_PAIR(5));
    box(win, 0, 0);

    mvwaddch(win, 2, 0, ACS_LTEE);  
    mvwhline(win, 2, 1, 0, 48);    
    mvwaddch(win, 2, 49, ACS_RTEE); 

    mvwaddch(win, 2, 25, ACS_TTEE);
    mvwvline(win, 3, 25, 0, 6);    
    mvwaddch(win, 9, 25, ACS_BTEE); 
    wattroff(win, COLOR_PAIR(5));

    wattron(win, COLOR_PAIR(2) | A_REVERSE | A_BOLD);
    mvwprintw(win, 1, 12, " BANCO DE REGISTRADORES MIPS ");
    wattroff(win, COLOR_PAIR(2) | A_REVERSE | A_BOLD);

    for (int i = 0; i < 4; i++) {
        wattron(win, COLOR_PAIR(1) | A_BOLD); 
        mvwprintw(win, 4 + i, 8, "R%d:", i);
        wattron(win, COLOR_PAIR(3));          
        mvwprintw(win, 4 + i, 16, "%d", regs[i]);

        wattron(win, COLOR_PAIR(1) | A_BOLD); 
        mvwprintw(win, 4 + i, 31, "R%d:", i + 4);
        wattron(win, COLOR_PAIR(3));          
        mvwprintw(win, 4 + i, 39, "%d", regs[i + 4]);
    }
    wrefresh(win);
    wgetch(win); 
    wclear(win);
    wrefresh(win);
}