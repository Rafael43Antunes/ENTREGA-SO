#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "central.h"

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


        // --- INÍCIO DO NOSSO TESTE ---
        char mensagem_envio[] = "restaurante_ze isec 2.4\n";
        char resposta[100];

        // A Central escreve no tubo de ida (Central -> Tarifador)
        write(tar->pipe_ct[1], mensagem_envio, sizeof(mensagem_envio) - 1);
        printf("[Central] Enviei para o tarifador: %s", mensagem_envio);

        // A Central lê do tubo de volta (Tarifador -> Central)
        int lidos = read(tar->pipe_tc[0], resposta, sizeof(resposta) - 1);
        if (lidos > 0) {
            resposta[lidos] = '\0'; // Colocar o terminador de string do C
            printf("[Central] O Tarifador respondeu: %s", resposta);
        }
        // --- FIM DO NOSSO TESTE ---

    }
}

int main(){
    Tarifador tar;

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

    sleep(2);

    return 0;
}