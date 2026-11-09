#include <stdio.h>
#include <stdlib.h>

// 1. Estrutura idêntica à do enunciado
typedef struct {
    int dia;
    int novos_casos;
    int obitos;
} RegistroCovid;

int main(int argc, char** argv) {
    FILE *arquivo;
    
    // 2. Abre o arquivo em modo de escrita binária ("wb" = write binary)
    arquivo = fopen("dados_covid.dat", "wb");
    if (arquivo == NULL) {
        printf("Erro ao criar o arquivo.\n");
        return 1;
    }

    // 3. Criando um vetor com dados de exemplo (mock data)
    RegistroCovid dados_teste[] = {
        {0, 150, 2},
        {1, 180, 5},
        {2, 160, 3},
        {3, 200, 7},
        {4, 250, 10},
        {7, 300, 15},
        {8,1400, 40},
        {9,1500, 50},
        {10,1600, 60},
        {11,1700, 70},
        {14,1800, 80},
        {15,1900, 90},
        {18,2500, 120},
        {19,2600, 150},
        {20,2800, 160}
    };
    
    // Calcula quantos registros temos no vetor
    int qtd_registros = sizeof(dados_teste) / sizeof(dados_teste[0]);

    // 4. Escreve o bloco de dados de uma vez no arquivo binário
    // Sintaxe: fwrite(ponteiro_origem, tamanho_do_elemento, quantidade, ponteiro_arquivo)
    fwrite(dados_teste, sizeof(RegistroCovid), qtd_registros, arquivo);

    printf("Arquivo 'dados_covid.dat' criado com sucesso com %d registros!\n", qtd_registros);

    // 5. Fecha o arquivo
    fclose(arquivo);
    
    return 0;
}