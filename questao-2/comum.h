#ifndef COMUM_H
#define COMUM_H
#include <sys/types.h>

/* Prefixo personalizado para evitar conflito com filas de outros alunos */
#define PREFIXO   "fila-ligia-pedro-"

/* Versao 1: duas filas fixas e compartilhadas */
#define FILA_REQ  "/" PREFIXO "req"     /* A -> B */
#define FILA_RESP "/" PREFIXO "resp"    /* B -> A */

/* Versao 2: fila de requisicoes compartilhada + 1 fila de resposta por A */
#define FILA_REQ2 "/" PREFIXO "req2"
#define FMT_RESP2 "/" PREFIXO "resp2-%d"   /* %d = pid do processo A */

typedef struct {
    int   n1, n2;
    pid_t pid_a;          /* usado somente na versao 2 */
} requisicao_t;

typedef struct {
    int   soma;
    pid_t pid_b;          /* quem calculou (informativo) */
} resposta_t;

#endif
