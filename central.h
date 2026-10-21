#ifndef CENTRAL_H
#define CENTRAL_H

// 1. Constantes com os valores por omissão (Enunciado Tabela 7.2)
#define DEFAULT_MAXESTAFETAS 10
#define DEFAULT_PRAZOACEITA 30

// 2. Estrutura para armazenar os parâmetros de funcionamento da central
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

// Podes vir a adicionar outras mensagens no futuro, como MsgRegistoEstafeta, etc.

#endif // CENTRAL_H