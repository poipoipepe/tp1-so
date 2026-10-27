/* Processo B - versao 1 (filas compartilhadas, ingenua) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include "comum.h"

int main(void)
{
    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;

    attr.mq_msgsize = sizeof(requisicao_t);
    mqd_t req = mq_open(FILA_REQ, O_CREAT | O_RDONLY, 0666, &attr);
    attr.mq_msgsize = sizeof(resposta_t);
    mqd_t resp = mq_open(FILA_RESP, O_CREAT | O_WRONLY, 0666, &attr);
    if (req == (mqd_t)-1 || resp == (mqd_t)-1) { perror("mq_open"); return 1; }

    while (1) {
        requisicao_t r;
        resposta_t   s;

        if (mq_receive(req, (char *)&r, sizeof r, NULL) < 0) { perror("mq_receive"); break; }
        s.soma  = r.n1 + r.n2;
        s.pid_b = getpid();
        printf("[B pid=%d] recebi %d e %d\n", getpid(), r.n1, r.n2);
        fflush(stdout);
        sleep(1);                                   /* aguarda 1 segundo */
        if (mq_send(resp, (char *)&s, sizeof s, 0) < 0) { perror("mq_send"); break; }
    }
    return 0;
}
