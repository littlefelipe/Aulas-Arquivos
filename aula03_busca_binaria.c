#include <stdio.h>
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
	char lixo[2]; // Ao Espaço no final da linha + quebra de linha
};

int main(int argc, char**argv)
{
	FILE *f;
	Endereco e;
	int qt;
	int c;

	if(argc != 2)
	{
		fprintf(stderr, "USO: %s [CEP]", argv[0]);
		return 1;
	}

	c = 0;
	printf("Tamanho da Estrutura: %ld\n\n", sizeof(Endereco));
	f = fopen("cep_ordenado.dat","rb");
	fseek(f,0,SEEK_END);
	long tamanhoBytes = ftell(f);
	long tamanhoRegistros = tamanhoBytes/sizeof(Endereco);
	long inicio = 0;
	long fim = tamanhoRegistros-1;
    printf("%d \n", tamanhoRegistros);
	long meio = (inicio+fim)/2;
	fseek(f,meio*sizeof(Endereco),SEEK_SET);
	qt = fread(&e,sizeof(Endereco),1,f);		
	while(inicio <= fim)
	{
		c++;
		// argv[1] < e.cep  => strcmp(argv[1],e.cep) < 0
		// argv[1] > e.cep  => strcmp(argv[1],e.cep) > 0
		// argv[1] == e.cep  => strcmp(argv[1],e.cep) == 0
		// pode usar o strstr
		if(strncmp(argv[1],e.cep,8)==0)
		{
			printf("%.72s\n%.72s\n%.72s\n%.72s\n%.2s\n%.8s\n",e.logradouro,e.bairro,e.cidade,e.uf,e.sigla,e.cep);
			break;
		}else if (strncmp(argv[1],e.cep,8) > 0)
        {
            inicio = meio + 1;
        }else
        {
            fim = meio - 1;
        }
        meio = (inicio+fim)/2;
        fseek(f,meio*sizeof(Endereco), SEEK_SET);
        qt = fread(&e,sizeof(Endereco),1,f);
	}
	printf("Total Lido: %d\n", c);
	fclose(f);
}