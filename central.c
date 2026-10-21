#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "central.h"

void inicia_tarifador(Tarifador *tar){
    //criar os 2 pipes e assim
}

int main(int argc, char *argv[]){
    Tarifador tar;
    ConfigCentral config;

    printf("[Central] A iniciar sistema ...\n");
    
    // --- LEITURA DE PARÂMETROS ---

    // 1. Inicializar com os valores por omissão
    config.max_estafetas = DEFAULT_MAXESTAFETAS;
    config.prazo_aceita = DEFAULT_PRAZOACEITA;
    config.bindir = "."; // Diretoria atual por omissão
    config.central_pipe = NULL;
    
    // 2. Obter variáveis de ambiente
    char *env_max = getenv("MAXESTAFETAS");
    if (env_max != NULL) {
        config.max_estafetas = atoi(env_max);
    }

    char *env_prazo = getenv("PRAZOACEITA");
    if (env_prazo != NULL) {
        config.prazo_aceita = atoi(env_prazo);
    }

    char *env_bindir = getenv("BINDIR");
    if (env_bindir != NULL) {
        config.bindir = env_bindir;
    }

    char *env_pipe = getenv("CENTRAL_PIPE");
    if (env_pipe != NULL) {
        config.central_pipe = env_pipe;
    } else {
        // Obrigatório: se falhar a leitura, abortar.
        fprintf(stderr, "Erro: A variavel de ambiente CENTRAL_PIPE e obrigatoria.\n");
        exit(EXIT_FAILURE);
    }

    // 3. Obter argumentos da linha de comandos (-m e -p)
    int opt;
    while((opt = getopt(argc, argv, "m:p:")) != -1){
        switch (opt){
            case 'm':
                config.max_estafetas = atoi(optarg);
                break;
            case 'p':
                config.prazo_aceita = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Uso: %s [-m max_estafetas] [-p prazo_aceita]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Apenas para testar se tudo carregou bem
    printf("=== Parametros Carregados ===\n");
    printf("Max Estafetas: %d\n", config.max_estafetas);
    printf("Prazo Aceitacao: %d\n", config.prazo_aceita);
    printf("Diretoria Tarifador: %s\n", config.bindir);
    printf("Named Pipe Central: %s\n", config.central_pipe);
    printf("=============================\n");

    inicia_tarifador(&tar);

    return 0;
}