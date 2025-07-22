#ifndef INTERFACE_H
#define INTERFACE_H

// bibliotecas necessárias
#include <stdio.h>

//==========================
// Prints do Cliente
void printClienteEntrou(const char* nome);
void printClienteSaiu(const char* nome);
void printClienteAtendido(const char* nome);

//==========================
// Prints do Gerente
void printGerenteInanicao();
void printGerenteDeadlock();

#endif
