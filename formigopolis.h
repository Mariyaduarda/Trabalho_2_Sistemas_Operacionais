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
#define MAX_CLIENTES 8      // Num. maximo de clientes que o sistema suporta
#define MAX_NOME 16         // Tamanho do nome um pouco maior, pra n causar problemas
#define VERIFICA_DEADLOCK 5 // 

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
    int id_cliente;              // id do cliente
    char nome[MAX_NOME];         // nome do cliente
    int prioridade_original;     // Prioridade antes de lidar com inanicao
    Prioridade prioridade_atual; // Prioridade atual, lidando com inanicao
    Casal casal_id;              // ID do casal que o cliente faz parte
    bool esta_na_fila;           // Se ele esta na fila ou nao
    time_t timestamp_chegada;    // Hora que ele chegou
    int vezes_frustrado;         // Quantas vezes cortaram fila na frente dele
} Cliente;

typedef struct {
    int timestamp;   // para ordem de chegada (fifo dentro da mesma prioridade)
    int frustracoes; // contador de frustrações para inanição
} ItemFila;

typedef struct {
    ItemFila items[MAX_CLIENTES * 10]; // buffer maior para múltiplas iterações
    int tamanho;
    int contador_timestamp; // contador global para timestamps
} FilaPrioridade;

// monitor do caixa - estrutura principal de sincronização
typedef struct {
    // mutex e variaveis de cond
    pthread_mutex_t mutex;       // mutex principal
    pthread_cond_t cond_fila;    // cond p/ pessoas esperando na fila

    bool caixa;                  // se o caixa esta ocupado
    int id_cliente_atendida;     // id da pessoa sendo atendia

    // controle de prioridade p/ deadlock
    bool categoria_esperando[4]; // se algm de cada categoria esta esperando
    int tempo_global;            // contador global p/ ordem de chegada

    // controle de casai - p/ nao serem atendidos juntos
    bool casal_no_caixa[4];      //se algm dos casais esta sendo atendido

    // controle de execucao
    bool programa_ativo;         // se o programa ainda estiver rodando
} Monitor_do_Caixa;


// FUNCOES DE PRINT
void print_monitor_inicializado();
void print_monitor_finalizado();
void print_cliente_entrou_fila(int id_cliente, const char* nome);
void print_cliente_sendo_atendido(int id_cliente, const char* nome);
void print_cliente_finalizou_atendimento(int id_cliente, const char* nome);
void print_aviso_deadlock();
void print_clientes_inicializados();


#endif