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
int filaInsere(FilaPrioridade* fila, int prioridade, int id_cliente) {
    // se a fila ta' cheia, n tem como inserir
    if (isFilaCheia(fila, prioridade)) return 0;

    // faz a insercao em si
    int pos = fila->fim[prioridade];
    fila->fila[prioridade][pos].id_cliente = id_cliente;
    fila->fila[prioridade][pos].timestamp_chegada = fila->contador_timestamp++;
    fila->fila[prioridade][pos].frustracoes = 0;

    // atualiza o atributo fim
    fila->fim[prioridade] = (pos + 1) % TAMANHO_MAX_FILA;
    return 1; // Sucesso
}

// Remove o primeiro cliente da fila de determinada prioridade
int filaRemove(FilaPrioridade* fila, int prioridade, ItemFila* removido) {
    // se a fila ta' vazia, n tem como remover
    if (isFilaVazia(fila, prioridade)) return 0;

    // faz a remocao em si
    int pos = fila->inicio[prioridade];
    *removido = fila->fila[prioridade][pos];

    // atualiza o atributo inicio
    fila->inicio[prioridade] = (pos + 1) % TAMANHO_MAX_FILA;
    return 1; // Sucesso
}