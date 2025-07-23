#include <stdio.h> // inclui biblioteca padrão de entrada e saída
#include <stdlib.h> // inclui biblioteca padrão para funções utilitárias
#include <pthread.h> // inclui biblioteca para threads
#include <stdbool.h> // para usar o tipo bool
#include "formigopolis.h" // inclui cabeçalho do projeto

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
    int* ids[MAX_CLIENTES]; // array para guardar os ponteiros id

    // cria threads dos clientes
    for (int i = 0; i < MAX_CLIENTES; i++) {
        ids[i] = malloc(sizeof(int));
        if (!ids[i]) {
            fprintf(stderr, "erro ao alocar memoria para id\n");
            exit(1);
        }
        *ids[i] = i;
        if (pthread_create(&threads_clientes[i], NULL, thread_cliente, ids[i]) != 0) {
            fprintf(stderr, "erro ao criar thread do cliente %d\n", i);
            exit(1);
        }
    }

    // cria thread do gerente
    if (pthread_create(&thread_gerente, NULL, thread_gerente_func, NULL) != 0) {
        fprintf(stderr, "erro ao criar thread do gerente\n");
        exit(1);
    }

    // aguarda as threads dos clientes terminarem
    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (pthread_join(threads_clientes[i], NULL) != 0) {
            fprintf(stderr, "erro ao esperar thread do cliente %d\n", i);
        }
        free(ids[i]);
    }

    //sinaliza p/ o gerente encerrar
    gerente_ativo = false; // sinaliza para o gerente encerrar
    pthread_join(thread_gerente, NULL); // espera thread do gerente terminar

    destruir_monitor(); // destrói monitor

    return 0; // retorna sucesso
}
