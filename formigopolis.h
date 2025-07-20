//
// header do formigopolis .c
//

#ifndef FORMIGOPOLIS_H
#define FORMIGOPOLIS_H

// bibliotecas utilizadas na simulação
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// constantes do programa
#define MAX_CLIENTES 8
#define MAX_NOME 6
#define VERIFICA_DEADLOCK 5

// enum para conduzir a prioridade
typedef enum {
    GRAVIDA    = 0,  // alta prioridade
    IDOSO      = 1,
    DEFICIENTE = 2,
    PADRAO     = 3  // pouca prioridade
} Prioridade;

// declarar casais p/ nao serem atendidos juntos
// entre conjuges atende-se em ordem de chegada
typedef enum {
    CASAL_IDOSOS       = 1,
    CASAL_GRAVIDOS     = 2,
    CASAL_DEFICIENCTES = 3,
    CASAL_PADRAO       = 4,
} Casal;

typedef enum {
    VANDA = 0, VALTER = 1, // casal idoso
    MARIA = 2, MARCOS = 3, // casal gravidos
    PAULA = 4, PEDRO  = 5, // casal deficientes
    SUELI = 6, SILAS  = 7  // casal padrao
} ClienteId;

// representar cada pessoa
typedef struct {
    int id_cliente;
    char nome[MAX_NOME];
    Prioridade prioridade_atual;
    Casal casal_id;
    bool esta_na_fila;
} Cliente;

// monitor do caixa - estrutura principal de sincronização
typedef struct {
    // mutex e variaveis de cond
    pthread_mutex_t mutex;    // mutex principal
    pthread_cond_t cond_fila;// cond p/ pessoas esperando na fila

    bool caixa;               // se o caixa esta ocupado
    int id_cliente_atendida; // id da pessoa sendo atendia

    // controle de prioridade p/ deadlock
    bool categoria_esperando[4]; // se algm de cada categoria esta esperando
    int tempo_global;           // contador global p/ ordem de chegada

    // controle de casai - p/ nao serem atendidos juntos
    bool casal_no_caixa[4]; //se algm dos casais esta sendo atendido

    // controle de execucao
    bool programa_ativo; // se o programa ainda estiver rodando
} Monitor_do_Caixa;


// FUNCOES DE PRINT
void print_monitor_inicializado();
void print_monitor_finalizado();
void print_cliente_entrou_fila(int id_cliente, const char* nome);
void print_cliente_sendo_atendido(int id_cliente, const char* nome);
void print_cliente_finalizou_atendimento(int id_cliente, const char* nome);
void print_aviso_deadlock();
void print_clientes_inicializados();
void print_cliente_info(const char* nome, int prioridade, int casal);
void print_cabecalho_sistema();
void print_iniciando_atendimento();
void print_atendimento_finalizado();
void print_erro_thread(int id_cliente);

#endif