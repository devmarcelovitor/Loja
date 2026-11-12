3/* Cadastro de loja em C
 * Os produtos ficam salvos no arquivo loja.txt, com os campos separados por "|".
 * Para compilar: gcc loja.c -o loja
 */

#include <stdio.h>   // funções de entrada, saída e arquivos (printf, fopen, fgets, fprintf, remove, rename)
#include <stdlib.h>  // alocação de memória (malloc, realloc, free) e conversão de números (strtol, strtod)
#include <string.h>  // funções para trabalhar com textos (strlen, strcpy, strcspn)
#include <ctype.h>   // tolower, para comparar textos sem diferenciar maiúsculas de minúsculas

#define ARQ "loja.txt"   // nome do arquivo principal, onde ficam os produtos
#define TMP "temp.txt"   // arquivo temporário, usado ao alterar ou excluir
#define CSV "loja.csv"   // arquivo gerado na exportação para CSV

// Estrutura que guarda os dados de um produto.
// Os textos têm tamanho limitado; se precisar de nomes maiores, basta aumentar os valores.
typedef struct {
    int id;              // número que identifica o produto, gerado automaticamente
    char nome[50];       // nome do produto
    char categoria[30];  // categoria do produto
    float preco;         // preço em reais
    int quantidade;      // quantidade em estoque
    char fornecedor[50]; // nome do fornecedor
} Produto;

// Tipo de ponteiro para função de comparação. Com ele, a mesma função de ordenação
// serve para qualquer critério (nome ou preço, crescente ou decrescente).
typedef int (*Comparador)(const Produto *, const Produto *);

/* ======================= LEITURA DE DADOS ======================= */

// Lê um texto digitado pelo usuário, verifica se é válido e copia para dest.
static void lerTexto(const char *msg, char *dest, int tam) {
    char buf[256];                                   // área temporária para guardar o que foi digitado
    while (1) {                                      // repete até o usuário digitar um texto válido
        printf("%s", msg);                           // mostra a pergunta na tela
        if (!fgets(buf, sizeof buf, stdin)) exit(0); // lê a linha digitada; se a entrada acabar, encerra o programa
        buf[strcspn(buf, "\r\n")] = '\0';            // remove o ENTER que fica no final do texto
        if (strlen(buf) == 0) {                      // não aceita campo vazio
            printf("Campo vazio. Tente novamente.\n");
            continue;                                // volta ao início para perguntar de novo
        }
        if (strlen(buf) >= (size_t)tam) {            // verifica se o texto cabe no campo da estrutura
            printf("Texto muito longo (maximo %d caracteres).\n", tam - 1);
            continue;
        }
        for (char *p = buf; *p; p++)                 // percorre o texto usando um ponteiro
            if (*p == '|') *p = '-';                 // o "|" separa os campos no arquivo, por isso é trocado por "-"
        strcpy(dest, buf);                           // copia o texto validado para o destino
        return;
    }
}

// Lê um número inteiro e confere se o que foi digitado é realmente um número.
static int lerInt(const char *msg) {
    char buf[64];                                    // área temporária para o texto digitado
    char *fim;                                       // vai apontar para o ponto onde a conversão parou
    long x;                                          // guarda o número convertido
    while (1) {
        printf("%s", msg);
        if (!fgets(buf, sizeof buf, stdin)) exit(0);
        x = strtol(buf, &fim, 10);                   // converte o texto em número (base 10)
        if (fim != buf && (*fim == '\n' || *fim == '\0' || *fim == '\r'))
            return (int)x;                           // só aceita se leu algum dígito e não sobrou nenhuma letra
        printf("Entrada invalida. Tente novamente.\n");
    }
}

// Lê a quantidade em estoque, que não pode ser negativa.
static int lerQtd(const char *msg) {
    int q;
    do {
        q = lerInt(msg);
        if (q < 0) printf("A quantidade nao pode ser negativa.\n");
    } while (q < 0);                                 // repete enquanto o valor for negativo
    return q;
}

// Lê o preço. Aceita tanto vírgula quanto ponto como separador decimal.
static float lerPreco(const char *msg) {
    char buf[64];
    char *fim;
    double x;
    while (1) {
        printf("%s", msg);
        if (!fgets(buf, sizeof buf, stdin)) exit(0);
        for (char *p = buf; *p; p++)                 // percorre o texto procurando vírgulas
            if (*p == ',') *p = '.';                 // troca a vírgula por ponto, que é o formato que o C entende
        x = strtod(buf, &fim);                       // converte o texto em número decimal
        if (fim != buf && (*fim == '\n' || *fim == '\0' || *fim == '\r') && x >= 0)
            return (float)x;                         // aceita se for um número válido e não negativo
        printf("Preco invalido. Tente novamente.\n");
    }
}

/* ======================= COMPARAÇÃO DE TEXTOS ======================= */

// Compara dois textos sem diferenciar maiúsculas de minúsculas.
// Devolve 0 se forem iguais, negativo se a vier antes de b e positivo se vier depois.
static int cmpTexto(const char *a, const char *b) {
    while (*a && *b) {                               // continua enquanto nenhum dos textos terminou
        int ca = tolower((unsigned char)*a);         // letra atual de a, em minúscula
        int cb = tolower((unsigned char)*b);         // letra atual de b, em minúscula
        if (ca != cb) return ca - cb;                // se as letras forem diferentes, a diferença indica a ordem
        a++;                                         // avança para a próxima letra de a
        b++;                                         // avança para a próxima letra de b
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b); // um dos textos acabou: compara o que restou
}

// Verifica se "texto" contém "busca" em qualquer posição, sem diferenciar maiúsculas.
static int contem(const char *texto, const char *busca) {
    size_t lb = strlen(busca);                       // tamanho do texto procurado
    for (; *texto; texto++) {                        // tenta começar a comparação em cada posição do texto
        size_t i = 0;                                // quantidade de letras que já coincidiram
        while (i < lb && texto[i] &&
               tolower((unsigned char)texto[i]) == tolower((unsigned char)busca[i]))
            i++;                                     // conta as letras iguais em sequência
        if (i == lb) return 1;                       // todas as letras coincidiram: encontrou
    }
    return 0;                                        // percorreu tudo e não encontrou
}

/* ======================= TRABALHO COM O ARQUIVO ======================= */

// Converte uma linha do arquivo no formato "id|nome|categoria|preco|qtd|fornecedor" em um Produto.
// Devolve 1 se a linha estava correta e 0 se estava com problema.
static int lerLinha(const char *linha, Produto *p) {
    return sscanf(linha, "%d|%49[^|]|%29[^|]|%f|%d|%49[^\n\r]",
                  &p->id, p->nome, p->categoria,
                  &p->preco, &p->quantidade, p->fornecedor) == 6; // se os 6 campos foram lidos, a linha é válida
}

// Grava um produto no arquivo, em uma única linha.
static void gravarProduto(FILE *fp, const Produto *p) {
    fprintf(fp, "%d|%s|%s|%.2f|%d|%s\n",
            p->id, p->nome, p->categoria, p->preco, p->quantidade, p->fornecedor);
}

// Lê todos os produtos do arquivo e guarda em um vetor alocado dinamicamente.
// A quantidade de produtos é devolvida em *n. Quem chama a função deve liberar o vetor com free.
static Produto *carregar(int *n) {
    FILE *fp = fopen(ARQ, "r");                      // abre o arquivo apenas para leitura
    int cap = 16;                                    // capacidade inicial do vetor
    *n = 0;                                          // ainda não há nenhum produto carregado
    if (!fp) {                                       // o arquivo não existe
        printf("Arquivo %s nao existe. Use a opcao 1 para criar.\n", ARQ);
        return NULL;
    }
    Produto *lista = malloc(cap * sizeof(Produto));  // reserva memória para o vetor
    if (!lista) { fclose(fp); return NULL; }         // se não houver memória, encerra a função
    char linha[256];                                 // guarda cada linha lida do arquivo
    Produto p;                                       // produto auxiliar para receber os dados da linha
    while (fgets(linha, sizeof linha, fp)) {         // lê o arquivo linha por linha
        if (!lerLinha(linha, &p)) continue;          // ignora linhas que não estão no formato esperado
        if (*n == cap) {                             // se o vetor encheu, dobra o tamanho
            cap *= 2;
            Produto *novo = realloc(lista, cap * sizeof(Produto));
            if (!novo) { free(lista); fclose(fp); return NULL; }
            lista = novo;
        }
        lista[(*n)++] = p;                           // guarda o produto no vetor e aumenta a contagem
    }
    fclose(fp);                                      // fecha o arquivo
    return lista;
}

// Gera o próximo ID: o maior ID que já existe no arquivo, mais 1.
static int proximoId(void) {
    FILE *fp = fopen(ARQ, "r");
    int maior = 0;                                   // guarda o maior ID encontrado
    char linha[256];
    Produto p;
    if (!fp) return 1;                               // se o arquivo não existe, o primeiro ID é 1
    while (fgets(linha, sizeof linha, fp))
        if (lerLinha(linha, &p) && p.id > maior)     // compara o ID de cada produto com o maior até agora
            maior = p.id;
    fclose(fp);
    return maior + 1;
}

// Verifica se já existe um produto com o mesmo nome.
// O parâmetro idIgnorar permite desconsiderar o próprio produto durante uma alteração.
static int nomeExiste(const char *nome, int idIgnorar) {
    FILE *fp = fopen(ARQ, "r");
    char linha[256];
    Produto p;
    int achou = 0;                                   // passa para 1 se encontrar um nome igual
    if (!fp) return 0;
    while (fgets(linha, sizeof linha, fp))
        if (lerLinha(linha, &p) && p.id != idIgnorar && cmpTexto(p.nome, nome) == 0)
            achou = 1;
    fclose(fp);
    return achou;
}

// Usada para alterar e excluir. Copia os produtos para um arquivo temporário e depois
// substitui o original. Assim, se algo der errado no meio, o arquivo original não é corrompido.
// Se "novo" não for NULL, o produto com esse ID é substituído; se for NULL, o produto é excluído.
static int reescrever(int id, const Produto *novo) {
    FILE *orig = fopen(ARQ, "r");                    // arquivo original, aberto para leitura
    FILE *tmp = fopen(TMP, "w");                     // arquivo temporário, aberto para escrita
    char linha[256];
    Produto p;
    if (!orig || !tmp) {                             // verifica se os dois arquivos foram abertos
        if (orig) fclose(orig);
        if (tmp) fclose(tmp);
        printf("Erro ao abrir os arquivos.\n");
        return 0;
    }
    while (fgets(linha, sizeof linha, orig)) {
        if (!lerLinha(linha, &p)) continue;
        if (p.id != id) gravarProduto(tmp, &p);      // os outros produtos são copiados sem alteração
        else if (novo) gravarProduto(tmp, novo);     // o produto alterado é gravado com os dados novos
        // se novo for NULL, o produto simplesmente não é copiado, o que equivale a excluí-lo
    }
    fclose(orig);
    fclose(tmp);
    remove(ARQ);                                     // apaga o arquivo original
    rename(TMP, ARQ);                                // o temporário passa a ser o arquivo original
    return 1;
}

/* ======================= EXIBIÇÃO EM TABELA ======================= */

// Imprime uma linha de traços para separar as partes da tabela.
static void tracos(void) {
    for (int i = 0; i < 99; i++) putchar('-');
    putchar('\n');
}

// Mostra os produtos do vetor em formato de tabela. O arquivo não é modificado.
static void imprimirTabela(const Produto *lista, int n) {
    tracos();
    printf("| %-4s | %-24s | %-16s | %10s | %10s | %-16s |\n",
           "ID", "Produto", "Categoria", "Preco (R$)", "Quantidade", "Fornecedor");
    tracos();
    for (int i = 0; i < n; i++) {
        const Produto *p = &lista[i];                // ponteiro para o produto da vez
        printf("| %-4d | %-24s | %-16s | %10.2f | %10d | %-16s |\n",
               p->id, p->nome, p->categoria, p->preco, p->quantidade, p->fornecedor);
    }
    tracos();
}

/* ======================= OPERAÇÕES DO MENU ======================= */

// Opção 1: cria o arquivo com os quatro produtos iniciais da tabela.
static void criarArquivo(void) {
    FILE *teste = fopen(ARQ, "r");                   // tenta abrir para descobrir se o arquivo já existe
    if (teste) {
        fclose(teste);
        if (lerInt("O arquivo ja existe. Sobrescrever? (1 = sim, 0 = nao): ") != 1) {
            printf("Operacao cancelada.\n");
            return;
        }
    }
    Produto inicial[] = {                            // produtos iniciais, conforme a tabela da atividade
        {1, "Camiseta Basica",  "Vestuario",       49.90f, 30, "Moda Brasil"},
        {2, "Mouse Optico USB", "Informatica",     59.99f, 15, "TecnoShop"},
        {3, "Shampoo 300ml",    "Higiene Pessoal", 22.50f, 40, "Beleza Pura"},
        {4, "Cafe 500g",        "Alimentos",       18.00f, 25, "Graos do Sul"}
    };
    int qtd = sizeof inicial / sizeof inicial[0];    // calcula quantos produtos o vetor tem
    FILE *fp = fopen(ARQ, "w");                      // "w" cria o arquivo ou apaga o conteúdo anterior
    if (!fp) { printf("Erro ao criar o arquivo.\n"); return; }
    for (int i = 0; i < qtd; i++) gravarProduto(fp, &inicial[i]);
    fclose(fp);
    printf("Arquivo %s criado com %d produtos.\n", ARQ, qtd);
}

// Opção 2: lista todos os produtos.
static void listar(void) {
    int n;
    Produto *lista = carregar(&n);                   // carrega os produtos do arquivo
    if (!lista) return;
    if (n == 0) printf("Nenhum produto cadastrado.\n");
    else imprimirTabela(lista, n);
    free(lista);                                     // libera a memória reservada
}

// Opção 3: insere um produto novo. O ID é gerado automaticamente e o nome não pode se repetir.
static void inserir(void) {
    Produto p;
    lerTexto("Nome: ", p.nome, sizeof p.nome);
    if (nomeExiste(p.nome, 0)) {                     // verifica se o nome já foi cadastrado
        printf("Ja existe um produto com esse nome!\n");
        return;
    }
    lerTexto("Categoria: ", p.categoria, sizeof p.categoria);
    p.preco = lerPreco("Preco: ");
    p.quantidade = lerQtd("Quantidade: ");
    lerTexto("Fornecedor: ", p.fornecedor, sizeof p.fornecedor);
    p.id = proximoId();                              // o ID é calculado pelo programa, sem o usuário digitar
    FILE *fp = fopen(ARQ, "a");                      // "a" acrescenta o produto no final do arquivo
    if (!fp) { printf("Erro ao abrir o arquivo.\n"); return; }
    gravarProduto(fp, &p);
    fclose(fp);
    printf("Produto cadastrado com ID %d.\n", p.id);
}

// Opção 4: altera os dados de um produto, usando o ID para encontrá-lo.
static void alterar(void) {
    int id = lerInt("ID do produto a alterar: ");
    int n;
    Produto *lista = carregar(&n);
    if (!lista) return;
    Produto *p = NULL;                               // vai apontar para o produto encontrado
    for (int i = 0; i < n; i++)
        if (lista[i].id == id) { p = &lista[i]; break; }
    if (!p) {
        printf("ID nao encontrado.\n");
        free(lista);
        return;
    }
    Produto novo = *p;                               // copia os dados atuais para uma variável separada
    free(lista);                                     // depois da cópia, o vetor não é mais necessário
    printf("Produto atual:\n");
    imprimirTabela(&novo, 1);
    printf("Digite os novos dados:\n");
    lerTexto("Nome: ", novo.nome, sizeof novo.nome);
    if (nomeExiste(novo.nome, id)) {                 // confere se outro produto já usa esse nome
        printf("Ja existe outro produto com esse nome!\n");
        return;
    }
    lerTexto("Categoria: ", novo.categoria, sizeof novo.categoria);
    novo.preco = lerPreco("Preco: ");
    novo.quantidade = lerQtd("Quantidade: ");
    lerTexto("Fornecedor: ", novo.fornecedor, sizeof novo.fornecedor);
    if (reescrever(id, &novo)) printf("Produto alterado com sucesso.\n");
}

// Opção 5: exclui um produto, usando o ID para encontrá-lo.
static void excluir(void) {
    int id = lerInt("ID do produto a excluir: ");
    int n;
    Produto *lista = carregar(&n);
    if (!lista) return;
    int achou = 0;                                   // indica se o ID foi encontrado
    for (int i = 0; i < n; i++)
        if (lista[i].id == id) {
            achou = 1;
            imprimirTabela(&lista[i], 1);            // mostra o produto que será excluído
        }
    free(lista);
    if (!achou) { printf("ID nao encontrado.\n"); return; }
    if (lerInt("Confirmar exclusao? (1 = sim, 0 = nao): ") != 1) {
        printf("Operacao cancelada.\n");
        return;
    }
    if (reescrever(id, NULL)) printf("Produto excluido com sucesso.\n");
}

// Opções 6 e 7: busca por nome (porCategoria = 0) ou por categoria (porCategoria = 1).
static void buscar(int porCategoria) {
    char termo[50];                                  // texto que o usuário quer procurar
    lerTexto(porCategoria ? "Categoria a buscar: " : "Nome a buscar: ", termo, sizeof termo);
    int n;
    Produto *lista = carregar(&n);
    if (!lista) return;
    Produto *res = malloc((n > 0 ? n : 1) * sizeof(Produto)); // vetor que vai guardar os resultados
    if (!res) { free(lista); return; }
    int cont = 0;                                    // quantidade de produtos encontrados
    for (int i = 0; i < n; i++) {
        const char *campo = porCategoria ? lista[i].categoria : lista[i].nome; // campo em que a busca é feita
        if (contem(campo, termo)) res[cont++] = lista[i];
    }
    if (cont == 0) printf("Nenhum produto encontrado.\n");
    else imprimirTabela(res, cont);
    free(res);
    free(lista);
}

/* ======================= ORDENAÇÃO (BUBBLE SORT) ======================= */

// Funções de comparação, uma para cada critério de ordenação pedido.
static int cmpNomeAZ(const Produto *a, const Produto *b)    { return cmpTexto(a->nome, b->nome); }
static int cmpNomeZA(const Produto *a, const Produto *b)    { return cmpTexto(b->nome, a->nome); }
static int cmpPrecoAsc(const Produto *a, const Produto *b)  { return (a->preco > b->preco) - (a->preco < b->preco); }
static int cmpPrecoDesc(const Produto *a, const Produto *b) { return (b->preco > a->preco) - (b->preco < a->preco); }

// Bubble Sort: compara produtos vizinhos e troca de posição quando estão fora de ordem.
// O critério de comparação é recebido por ponteiro de função.
static void bubbleSort(Produto *v, int n, Comparador cmp) {
    for (int i = 0; i < n - 1; i++) {                // cada passada leva o último elemento desordenado para o lugar certo
        int trocou = 0;                              // indica se houve alguma troca nesta passada
        for (int j = 0; j < n - i - 1; j++) {        // compara cada produto com o seguinte
            if (cmp(&v[j], &v[j + 1]) > 0) {         // se estiverem fora de ordem, troca
                Produto t = v[j];                    // guarda o primeiro em uma variável auxiliar
                v[j] = v[j + 1];
                v[j + 1] = t;
                trocou = 1;
            }
        }
        if (!trocou) break;                          // sem trocas, o vetor já está ordenado
    }
}

// Carrega os produtos, ordena na memória e mostra o resultado. O arquivo não é alterado.
static void ordenar(Comparador cmp, const char *titulo) {
    int n;
    Produto *lista = carregar(&n);
    if (!lista) return;
    if (n == 0) printf("Nenhum produto cadastrado.\n");
    else {
        bubbleSort(lista, n, cmp);
        printf("\n%s\n", titulo);
        imprimirTabela(lista, n);
    }
    free(lista);
}

/* ======================= EXPORTAÇÃO PARA CSV ======================= */

// Opção 12: grava os dados em um arquivo CSV, com os campos separados por vírgula.
static void exportarCSV(void) {
    int n;
    Produto *lista = carregar(&n);
    if (!lista) return;
    FILE *fp = fopen(CSV, "w");
    if (!fp) { printf("Erro ao criar %s.\n", CSV); free(lista); return; }
    fprintf(fp, "ID,Produto,Categoria,Preco,Quantidade,Fornecedor\n"); // primeira linha com os nomes das colunas
    for (int i = 0; i < n; i++)
        fprintf(fp, "%d,\"%s\",\"%s\",%.2f,%d,\"%s\"\n",            // os textos ficam entre aspas, caso tenham vírgula
                lista[i].id, lista[i].nome, lista[i].categoria,
                lista[i].preco, lista[i].quantidade, lista[i].fornecedor);
    fclose(fp);
    free(lista);
    printf("Dados exportados para %s.\n", CSV);
}

/* ======================= PROGRAMA PRINCIPAL ======================= */

int main(void) {
    int op;                                          // opção escolhida no menu
    do {
        printf("\n===== CADASTRO DE LOJA =====\n");
        printf(" 1 - Criar arquivo\n");
        printf(" 2 - Listar produtos\n");
        printf(" 3 - Inserir produto\n");
        printf(" 4 - Alterar produto\n");
        printf(" 5 - Excluir produto\n");
        printf(" 6 - Buscar por nome\n");
        printf(" 7 - Buscar por categoria\n");
        printf(" 8 - Ordenar por nome (A-Z)\n");
        printf(" 9 - Ordenar por nome (Z-A)\n");
        printf("10 - Ordenar por preco (menor -> maior)\n");
        printf("11 - Ordenar por preco (maior -> menor)\n");
        printf("12 - Exportar para CSV\n");
        printf(" 0 - Sair\n");
        op = lerInt("Escolha: ");

        switch (op) {                                // executa a função correspondente à opção escolhida
            case 1:  criarArquivo(); break;
            case 2:  listar(); break;
            case 3:  inserir(); break;
            case 4:  alterar(); break;
            case 5:  excluir(); break;
            case 6:  buscar(0); break;
            case 7:  buscar(1); break;
            case 8:  ordenar(cmpNomeAZ, "Produtos por nome (A-Z)"); break;
            case 9:  ordenar(cmpNomeZA, "Produtos por nome (Z-A)"); break;
            case 10: ordenar(cmpPrecoAsc, "Produtos por preco (menor -> maior)"); break;
            case 11: ordenar(cmpPrecoDesc, "Produtos por preco (maior -> menor)"); break;
            case 12: exportarCSV(); break;
            case 0:  printf("Saindo...\n"); break;
            default: printf("Opcao invalida!\n");
        }
    } while (op != 0);                               // repete o menu até o usuário escolher sair

    return 0;
}
