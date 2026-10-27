# TP1 – Questão 2: Filas de mensagens POSIX

## 1. Decisões de projeto

**Número de filas.** Uma fila POSIX é um canal unidirecional "de fato" (qualquer processo com a fila aberta pode ler e escrever). Como A envia dados a B e B devolve dados a A, usei **duas filas**:

| Fila | Sentido | Conteúdo |
|---|---|---|
| `/fila-ligia-pedro-req`  | A → B | struct `{n1, n2, pid_a}` |
| `/fila-ligia-pedro-resp` | B → A | struct `{soma, pid_b}` |

Usar uma única fila para os dois sentidos é arriscado: um processo poderia consumir a própria mensagem que acabou de enviar.

**Mensagens.** Enviadas como `struct` binária de tamanho fixo (`mq_msgsize = sizeof(struct)`), evitando conversão texto↔inteiro.

**Criação.** Ambos os programas abrem as filas com `O_CREAT`, então a ordem de inicialização não importa.

**Fluxo.** A: sorteia → `mq_send` → `mq_receive` (bloqueia) → imprime. B: `mq_receive` → soma → `sleep(1)` → `mq_send`.

**Versão 2 (corrigida).** Descrita na seção 3. Códigos: `A.c`/`B.c` (v1) e `A2.c`/`B2.c` (v2).

## 2. Resultados observados (v1: duas filas compartilhadas)

Cada cenário rodou por ~6 s com as filas limpas (`mq_unlink`) antes de cada rodada. "Erradas" = linhas em que a soma impressa por A não corresponde a n1+n2.

| Cenário | Comportamento observado |
|---|---|
| 1 A, 1 B | Correto. Uma soma por segundo, sem erros. |
| 2 A, 1 B | **Resultados trocados** (ex.: `95 + 82 = 124`). B é serial, então o throughput total continua 1 soma/s, dividido entre os dois A. |
| 1 A, 2 B | Correto, mas o 2º B é inútil: A só envia a próxima requisição depois de receber a resposta, então só um B trabalha por vez. |
| 2 A, 2 B | **Intermitente**: em 4 execuções, 3 saíram sem erro e 1 teve 4 erros em 10 linhas. Throughput ~2 somas/s. |

**Por que erra.** Todos os A leem da *mesma* fila de respostas e `mq_receive` entrega a mensagem mais antiga ao primeiro leitor que estiver esperando. A resposta não carrega nenhuma informação sobre a quem pertence. Se A1 e A2 estão esperando e B responde à requisição de A1, o kernel pode entregar a resposta a A2. Com 2 A + 2 B o erro depende da temporização (qual B termina o `sleep` primeiro, qual A entra no `mq_receive` primeiro), por isso é intermitente. Isso é uma *condição de corrida* e seria um erro difícil de depurar em produção, porque nem sempre aparece.

## 3. Solução adequada (v2)

O problema é a fila de resposta ser compartilhada. A solução implementada:

1. Há uma única fila de requisições, compartilhada (`/fila-ligia-pedro-req2`): vários A escrevem, vários B leem. Isso é seguro e ainda **distribui o trabalho** entre os B (cada mensagem é entregue a um único B).
2. **Cada A cria sua própria fila de resposta**, com nome derivado do PID: `/fila-ligia-pedro-resp2-<pid>`.
3. A requisição inclui `pid_a`. B monta o nome da fila de resposta a partir dele, abre, envia a soma e fecha.
4. A remove a sua fila (`mq_unlink`) ao encerrar (tratamento de SIGINT/SIGTERM).

Resultado: nos 4 cenários (1A/1B, 2A/1B, 1A/2B, 2A/2B) **zero erros**, e na saída com 2A/2B se vê as requisições de cada A sendo atendidas por Bs diferentes (`B pid=484` e `B pid=483`).

Alternativas possíveis: manter uma fila de resposta única e usar um identificador na mensagem com filtragem pelo receptor (ruim: `mq_receive` não filtra, seria preciso re-enfileirar); ou usar um *tipo/prioridade* por cliente (`mq_receive` só entrega por prioridade, não por valor exato, então não resolve bem). A fila por cliente é a mais simples e robusta.

## 4. Observações práticas

- Filas POSIX **persistem** após o término dos processos. Mensagens esquecidas contaminam a execução seguinte (aconteceu durante os testes). Use `mq_unlink` / `rm /dev/mqueue/<fila>`; o programa `limpa` incluído faz isso.
- Compilar com `-lrt`.
- Nomes de fila devem começar com `/` e ser únicos. O prefixo personalizado `fila-ligia-pedro-` fica em `comum.h` (macro `PREFIXO`).
- O programa `limpa` varre `/dev/mqueue` e remove todas as filas com esse prefixo, inclusive as filas de resposta por PID que sobram quando um A2 é encerrado com `kill -9`.

## 5. Como executar

```
make
./B &            # v1; em outros terminais: ./A
./B2 &           # v2; em outros terminais: ./A2
./limpa          # remove as filas
```

## 6. Registrando os resultados

Exemplo para a v1 com 2 A e 1 B (as saídas ficam em `resultados/`):

```
mkdir -p resultados && ./limpa
./B >> resultados/v1_2A1B.txt &
./A >> resultados/v1_2A1B.txt &
./A >> resultados/v1_2A1B.txt &
sleep 6; kill %1 %2 %3; ./limpa
```
