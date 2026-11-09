#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct _RegistroCovid RegistroCovid;

#define NUMDIV 3

struct _RegistroCovid
{
    int dia;
    int novos_casos;
    int obitos;
};

bool verificaLacuna(FILE *arquivo){
    RegistroCovid reg;
    int qt = fread(&reg,sizeof(RegistroCovid), 1, arquivo);
    int dia_ant = reg.dia;
    while (qt > 0){
        qt = fread(&reg,sizeof(RegistroCovid), 1, arquivo);
        if (reg.dia != dia_ant + 1)
        {
            return false;
        }
        dia_ant = reg.dia;     
    }
    return true;
}

// Função para resolver o Item III
void geraMediaMovel(FILE *arquivo, int k) {
    FILE *saida = fopen("medias_moveis.dat", "wb");
    if (!saida) {
        fprintf(stderr, "Erro ao criar arquivo de médias móveis.\n");
        return;
    }

    RegistroCovid *bloco = malloc(k * sizeof(RegistroCovid));
    if (!bloco) {
        fprintf(stderr, "Erro de alocação de memória.\n");
        fclose(saida);
        return;
    }

    rewind(arquivo); 

    // Lê a primeira vez
    int qt = fread(bloco, sizeof(RegistroCovid), k, arquivo);
    // Condição: qt tem que ser igual a 'k'. Se for menor, a janela está incompleta.
    while (qt == k) {
        int soma_casos = 0;
        int soma_obitos = 0;

        for (int i = 0; i < k; i++) {
            soma_casos += bloco[i].novos_casos;
            soma_obitos += bloco[i].obitos;
        }

        RegistroCovid media_reg;
        media_reg.dia = bloco[k - 1].dia;
        media_reg.novos_casos = soma_casos / k;
        media_reg.obitos = soma_obitos / k;

        fwrite(&media_reg, sizeof(RegistroCovid), 1, saida);
        
        fseek(arquivo, -(k - 1) * sizeof(RegistroCovid), SEEK_CUR);
        
        // CORREÇÃO AQUI: Lê o próximo bloco e atualiza a variável qt!
        qt = fread(bloco, sizeof(RegistroCovid), k, arquivo);
    }

    free(bloco);
    fclose(saida);
    printf("Item III concluido: Arquivo 'medias_moveis.dat' gerado com sucesso (janela = %d)!\n", k);
}

int main(int argc, char** argv){
    FILE *arquivo;
    RegistroCovid registro, *bloco;
    int n = 0;


    if(argc != 2)
	{
		fprintf(stderr,"Erro na chamada do comando.\n");
		fprintf(stderr,"Uso: ./NomePrograma <ARQUIVO ORIGEM>.\n");
		return 1;
	}

	arquivo = fopen(argv[1],"rb");
	if(!arquivo)
	{
		fprintf(stderr,"Arquivo %s não pode ser aberto para leitura\n", argv[1]);
		return 1;
	}

    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    long qtdReg = tamanho/sizeof(RegistroCovid);
    rewind(arquivo);
    int k = qtdReg/NUMDIV;
	int resto = qtdReg%NUMDIV;

    printf("Tamanho arquivo: %ld\n", tamanho);
    printf("Qtd registros: %ld\n", qtdReg);

    bool lacuna = verificaLacuna(arquivo);
    printf("%d\n", lacuna);

    int tamanho_k = 7; 
    geraMediaMovel(arquivo, tamanho_k);

    fclose(arquivo);
}