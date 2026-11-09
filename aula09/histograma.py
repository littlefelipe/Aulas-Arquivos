import matplotlib.pyplot as plt

def plotar_histograma(caminho_arquivo):
    bytes_lista = []
    frequencias = []

    # 1. Leitura e parsing do arquivo
    with open(caminho_arquivo, 'r', encoding='utf-8') as f:
        for linha in f:
            linha = linha.strip()
            if not linha or '=' not in linha:
                continue
            
            # Separa o byte da frequência pelo caractere '='
            parte_byte, parte_freq = linha.split('=')
            
            # Converte valores (suporta decimal e hexadecimal ex: 0x0A)
            byte_val = int(parte_byte.strip(), 0)
            freq_val = int(parte_freq.strip())
            
            bytes_lista.append(byte_val)
            frequencias.append(freq_val)

    # 2. Criação do gráfico de barras (histograma de frequências)
    plt.figure(figsize=(12, 6))
    plt.bar(bytes_lista, frequencias, color='skyblue', edgecolor='black', width=1.0)

    # 3. Personalização do gráfico
    plt.title('Histograma de Frequência de Bytes (saida.txt)', fontsize=14)
    plt.xlabel('Valor do Byte (0-255)', fontsize=12)
    plt.ylabel('Frequência', fontsize=12)
    plt.xlim(-1, 256)  # Garante a visualização do intervalo total de bytes
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Exibe o gráfico
    plt.tight_layout()
    plt.show()

# Executa para o arquivo dito
plotar_histograma('saida.txt')