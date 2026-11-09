#include "bplus.h"
#include <stdio.h>
#include <string.h>

int main() {
    BP_TREE* t = bp_open("indice.dat");
    char chave[9];
    unsigned long valor;
    int procurado;
    scanf("%d",&procurado);
    sprintf(chave,"%08d",procurado);
    if(bp_find(t,chave,&valor))
    {
        printf("Encontado: %ld\n",valor);
    }
    else {
        printf("Não encontrado\n");
    }
    bp_close(t);
    return 0;
}