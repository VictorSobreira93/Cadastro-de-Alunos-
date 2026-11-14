#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define NOME_ARQUIVO "cadastro de alunos.txt"
#define TAM_TEXTO 50

typedef struct {
    int matricula;
    char nome[TAM_TEXTO];
    char curso[TAM_TEXTO];
} Aluno;

typedef struct No {
    Aluno dado;
    struct No *esquerda;
    struct No *direita;
} No;

static int fimEntrada = 0;

static int lerLinha(char *destino, int tamanho)
{
    size_t n;
    int c;

    if (fgets(destino, tamanho, stdin) == NULL) {
        fimEntrada = 1;
        destino[0] = '\0';
        return 0;
    }
    n = strlen(destino);
    if (n > 0 && destino[n - 1] == '\n') {
        destino[n - 1] = '\0';
    } else {
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }
    return 1;
}

static int lerInteiro(const char *pergunta, int minimo, int maximo, int *valor)
{
    char linha[64];
    char *fim;
    long lido;

    for (;;) {
        printf("%s", pergunta);
        if (!lerLinha(linha, (int)sizeof(linha))) {
            return 0;
        }
        lido = strtol(linha, &fim, 10);
        while (*fim == ' ' || *fim == '\t') {
            fim++;
        }
        if (fim == linha || *fim != '\0' || lido < minimo || lido > maximo) {
            printf("Valor invalido. Digite um numero inteiro entre %d e %d.\n", minimo, maximo);
        } else {
            *valor = (int)lido;
            return 1;
        }
    }
}

static int lerTexto(const char *pergunta, char *destino)
{
    char *p;

    for (;;) {
        printf("%s", pergunta);
        if (!lerLinha(destino, TAM_TEXTO)) {
            return 0;
        }
        for (p = destino; *p != '\0'; p++) {
            if (*p == ';') {
                *p = ',';
            }
        }
        if (destino[0] != '\0') {
            return 1;
        }
        printf("O campo nao pode ficar vazio.\n");
    }
}

static No *criarNo(Aluno dado)
{
    No *novo = (No *)malloc(sizeof(No));

    if (novo == NULL) {
        return NULL;
    }
    novo->dado = dado;
    novo->esquerda = NULL;
    novo->direita = NULL;
    return novo;
}

static int inserir(No **raiz, Aluno dado)
{
    if (*raiz == NULL) {
        *raiz = criarNo(dado);
        return (*raiz == NULL) ? -1 : 1;
    }
    if (dado.matricula < (*raiz)->dado.matricula) {
        return inserir(&(*raiz)->esquerda, dado);
    }
    if (dado.matricula > (*raiz)->dado.matricula) {
        return inserir(&(*raiz)->direita, dado);
    }
    return 0;
}

static No *buscar(No *raiz, int matricula)
{
    while (raiz != NULL && raiz->dado.matricula != matricula) {
        raiz = (matricula < raiz->dado.matricula) ? raiz->esquerda : raiz->direita;
    }
    return raiz;
}

static void imprimirCabecalho(void)
{
    printf("%-10s %-28s %-22s\n", "Matricula", "Nome", "Curso");
}

static void imprimirAluno(const Aluno *f)
{
    printf("%-10d %-28s %-22s\n", f->matricula, f->nome, f->curso);
}

static void listarEmOrdem(const No *raiz)
{
    if (raiz == NULL) {
        return;
    }
    listarEmOrdem(raiz->esquerda);
    imprimirAluno(&raiz->dado);
    listarEmOrdem(raiz->direita);
}

static void exibirArvore(const No *raiz, int nivel, const char *rotulo)
{
    int i;

    if (raiz == NULL) {
        return;
    }
    for (i = 0; i < nivel; i++) {
        printf("    ");
    }
    printf("%s%d (%s)\n", rotulo, raiz->dado.matricula, raiz->dado.nome);
    exibirArvore(raiz->esquerda, nivel + 1, "[esq] ");
    exibirArvore(raiz->direita, nivel + 1, "[dir] ");
}

static void gravarPreOrdem(FILE *arquivo, const No *raiz)
{
    if (raiz == NULL) {
        return;
    }
    fprintf(arquivo, "%d;%s;%s\n",
            raiz->dado.matricula, raiz->dado.nome, raiz->dado.curso);
    gravarPreOrdem(arquivo, raiz->esquerda);
    gravarPreOrdem(arquivo, raiz->direita);
}

static void destruirArvore(No *raiz)
{
    if (raiz == NULL) {
        return;
    }
    destruirArvore(raiz->esquerda);
    destruirArvore(raiz->direita);
    free(raiz);
}

static No *removerNo(No *raiz, int matricula, int *removido)
{
    No *filho;
    No *sucessor;
    int ignorado = 0;

    if (raiz == NULL) {
        return NULL;
    }
    if (matricula < raiz->dado.matricula) {
        raiz->esquerda = removerNo(raiz->esquerda, matricula, removido);
    } else if (matricula > raiz->dado.matricula) {
        raiz->direita = removerNo(raiz->direita, matricula, removido);
    } else {
        *removido = 1;
        if (raiz->esquerda == NULL) {
            filho = raiz->direita;
            free(raiz);
            return filho;
        }
        if (raiz->direita == NULL) {
            filho = raiz->esquerda;
            free(raiz);
            return filho;
        }

        sucessor = raiz->direita;
        while (sucessor->esquerda != NULL) {
            sucessor = sucessor->esquerda;
        }
        raiz->dado = sucessor->dado;
        raiz->direita = removerNo(raiz->direita, sucessor->dado.matricula, &ignorado);
    }
    return raiz;
}

static void carregarArquivo(No **raiz)
{
    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    Aluno f;
    int resultado;
    int lidos = 0;

    if (arquivo == NULL) {
        printf("Arquivo %s nao encontrado. Comecando com o cadastro vazio.\n", NOME_ARQUIVO);
        return;
    }
    while (fscanf(arquivo, "%d;%49[^;];%49[^;\n]\n",
                  &f.matricula, f.nome, f.curso) == 3) {
        resultado = inserir(raiz, f);
        if (resultado == -1) {
            printf("erro: memoria insuficiente ao carregar o arquivo\n");
            break;
        }
        if (resultado == 0) {
            printf("Aviso: matricula %d repetida no arquivo, linha ignorada.\n", f.matricula);
        } else {
            lidos++;
        }
    }
    fclose(arquivo);
    printf("%d registro(s) carregado(s) de %s.\n", lidos, NOME_ARQUIVO);
}

static int gravarArquivo(const No *raiz)
{
    FILE *arquivo = fopen(NOME_ARQUIVO, "w");

    if (arquivo == NULL) {
        printf("erro: nao foi possivel abrir o arquivo %s para gravacao\n", NOME_ARQUIVO);
        return 0;
    }
    gravarPreOrdem(arquivo, raiz);
    fclose(arquivo);
    return 1;
}

static void telaCadastrar(No **raiz)
{
    Aluno f;
    int resultado;

    if (!lerInteiro("Matricula: ", 1, INT_MAX, &f.matricula)) return;
    if (buscar(*raiz, f.matricula) != NULL) {
        printf("Ja existe um aluno com a matricula %d. Cadastro recusado.\n", f.matricula);
        return;
    }
    if (!lerTexto("Nome: ", f.nome)) return;
    if (!lerTexto("Curso: ", f.curso)) return;

    resultado = inserir(raiz, f);
    if (resultado == 1) {
        printf("Aluno cadastrado com sucesso.\n");
    } else if (resultado == 0) {
        printf("Ja existe um aluno com a matricula %d. Cadastro recusado.\n", f.matricula);
    } else {
        printf("erro: memoria insuficiente para cadastrar\n");
    }
}

static void telaBuscar(No *raiz)
{
    int matricula;
    No *achado;

    if (!lerInteiro("Matricula a buscar: ", 1, INT_MAX, &matricula)) return;
    achado = buscar(raiz, matricula);
    if (achado == NULL) {
        printf("A matricula %d nao existe no cadastro.\n", matricula);
    } else {
        imprimirCabecalho();
        imprimirAluno(&achado->dado);
    }
}

static void telaListar(No *raiz)
{
    if (raiz == NULL) {
        printf("O cadastro esta vazio.\n");
        return;
    }
    imprimirCabecalho();
    listarEmOrdem(raiz);
}

static void telaExibirArvore(No *raiz)
{
    if (raiz == NULL) {
        printf("A arvore esta vazia.\n");
        return;
    }
    exibirArvore(raiz, 0, "[raiz] ");
}

static void telaAlterarCurso(No *raiz)
{
    int matricula;
    char novoCurso[TAM_TEXTO];
    No *achado;

    if (!lerInteiro("Matricula do aluno: ", 1, INT_MAX, &matricula)) return;
    achado = buscar(raiz, matricula);
    if (achado == NULL) {
        printf("A matricula %d nao existe no cadastro.\n", matricula);
        return;
    }
    printf("Curso atual de \"%s\": %s\n", achado->dado.nome, achado->dado.curso);
    if (!lerTexto("Novo curso: ", novoCurso)) return;
    strcpy(achado->dado.curso, novoCurso);
    printf("Curso atualizado.\n");
}

static void telaRemover(No **raiz)
{
    int matricula;
    int removido = 0;

    if (!lerInteiro("Matricula a remover: ", 1, INT_MAX, &matricula)) return;
    *raiz = removerNo(*raiz, matricula, &removido);
    if (removido) {
        printf("Aluno de matricula %d removido.\n", matricula);
    } else {
        printf("A matricula %d nao existe no cadastro.\n", matricula);
    }
}

static void mostrarMenu(void)
{
    printf("\n===== CADASTRO DE ALUNOS =====\n");
    printf("1 - Cadastrar\n");
    printf("2 - Buscar\n");
    printf("3 - Listar (ordem crescente)\n");
    printf("4 - Exibir a arvore\n");
    printf("5 - Alterar curso\n");
    printf("6 - Remover (cancelar matricula)\n");
    printf("0 - Sair\n");
}

int main(void)
{
    No *raiz = NULL;
    int opcao = -1;

    carregarArquivo(&raiz);

    while (opcao != 0 && !fimEntrada) {
        mostrarMenu();
        if (!lerInteiro("Opcao: ", 0, 99, &opcao)) {
            break;
        }
        printf("\n");
        switch (opcao) {
            case 1: telaCadastrar(&raiz); break;
            case 2: telaBuscar(raiz); break;
            case 3: telaListar(raiz); break;
            case 4: telaExibirArvore(raiz); break;
            case 5: telaAlterarCurso(raiz); break;
            case 6: telaRemover(&raiz); break;
            case 0: break;
            default: printf("Opcao invalida.\n"); break;
        }
    }

    if (gravarArquivo(raiz)) {
        printf("Dados gravados em %s.\n", NOME_ARQUIVO);
    }
    destruirArvore(raiz);
    printf("Ate logo!\n");
    return 0;
}
