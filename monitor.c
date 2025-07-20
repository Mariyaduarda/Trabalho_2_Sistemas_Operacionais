/*
monitor.c - implementação do caixa de atendimento

1 - monitor de sincronizacao
2 - sistema de prioridade (gravidas, idosos, etc)
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

// Estrutura da fila de prioridade
typedef struct {
    int id_cliente;
    int prioridade;
    int timestamp; // para ordem de chegada (FIFO dentro da mesma prioridade)
} ItemFila;

typedef struct {
    ItemFila items[MAX_CLIENTES * 2]; // buffer maior para segurança
    int tamanho;
    int contador_timestamp; // contador global para timestamps
} FilaPrioridade;

FilaPrioridade fila_atendimento;

// Funções da fila de prioridade
void inicializar_fila() {
    fila_atendimento.tamanho = 0;
    fila_atendimento.contador_timestamp = 0;
}

void inserir_fila(int id_cliente) {
    if (fila_atendimento.tamanho >= MAX_CLIENTES * 2) {
        return; // fila cheia
    }

    ItemFila novo_item;
    novo_item.id_cliente = id_cliente;
    novo_item.prioridade = clientes[id_cliente].prioridade_atual;
    novo_item.timestamp = fila_atendimento.contador_timestamp++;

    // Inserção mantendo ordem de prioridade (menor número = maior prioridade)
    // Em caso de empate, ordem de chegada (timestamp menor primeiro)
    int pos = fila_atendimento.tamanho;

    // Encontra posição correta para inserir
    while (pos > 0 &&
           (fila_atendimento.items[pos-1].prioridade > novo_item.prioridade ||
            (fila_atendimento.items[pos-1].prioridade == novo_item.prioridade &&
             fila_atendimento.items[pos-1].timestamp > novo_item.timestamp))) {
        fila_atendimento.items[pos] = fila_atendimento.items[pos-1];
        pos--;
    }

    fila_atendimento.items[pos] = novo_item;
    fila_atendimento.tamanho++;
}

void remover_cliente_fila(int id_cliente) {
    for (int i = 0; i < fila_atendimento.tamanho; i++) {
        if (fila_atendimento.items[i].id_cliente == id_cliente) {
            // Remove o item deslocando os demais
            for (int j = i; j < fila_atendimento.tamanho - 1; j++) {
                fila_atendimento.items[j] = fila_atendimento.items[j + 1];
            }
            fila_atendimento.tamanho--;
            break;
        }
    }
}

int obter_proximo_cliente() {
    // Procura o primeiro cliente na fila que pode ser atendido
    for (int i = 0; i < fila_atendimento.tamanho; i++) {
        int id_cliente = fila_atendimento.items[i].id_cliente;

        // Verifica se o cônjuge não está sendo atendido
        if (!monitor.casal_no_caixa[clientes[id_cliente].casal_id - 1]) {
            return id_cliente;
        }
    }
    return -1; // nenhum cliente pode ser atendido
}

bool fila_vazia() {
    return fila_atendimento.tamanho == 0;
}

// configurar o sistema de sincronizacao
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

    // inicializar fila de prioridade
    inicializar_fila();

    print_monitor_inicializado();
}

// destroir o monitor
void destruir_monitor() {
    pthread_mutex_destroy(&monitor.mutex);
    pthread_cond_destroy(&monitor.cond_fila);
    print_monitor_finalizado();
}

/*
verificando espera infinita (deadlock)
-3 ou mais categorias de clientes estão esperando simultaneamente
-conjuges bloqueiam uns aos outros
-ninguém consegue avançar na fila
*/
bool verificar_deadlock_tempo() {
    int categorias_esperando = 0;
    for (int i = 0; i < 4; i++) {
        if (monitor.categoria_esperando[i]) {
            categorias_esperando++;
        }
    }

    // Deadlock se 3+ categorias esperando OU se a fila não está vazia mas ninguém pode ser atendido
    return categorias_esperando >= 3 ||
           (!fila_vazia() && obter_proximo_cliente() == -1);
}

void atualizar_categoria_esperando() {
    // Reset das categorias
    for (int i = 0; i < 4; i++) {
        monitor.categoria_esperando[i] = false;
    }

    // Verifica quais categorias ainda têm clientes na fila
    for (int i = 0; i < fila_atendimento.tamanho; i++) {
        int id_cliente = fila_atendimento.items[i].id_cliente;
        int prioridade = clientes[id_cliente].prioridade_atual;
        monitor.categoria_esperando[prioridade] = true;
    }
}

// cliente entra na fila e espera ser atendido
void solicitar_atendimento(int id_cliente, const char* nome) {
    pthread_mutex_lock(&monitor.mutex);

    // marca cliente na fila
    clientes[id_cliente].esta_na_fila = true;

    // adiciona cliente à fila de prioridade
    inserir_fila(id_cliente);

    // atualiza categorias esperando
    atualizar_categoria_esperando();

    print_cliente_entrou_fila(id_cliente, nome);

    // esperar enquanto não pode ser atendido
    while (monitor.programa_ativo &&
           (monitor.caixa || // caixa ocupado
            monitor.casal_no_caixa[clientes[id_cliente].casal_id - 1] || // cônjuge sendo atendido
            obter_proximo_cliente() != id_cliente)) { // não é o próximo na fila

        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += VERIFICA_DEADLOCK; // verifica deadlock const, add 5segundos

        int resultado = pthread_cond_timedwait(&monitor.cond_fila, &monitor.mutex, &timeout);

        if (resultado == ETIMEDOUT && verificar_deadlock_tempo()) {
            print_aviso_deadlock();
            // Em caso de deadlock, remove o cliente da fila e sai
            remover_cliente_fila(id_cliente);
            clientes[id_cliente].esta_na_fila = false;
            atualizar_categoria_esperando();
            pthread_mutex_unlock(&monitor.mutex);
            return;
        }
    }

    if (!monitor.programa_ativo) {
        // Programa finalizando, remove cliente da fila
        remover_cliente_fila(id_cliente);
        clientes[id_cliente].esta_na_fila = false;
        pthread_mutex_unlock(&monitor.mutex);
        return;
    }

    // cliente sendo atendido
    monitor.caixa = true; // marca que o caixa está ocupado
    monitor.id_cliente_atendida = id_cliente; // diz quem está sendo atendido

    // se o cliente for de casal, marca o id do casal
    // e impede que ambos sejam atendidos concomitantemente
    monitor.casal_no_caixa[clientes[id_cliente].casal_id - 1] = true;

    // remove cliente da fila de prioridade
    remover_cliente_fila(id_cliente);
    clientes[id_cliente].esta_na_fila = false;

    // atualiza categorias esperando
    atualizar_categoria_esperando();

    // imprime cliente que está sendo atendido e libera a mutex
    print_cliente_sendo_atendido(id_cliente, clientes[id_cliente].nome);
    pthread_mutex_unlock(&monitor.mutex);
}

// finaliza atendimento do cliente que estava sendo atendido
void finalizar_atendimento(int id_cliente) {
    pthread_mutex_lock(&monitor.mutex);

    if (monitor.id_cliente_atendida == id_cliente) {
        print_cliente_finalizou_atendimento(id_cliente, clientes[id_cliente].nome);

        // liberar caixa
        monitor.caixa = false;
        monitor.id_cliente_atendida = -1;
        monitor.casal_no_caixa[clientes[id_cliente].casal_id - 1] = false;

        // notificar todos os clientes que estão no aguardo
        pthread_cond_broadcast(&monitor.cond_fila);
    }
    // liberar mutex
    pthread_mutex_unlock(&monitor.mutex);
}

// simula o cliente
void* thread_cliente(void* arg) {
    int id_cliente = *((int*)arg);

    // simula tempo aleatório antes de chegar
    sleep(rand() % 3 + 1);

    solicitar_atendimento(id_cliente, clientes[id_cliente].nome);

    // simula tempo de atendimento
    sleep(rand() % 4 + 2);

    // finalizar atendimento
    finalizar_atendimento(id_cliente);

    return NULL;
}

void inicializar_clientes() {
    // casal idoso
    clientes[VANDA] = (Cliente){VANDA, "VANDA", IDOSO, CASAL_IDOSOS, false};
    clientes[VALTER] = (Cliente){VALTER, "VALTER", IDOSO, CASAL_IDOSOS, false};

    // casal grávidos
    clientes[MARIA] = (Cliente){MARIA, "MARIA", GRAVIDA, CASAL_GRAVIDOS, false};
    clientes[MARCOS] = (Cliente){MARCOS, "MARCOS", PADRAO, CASAL_GRAVIDOS, false};

    // casal limitantes físicos
    clientes[PAULA] = (Cliente){PAULA, "PAULA", DEFICIENTE, CASAL_DEFICIENCTES, false};
    clientes[PEDRO] = (Cliente){PEDRO, "PEDRO", DEFICIENTE, CASAL_DEFICIENCTES, false};

    // casal padrão
    clientes[SUELI] = (Cliente){SUELI, "SUELI", PADRAO, CASAL_PADRAO, false};
    clientes[SILAS] = (Cliente){SILAS, "SILAS", PADRAO, CASAL_PADRAO, false};

    print_clientes_inicializados();

    for (int i = 0; i < MAX_CLIENTES; i++) {
        printf("- %s (Prioridade: %d, Casal: %d)\n",
               clientes[i].nome, clientes[i].prioridade_atual, clientes[i].casal_id);
    }
}

// finalizar o programa
void finalizar_programa() {
    pthread_mutex_lock(&monitor.mutex);
    monitor.programa_ativo = false; // programa finalizando, para atendimento
    pthread_cond_broadcast(&monitor.cond_fila); // libera threads presas
    pthread_mutex_unlock(&monitor.mutex); // sai da seção crítica
}