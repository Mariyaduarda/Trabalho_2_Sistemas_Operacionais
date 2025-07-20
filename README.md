# Segundo Trabalho - Sistemas Operacionais
O 2º trabalho da disciplina Sistemas Operacionais  no 1º semestre de 2025, com o professor Everthon Valadão


# 🏪 Sistema de Caixa - Formigópolis

Simulador de sistema de atendimento com controle de concorrência, prioridades e prevenção de deadlock usando threads em C.

## 📋 Descrição

Este projeto implementa um sistema de caixa de atendimento que simula um cenário real de banco/loja onde diferentes tipos de clientes (grávidas, idosos, deficientes, padrão) são atendidos respeitando:

- **Prioridades**: Grávidas > Idosos > Deficientes > Padrão
- **Controle de casais**: Membros do mesmo casal não podem ser atendidos simultaneamente
- **Prevenção de deadlock**: Sistema detecta e resolve situações de bloqueio
- **Prevenção de inanição**: Envelhecimento de prioridade para evitar espera infinita
- **Ordem de chegada**: FIFO dentro da mesma prioridade

## 🏗️ Arquitetura

### Componentes principais:

- **Monitor de sincronização**: Controla acesso ao caixa usando mutex e condition variables
- **Sistema de prioridades**: Fila ordenada por prioridade e timestamp
- **Thread gerente**: Monitora deadlocks a cada 5 segundos
- **Threads de clientes**: Simulam chegada e atendimento dos clientes

### Estrutura de arquivos:

```
📁 formigopolis/
├── 📄 formigopolis.h    # definições e estruturas
├── 📄 monitor.c         # lógica principal do monitor
├── 📄 utils.c           # funções de output colorido
├── 📄 main.c            # ponto de entrada
└── 📄 README.md         # este arquivo
```

## 🚀 Como usar

### Compilação:
```bash
gcc -o formigopolis main.c monitor.c utils.c -lpthread
```

### Execução:
```bash
# execução padrão (1 iteração por cliente)
./formigopolis

# execução com múltiplas iterações
./formigopolis 3  # cada cliente vai 3 vezes ao caixa
```

## 👥 Clientes do sistema

O sistema simula 8 clientes organizados em 4 casais:

| Cliente | Prioridade | Casal | Descrição |
|---------|------------|-------|-----------|
| Maria   | 0 (Grávida) | 1 | Gestante |
| Marcos  | 0 (Grávida) | 1 | Pai com criança de colo |
| Vanda   | 1 (Idoso)   | 2 | Pessoa idosa |
| Valter  | 1 (Idoso)   | 2 | Pessoa idosa |
| Paula   | 2 (Deficiente) | 3 | Pessoa com deficiência |
| Pedro   | 2 (Deficiente) | 3 | Pessoa com deficiência |
| Sueli   | 3 (Padrão)  | 4 | Cliente comum |
| Silas   | 3 (Padrão)  | 4 | Cliente comum |

## 🎨 Sistema visual

O programa usa cores ANSI para facilitar o acompanhamento:

- 🟡 **Amarelo**: Cliente entrou na fila
- 🟢 **Verde**: Cliente sendo atendido
- 🔵 **Ciano**: Cliente finalizou atendimento
- 🔴 **Vermelho**: Aviso de deadlock
- 🔵 **Azul**: Informações do sistema

## 🔧 Funcionalidades implementadas

### ✅ Controle de sincronização
- Mutex para proteção de seção crítica
- Condition variables para sinalização entre threads
- Monitor para coordenar acesso ao caixa único

### ✅ Sistema de prioridades
- Fila ordenada por prioridade (0=maior, 3=menor)
- FIFO dentro da mesma prioridade usando timestamps
- Inserção e remoção eficientes mantendo ordem

### ✅ Controle de casais
- Array de controle `casal_no_caixa[4]`
- Impede atendimento simultâneo de membros do mesmo casal
- Verificação antes de cada atendimento

### ✅ Detecção de deadlock
- Thread gerente monitora situações de bloqueio
- Verifica: caixa vazio + 3+ categorias esperando + ninguém pode ser atendido
- Resolução: libera temporariamente restrição de casal aleatório

### ✅ Prevenção de inanição
- Contador de frustrações por cliente na fila
- Envelhecimento: promove prioridade após 2 frustrações
- Restaura prioridade original após atendimento

### ✅ Ordem de chegada
- Timestamp único para cada entrada na fila
- Desempate por ordem cronológica dentro da mesma prioridade
- Garante justiça no atendimento

## 📊 Fluxo de execução

1. **Inicialização**: Monitor, clientes e thread gerente são criados
2. **Chegada**: Clientes chegam aleatoriamente (3-5s de intervalo)
3. **Fila**: Cliente entra na fila de prioridade ordenada
4. **Espera**: Cliente aguarda sua vez respeitando restrições
5. **Atendimento**: Cliente é atendido por 1 segundo
6. **Saída**: Cliente finaliza e libera o caixa
7. **Repetição**: Processo se repete conforme iterações configuradas

## 🐛 Depuração

### Logs do sistema:
- Cada ação é logada com cores e identificadores
- Thread gerente reporta detecções de deadlock/inanição
- Timestamps implícitos através da ordem das mensagens

### Problemas comuns:
- **Deadlock**: Todas as categorias esperando mas ninguém pode ser atendido
- **Inanição**: Cliente espera muito tempo por ter prioridade baixa
- **Race condition**: Acesso desprotegido às estruturas compartilhadas

## 🔬 Conceitos aplicados

- **Programação concorrente**: Múltiplas threads executando simultaneamente
- **Sincronização**: Mutex e condition variables para coordenação
- **Monitor**: Padrão de sincronização de alto nível
- **Deadlock**: Detecção e resolução de bloqueios mútuos
- **Starvation**: Prevenção de inanição com envelhecimento
- **Priority queue**: Estrutura de dados para fila de prioridades

## 📈 Possíveis melhorias

- [ ] Interface gráfica para visualização em tempo real
- [ ] Métricas de desempenho (tempo médio de espera, throughput)
- [ ] Configuração dinâmica de prioridades
- [ ] Múltiplos caixas de atendimento
- [ ] Persistência de logs em arquivo
- [ ] Testes automatizados de cenários específicos

## 👨‍💻 Desenvolvimento

Este projeto demonstra conceitos avançados de programação concorrente em C, incluindo:
- Gerenciamento de threads com pthreads
- Sincronização com mutex e condition variables
- Estruturas de dados personalizadas
- Algoritmos de escalonamento por prioridade
- Detecção e resolução de problemas de concorrência

---

**Nota**: Este é um projeto educacional para demonstrar conceitos de sistemas operacionais e programação concorrente.
