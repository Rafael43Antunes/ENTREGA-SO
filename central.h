#ifndef CENTRAL_H
#define CENTRAL_H

#include <sys/types.h>


#define DEFAULT_MAXESTAFETAS 10
#define DEFAULT_PRAZOACEITA 30

typedef struct{
    int pipe_ct[2]; //pipe direção central - tarifador, ou seja, central escreve (1) tarifador lê (0)
    int pipe_tc[2]; //pipe direção tarifador - central, ou seka, tarifador escreve (1) central lê (0)
    pid_t pid;
} Tarifador;

// Estrutura para armazenar os parâmetros de funcionamento da central
typedef struct {
    int max_estafetas;
    int prazo_aceita;
    char *bindir;        
    char *central_pipe; 
    
    int total_pedidos;
    int total_concluidos;
    float valor_total;
} ConfigCentral;

// 3. Estruturas das mensagens
typedef struct {
    char tipo[12];
} MsgGenerica;

// Estrutura obrigatória do pedido gerado
typedef struct {
    char tipo[12];  
    char corpo[50]; 
} MsgPedido;

void inicia_tarifador(Tarifador *tar);

#endif // CENTRAL_H
