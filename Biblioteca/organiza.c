#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definição das dimensões das estantes e prateleiras
#define LARGURA_ESTANTE 96
#define ALTURA_ESTANTE 188
#define PROFUNDIDADE_ESTANTE 32
#define PRATELEIRAS 6
#define ALTURA_PRATELEIRA (ALTURA_ESTANTE / PRATELEIRAS)

typedef struct Livro{
    char titulo[100];
    char autor[100];
    int largura;
    int altura;
    int profundidade;
} Livro;

// Estrutura de nó para lista duplamente encadeada
typedef struct Node{
    Livro livro;
    struct Node* prev;
    struct Node* next;
} Node;

// Declarações de funções
Node* criarNode(Livro livro);
void inserirNoFinal(Node** head, Livro livro);
int calcularVolume(Livro livro);
Node* dividir(Node* head);
Node* merge(Node* esquerda, Node* direita);
Node* mergeSort(Node* head);
Node* lerLivros(FILE* arquivo);
void organizarLivros(Node* lista, FILE* saida);
void adicionarLivro(const char *filename, Livro livro);
void removerLivro(const char *filename, const char *titulo);
void consultarLivro(Node* lista, const char* titulo, const char* autor);
void exibirMenu();

// Funções (implementações das funções já existentes na sua versão original)
Node* criarNode(Livro livro){
    Node* novo = (Node*)malloc(sizeof(Node));
    novo->livro = livro;
    novo->prev = NULL;
    novo->next = NULL;
    return novo;
}

void inserirNoFinal(Node** head, Livro livro){
    Node* novo = criarNode(livro);
    if (*head == NULL){
        *head = novo;
    } else{
        Node* temp = *head;
        while (temp->next != NULL){
            temp = temp->next;
        }
        temp->next = novo;
        novo->prev = temp;
    }
}

void freeList(Node* head){
    Node* current = head;
    Node* next;

    while (current != NULL) {
        next = current->next; // Armazena o próximo nó
        free(current);         // Libera o nó atual
        current = next;        // Avança para o próximo nó
    }
}

int calcularVolume(Livro livro){
    return livro.largura * livro.altura * livro.profundidade;
}

Node* dividir(Node* head){
    Node* slow = head;
    Node* fast = head;
    
    while (fast->next != NULL && fast->next->next != NULL){
        slow = slow->next;
        fast = fast->next->next;
    }
    
    Node* metade = slow->next;
    slow->next = NULL;
    
    return metade;
}

Node* merge(Node* esquerda, Node* direita){
    if (!esquerda) return direita;
    if (!direita) return esquerda;

    if (calcularVolume(esquerda->livro) <= calcularVolume(direita->livro)){
        esquerda->next = merge(esquerda->next, direita);
        esquerda->next->prev = esquerda;
        esquerda->prev = NULL;
        return esquerda;
    } else{
        direita->next = merge(esquerda, direita->next);
        direita->next->prev = direita;
        direita->prev = NULL;
        return direita;
    }
}

Node* mergeSort(Node* head){
    if (!head || !head->next) return head;

    Node* metade = dividir(head);

    Node* esquerda = mergeSort(head);
    Node* direita = mergeSort(metade);

    return merge(esquerda, direita);
}

Node* lerLivros(FILE* arquivo){
    Node* lista = NULL;
    char linha[256];
    Livro livro;
    
    while (fgets(linha, sizeof(linha), arquivo)){
        char *pos;
        if ((pos = strchr(linha, '\n')) != NULL)
            *pos = '\0';

        int campos_lidos = sscanf(linha, "ticulo=%99[^,], autor=%99[^,], largura=%dcm, altura=%dcm, profundidade=%dcm",
               livro.titulo, livro.autor, &livro.largura, &livro.altura, &livro.profundidade);

        if (campos_lidos == 5){
            inserirNoFinal(&lista, livro);
        } else{
            printf("Erro ao ler os campos do livro na linha: %s\n", linha);
        }
    }
    
    return lista;
}

void organizarLivros(Node* lista, FILE* saida){
    int estantes = 1;
    int prateleira_atual = 0;
    int largura_atual = 0;

    fprintf(saida, "Organização dos livros:\n");

    Node* temp = lista;
    while (temp != NULL){
        if (temp->livro.altura <= ALTURA_PRATELEIRA && 
            largura_atual + temp->livro.largura <= LARGURA_ESTANTE){
            
            fprintf(saida, "Livro: %s, Autor: %s, Estante: %d, Prateleira: %d\n", 
                    temp->livro.titulo, temp->livro.autor, estantes, prateleira_atual + 1);
            
            largura_atual += temp->livro.largura;
        } else{
            if (prateleira_atual < PRATELEIRAS - 1){
                prateleira_atual++;
                largura_atual = temp->livro.largura;
                fprintf(saida, "Livro: %s, Autor: %s, Estante: %d, Prateleira: %d\n", 
                        temp->livro.titulo, temp->livro.autor, estantes, prateleira_atual + 1);
            } else{
                estantes++;
                prateleira_atual = 0;
                largura_atual = temp->livro.largura;
                fprintf(saida, "Livro: %s, Autor: %s, Estante: %d, Prateleira: %d\n", 
                        temp->livro.titulo, temp->livro.autor, estantes, prateleira_atual + 1);
            }
        }
        temp = temp->next;
    }
}

void adicionarLivro(const char *filename, Livro livro){
    FILE *arquivo = fopen(filename, "a");
    if (arquivo == NULL){
        printf("Erro ao abrir o arquivo para adicionar o livro.\n");
        return;
    }
    fprintf(arquivo, "ticulo=%s, autor=%s, largura=%dcm, altura=%dcm, profundidade=%dcm\n", 
            livro.titulo, livro.autor, livro.largura, livro.altura, livro.profundidade);
    fclose(arquivo);
    printf("Livro '%s' adicionado com sucesso.\n", livro.titulo);
}

void removerLivro(const char *filename, const char *titulo){
    FILE *arquivo = fopen(filename, "r");
    if (arquivo == NULL){
        printf("Erro ao abrir o arquivo para leitura.\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL){
        printf("Erro ao criar arquivo temporário.\n");
        fclose(arquivo);
        return;
    }

    char linha[256];
    int encontrou = 0;

    while (fgets(linha, sizeof(linha), arquivo)){
        Livro livro;
        int campos_lidos = sscanf(linha, "ticulo=%99[^,], autor=%99[^,], largura=%dcm, altura=%dcm, profundidade=%dcm",
                                  livro.titulo, livro.autor, &livro.largura, &livro.altura, &livro.profundidade);

        if (campos_lidos == 5 && strcmp(livro.titulo, titulo) == 0){
            encontrou = 1;
            printf("Livro '%s' removido com sucesso.\n", titulo);
        } else{
            fputs(linha, temp);
        }
    }

    fclose(arquivo);
    fclose(temp);

    if (encontrou){
        remove(filename);
        rename("temp.txt", filename);
    } else{
        printf("Livro '%s' não encontrado.\n", titulo);
        remove("temp.txt");
    }
}

void consultarLivroNoArquivo(const char* filename, const char* titulo, const char* autor){
    FILE* arquivo = fopen(filename, "r");
    if (arquivo == NULL){
        printf("Erro ao abrir o arquivo de saída para consulta.\n");
        return;
    }

    char linha[256];
    int encontrado = 0;
    int linha_numero = 0;

    while (fgets(linha, sizeof(linha), arquivo)){
        linha_numero++;
        if (strstr(linha, titulo) != NULL && strstr(linha, autor) != NULL){
            printf("\nLivro encontrado: %s na linha %d do arquivo '%s'\n", linha, linha_numero, filename);
            encontrado = 1;
            break;
        }
    }

    fclose(arquivo);

    if (!encontrado){
        printf("Livro não encontrado: %s, Autor: %s\n", titulo, autor);
    }
}

void emprestarLivro(const char* filename, const char* titulo, const char* autor, const char* nomePessoa) {
    FILE* arquivo = fopen(filename, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de saída para leitura.\n");
        return;
    }

    FILE* emprestados = fopen("emprestados.txt", "a");
    if (emprestados == NULL) {
        printf("Erro ao abrir o arquivo de empréstimos.\n");
        fclose(arquivo);
        return;
    }

    FILE* temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Erro ao criar arquivo temporário.\n");
        fclose(arquivo);
        fclose(emprestados);
        return;
    }

    char linha[256];
    int encontrado = 0;

    while (fgets(linha, sizeof(linha), arquivo)) {
        Livro livro;
        int campos_lidos = sscanf(linha, "Livro: %99[^,], Autor: %99[^,]", livro.titulo, livro.autor);

        if (campos_lidos == 2 && strcmp(livro.titulo, titulo) == 0 && strcmp(livro.autor, autor) == 0) {
            encontrado = 1;
            fprintf(emprestados, "Titulo: %s, Autor: %s, Emprestado para: %s\n", livro.titulo, livro.autor, nomePessoa);
            printf("Livro '%s' emprestado com sucesso.\n", titulo);
        } else {
            fputs(linha, temp);
        }
    }

    fclose(arquivo);
    fclose(emprestados);
    fclose(temp);

    if (encontrado) {
        remove(filename);
        rename("temp.txt", filename);
    } else {
        printf("Livro não encontrado: %s, Autor: %s\n", titulo, autor);
        remove("temp.txt");
    }
}


// Função para exibir o menu
void exibirMenu(){
    printf("Menu:\n");
    printf("1. Organizar livros\n");
    printf("2. Inserir livro\n");
    printf("3. Remover livro\n");
    printf("4. Consultar livro\n");
    printf("5. Emprestar livro\n");
    printf("6. Sair\n");
}

int main(){
    const char *filename = "lista.txt";
    Livro novoLivro;

    while (1){
        exibirMenu();
        int opcao;
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        getchar(); // Limpa o buffer do teclado

        switch (opcao){
            case 1: // Organizar livros
          {
                FILE *arquivo = fopen(filename, "r");
                if (arquivo == NULL){
                    printf("Erro ao abrir o arquivo de entrada.\n");
                    break;
                }

                FILE *saida = fopen("saida_organizada.txt", "w");
                if (saida == NULL){
                    printf("Erro ao abrir o arquivo de saída.\n");
                    fclose(arquivo);
                    break;
                }

                Node* lista = lerLivros(arquivo);
                lista = mergeSort(lista);
                organizarLivros(lista, saida); //organizar os livros ja ordenados da lista encadeada nas estantes e prateleiras da biblioteca

                freeList(lista);
                fclose(arquivo);
                fclose(saida);
                printf("A organização dos livros foi salva no arquivo 'saida_organizada.txt'.\n");
            }
            break;

            case 2: // Inserir livro
                printf("Insira o título do livro: ");
                fgets(novoLivro.titulo, sizeof(novoLivro.titulo), stdin);
                novoLivro.titulo[strcspn(novoLivro.titulo, "\n")] = 0; // Remove nova linha

                printf("Insira o autor do livro: ");
                fgets(novoLivro.autor, sizeof(novoLivro.autor), stdin);
                novoLivro.autor[strcspn(novoLivro.autor, "\n")] = 0; // Remove nova linha

                printf("Insira a largura do livro (em cm): ");
                scanf("%d", &novoLivro.largura);
                printf("Insira a altura do livro (em cm): ");
                scanf("%d", &novoLivro.altura);
                printf("Insira a profundidade do livro (em cm): ");
                scanf("%d", &novoLivro.profundidade);
                getchar(); // Limpa o buffer do teclado

                adicionarLivro(filename, novoLivro);
                break;

            case 3: // Remover livro
          {
                char tituloRemover[100];
                printf("Insira o título do livro a ser removido: ");
                fgets(tituloRemover, sizeof(tituloRemover), stdin);
                tituloRemover[strcspn(tituloRemover, "\n")] = 0; // Remove nova linha

                removerLivro(filename, tituloRemover);
            }
            break;

            case 4: // Consultar livro
          {
                char tituloConsultar[100];
                char autorConsultar[100];

                printf("Insira o título do livro a ser consultado: ");
                fgets(tituloConsultar, sizeof(tituloConsultar), stdin);
                tituloConsultar[strcspn(tituloConsultar, "\n")] = 0; // Remove nova linha

                printf("Insira o autor do livro a ser consultado: ");
                fgets(autorConsultar, sizeof(autorConsultar), stdin);
                autorConsultar[strcspn(autorConsultar, "\n")] = 0; // Remove nova linha

                // Consultar no arquivo de saída organizado
                consultarLivroNoArquivo("saida_organizada.txt", tituloConsultar, autorConsultar);
            }
            break;

            case 5: // Emprestar livro
            {
                char tituloConsultar[100];
                char autorConsultar[100];
                char nomePessoa[100];

                printf("Insira o título do livro a ser emprestado: ");
                fgets(tituloConsultar, sizeof(tituloConsultar), stdin);
                tituloConsultar[strcspn(tituloConsultar, "\n")] = 0; // Remove nova linha

                printf("Insira o autor do livro a ser emprestado: ");
                fgets(autorConsultar, sizeof(autorConsultar), stdin);
                autorConsultar[strcspn(autorConsultar, "\n")] = 0; // Remove nova linha

                printf("Insira o nome de quem pegou emprestado: ");
                fgets(nomePessoa, sizeof(nomePessoa), stdin);
                nomePessoa[strcspn(nomePessoa, "\n")] = 0; // Remove nova linha

                // Consultar no arquivo de saída organizado e emprestar o livro
                emprestarLivro("saida_organizada.txt", tituloConsultar, autorConsultar, nomePessoa);
            }
             break;


            case 6: // Sair
                printf("Saindo do programa.\n");
                return 0;

            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    }

    return 0;
}