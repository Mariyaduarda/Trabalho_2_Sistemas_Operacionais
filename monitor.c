/*
monitor.c - implementação do caixa de atendimento

1 - monitor de sincronizaccao
2 - sistema de prioridade(gravidas, idosod, etc)
3 - controle de casais - impedir atendimento concomitante
4 - deteccao de deadlock - verificar por tempo
5 - ordem de chegada - se da msm prioridade, atende por ordem de chegada
*/

#include "formigopolis.h"

// declarando as funções de print do utils.c
void print_monitor_inicializado();
void print_monitor_finalizado();
void print_cliente_entrou_fila(int id_cliente, const char* nome);
void print_cliente_sendo_atendido(int id_cliente, const char* nome);
void print_cliente_finalizou_atendimento(int id_cliente, const char* nome);
void print_aviso_deadlock();
void print_clientes_inicializados();

// monitor e cliente = var. globais
Monitor_do_Caixa monitor;
Cliente clientes[MAX_CLIENTES];

// configurar o sitema de sincronizacao
void inicializar_monitor() {
    // inicializa mutex e variavel de condicao
    pthread_mutex_init(&monitor.mutex, NULL);
    pthread_cond_init(&monitor.cond_fila, NULL);

    // define estados iniciais do caixa
    monitor.caixa = false;
    monitor.id_cliente_atendida = -1;
    monitor.tempo_global = 0;
    monitor.programa_ativo = true;

    // init array de controle
    for (int i = 0; i < 4; i++) {
        monitor.categoria_esperando[i] = false;
        monitor.casal_no_caixa[i] = false;
    }
    print_monitor_inicializado();
}

// destroir o monitor
void destruir_monitor() {
    pthread_mutex_destroy(&monitor.mutex);
    pthread_cond_destroy(&monitor.cond_fila);
    print_monitor_finalizado();
}

/*
verificando espera infinita
-3 ou mais categorias de clientes estão esperando simultaneamente
-conjuge bloqueiam uns aos outros
-ngm consegue avançar na fila
*/
bool verificar_deadlock_tempo() {
    int categoria_esperando = 0;
    for (int i = 0; i < 4; i++) {
        if (monitor.categoria_esperando[i]) {
            categoria_esperando++;
        }
    }
    return categoria_esperando >= 3; // pode ocorrer caso +3 cat. estejam no aguardo da vrz
}

// cliente libera o caixa apos atendido
int encontrar_prox_cliente() {
    int prox = -1;
    int menor_prioridade = 4;
    int menor_timestamp = INT_MAX;

    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (clientes[i].esta_na_fila) {
            bool casal_no_caixa = monitor.casal_no_caixa[clientes[i].casal_id - 1];

            if (!casal_no_caixa) {
                if (clientes[i].prioridade_atual < menor_prioridade ||
                    (clientes[i].prioridade_atual == menor_prioridade &&
                     clientes[i].id_cliente < menor_timestamp)) {

                    menor_prioridade = clientes[i].prioridade_atual;
                    menor_timestamp = clientes[i].id_cliente;
                    prox = i;
                }
            }
        }
    }
    return prox;
}

// cliente entra na fila e espera ser atendido
void solicitar_atendimento(int id_cliente, const char* nome) {
    pthread_mutex_lock(&monitor.mutex);

    // marca cliente à fila
    clientes[id_cliente].esta_na_fila = true;
    monitor.categoria_esperando[clientes[id_cliente].prioridade_atual] = true;

    print_cliente_entrou_fila(id_cliente, nome);

    // esperar enquanto n pd ser atendido
    while (monitor.programa_ativo && // caixa ocupado
           (monitor.caixa ||
            monitor.casal_no_caixa[clientes[id_cliente].casal_id - 1] || // conjuge sendo attendido
            encontrar_prox_cliente() != id_cliente)) { // n é o proximo na fila

        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += VERIFICA_DEADLOCK; // verifica dealock const, add 5segundos (formigopoli.h)

        int resultado = pthread_cond_timedwait(&monitor.cond_fila, &monitor.mutex, &timeout);

        if (resultado == ETIMEDOUT && verificar_deadlock_tempo()) {
            print_aviso_deadlock();
            break;
        }
    }

    if (!monitor.programa_ativo) {
        pthread_mutex_unlock(&monitor.mutex);
        return;
    }

    // cliente sendo atendido
    monitor.caixa = true; // marca que o caixa ta ocupado
    monitor.id_cliente_atendida = id_cliente; // diz qm ta sendo atendido
    // se o cliente for casal, marca o id do casal
    // e impede que ambos sejam atendido concomitantemente
    monitor.casal_no_caixa[clientes[id_cliente].casal_id - 1] = true;
    clientes[id_cliente].esta_na_fila = false; // remove o cliente da fila

    // existe algm da mesma categoria?
    bool ainda_tem_categoria = false;
    // pecorro os cleitnes, ignorando o que ta sendo atendido
    for (int i = 0; i < MAX_CLIENTES; i++) {
        if (i != id_cliente && clientes[i].esta_na_fila &&
            clientes[i].prioridade_atual == clientes[id_cliente].prioridade_atual) { // verifica se é da msm categoria
            ainda_tem_categoria = true; // se for da msm categoria devolve true e sai do for
            break;
        }
    }
    // se x categoria nao tem cliente marca que n tem ngm mais esperando
    if (!ainda_tem_categoria) {
        monitor.categoria_esperando[clientes[id_cliente].prioridade_atual] = false;
    }
    // imprime cliente q ta sendo atendido e libera a mutex
    print_cliente_sendo_atendido(id_cliente, clientes[id_cliente].nome);
    pthread_mutex_unlock(&monitor.mutex);
}

// finaliza atendimento do cliente q tava sendo atendido
void finalizar_atendimento(int id_cliente) {
    pthread_mutex_lock(&monitor.mutex);

    if (monitor.id_cliente_atendida == id_cliente) {
        print_cliente_finalizou_atendimento(id_cliente, clientes[id_cliente].nome);

        //liberar caixa
        monitor.caixa = false;
        monitor.id_cliente_atendida = -1;
        monitor.casal_no_caixa[clientes[id_cliente].casal_id - 1] = false;

        // notificar todos os clientes q estao no aguardo
        pthread_cond_broadcast(&monitor.cond_fila);
    }
    // liberar mutex
    pthread_mutex_unlock(&monitor.mutex);
}

// simula o cliente
void* thread_cliente(void* arg) {
    int id_cliente = *((int*)arg);

    // simula tempo andom antes de chegar
    sleep(rand() % 3 + 1);

    solicitar_atendimento(id_cliente, clientes[id_cliente].nome);

    // simula tempo de atendimento
    sleep(rand() % 4 + 2);

    // finalziar atendimento
    finalizar_atendimento(id_cliente);

    return NULL;
}

void inicializar_clientes() {
    // casal idoso
    clientes[VANDA] = (Cliente){VANDA, "VANDA", IDOSO, CASAL_IDOSOS, false};
    clientes[VALTER] = (Cliente){VALTER, "VALTER", IDOSO, CASAL_IDOSOS, false};

    // casal gravidos
    clientes[MARIA] = (Cliente){MARIA, "MARIA", GRAVIDA, CASAL_GRAVIDOS, false};
    clientes[MARCOS] = (Cliente){MARCOS, "MARCOS", PADRAO, CASAL_GRAVIDOS, false};

    // casal limitantes fisicos
    clientes[PAULA] = (Cliente){PAULA, "PAULA", DEFICIENTE, CASAL_DEFICIENCTES, false};
    clientes[PEDRO] = (Cliente){PEDRO, "PEDRO", DEFICIENTE, CASAL_DEFICIENCTES, false};

    // casal padrao
    clientes[SUELI] = (Cliente){SUELI, "SUELI", PADRAO, CASAL_PADRAO, false};
    clientes[SILAS] = (Cliente){SILAS, "SILAS", PADRAO, CASAL_PADRAO, false};

    print_clientes_inicializados();

    for (int i = 0; i < MAX_CLIENTES; i++) {
        printf("- %s (Prioridade: %d, Casal: %d)\n",
               clientes[i].nome, clientes[i].prioridade_atual, clientes[i].casal_id);
    }
}

//finalizar o programa
void finalizar_programa() {
    pthread_mutex_lock(&monitor.mutex);
    monitor.programa_ativo = false; // programa finalizando, para atendimento
    pthread_cond_broadcast(&monitor.cond_fila); //libera threads presa
    pthread_mutex_unlock(&monitor.mutex); // sai da secao critica
}