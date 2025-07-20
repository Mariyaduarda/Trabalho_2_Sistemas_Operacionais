/*
monitor.c - implementação do caixa de atendimento

1 - monitor de sincronizacao
2 - sistema de prioridade (gravidas, idosos, etc)
3 - controle de casais - impedir atendimento concomitante
4 - deteccao de deadlock - verificar por tempo
5 - ordem de chegada - se da msm prioridade, atende por ordem de chegada
6 - prevenção de inanição com envelhecimento
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
int iteracoes_por_pessoa = 1; // número de vezes que cada pessoa vai ao caixa

// fila de prioridad ed
FilaPrioridade fila_atendimento;
pthread_t thread_gerente;
bool gerente_ativo = true;

// funções da fila de prioridade
void inicializar_fila() {
    fila_atendimento.tamanho = 0;
    fila_atendimento.contador_timestamp = 0;
}

void inserir_fila(int id_cliente) {
    if (fila_atendimento.tamanho >= MAX_CLIENTES * 10) {
        return; // fila cheia
    }

    ItemFila novo_item;
    novo_item.id_cliente = id_cliente;
    novo_item.prioridade = clientes[id_cliente].prioridade_atual;
    novo_item.timestamp = fila_atendimento.contador_timestamp++;
    novo_item.frustracoes = 0;

    // inserção mantendo ordem de prioridade (menor número = maior prioridade)
    // em caso de empate, ordem de chegada (timestamp menor primeiro)
    int pos = fila_atendimento.tamanho;

    // encontra posição correta para inserir
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
            // remove o item deslocando os demais
            for (int j = i; j < fila_atendimento.tamanho - 1; j++) {
                fila_atendimento.items[j] = fila_atendimento.items[j + 1];
            }
            fila_atendimento.tamanho--;
            break;
        }
    }
}

int obter_proximo_cliente() {
    // procura o primeiro cliente na fila que pode ser atendido
    for (int i = 0; i < fila_atendimento.tamanho; i++) {
        int id_cliente = fila_atendimento.items[i].id_cliente;

        // verifica se o cônjuge não está sendo atendido
        if (!monitor.casal_no_caixa[clientes[id_cliente].casal_id - 1]) {
            return id_cliente;
        }
    }
    return -1; // nenhum cliente pode ser atendido
}

bool fila_vazia() {
    return fila_atendimento.tamanho == 0;
}

//  detectar e tratar inanição
void verificar_inanicao() {
    for (int i = 0; i < fila_atendimento.tamanho; i++) {
        ItemFila *item = &fila_atendimento.items[i];

        // verifica se ha clientes com maior prioridade que chegaram depois
        bool tem_prioridade_maior_posterior = false;
        for (int j = 0; j < fila_atendimento.tamanho; j++) {
            if (j != i &&
                fila_atendimento.items[j].prioridade < item->prioridade &&
                fila_atendimento.items[j].timestamp > item->timestamp) {
                tem_prioridade_maior_posterior = true;
                break;
            }
        }

        if (tem_prioridade_maior_posterior) {
            item->frustracoes++;

            // a cada 2 frustrações, promove a prioridade (envelhecimento)
            if (item->frustracoes >= 2 && item->prioridade > 0) {
                printf("gerente detectou inanição, aumentando prioridade de %s\n",
                       clientes[item->id_cliente].nome);

                // promove a prioridade (diminui o número = maior prioridade)
                clientes[item->id_cliente].prioridade_atual--;
                item->prioridade = clientes[item->id_cliente].prioridade_atual;
                item->frustracoes = 0;

                // reordena a fila após mudança de prioridade
                ItemFila temp = *item;
                // remove o item da posição atual
                for (int k = i; k < fila_atendimento.tamanho - 1; k++) {
                    fila_atendimento.items[k] = fila_atendimento.items[k + 1];
                }
                fila_atendimento.tamanho--;

                // reinsere na posição correta
                int pos = fila_atendimento.tamanho;
                while (pos > 0 &&
                       (fila_atendimento.items[pos-1].prioridade > temp.prioridade ||
                        (fila_atendimento.items[pos-1].prioridade == temp.prioridade &&
                         fila_atendimento.items[pos-1].timestamp > temp.timestamp))) {
                    fila_atendimento.items[pos] = fila_atendimento.items[pos-1];
                    pos--;
                }
                fila_atendimento.items[pos] = temp;
                fila_atendimento.tamanho++;
                break; // sai do loop para evitar alterações durante reordenação
            }
        }
    }
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
    gerente_ativo = false;
    if (thread_gerente) {
        pthread_join(thread_gerente, NULL);
    }
    pthread_mutex_destroy(&monitor.mutex);
    pthread_cond_destroy(&monitor.cond_fila);
    print_monitor_finalizado();
}

/*
verificando espera infinita (deadlock)
segunda etapa: grávida > idoso > deficiente > grávida (ciclo)
-caixa vazio e alguém de cada categoria esperando
*/
bool verificar_deadlock_tempo() {
    // verifica se há pelo menos 3 categorias diferentes esperando
    int categorias_esperando = 0;
    bool categorias[4] = {false, false, false, false};

    for (int i = 0; i < fila_atendimento.tamanho; i++) {
        int id_cliente = fila_atendimento.items[i].id_cliente;
        int prioridade_original = clientes[id_cliente].prioridade_original; // usar prioridade original
        if (!categorias[prioridade_original]) {
            categorias[prioridade_original] = true;
            categorias_esperando++;
        }
    }

    // deadlock se caixa vazio + 3+ categorias esperando + ninguém pode ser atendido
    return !monitor.caixa &&
           categorias_esperando >= 3 &&
           !fila_vazia() &&
           obter_proximo_cliente() == -1;
}

void atualizar_categoria_esperando() {
    // reset das categorias
    for (int i = 0; i < 4; i++) {
        monitor.categoria_esperando[i] = false;
    }

    // verifica quais categorias ainda têm clientes na fila
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

        pthread_cond_wait(&monitor.cond_fila, &monitor.mutex);
    }

    if (!monitor.programa_ativo) {
        // programa finalizando, remove cliente da fila
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

    // verifica inanição após cada atendimento
    verificar_inanicao();

    // imprime cliente que está sendo atendido e libera a mutex
    print_cliente_sendo_atendido(id_cliente, clientes[id_cliente].nome);
    pthread_mutex_unlock(&monitor.mutex);
}

// finaliza atendimento do cliente que estava sendo atendido
void finalizar_atendimento(int id_cliente) {
    pthread_mutex_lock(&monitor.mutex);

    if (monitor.id_cliente_atendida == id_cliente) {
        print_cliente_finalizou_atendimento(id_cliente, clientes[id_cliente].nome);

        // restaura prioridade original se foi promovida por inanição
        if (clientes[id_cliente].prioridade_atual < clientes[id_cliente].prioridade_original) {
            clientes[id_cliente].prioridade_atual = clientes[id_cliente].prioridade_original;
        }

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

// thread do gerente para verificar deadlocks
void* thread_gerente_func(void* arg) {
    while (gerente_ativo && monitor.programa_ativo) {
        sleep(VERIFICA_DEADLOCK); // 5 segundos

        pthread_mutex_lock(&monitor.mutex);

        if (verificar_deadlock_tempo()) {
            print_aviso_deadlock();

            // escolhe aleatoriamente um cliente para liberar
            if (fila_atendimento.tamanho > 0) {
                int indice_aleatorio = rand() % fila_atendimento.tamanho;
                int cliente_liberado = fila_atendimento.items[indice_aleatorio].id_cliente;

                printf("gerente detectou deadlock, liberando %s para atendimento\n",
                       clientes[cliente_liberado].nome);

                // remove temporariamente as restrições de casal
                int casal_backup = monitor.casal_no_caixa[clientes[cliente_liberado].casal_id - 1];
                monitor.casal_no_caixa[clientes[cliente_liberado].casal_id - 1] = false;

                pthread_cond_broadcast(&monitor.cond_fila);

                // aguarda um pouco e restaura as restrições se necessário
                pthread_mutex_unlock(&monitor.mutex);
                usleep(100000); // 100ms
                pthread_mutex_lock(&monitor.mutex);

                // só restaura se não houver outro membro do casal sendo atendido
                if (monitor.id_cliente_atendida == -1 ||
                    clientes[monitor.id_cliente_atendida].casal_id != clientes[cliente_liberado].casal_id) {
                    monitor.casal_no_caixa[clientes[cliente_liberado].casal_id - 1] = casal_backup;
                }
            }
        }

        pthread_mutex_unlock(&monitor.mutex);
    }
    return NULL;
}

// simula o cliente
void* thread_cliente(void* arg) {
    int id_cliente = *((int*)arg);

    for (int iter = 0; iter < iteracoes_por_pessoa; iter++) {
        // simula tempo aleatório antes de chegar (3-5 segundos conforme especificação)
        sleep(rand() % 3 + 3);

        solicitar_atendimento(id_cliente, clientes[id_cliente].nome);

        // simula tempo de atendimento (1 segundo conforme especificação)
        sleep(1);

        // finalizar atendimento
        finalizar_atendimento(id_cliente);

        printf("%s vai para casa\n", clientes[id_cliente].nome);
    }

    return NULL;
}

void inicializar_clientes() {
    // segunda etapa: grávida > idoso > deficiente > grávida (pode causar deadlock)
    // prioridades: 0=grávida, 1=idoso, 2=deficiente, 3=padrão

    // casal idoso
    clientes[VANDA] = (Cliente){VANDA, "vanda", IDOSO, IDOSO, CASAL_IDOSOS, false};
    clientes[VALTER] = (Cliente){VALTER, "valter", IDOSO, IDOSO, CASAL_IDOSOS, false};

    // casal grávidos
    clientes[MARIA] = (Cliente){MARIA, "maria", GRAVIDA, GRAVIDA, CASAL_GRAVIDOS, false};
    clientes[MARCOS] = (Cliente){MARCOS, "marcos", GRAVIDA, GRAVIDA, CASAL_GRAVIDOS, false}; // pai com criança de colo

    // casal deficientes
    clientes[PAULA] = (Cliente){PAULA, "paula", DEFICIENTE, DEFICIENTE, CASAL_DEFICIENCTES, false};
    clientes[PEDRO] = (Cliente){PEDRO, "pedro", DEFICIENTE, DEFICIENTE, CASAL_DEFICIENCTES, false};

    // casal padrão
    clientes[SUELI] = (Cliente){SUELI, "sueli", PADRAO, PADRAO, CASAL_PADRAO, false};
    clientes[SILAS] = (Cliente){SILAS, "silas", PADRAO, PADRAO, CASAL_PADRAO, false};

    print_clientes_inicializados();

    for (int i = 0; i < MAX_CLIENTES; i++) {
        printf("- %s (prioridade: %d, casal: %d)\n",
               clientes[i].nome, clientes[i].prioridade_atual, clientes[i].casal_id);
    }
}

// inicializar sistema completo
void iniciar_sistema(int argc, char* argv[]) {
    // processa argumentos da linha de comando
    if (argc >= 2) {
        iteracoes_por_pessoa = atoi(argv[1]);
        if (iteracoes_por_pessoa <= 0) {
            iteracoes_por_pessoa = 1;
        }
    }

    srand(time(NULL)); // inicializa gerador de números aleatórios

    inicializar_monitor();
    inicializar_clientes();

    // cria thread do gerente
    pthread_create(&thread_gerente, NULL, thread_gerente_func, NULL);

    // cria threads dos clientes
    pthread_t threads[MAX_CLIENTES];
    int ids[MAX_CLIENTES];

    for (int i = 0; i < MAX_CLIENTES; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_cliente, &ids[i]);
    }

    // aguarda todas as threads terminarem
    for (int i = 0; i < MAX_CLIENTES; i++) {
        pthread_join(threads[i], NULL);
    }

    destruir_monitor();
}

// finalizar o programa
void finalizar_programa() {
    pthread_mutex_lock(&monitor.mutex);
    monitor.programa_ativo = false; // programa finalizando, para atendimento
    gerente_ativo = false;
    pthread_cond_broadcast(&monitor.cond_fila); // libera threads presas
    pthread_mutex_unlock(&monitor.mutex); // sai da seção crítica
}