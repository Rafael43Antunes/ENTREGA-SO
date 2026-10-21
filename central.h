#ifndef CENTRAL_H
#define CENTRAL_H

#include <sys/types.h> //para o pipe

typedef struct{
    int pipe_ct[2]; //pipe direção central - tarifador, ou seja, central escreve (1) tarifador lê (0)
    int pipe_tc[2]; //pipe direção tarifador - central, ou seka, tarifador escreve (1) central lê (0)
    pid_t pid;
} Tarifador;

void inicia_tarifador(Tarifador *tar);

#endif