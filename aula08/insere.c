#include <bplus.h>
#include <stdio.h>
#include <string.h>

int main() {
    bp_create_if_not_exists("indice.dat",8,300);
    BP_TREE* t = bp_open("indice.dat");
    char chave[9];
    unsigned long valor;
    for(int i=0; i<1000000; i++) {
        sprintf(chave,"%08d",i);
        valor = i;
        bp_insert(t,chave,valor);
    }
    bp_close(t);
    return 0;
}