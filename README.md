# Cadastro de Loja em C

Aplicação de terminal desenvolvida em linguagem C para gerenciar o cadastro de produtos de uma loja. Os dados são armazenados em arquivo de texto, o que garante a persistência das informações entre as execuções.

O projeto integra manipulação de arquivos, estruturas de dados, ponteiros e algoritmos de ordenação.

## Funcionalidades

- **Geração automática de ID:** identificador incremental, calculado pelo programa a partir do maior ID existente.
- **Controle de duplicidade:** não permite cadastrar dois produtos com o mesmo nome, independentemente de letras maiúsculas ou minúsculas.
- **Operações completas:** criação do arquivo, listagem, inserção, alteração e exclusão de produtos.
- **Busca:** por nome e por categoria, sem distinção entre maiúsculas e minúsculas e com suporte a correspondência parcial.
- **Ordenação com Bubble Sort:**
  - por nome, de A a Z e de Z a A;
  - por preço, do menor para o maior e do maior para o menor.
- **Exportação para CSV**, com os campos separados por vírgula.
- **Alteração e exclusão seguras:** os dados são copiados para um arquivo temporário que, ao final, substitui o original, evitando a corrupção do arquivo.
- **Validação de entrada:** valores não numéricos, preços e quantidades negativas e campos vazios são tratados sem interromper a execução.

## Requisitos

- Compilador C (GCC ou compatível)

## Compilação e execução

```bash
git clone https://github.com/SEU-USUARIO/NOME-DO-REPOSITORIO.git
cd NOME-DO-REPOSITORIO
gcc loja.c -o loja
./loja
```

No Windows, o executável gerado é `loja.exe`.

Na primeira execução, utilize a opção 1 do menu para criar o arquivo `loja.txt` com os produtos iniciais.

### Ambientes alternativos

Caso não haja um compilador instalado, o programa pode ser executado em:

- [OnlineGDB](https://www.onlinegdb.com/online_c_compiler), colando o conteúdo de `loja.c`;
- GitHub Codespaces, que já disponibiliza o GCC;
- WSL (Windows Subsystem for Linux), em máquinas Windows que bloqueiem a execução de arquivos `.exe` compilados localmente.

## Menu

```
===== CADASTRO DE LOJA =====
 1 - Criar arquivo
 2 - Listar produtos
 3 - Inserir produto
 4 - Alterar produto
 5 - Excluir produto
 6 - Buscar por nome
 7 - Buscar por categoria
 8 - Ordenar por nome (A-Z)
 9 - Ordenar por nome (Z-A)
10 - Ordenar por preco (menor -> maior)
11 - Ordenar por preco (maior -> menor)
12 - Exportar para CSV
 0 - Sair
```

## Exemplo de saída

```
---------------------------------------------------------------------------------------------------
| ID   | Produto                  | Categoria        | Preco (R$) | Quantidade | Fornecedor       |
---------------------------------------------------------------------------------------------------
| 1    | Camiseta Basica          | Vestuario        |      49.90 |         30 | Moda Brasil      |
| 2    | Mouse Optico USB         | Informatica      |      59.99 |         15 | TecnoShop        |
| 3    | Shampoo 300ml            | Higiene Pessoal  |      22.50 |         40 | Beleza Pura      |
| 4    | Cafe 500g                | Alimentos        |      18.00 |         25 | Graos do Sul     |
---------------------------------------------------------------------------------------------------
```

## Estrutura do projeto

```
.
├── loja.c        Código-fonte com comentários explicativos
├── loja.txt      Dados dos produtos (gerado e atualizado pelo programa)
├── loja.csv      Exportação em CSV (gerado pela opção 12)
└── README.md     Documentação
```

### Formato do arquivo de dados

Cada produto ocupa uma linha de `loja.txt`, com os campos separados pelo caractere `|`:

```
ID|Produto|Categoria|Preco|Quantidade|Fornecedor
1|Camiseta Basica|Vestuario|49.90|30|Moda Brasil
```

## Conceitos aplicados

- Estruturas (`struct`) e vetores de estruturas
- Alocação dinâmica de memória (`malloc`, `realloc` e `free`)
- Ponteiros, incluindo ponteiro para função, utilizado para definir o critério de ordenação
- Manipulação de arquivos de texto (`fopen`, `fgets`, `fprintf`, `remove` e `rename`)
- Algoritmo de ordenação Bubble Sort
- Comparação e busca de textos sem distinção entre maiúsculas e minúsculas
- Validação de dados informados pelo usuário

## Autor

Marcelo Vitor Sousa Ribeiro
Estudante de Engenharia de Software

## Licença

Projeto de uso livre para fins de estudo e aprendizado.
