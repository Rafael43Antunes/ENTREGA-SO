#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "central.h"

void encerra_tarifador(Tarifador *tar){
    printf("[Central] A encerrar o sistema e a desligar o tarifador...\n");

    close(tar->pipe_ct[1]);

    close(tar->pipe_tc[0]);

    waitpid(tar->pid, NULL, 0);

    printf("[Central] Tarifador encerrado. Recursos libertados.\n");
}

void simula_pedido(Tarifador *tar, const char *origem, const char *destino, float peso) {
    char mensagem_envio[150];
    char resposta[100];

    snprintf(mensagem_envio, sizeof(mensagem_envio), "%s %s %.2f\n", origem, destino, peso);
    
    // Escreve no pipe
    write(tar->pipe_ct[1], mensagem_envio, strlen(mensagem_envio));
    
    // Lê a resposta do pipe
    int lidos = read(tar->pipe_tc[0], resposta, sizeof(resposta) - 1);
    if (lidos > 0) {
        resposta[lidos] = '\0';
        printf("[Debug] Resultado da tarifa: %s", resposta);
    }
}

void inicia_tarifador(Tarifador *tar){
    // Criar os pipes
    if(pipe(tar->pipe_ct) == -1){
        perror("[Erro] Falha ao criar o pipe_ct");
        exit(EXIT_FAILURE);
    }

    if(pipe(tar->pipe_tc) == -1){
       perror("[Erro] Falha ao criar o pipe_tc");
        exit(EXIT_FAILURE); 
    }

    tar->pid = fork();

    if(tar->pid == -1){
        perror("[Erro] Falha ao executar o fork");
        exit(EXIT_FAILURE);
    }

    //Se estivermos no filho (tarifador)
    if(tar->pid == 0){
        close(tar->pipe_ct[1]); //nao escreve no sentido central->tarifador
        close(tar->pipe_tc[0]); //nao le no sentido tarifador -> central

        dup2(tar->pipe_ct[0], STDIN_FILENO);

        dup2(tar->pipe_tc[1], STDOUT_FILENO);

        close(tar->pipe_ct[0]);
        close(tar->pipe_tc[1]);

        execl("./tarifador", "tarifador", NULL);

        perror("[Erro] Falha ao executar o tarifador");
        exit(EXIT_FAILURE);
    }

    else{
        close(tar->pipe_ct[0]);
        close(tar->pipe_tc[1]);

        printf("[Central] Tarifador arrancou com PID %d.\n", tar->pid);


    }
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

    char linha[256];
    char comando[50];

    printf("\nSistema pronto. Aguardando comandos...\n");

    while (1) {
        printf("central> ");
        
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            break;
        }

        if (sscanf(linha, "%s", comando) != 1) {
            continue; // Se o utilizador apenas carregou no Enter, ignora
        }

        // --- VALIDAÇÃO DOS COMANDOS ---
        if (strcmp(comando, "encerra") == 0) {
            encerra_tarifador(&tar);
            break; 
            
        } else if (strcmp(comando, "pedido") == 0) {
            char origem[50], destino[50];
            float peso;
            
            if (sscanf(linha, "pedido %49s %49s %f", origem, destino, &peso) == 3) {
                simula_pedido(&tar, origem, destino, peso);
            } else {
                printf("[Erro] Sintaxe incorreta. Uso: pedido <origem> <destino> <peso>\n");
            }
            
        } else if (strcmp(comando, "stats") == 0) {
            printf("[Stats] Totais desde o arranque: a implementar...\n");
            
        } else {
            printf("[Erro] Comando desconhecido.\n");
        }
    }

   
    sleep(2);

    return 0;
}