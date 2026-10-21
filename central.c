#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "central.h"

void inicia_tarifador(Tarifador *tar){
    //criar os 2 pipes e assim
}

int main(){
    Tatifador tar;

    printf("[Central] A iniciar sistema ...\n");
    
    inicia_tarifador(&tar);

    return 0;
}