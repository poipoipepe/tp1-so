/* Processo A - versao 2 (corrigida): cada A tem sua propria fila de resposta */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>
#include "comum.h"

static volatile sig_atomic_t parar = 0;
static void trata_sinal(int s) { (void)s; parar = 1; }

int main(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = trata_sinal;          /* sem SA_RESTART: interrompe mq_receive */
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    char nome_resp[64];
    snprintf(nome_resp, sizeof nome_resp, FMT_RESP2, getpid());

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(requisicao_t);
    mqd_t req = mq_open(FILA_REQ2, O_CREAT | O_WRONLY, 0666, &attr);
    attr.mq_msgsize = sizeof(resposta_t);
    mqd_t resp = mq_open(nome_resp, O_CREAT | O_RDONLY, 0600, &attr);
    if (req == (mqd_t)-1 || resp == (mqd_t)-1) { perror("mq_open"); return 1; }

    srandom(time(NULL) ^ getpid());
    while (!parar) {
        requisicao_t r = { random() % 100, random() % 100, getpid() };
        resposta_t   s;

        if (mq_send(req, (char *)&r, sizeof r, 0) < 0) break;
        if (mq_receive(resp, (char *)&s, sizeof s, NULL) < 0) break;

        printf("[A pid=%d] %d + %d = %d   (calculado por B pid=%d)%s\n",
               getpid(), r.n1, r.n2, s.soma, s.pid_b,
               (r.n1 + r.n2 == s.soma) ? "" : "   <-- ERRADO");
        fflush(stdout);
    }
    /* limpeza: remove a fila privada deste A */
    mq_close(req); mq_close(resp); mq_unlink(nome_resp);
    return 0;
}
