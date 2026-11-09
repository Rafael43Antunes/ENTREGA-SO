#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <signal.h>

#include "central.h"

Tarifador *ptr_tar_sinal = NULL;
//char *ptr_pipe_sinal = NULL;

void encerra_tarifador(Tarifador *tar){
    printf("[Central] A encerrar o sistema e a desligar o tarifador...\n");

    close(tar->pipe_ct[1]);
    close(tar->pipe_tc[0]);
    waitpid(tar->pid, NULL, 0);

    if(unlink(FICHEIRO_LOCK) == 0){
        printf("[Central] Ficheiro de lock apagado.\n");
    }else {
        perror("[Aviso] Nao foi possivel apagar o ficheiro de lock");
    }

    printf("[Central] Tarifador encerrado. Recursos libertados.\n");
}

void trata_sigint(int signum) {
    printf("\n[Sinal] Capturado o sinal SIGINT (Ctrl+C). A forcar encerramento...\n");
    
    if (ptr_tar_sinal != NULL) {
        encerra_tarifador(ptr_tar_sinal);
    }
    
    exit(EXIT_SUCCESS);
}



int simula_pedido(Tarifador *tar, ConfigCentral *config, const char *origem, const char *destino, float peso) {
    char mensagem_envio[150];
    char resposta[100];

    snprintf(mensagem_envio, sizeof(mensagem_envio), "%s %s %.2f\n", origem, destino, peso);
    
    // Escreve no pipe
    if (write(tar->pipe_ct[1], mensagem_envio, strlen(mensagem_envio)) == -1) {
        perror("[Erro] Falha ao escrever no tarifador");
        return -1;
    }
    
    // Lê a resposta do pipe
    int lidos = read(tar->pipe_tc[0], resposta, sizeof(resposta) - 1);
    if (lidos == -1) {
        perror("[Erro] Falha ao ler do tarifador");
        return -1;
    }
    if (lidos == 0) { 
        fprintf(stderr, "[Erro] O tarifador terminou inesperadamente.\n");
        return -1;
    }

    if (lidos > 0) {
        resposta[lidos] = '\0';
        printf("[Debug] Resultado da tarifa: %s", resposta);

        float distancia, valor;
        if(sscanf(resposta, "%f %f", &distancia, &valor) == 2){
            config->total_pedidos++;
            config->total_concluidos++;
            config->valor_total += valor;
        }

        return 0;
    }
}

void inicia_tarifador(Tarifador *tar, const char *bindir){
    char caminho[256];
    int n = snprintf(caminho, sizeof(caminho), "%s/tarifador", bindir);
    if(n < 0 || n >= (int)sizeof(caminho)){
        fprintf(stderr, "[Erro] BINDIR demasiado longo.\n");
        unlink(FICHEIRO_LOCK);
        exit(EXIT_FAILURE);
    }

    // Criar os pipes
    if(pipe(tar->pipe_ct) == -1){
        perror("[Erro] Falha ao criar o pipe_ct");
        unlink(FICHEIRO_LOCK);
        exit(EXIT_FAILURE);
    }

    if(pipe(tar->pipe_tc) == -1){
       perror("[Erro] Falha ao criar o pipe_tc");
       close(tar->pipe_ct[0]);
       close(tar->pipe_ct[1]);
       unlink(FICHEIRO_LOCK);
        exit(EXIT_FAILURE); 
    }

    tar->pid = fork();

    if(tar->pid == -1){
        perror("[Erro] Falha ao executar o fork");
        close(tar->pipe_ct[0]);
        close(tar->pipe_ct[1]);
        close(tar->pipe_tc[0]);
        close(tar->pipe_tc[1]);
        unlink(FICHEIRO_LOCK);
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

        execl(caminho, "tarifador", NULL);

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

    config.total_pedidos = 0;
    config.total_concluidos = 0;
    config.valor_total = 0.0;

    
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
    if (env_pipe != NULL){
        config.central_pipe = env_pipe;
    } else {
        fprintf(stderr, "[Erro] A variavel de ambiente CENTRAL_PIPE e obrigatoria.\n");
        exit(EXIT_FAILURE);
    }


    for (int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            config.max_estafetas = atoi(argv[i + 1]);
            i++;
        }
        else if(strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            config.prazo_aceita = atoi(argv[i + 1]);
            i++;
        } 
        else {
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

    int fd_lock = open("central.lock", O_CREAT | O_EXCL, 0666);
    if (fd_lock == -1) {
        fprintf(stderr, "[Erro] A Central ja esta a correr ou o ficheiro '%s' ficou pendurado.\n", FICHEIRO_LOCK);
        exit(EXIT_FAILURE);
    }
    close(fd_lock);

    printf("[Central] Ficheiro de bloqueio '%s' criado com sucesso.\n", FICHEIRO_LOCK);

    inicia_tarifador(&tar, config.bindir);

    ptr_tar_sinal = &tar;
    signal(SIGINT, trata_sigint);


    char linha[256];
    char comando[50];

    printf("\nSistema pronto. Aguardando comandos...\n");

    while (1) {
        printf("central> ");
        
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            break;
        }

        if (sscanf(linha, "%49s", comando) != 1) {
            continue; // Se o utilizador apenas carregou no Enter, ignora
        }

        // --- VALIDAÇÃO DOS COMANDOS ---
        if (strcmp(comando, "encerra") == 0) {
            break; 
            
        } else if (strcmp(comando, "pedido") == 0) {
            char origem[50], destino[50];
            float peso;
            
            if (sscanf(linha, "pedido %49s %49s %f", origem, destino, &peso) == 3) {

                if (simula_pedido(&tar, &config, origem, destino, peso) == -1) {
                    printf("[Erro] Tarifador indisponivel. A encerrar a central.\n");
                    break;
                }
            } else {
                printf("[Erro] Sintaxe incorreta. Uso: pedido <origem> <destino> <peso>\n");
            }
            
        } else if (strcmp(comando, "stats") == 0) {
            printf("\n--- ESTATISTICAS ---\n");
            printf("Pedidos recebidos: %d\n", config.total_pedidos);
            printf("Pedidos atribuidos: %d\n", config.total_concluidos);
            printf("Pedidos concluidos: %d\n", config.total_concluidos);
            printf("Pedidos expirados/perdidos: 0\n"); 
            printf("Valor total das entregas: %.2f euros\n", config.valor_total);
            printf("--------------------\n");
            
        } else {
            printf("[Erro] Comando desconhecido.\n");
        }
    }

   
    encerra_tarifador(&tar);

    return 0;
}