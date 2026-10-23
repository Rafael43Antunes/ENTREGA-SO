#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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
            resposta[lidos] = '\0'; 
            printf("[Central] O Tarifador respondeu: %s", resposta);
        }
        // --- FIM DO NOSSO TESTE ---

    }
}

int main(){
    Tarifador tar;

    printf("[Central] A iniciar sistema ...\n");
    
    inicia_tarifador(&tar);

    sleep(2);

    return 0;
}