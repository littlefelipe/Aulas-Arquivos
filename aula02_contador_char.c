#include <stdio.h>
// Integrantes do grupo: Felipe Provençano, Isabelle Rocha, Samuel Trindade

// stdout, stdin, stderr

int main(int argc, char** argv)
{
	FILE *entrada, *saida;
	int caractere;

	if(argc != 2)
	{
		fprintf(stderr,"Erro na chamada do comando.\n");
		fprintf(stderr,"Uso: ./NomePrograma <ARQUIVO ORIGEM>.\n", argv[0]);
		return 1;
	}

	entrada = fopen(argv[1],"rb");
	if(!entrada)
	{
		fprintf(stderr,"Arquivo %s não pode ser aberto para leitura\n", argv[1]);
		return 1;
	}


	caractere = fgetc(entrada);
	int contador = 0;
	int qtdc[256] = {0};
	while(caractere != EOF)
	{
		if( caractere == '\n'){
			contador += 1;
		}else{
			qtdc[caractere] += 1;
		}
		caractere = fgetc(entrada);
	}
	if(caractere == EOF){
		contador += 1;
	}
	
	fprintf(stdout , "Quantidade de linhas = %d\n", contador);
	for(int i = 0; i < 256; i ++){
		fprintf(stdout , "Quantidade de caracteres do tipo %c = %d\n", i , qtdc[i]);
	}
	
	fclose(entrada);
	return 0;
}

