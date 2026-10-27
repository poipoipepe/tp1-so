/*
 * Trabalho Pratico 1 - Sistemas Operacionais - CEFET-MG
 * Questao 1: Calculo de pi com POSIX Threads (serie de Leibniz-Gregory)
 *
 *   pi/4 = soma_{n=0}^{inf} (-1)^n / (2n + 1)
 *
 * Uso:   ./pi_threads <num_threads> [num_termos]
 *        num_termos padrao = 1.000.000.000 (10^9)
 *
 * Compilacao: gcc -O2 -Wall -o pi_threads pi_threads.c -lpthread -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define TERMOS_PADRAO 1000000000LL   /* 10^9 termos */
#define MAX_THREADS   1024

/* ---------- Variaveis globais (somente leitura dentro das threads) ---------- */
static long long n_termos;            /* total de termos da serie      */
static int       n_threads;           /* numero de threads             */
static double    result[MAX_THREADS]; /* result[i] = soma parcial de T[i] */

/* Argumento de cada thread: apenas seu indice */
typedef struct {
    int id;
} thread_arg_t;

/*
 * Funcao executada por cada thread.
 * O intervalo [0, n_termos) e dividido uniformemente: cada thread recebe
 * n_termos / n_threads termos, e as primeiras (n_termos % n_threads)
 * threads recebem 1 termo extra, para cobrir o resto da divisao.
 */
static void *soma_parcial(void *arg)
{
    /* Copia das globais para variaveis locais (pilha da thread) */
    const int       id    = ((thread_arg_t *) arg)->id;
    const long long total = n_termos;
    const int       nt    = n_threads;

    const long long base  = total / nt;
    const long long resto = total % nt;

    /* Calculo do intervalo [inicio, fim) desta thread */
    const long long inicio = id * base + (id < resto ? id : resto);
    const long long fim    = inicio + base + (id < resto ? 1 : 0);

    /* Sinal do primeiro termo: (-1)^inicio */
    double sinal = (inicio % 2 == 0) ? 1.0 : -1.0;
    double soma  = 0.0;

    for (long long k = inicio; k < fim; k++) {
        soma  += sinal / (2.0 * (double) k + 1.0);
        sinal  = -sinal;
    }

    /* Escrita unica na posicao exclusiva do vetor global: sem sincronizacao */
    result[id] = soma;
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <num_threads> [num_termos]\n", argv[0]);
        return EXIT_FAILURE;
    }

    n_threads = atoi(argv[1]);
    n_termos  = (argc >= 3) ? atoll(argv[2]) : TERMOS_PADRAO;

    if (n_threads < 1 || n_threads > MAX_THREADS) {
        fprintf(stderr, "Erro: num_threads deve estar entre 1 e %d\n", MAX_THREADS);
        return EXIT_FAILURE;
    }
    if (n_termos < n_threads) {
        fprintf(stderr, "Erro: num_termos deve ser >= num_threads\n");
        return EXIT_FAILURE;
    }

    pthread_t    threads[MAX_THREADS];
    thread_arg_t args[MAX_THREADS];

    /* Criacao das N threads */
    for (int i = 0; i < n_threads; i++) {
        args[i].id = i;
        if (pthread_create(&threads[i], NULL, soma_parcial, &args[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    /* Espera o termino de todas as threads */
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
    }

    /* Thread principal soma os resultados parciais */
    double soma_total = 0.0;
    for (int i = 0; i < n_threads; i++)
        soma_total += result[i];

    double pi = 4.0 * soma_total;

    printf("Threads : %d\n", n_threads);
    printf("Termos  : %lld\n", n_termos);
    printf("Pi      : %.15f\n", pi);
    printf("Erro    : %.3e\n", fabs(pi - M_PI));

    return EXIT_SUCCESS;
}
