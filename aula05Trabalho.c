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

int main(int argc, char** argv)
{
	FILE *arquivoCep;
	Endereco e, *buffer;
	int n;
	char nome[80];

	if(argc != 2)
	{
		fprintf(stderr,"Erro na chamada do comando.\n");
		fprintf(stderr,"Uso: ./NomePrograma <ARQUIVO ORIGEM>.\n");
		return 1;
	}

	arquivoCep = fopen(argv[1],"rb");
	if(!arquivoCep)
	{
		fprintf(stderr,"Arquivo %s não pode ser aberto para leitura\n", argv[1]);
		return 1;
	} 

	fseek(arquivoCep,0,SEEK_END);
	long tamanho = ftell(arquivoCep);
	long qtdRegistros = tamanho/sizeof(Endereco);
	rewind(arquivoCep);
	int qtdPartes = qtdRegistros/8;
	int resto = qtdRegistros%8;
	
	buffer = malloc((qtdPartes + 1) * sizeof(Endereco));

	printf("%ld\n", qtdRegistros);

	for(int i = 0; i < 8; i++){
		if (resto > 0)
		{
			n = qtdPartes + 1;
			resto--;
		}else{
			n = qtdPartes;
		}
		sprintf(nome, "b%d.dat", i);
		FILE *f = fopen(nome,"wb");
		fread(buffer, sizeof(Endereco), n, arquivoCep);
		qsort(buffer,n,sizeof(Endereco),compara);
		fwrite(buffer,sizeof(Endereco),n ,f);
		fclose(f);

	}
	
	int a = 0, b = 8;

	while (a + 1 < b){
		FILE *ar1, *ar2, *saida;
		Endereco ea, eb;
		sprintf(nome, "b%d.dat", a);
		ar1 = fopen(nome,"rb");
		sprintf(nome, "b%d.dat", a + 1);
		ar2 = fopen(nome,"rb");
		sprintf(nome, "b%d.dat", b);
		saida = fopen(nome,"wb");
		fread(&ea,sizeof(Endereco),1,ar1);
		fread(&eb,sizeof(Endereco),1,ar2);
		while(!feof(ar1) && !feof(ar2))
		{
			if(compara(&ea,&eb)<0) // ea < eb
			{
				fwrite(&ea,sizeof(Endereco),1,saida);
				fread(&ea,sizeof(Endereco),1,ar1);
			}
			else // ea >= eb
			{
				fwrite(&eb,sizeof(Endereco),1,saida);
				fread(&eb,sizeof(Endereco),1,ar2);
			}
		}

		while(!feof(ar1))
		{
			fwrite(&ea,sizeof(Endereco),1,saida);
			fread(&ea,sizeof(Endereco),1,ar1);		
		}
		
		while(!feof(ar2))
		{
			fwrite(&eb,sizeof(Endereco),1,saida);
			fread(&eb,sizeof(Endereco),1,ar2);		
		}

		fclose(ar1);
		fclose(ar2);
		fclose(saida);
		a = a + 2;
		b++;
	}
	for (int i = 0; i < b - 1; i++)
	{
		sprintf(nome, "b%d.dat", i)/
		remove(nome);
	}
		
	fclose(arquivoCep);
}