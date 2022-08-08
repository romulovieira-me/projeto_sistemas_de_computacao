/* 
Trabalho Prático - Sistemas de Computação 
Rômulo Vieira & Rodrigo Dracxler
Universidade Federal Fluminense 
2022.1

REF
https://docs.oracle.com/cd/E19504-01/802-5938/6i9lnfe4p/index.html
http://faculty.cs.niu.edu/~hutchins/csci480/semaphor.htm
https://pubs.opengroup.org/onlinepubs/007908799/xsh/sem_wait.html

*/


// Declarando bibliotecas
#include <stdio.h> // biblioteca que permite implementar subrotinas relativas às operações de entrada/saída
#include <string.h> // biblioteca que permite a manipulação de cadeias de caracteres e regiões de memória
#include <stdlib.h> // biblioteca que permite alocação de memória, controle de processos e conversões
#include <pthread.h> // biblioteca padrão para usar threads em sistemas POSIX
#include <semaphore.h> // biblioteca que permite o uso de semáforos 
#include <stdbool.h> // biblioteca que permite manipular variáveis lógicas 

// Definindo constantes
#define SHARED_SIZE 4       // Número de shared (memória compartilhada)
#define BUFF_SIZE 5       // Tamanho do buffer em cada Memória compartilhada
#define NP          1       // Número de threads Produtoras
#define NCP1        5       // Número de threads Consumidoras & Produtoras 1
#define NCP2        4       // Número de threads Consumidoras & Produtoras 2
#define NCP3        3       // Número de threads Consumidoras & Produtoras 3
#define NC          1       // Número de threads Consumidoras
#define NITERS      50      // Número de itens produzidos/consumidos

bool keep_running = true;   // Analisa se os processos ainda estão rodando. Caso seja falso, eles encerram.

// typedef é usado para criar um "alias" para tipos de dados existentes
typedef struct {
    char Nome[FILENAME_MAX];    // Nome do arquivo de entrada
    double A[10][10];           // Matriz A (Entrada)
    double B[10][10];           // Matriz B (Entrada)
    double C[10][10];           // Matriz C (A * B)
    double V[10];               // Vetor com a soma dos elementos das colunas da Matriz C
    double E;                   // Soma dos elementos de V
} S; // inciializando buffer do semáforo

/* Este trecho foi baseado neste código: http://www.csc.villanova.edu/~mprobson/courses/sp21-csc2405/PC.htm */
typedef struct {
    S *buffer[BUFF_SIZE];       // buffer do semáforo
    sem_t full;                 // Semáforo dos itens armazenados
    sem_t empty;                // Semáforo dos espaços livres
    sem_t mutex;                // Semáforo que permite que apenas uma thread acesse os dados no buffer por vez
    int in;                     // Local de inserção no buffer --> buff[in%BUFF_SIZE] is the first empty slot
    int out;                     // Local de remoção no buffer --> buff[out%BUFF_SIZE] is the first full slot
} compartilhada; // buffer da memória compartilhada


compartilhada shared[BUFF_SIZE]; // inicializando buffer da memória compartilhada

void* produtor(void *arg){ // ponteiro para a classe "produtor"
    int index = *((int *)arg); // definindo índice para classe 

   
    if (!keep_running) { // se o processo ainda está em execução
        return NULL; // indica que a função não retorna nenhum valor
    }

    // Abrindo o arquivo "entrada.in"
    // baseado neste código: https://www.cs.utah.edu/~germain/PPS/Topics/C_Language/file_IO.html
    FILE *entrada = fopen("entrada.in", "r");

    // condição para erro no arquivo "entrada.in"
    if (entrada == NULL) { // se arquivo de entrada não retornar valores
        printf("Erro ao abrir o arquivo\n"); // imprime mensagem de erro
        fclose(entrada); // fecha o arquivo aberto pela função fopen
        exit(EXIT_FAILURE); // retorna código de erro
    }

    char file_name[100]; // permite leitura de arquivos maiores que 100 caracteres
    while (fgets(file_name, sizeof(file_name), entrada)) { // Calcula quantos bytes o arquivo ocupa na memória e lê os "n" caracteres deste arquivo
        char *clean_name = strtok(file_name, "\n"); // removendo a quebra de linha ("\n") do nome.

        // Alocação dinâmica de memória do buffer de semáforo 
        S *s = (S *) malloc(sizeof(S)); // diz quantos bytes o objeto em questão tem
        strcpy((*s).Nome, clean_name); // copia o valor da string na segunda posição do argumento para uma variável string na primeira posição

        
        int linha_in = 0; // ponteiro para as linhas das matrizes
        int coluna_in = 0; // ponteiro para a coluna das matrizes
        double (*matriz)[10][10] = &((*s).A); // ponteiro que indica se os dados são da Matriz de entrada A ou Matriz de entrada B

        // abrindo arquivo das matrizes.
        FILE *arquivo_matriz = fopen(clean_name, "r");

        // condição para erro no arquivo "entrada.in"
        if (arquivo_matriz == NULL) { // se arquivo de entrada não retornar valores
            printf("Erro ao abrir o arquivo\n", clean_name); // imprime mensagem de erro
            fclose(arquivo_matriz); // fecha o arquivo aberto pela função fopen
            exit(EXIT_FAILURE); // retorna código de erro
        }

        // Lendo as linhas do arquivo com a Matriz
        char linha_matriz[120];
        while(fgets(linha_matriz, sizeof(linha_matriz), arquivo_matriz)){ // pega linhas das matrizes do arquivo_matriz
            if (!strcmp(linha_matriz, "\n")) { // strcmp compara duas strings. Neste caso, se "linha_matriz" = \n, começa preencimento da matriz B
                matriz = &((*s).B); // preenchimento de B
                linha_in = 0; // linhas do arquivo
                coluna_in = 0; // colunas do arquivo
                continue; // leva o controle do programa para o início do loop 
            }

            // Separando os elementos da matriz e salvando em s
            char *item = strtok(linha_matriz, ","); // strtok devolve ponteiro para próximo item da matriz
            while (item != NULL) { // enquanto item diferente de NULL
                double num; // declarando num
                sscanf(item, "%lf", &num); // caracteres de entrada são recebidos de item; %lf indica que são do tipo double; &num aponta para primeiro elemento da matriz
                item = strtok(NULL, ","); // item da matriz apontar para NULL
                (*matriz)[linha_in][coluna_in] = num; // ponteiro da coluna e linha da matriz aponta para num
                coluna_in++; // incrementa coluna
            }
            coluna_in = 0; // colunas do arquivo
            linha_in++; // incrementa linha
        }

        fclose(arquivo_matriz); // fecha arquivo aberto pela função fopen na linha 90 

        // Acesso a memória compartilhada
        // sem_wait bloqueia o semáforo 
        // veja mais em: https://pubs.opengroup.org/onlinepubs/007908799/xsh/sem_wait.html
        sem_wait(&shared[0].empty);
        sem_wait(&shared[0].mutex);

        printf("[P-%d] Produzindo...\n", index); // Imprime mensagem de produção e indíce do produtor
        shared[0].buffer[shared[0].in] = s; // posição inicial do buffer será salvo em "s"
        shared[0].in = (shared[0].in + 1) % SHARED_SIZE; // módulo da segunda posição da memória compartilhada pelo tamanho da memória compartilhada

        // avança o semáforo
        // veja mais em: https://pubs.opengroup.org/onlinepubs/009695399/functions/sem_post.html
        sem_post(&shared[0].mutex);
        sem_post(&shared[0].full);
    }
    fclose(entrada); // fecha arquivo de entrada

    return NULL; // retorna NULL, ou seja, nenhum valor númerico 
}

// classe do CP1
void* consumidor_produtor1(void *arg)
{
    int index = *((int *)arg); // definindo índice do CP1

    while (keep_running) { // enquanto processo tiver rodando...
        // pausa semáforo
        sem_wait(&shared[0].full); // memória compartilhada está cheia
        sem_wait(&shared[0].mutex); // enquanto o buffer é preenchido pelo produtor, o consumidor precisa esperar. E vice-versa

        printf("[CP1-%d] Consumindo...\n", index); // mensagem e índice de consumidor
        S *temp = (S *) malloc(sizeof(S)); // Copiando informações para espaço temporário
        *temp = *(shared[0].buffer[shared[0].out]);
        // para i e j da matriz menor que 10
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                temp->C[i][j] = 0; // matriz C será obtida do espaço temporário temp
                for (int k = 0; k < 10; k++) {
                    temp->C[i][j] += temp->A[i][k] * temp->B[k][j]; // Matriz C = A * B
                }
            }
        }

        shared[0].out = (shared[0].out + 1) % SHARED_SIZE; // módulo da segunda posição da memória compartilhada pelo tamanho da memória compartilhada
        // avança o semáforo 
        sem_post(&shared[0].mutex);
        sem_post(&shared[0].empty);

        // pausa semáforo para produzir
        sem_wait(&shared[1].empty);
        sem_wait(&shared[1].mutex);

        printf("[CP1-%d] Produzindo...\n", index); // mensagem que CP1 está produzindo
        shared[1].buffer[shared[1].in] = temp; // salva valores de CP1 no temp 

        // Incrementando o índice "in"
        shared[1].in = (shared[1].in + 1) % SHARED_SIZE; // módulo da segunda posição da memória compartilhada pelo tamanho da memória compartilhada
        // avança o semáforo 
        sem_post(&shared[1].mutex); // produz no espaço de memeória compartilhada [1]
        sem_post(&shared[1].full);
    }

    return NULL;
}

// classe do CP2
void* consumidor_produtor2(void *arg)
{
    int index = *((int *)arg);

    while (keep_running) {
        // Consumindo
        sem_wait(&shared[1].full);
        sem_wait(&shared[1].mutex);

        printf("[CP2-%d] Consumindo...\n", index);
        // Alocação dinamica do buffer consumido
        S *temp = (S *) malloc(sizeof(S));
        *temp = *(shared[1].buffer[shared[1].out]);

        // Inicializando os elementos de V
        for (int i = 0; i < 10; ++i) {
            temp->V[i] = 0.0; // inicia com float 0.0
        }

        // somando as colunas da matriz C
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                temp->V[j] = temp->V[j] + temp->C[i][j];
            }
        }

        // Incrementando out (indice de lista circular)
        shared[1].out = (shared[1].out + 1) % SHARED_SIZE;

        sem_post(&shared[1].mutex);
        sem_post(&shared[1].empty);

        // Produzindo
        sem_wait(&shared[2].empty);
        sem_wait(&shared[2].mutex);

        printf("[CP2-%d] Produzindo...\n", index);
        
        shared[2].buffer[shared[2].in] = temp; // Salvando CP2 na variável temporária.

        // incrementando o indice in de shared[2]
        shared[2].in = (shared[2].in + 1) % SHARED_SIZE;

        sem_post(&shared[2].mutex);
        sem_post(&shared[2].full);
    }

    return NULL;
}

// classe do CP3
void* consumidor_produtor3(void *arg)
{
    int index = *((int *)arg);

    while (keep_running) {
        // Consumindo
        sem_wait(&shared[2].full);
        sem_wait(&shared[2].mutex);

        printf("[CP3-%d] Consumindo...\n", index);
        // Alocação dinâmica do buffer consumidor
        S *temp = (S *) malloc(sizeof(S));
        *temp = *(shared[2].buffer[shared[2].out]); // salvando espaço de memória compartilhada na variável temp

        // inicializando E
        temp->E = 0.0;
        for (int i = 0; i < 10; ++i) {
            temp->E = temp->E + temp->V[i];
        }

        // Incrementando o indice "out"
        shared[2].out = (shared[2].out + 1) % SHARED_SIZE;

        sem_post(&shared[2].mutex);
        sem_post(&shared[2].empty);

        // Produzindo
        printf("[CP3-%d] Produzindo...\n", index);
        sem_wait(&shared[3].empty);
        sem_wait(&shared[3].mutex);

        shared[3].buffer[shared[3].in] = temp; // salvando espaço de memória compartilhada na variável temp

        // Incrementando o indice "out"
        shared[3].in = (shared[3].in + 1) % SHARED_SIZE;

        sem_post(&shared[3].mutex);
        sem_post(&shared[3].full);
    }

    return NULL;
}

// criando classe do consumidor
void* consumidor(void *arg)
{
    int index = *((int *)arg);

    // Conta o número de itens consumidos
    int contador_de_consumidos = 0;

    while(keep_running)
    {
        sem_wait(&shared[3].full);
        sem_wait(&shared[3].mutex);

        printf("[C-%d] Consumindo...\n", index);
        // Alocação dinâmica do buffer consumido
        S temp = *(shared[3].buffer[shared[3].out]);

        // Abrindo o arquivo "saida.out" para escrita
        FILE *arquivo_saida = fopen("saida.out", "a"); // a = append

        // Verificando se houve erro na abertura do arquivo.
        if (arquivo_saida == NULL) {
            printf("Erro ao abrir o arquivo\n");
            fclose(arquivo_saida);
            exit(EXIT_FAILURE);
        }

        /* Escrevendo em "saida.out" */
        // Cabeçalho conforme exigido na descrição do trabalho
        fprintf(arquivo_saida, "==============================\n");
        fprintf(arquivo_saida, "Entrada: %s;\n", temp.Nome);
        fprintf(arquivo_saida, "- - - - - - - - -\n");

        // Escrevendo a matriz A no arquivo de saída.
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                fprintf(arquivo_saida, "%f ", temp.A[i][j]);
            }
            fprintf(arquivo_saida, "\n");
        }
        fprintf(arquivo_saida, "- - - - - - - - -\n");

        // Escrevendo a matriz B no arquivo de saída.
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                fprintf(arquivo_saida, "%f ", temp.B[i][j]);
            }
            fprintf(arquivo_saida, "\n");
        }
        fprintf(arquivo_saida, "- - - - - - - - -\n");

        // Escrevendo a matriz C no arquivo de saída.
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                fprintf(arquivo_saida, "%f ", temp.C[i][j]);
            }
            fprintf(arquivo_saida, "\n");
        }
        fprintf(arquivo_saida, "- - - - - - - - -\n");

        // Escrevendo V no arquivo de saída [vetor com 10 elementos double]
        for (int i = 0; i < 10; ++i) {
            fprintf(arquivo_saida, "%f\n", temp.V[i]);
        }
        fprintf(arquivo_saida, "- - - - - - - - -\n");

        // Escrevendo E no arquivo de saída [variável double]
        fprintf(arquivo_saida, "%f\n", temp.E);
        fprintf(arquivo_saida, "==============================\n");

        // Fechando o arquivo.
        fclose(arquivo_saida);

        // Incrementando o índice "out"
        shared[3].out = (shared[3].out + 1) % SHARED_SIZE;

        sem_post(&shared[3].mutex);
        sem_post(&shared[3].empty);

        // Incrementando o contador de itens consumidos
        contador_de_consumidos++;

        if (contador_de_consumidos >= NITERS) { // se todos os itens forem consumidos
            keep_running = false; // altera o keep_running para false informando que as outras threads que consumiram todos os processos
            break;
        }
    }

    return NULL;
}

int main()
{
    // Criação dos identificadores das threads
    pthread_t idP, idCP1, idCP2, idCP3, idC;
    int sP, sCP1, sCP2, sCP3,sC ;
    sP = 0;
    sCP1 = 0;
    sCP2 = 0;
    sCP3 = 0;
    sC = 0;


    // Inicialização dos Semáforos e índices
    for (int i = 0; i < BUFF_SIZE; i++) {
        // Inicialização do semáforo full:
        sem_init(&shared[i].full, 0, 0);

        // Inicialização do semáforo empty:
        sem_init(&shared[i].empty, 0, BUFF_SIZE);

        // Inicialização do semáforo mutex:
        sem_init(&shared[i].mutex,  0, 1);

        // Inicialização do índice do arquivo entrada.in:
        shared[i].in = 0;

        // Inicialização do índice do arquivo saida.out:
        shared[i].out = 0;
    }

    // Criação das Threads
    // inspirado nos arquivos de thread apresentados nas aulas
    pthread_create(&idP, NULL, &produtor, &sP); // Criando P

    // Criando CP1
    for (int i = 0; i < NCP1; ++i) {
        pthread_create(&idCP1, NULL, &consumidor_produtor1, &sCP1);
        sCP1++;
    }

    // Criando CP2
    for (int i = 0; i < NCP2; ++i) {
        pthread_create(&idCP2, NULL, &consumidor_produtor2, &sCP2);
        sCP2++;
    }

    // Criando CP3
    for (int i = 0; i < NCP3; ++i) {
        pthread_create(&idCP3, NULL, &consumidor_produtor3, &sCP3);
        sCP3++;
    }

    // Criando thread da matriz C
    pthread_create(&idC, NULL, &consumidor, &sC);


    // Aplicação espera o processo Consumidor terminar
    pthread_join(idC, NULL);

    return 0;
}
