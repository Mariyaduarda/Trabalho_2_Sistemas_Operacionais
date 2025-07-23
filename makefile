# para compilar o projeto Formigopolis: make
# para limpar os arquivos gerados: make clean

# compilador a ser usado
CC = gcc

# opções de compilação guardadas em uma variável
# -Wall ativa todos os avisos, -pthread permite o uso de threads, -g ativa a geração de informações de depuração
CFLAGS = -Wall -pthread -g

#lista dos arquivos fonte
SRCS = monitor.c utils.c interface.c formigopolis.c

# nome do executável
EXEC = formigopolis

# regra padrão para compilar o executável
all: $(EXEC)
	@echo "Compilação concluída. Executável: $(EXEC)"

# como gerar o executável a partir dos arquivos fonte
$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRCS)

# limpa executavel e arquivos objeto
clean:
	rm -f $(EXEC) *.o