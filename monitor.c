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
void print_cliente_info(const char* nome, int prioridade, int casal);
void print_cabecalho_sistema();
void print_iniciando_atendimento();
void print_atendimento_finalizado();
void print_erro_thread(int id_cliente);

// monitor e cliente = var. globais
Monitor_do_Caixa monitor;
Clientes clientes[MAX_CLIENTES];

// configurar o sitema de sincronizacao
void inicializar_monitor(){
    // inicializa mutex e variavel de condicao
    phthread_mutex_init(&monitor.mutex,NULL);
    pthread_cond_init(&cond_fila,NULL);

    // define estados iniciais do caixa
    monitor.caixa = false;
    monitor.id_cliente_atendida = -1;
    monitor.tempo_global = 0;
    monitor.programa_ativo = false;

    // init array de controle
    for (int i = 0; i < 4; i++) {
        monitor.categoria_esperando[i] = false;
        monitor.casal_no_caixa[i] = false;
    }
    print_monitor_inicializado();
}

// destroir o monitor
void destruir_monitor(){
    pthread_mutex_destroy(&monitor.mutex);
    pthread_cond_destroy(&monitor.cond_fila);
    print_monitor_finalizado();
}

// verificando possivel dl
bool verificar_deadlock(){
    int categoria_esperando = 0;
    for (int i = 0; i < 4; i++) {
        if (monitor.categoria_esperando[i]){
            categoria_esperando++;
        }
    }
    return categoria_esperando >= 3; // pode ocorrer caso +3 cat. estejam no aguardo da vrz
}

// cliente entra na fila e espera ser atendido
solicitar_atendimento(){


}

// cliente libera o caixa apos atendido
encontrar_prox_cliente()

// deteecta deadlocks caso apareceçam
verificar_deadlock_tempo()

// simula o cliente
thread_cliente()