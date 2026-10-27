/* Remove todas as filas deste trabalho (prefixo definido em comum.h),
 * inclusive as filas de resposta por PID da versao 2 que sobram quando
 * um processo A2 e encerrado com kill -9. */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <mqueue.h>
#include "comum.h"

int main(void)
{
    DIR *dir = opendir("/dev/mqueue");
    if (dir == NULL) { perror("opendir /dev/mqueue"); return 1; }

    struct dirent *e;
    int removidas = 0;
    while ((e = readdir(dir)) != NULL) {
        if (strncmp(e->d_name, PREFIXO, strlen(PREFIXO)) != 0)
            continue;
        char nome[300];
        snprintf(nome, sizeof nome, "/%s", e->d_name);
        if (mq_unlink(nome) == 0) {
            printf("removida: %s\n", nome);
            removidas++;
        } else {
            perror(nome);
        }
    }
    closedir(dir);
    printf("%d fila(s) removida(s)\n", removidas);
    return 0;
}
