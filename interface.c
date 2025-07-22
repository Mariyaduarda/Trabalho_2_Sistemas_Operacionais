#include "interface.h"

// pra quando o cliente entrou na fila
void printClienteEntrou(const char* nome) {
    printf("%s está na fila do caixa \n", nome);
}

// pra quando o cliente terminou tudo
void printClienteSaiu(const char* nome) {
    printf("%s vai para casa \n", nome);
}

// pra quando o cliente entra no mutex e sai da fila
void printClienteAtendido(const char* nome) {
    printf("%s está sendo atendido(a) \n", nome);
}

// quando o gerente detecta inanicao
void printGerenteInanicao() {
    printf("Gerente detectou inanição... \n");
}

// quando o gerente detecta deadlock
void printGerenteDeadlock() {
    printf("Gerente detectou deadlock... \n");
}
