//
// Created by alunos on 19/07/2025.
//

#include "formigopolis.h"

//=================================================
// FILA DE PRIORIDADE

// Inicializa a fila
void filaInicializa(FilaPrioridade* fila) {
    for (int i = 0; i < NUM_PRIORIDADES; i++) {
        fila->inicio[i] = 0;
        fila->fim[i] = 0;
    }
    fila->contador_timestamp = 0;
}

// Verifica se uma fila de certa prioridade está vazia
int filaVazia(FilaPrioridade* fila, int prioridade) {
    return fila->inicio[prioridade] == fila->fim[prioridade];
}

// Verifica se uma fila de certa prioridade está cheia
int filaCheia(FilaPrioridade* fila, int prioridade) {
    return (fila->fim[prioridade] + 1) % TAMANHO_MAX_FILA == fila->inicio[prioridade];
}

// Retorna o tamanho atual da fila de uma certa prioridade
int filaTamanho(FilaPrioridade* fila, int prioridade) {
    return (fila->fim[prioridade] - fila->inicio[prioridade] + TAMANHO_MAX_FILA) % TAMANHO_MAX_FILA;
}

// Adiciona um cliente na fila de determinada prioridade
int filaInsere(FilaPrioridade* fila, int prioridade, int id_cliente, Cliente clientes[]) {
    if (filaCheia(fila, prioridade)) return 0;

    int pos = fila->fim[prioridade];
    fila->fila[prioridade][pos] = id_cliente;
    fila->fim[prioridade] = (pos + 1) % TAMANHO_MAX_FILA;

    clientes[id_cliente].timestamp_chegada = fila->contador_timestamp++;
    clientes[id_cliente].vezes_frustrado = 0;

    return 1; // Sucesso
}

// Remove o primeiro client e da fila de determinada prioridade
int filaRemove(FilaPrioridade* fila, int prioridade, int* id_removido, Cliente clientes[]) {
    if (filaVazia(fila, prioridade)) return 0;

    int pos = fila->inicio[prioridade];
    *id_removido = fila->fila[prioridade][pos];
    fila->inicio[prioridade] = (pos + 1) % TAMANHO_MAX_FILA;

    // Atualiza o cliente, se necessário
    clientes[*id_removido].vezes_frustrado++; // ou outra lógica desejada

    return 1; // Sucesso
}


//=================================================
// CLIENTES

// Inicializa os clientes
void clienteInit(Cliente clientes[]) {
    
    //define os nomes de tds os clientes
    char* nomes[MAX_CLIENTES] = {
        "Maria", "Marcos",    // casal gravidos
        "Vanda", "Valter",    // casal idosos
        "Paula", "Pedro",     // casal deficientes
        "Sueli", "Silas"      // casal padrao
    };

    //define a prioridade de tds os clientes
    Prioridade prioridades[MAX_CLIENTES] = {
        GRAVIDA, GRAVIDA,
        IDOSO, IDOSO,
        DEFICIENTE, DEFICIENTE,
        PADRAO, PADRAO
    };

    for (int i = 0; i < MAX_CLIENTES; i++) {
        clientes[i].id_cliente = i; // define o id do cliente

        //atribui o nome ao struct
        strncpy(clientes[i].nome, nomes[i], MAX_NOME);
        //coloca o EOF no final do nome, só pra ter ctz
        clientes[i].nome[MAX_NOME - 1] = '\0';

        //atribui a prioridade ao struct
        clientes[i].prioridadeOriginal = prioridades[i];
        clientes[i].prioridadeAtual = prioridades[i];

        // atribui os valores que sao pra tds os clientes
        clientes[i].estaNaFila = false;
        clientes[i].timestamp_chegada = 0;
        clientes[i].vezes_frustrado = 0;
    }
}