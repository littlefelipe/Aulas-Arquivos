#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Endereco Endereco;

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

int compara(const void *e1, const void *e2)
{
	return strncmp(((Endereco*)e1)->cep,((Endereco*)e2)->cep,8);
}

int main(int argc, char**argv)
{
	FILE *f, *saida;
	Endereco *e;
	long posicao, qtd, metade;

	f = fopen("cep.dat","rb");
	fseek(f,0,SEEK_END);
	posicao = ftell(f);
	qtd = posicao/sizeof(Endereco);
	e = (Endereco*) malloc(qtd*sizeof(Endereco));
	rewind(f);
	if(fread(e,sizeof(Endereco),qtd,f) == qtd)
	{
		printf("Lido = OK\n");
	}
	qsort(e,qtd,sizeof(Endereco),compara);
	printf("Ordenado = OK\n");
	saida = fopen("cep-ordenado.dat","wb");
	fwrite(e,sizeof(Endereco),qtd,saida);
	fclose(saida);
	free(e);
	return 0;
}

