#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Integrantes do grupo: Felipe Provençano, Isabelle Rocha, Samuel Trindade

typedef struct _Endereco Endereco;
typedef struct _Indice_CEP Indice_CEP;

struct _Endereco
{
	char logradouro[72];
	char bairro[72];
	char cidade[72];
	char uf[72];
	char sigla[2];
	char cep[8];
	char lixo[2];
};

struct _Indice_CEP
{
    char cep[8];
    long pos;
};

int compara(const void *i1, const void *i2)
{
	return strncmp(((Indice_CEP*)i1)->cep,((Indice_CEP*)i2)->cep,8);
}

int main(int argc, char**argv)
{
	FILE *arquivoCep, *saida, *indiceCep;
	Endereco e;
    Indice_CEP *i, indiceBusca;
	long posicao, qtd, metade;
    int qt, indice;
    int c = 0;
	int contador = 0;

	arquivoCep = fopen("cep.dat","rb");
	fseek(arquivoCep,0,SEEK_END);
	posicao = ftell(arquivoCep);
	qtd = posicao/sizeof(Endereco);
	i = (Indice_CEP*) malloc(qtd*sizeof(Indice_CEP));
	rewind(arquivoCep);
    qt = fread(&e,sizeof(Endereco),1,arquivoCep);

	while(qt > 0)
	{
        strncpy(i[c].cep, e.cep, 8);
        i[c].pos = c;
		c++;
		qt = fread(&e,sizeof(Endereco),1,arquivoCep);		
	}

	printf("Qtd leituras: %d\n", c);
	qsort(i,qtd,sizeof(Indice_CEP),compara);
	printf("Ordenado = OK\n");
	saida = fopen("cep-indice.dat","wb");
	fwrite(i,sizeof(Indice_CEP),qtd,saida);
	fclose(saida);
	free(i);

	indiceCep = fopen("cep-indice.dat","rb");
	fseek(indiceCep,0,SEEK_END);
	long tamanhoBytes = ftell(indiceCep);
	long tamanhoRegistros = tamanhoBytes/sizeof(Indice_CEP);
	long inicio = 0;
	long fim = tamanhoRegistros-1;
    printf("Qtd Registros: %d \n", tamanhoRegistros);
	long meio = (inicio+fim)/2;
	fseek(indiceCep,meio*sizeof(Indice_CEP),SEEK_SET);
	qt = fread(&indiceBusca,sizeof(Indice_CEP),1,indiceCep);	

	while(inicio <= fim)
	{
		contador++;
		// argv[1] < e.cep  => strcmp(argv[1],e.cep) < 0
		// argv[1] > e.cep  => strcmp(argv[1],e.cep) > 0
		// argv[1] == e.cep  => strcmp(argv[1],e.cep) == 0
		// pode usar o strstr
		if(strncmp(argv[1],indiceBusca.cep,8)==0)
		{
			printf("\n%.8s\n",indiceBusca.cep);
			indice = indiceBusca.pos;
			break;
			//printf("primeiro if\n");
		}else if (strncmp(argv[1],indiceBusca.cep,8) > 0)
        {
			//printf("segundo if\n");
            inicio = meio + 1;
        }else
        {
			//printf("terceiro if\n");
            fim = meio - 1;
        }
        meio = (inicio+fim)/2;
        fseek(indiceCep,meio*sizeof(Indice_CEP), SEEK_SET);
        qt = fread(&indiceBusca,sizeof(Indice_CEP),1,indiceCep);
		
	}
	printf("Qtd buscas: %d\n", contador);
	fseek(arquivoCep, indice * sizeof(Endereco), SEEK_SET);
	qt = fread(&e, sizeof(Endereco), 1, arquivoCep);
	printf("%.72s\n%.72s\n%.72s\n%.72s\n%.2s\n%.8s\n",e.logradouro,e.bairro,e.cidade,e.uf,e.sigla,e.cep);
	fclose(arquivoCep);
	fclose(indiceCep);
	return 0;
}
