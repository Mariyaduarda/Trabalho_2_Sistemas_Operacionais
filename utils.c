//
// utils.c p/ funções de output/print
//

#include "formigopolis.h"

// defini as cores ansi aqui em cima icto, caso queira mudar!!
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

// imprimir inicialização feita com sucesso
void print_monitor_inicializado() {
    printf(GREEN "Monitor foi iniciado.\n" RESET);
}

// imprimir finalização do monitor
void print_monitor_finalizado() {
    printf(GREEN "Monitor finalizado.\n" RESET);
}

// imprimir entrada de cliente na fila
void print_cliente_entrou_fila(int id_cliente, const char* nome) {
    printf(YELLOW "[FILA] " RESET "Cliente %s" BOLD " (ID: %d)" RESET " entrou na fila de atendimento\n",
           nome, id_cliente);
}

// imprimir cliente sendo atendido
void print_cliente_sendo_atendido(int id_cliente, const char* nome) {
    printf(GREEN "[CAIXA] " RESET "Cliente %s" BOLD " (ID: %d)" RESET " está sendo atendido no caixa 💳\n",
           nome, id_cliente);
}

// imprimir finalização do atendimento
void print_cliente_finalizou_atendimento(int id_cliente, const char* nome) {
    printf(CYAN "[SAÍDA] " RESET "Cliente %s" BOLD " (ID: %d)" RESET " finalizou o atendimento ✅\n",
           nome, id_cliente);
}

// imprimir aviso de deadlock
void print_aviso_deadlock() {
    printf(RED BOLD "[DEADLOCK] " RESET RED "Possível deadlock detectado!\n" RESET);
}

// imprimir cabeçalho dos clientes inicializados
void print_clientes_inicializados() {
    printf(BLUE "[SETUP] " RESET "Clientes inicializados:\n");
}
