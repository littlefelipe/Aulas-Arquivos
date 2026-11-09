#ifndef BPLUS_H
#define BPLUS_H

#include <stdbool.h>
#include <stdio.h>

/*
 * O arquivo contem o cabecalho seguido de paginas de tamanho fixo.
 * Os deslocamentos de paginas sao medidos em bytes a partir do inicio
 * do arquivo. O deslocamento zero indica ausencia de pagina.
 * O formato usa os tipos nativos da plataforma, sem portabilidade.
 */
#pragma pack(push, 1)

/* Apenas esta estrutura e gravada como cabecalho do arquivo. */
typedef struct bp_header {
    char magic[2];                 /* Os dois caracteres 'B' e 'P'. */
    unsigned short tree_height;   /* Numero de niveis; raiz folha: 1. */
    unsigned long left_leaf;      /* Deslocamento da primeira folha. */
    unsigned long root_offset;    /* Deslocamento da raiz. */
    unsigned long count;          /* Total de chaves nas folhas. */
    unsigned short key_size;      /* Bytes por chave, sem terminador. */
    unsigned short page_max_size; /* Capacidade de chaves por pagina. */
    unsigned short page_size_bytes; /* Deve comportar a pagina calculada. */
} BP_HEADER;

#pragma pack(pop)

/* Descritor em memoria; somente o conteudo de buffer e gravado. */
typedef struct bp_page {
    char *buffer;            /* Pagina completa, alocada com malloc. */
    unsigned long *count;    /* Aponta para a contagem no inicio do buffer. */
    unsigned long *pointers; /* Aponta para os N+1 valores apos a contagem. */
    char *keys;              /* Aponta para as N chaves dentro do buffer. */
} BP_PAGE;

/* Handler em memoria; esta estrutura nao e gravada diretamente. */
typedef struct bp_tree {
    BP_HEADER header;
    FILE *f;
    BP_PAGE *root; /* A raiz permanece carregada enquanto a arvore esta aberta. */
} BP_TREE;

/*
 * Layout de cada pagina, sendo N = header.page_max_size:
 * | COUNT | PT0 | PT1 | ... | PT(N) | KEY0 | KEY1 | ... | KEY(N-1) |
 *
 * Tamanho do buffer:
 * sizeof(unsigned long) * (N + 2) + key_size * N
 *
 * count = (unsigned long *) buffer
 * pointers = (unsigned long *) (buffer + sizeof(unsigned long))
 * keys = buffer + sizeof(unsigned long) * (N + 2)
 *
 * O contador e o vetor de ponteiros usam unsigned long e ficam alinhados
 * em um buffer alocado com malloc, independentemente do tamanho da chave.
 * O acesso usa diretamente pointers[i] e *count.
 *
 * Folha com C chaves: PT0 ate PT(C-1) referenciam registros em outro
 * arquivo. PT(N), em posicao fixa, aponta para a proxima folha, ou zero
 * quando nao existe proxima folha.
 * Pagina interna com C chaves: PT0 ate PT(C) apontam para filhos.
 * Nesse caso, PT(N) tambem pode ser um filho, e nao um elo entre paginas.
 * O nivel da pagina, junto com tree_height, distingue folhas de internas.
 */

/*
 * Chaves: exatamente key_size bytes, completados a direita com espacos
 * pelo chamador, sem terminador '\0'. Comparacao por memcmp usando todos
 * os bytes, distinguindo maiusculas de minusculas. Os separadores nas
 * paginas internas sao copias de chaves; apenas as folhas armazenam os
 * valores associados aos registros.
 */

/* Cria o arquivo com uma raiz folha vazia. Retorna false se ja existir,
 * se os parametros forem invalidos ou se ocorrer um erro.
 * key_size deve ser positivo e page_max_size deve ser pelo menos 2.
 * O tamanho calculado da pagina deve caber em unsigned short. */
bool bp_create_if_not_exists(const char *filename, unsigned short key_size,
                             unsigned short page_max_size);

/* Abre em "r+b", le o cabecalho e carrega a raiz. Retorna NULL em erro. */
BP_TREE *bp_open(const char *filename);

/* Grava as alteracoes pendentes, fecha o arquivo e libera a memoria. */
void bp_close(BP_TREE *tree);

/* Insere uma chave e seu valor. Retorna false em erro ou chave duplicada;
 * uma tentativa duplicada preserva o valor ja associado a chave. */
bool bp_insert(BP_TREE *tree, const char key[], unsigned long ptr);

/* Retorna true e preenche *ptr quando encontra a chave. Retorna false
 * se nao encontrar ou em erro; nesses casos, nao altera *ptr. */
bool bp_find(BP_TREE *tree, const char key[], unsigned long *ptr);

#endif
