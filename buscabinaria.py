a = [10, 20, 30, 40, 50, 60, 70, 80]
b = 10

contador = 0
inicio = 0
fim = len(a)-1
while inicio <= fim:
    contador += 1
    meio = (inicio+fim)//2
    if a[meio] == b:
        print(f"Achei {b} na posição {meio}")
        break
    elif b > a[meio]:
        inicio = meio + 1
    else:
        fim = meio - 1

print(f"Contador: {contador}")

