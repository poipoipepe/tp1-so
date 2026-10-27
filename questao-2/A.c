/* Processo A - versao 1 (filas compartilhadas, ingenua) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <mqueue.h>
#include "comum.h"

int main(void)
{
    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;

    /* estabelece o canal de comunicacao */
    attr.mq_msgsize = sizeof(requisicao_t);
    mqd_t req = mq_open(FILA_REQ, O_CREAT | O_WRONLY, 0666, &attr);
    attr.mq_msgsize = sizeof(resposta_t);
    mqd_t resp = mq_open(FILA_RESP, O_CREAT | O_RDONLY, 0666, &attr);
    if (req == (mqd_t)-1 || resp == (mqd_t)-1) { perror("mq_open"); return 1; }

    srandom(time(NULL) ^ getpid());
    while (1) {
        requisicao_t r = { random() % 100, random() % 100, getpid() };
        resposta_t   s;

        if (mq_send(req, (char *)&r, sizeof r, 0) < 0) { perror("mq_send"); break; }
        if (mq_receive(resp, (char *)&s, sizeof s, NULL) < 0) { perror("mq_receive"); break; }

        printf("[A pid=%d] %d + %d = %d%s\n", getpid(), r.n1, r.n2, s.soma,
               (r.n1 + r.n2 == s.soma) ? "" : "   <-- RESULTADO ERRADO (soma de outro A!)");
        fflush(stdout);
    }
    return 0;
}
