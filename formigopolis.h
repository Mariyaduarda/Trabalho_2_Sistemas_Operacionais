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
#define NUM_PRIORIDADES 4   // Qtd. de niveis de prioridades
#define TAMANHO_MAX_FILA 32 // Num. maximo de itens por fila

//=================================================
// CLIENTES

// enum para conduzir a prioridade
typedef enum {
    GRAVIDA    = 0, // alta prioridade
    IDOSO      = 1, //
    DEFICIENTE = 2, //
    PADRAO     = 3  // pouca prioridade
} Prioridade;

typedef enum {
    MARIA = 0, MARCOS = 1, // casal gravidos
    VANDA = 2, VALTER = 3, // casal idoso
    PAULA = 4, PEDRO  = 5, // casal deficientes
    SUELI = 6, SILAS  = 7  // casal padrao
} ClienteId;

// representar cada pessoa
typedef struct {
    // dados do cliente
    int id_cliente;                // id do cliente
    char nome[MAX_NOME];           // nome do cliente
    
    // prioridade
    Prioridade prioridadeOriginal; // Prioridade antes de lidar com inanicao
    Prioridade prioridadeAtual;    // Prioridade atual, lidando com inanicao
    
    // dados da fila
    bool estaNaFila;               // Se ele esta na fila ou nao
    time_t timestamp_chegada;      // Hora que ele chegou
    int vezes_frustrado;           // Quantas vezes cortaram fila na frente dele
} Cliente;

//=================================================
// FILA DE PRIORIDADE
typedef struct {
    // matriz das filas
    //  - primeiro indice define qual fila de prioridade e'
    //  - segundo indice define  qual item dessa fila e'
    int fila[NUM_PRIORIDADES][TAMANHO_MAX_FILA];

    // dados da fila (e' circular)
    int inicio[NUM_PRIORIDADES]; // indice do comeco da fila
    int fim[NUM_PRIORIDADES];    // indice do fim da fila
    int contador_timestamp;      // contador global p/ ordem de chegada
    //  - fila vazia: ( inicio == fim )
    //  - fila cheia: ( fim+1 % TAM == inicio )
    //  - tamanho:    ( fim - inicio + TAM ) % TAM
    //  - proximo:    ( indice + 1 ) % TAM
} FilaPrioridade;

//=================================================
// MONITOR
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


#endif