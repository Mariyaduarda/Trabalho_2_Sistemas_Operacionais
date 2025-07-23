#include <stdio.h> // inclui biblioteca padrão de entrada e saída
#include <stdlib.h> // inclui biblioteca padrão para funções utilitárias
#include <pthread.h> // inclui biblioteca para threads

#include "formigopolis.c" // inclui o arquivo do projeto fromigopolis
#include "formigopolis.h" // inclui cabeçalho do projeto
#include "interface.h" // inclui cabeçalho do projeto
#include "monitor.c"     // inclui a biblioteca do monitor

void inicializar_monitor(); // declara função para inicializar monitor
void destruir_monitor(); // declara função para destruir monitor
void inicializar_clientes(); // declara função para inicializar clientes
void* thread_cliente(void* arg); // declara função da thread do cliente
void* thread_gerente_func(void* arg); // declara função da thread do gerente

//extern externaliza uma variavel global para outros arquivos
extern int iteracoes_por_pessoa; // número de vezes que cada pessoa vai ao caixa
extern pthread_t thread_gerente; // thread do gerente
extern bool gerente_ativo; // flag para controle do gerente

int main (int argc, char* argv[]) {

    if (argc != 2) { // verifica se o número de argumentos está correto
        printf("Uso: %s <num_iteracoes>\n", argv[0]); // exibe mensagem de uso
        return 1; // retorna erro
    }

    iteracoes_por_pessoa = atoi(argv[1]); // converte argumento para inteiro
    if (iteracoes_por_pessoa <= 0) { // verifica se o número de iterações é válido
        printf("Número de iterações deve ser maior que zero.\n"); // exibe erro
        return 1; // retorna erro
    }

    inicializar_monitor(); // inicializa monitor
    inicializar_clientes(); // inicializa clientes

    pthread_t threads_clientes[MAX_CLIENTES]; // declara array de threads dos clientes

    // cria threads dos clientes
    for (int i = 0; i < MAX_CLIENTES;i++){
        int* id = malloc(sizeof(int)); // aloca memória para id do cliente
        *id = i; // define id do cliente
        pthread_create(&threads_clientes[i], NULL, thread_cliente, id); // cria thread do cliente
    }
    // cria thread do gerente
    pthread_create(&thread_gerente, NULL, thread_gerente_func, NULL); // cria thread do gerente
    // aguarda as threads dos clientes terminarem
    for (int i = 0; i < MAX_CLIENTES; i++) {
        pthread_join(threads_clientes[i], NULL); // espera thread do cliente terminar
        free(threads_clientes[i]); // libera memória (erro: deveria liberar 'id', não o array)
    }

    //sinaliza p/ o gerente encerrar
    gerente_ativo = false; // sinaliza para o gerente encerrar
    pthread_join(thread_gerente, NULL); // espera thread do gerente terminar

    destruir_monitor(); // destrói monitor

    return 0; // retorna sucesso
}
