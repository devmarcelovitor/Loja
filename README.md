# Cadastro de Loja em C

Programa em C para gerenciar produtos de uma loja, com dados salvos em arquivo de texto (`loja.txt`).

## Recursos

- ID automatico e incremental
- Bloqueio de produtos com nome repetido (sem diferenciar maiusculas)
- Criar, listar, inserir, alterar e excluir
- Busca por nome e por categoria
- Ordenacao por nome (A-Z / Z-A) e por preco (menor/maior) com Bubble Sort
- Exportacao para CSV
- Alterar/excluir usando arquivo temporario para evitar corrupcao de dados

## Como usar

```
gcc loja.c -o loja
./loja
```

No Windows: `loja.exe`.
