#ifndef CENTRAL_H
#define CENTRAL_H

#include <sys/types.h> //para o pipe

// Constantes com os valores por omissão
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
    char *bindir;         // Diretoria onde está o tarifador
    char *central_pipe;   // Nome do named pipe da central
} ConfigCentral;

// 3. Estruturas das mensagens
// O enunciado pede um campo de tipo comum a todas as mensagens com 12 caracteres.
typedef struct {
    char tipo[12];
} MsgGenerica;

// Estrutura obrigatória do pedido gerado (Enunciado Secção 4.1)
typedef struct {
    char tipo[12];  /* contém sempre "pedido" */
    char corpo[50]; /* dados separados por espaços: <id> <origem> <destino> <peso> */
} MsgPedido;

void inicia_tarifador(Tarifador *tar);

#endif // CENTRAL_H
