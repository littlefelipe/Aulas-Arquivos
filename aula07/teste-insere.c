#include "ArvoreB.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    ArvoreB* a = ArvoreB_Abre("arvore.dat");
    char chave[TAM_CHAVE+1];
    long posicao;
    int i;

    for(i=0; i<1000000; i++)
    {
        sprintf(chave,"%08d",i);
        posicao = i;
        ArvoreB_Insere(a, chave, posicao);
    }

//  ArvoreB_PrintDebug(a);

    ArvoreB_Fecha(a);
    return 0;
}
