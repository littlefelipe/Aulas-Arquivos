#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ArvoreB.h"

int main(int argc, char** argv)
{
    ArvoreB* a = ArvoreB_Abre("arvore.dat");
    char chave[TAM_CHAVE+1];
    long posicao;
    sprintf(chave,"%08d",atoi(argv[1]));
    posicao = ArvoreB_Busca(a, chave);
    printf("%ld\n",posicao);
    ArvoreB_Fecha(a);
    return 0;
}

