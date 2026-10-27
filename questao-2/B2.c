/* Processo B - versao 2 (corrigida): responde na fila privada do A solicitante */
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
    mqd_t req = mq_open(FILA_REQ2, O_CREAT | O_RDONLY, 0666, &attr);
    if (req == (mqd_t)-1) { perror("mq_open"); return 1; }

    while (1) {
        requisicao_t r;
        resposta_t   s;
        char nome_resp[64];

        if (mq_receive(req, (char *)&r, sizeof r, NULL) < 0) { perror("mq_receive"); break; }
        printf("[B pid=%d] recebi %d e %d de A pid=%d\n", getpid(), r.n1, r.n2, r.pid_a);
        fflush(stdout);

        s.soma  = r.n1 + r.n2;
        s.pid_b = getpid();
        sleep(1);                                   /* aguarda 1 segundo */

        snprintf(nome_resp, sizeof nome_resp, FMT_RESP2, r.pid_a);
        mqd_t resp = mq_open(nome_resp, O_WRONLY);
        if (resp == (mqd_t)-1) continue;            /* A ja terminou */
        mq_send(resp, (char *)&s, sizeof s, 0);
        mq_close(resp);
    }
    return 0;
}
