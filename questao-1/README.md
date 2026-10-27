# TP1 – Sistemas Operacionais (CEFET-MG) – Cálculo de π com POSIX Threads

Cálculo de π pela série de Leibniz-Gregory, `π/4 = Σ (-1)^n / (2n+1)`, com 10⁹ termos divididos uniformemente entre N threads POSIX.

## Decisões de implementação

- Cada thread calcula a soma parcial de um intervalo contíguo `[início, fim)`. Se `termos % N ≠ 0`, as primeiras threads recebem um termo a mais.
- Cada thread grava seu resultado apenas em `result[i]`, então não há mutex nem outra sincronização. A thread principal faz `pthread_join` e soma os parciais.
- As variáveis globais são copiadas para variáveis locais (pilha da thread) no início, e o resultado é escrito na global só no final.
- O sinal `(-1)^n` é alternado a cada iteração em vez de usar `pow()`, que seria muito mais lento.
- Todos os cálculos usam `double`.

## Compilação

```bash
make
# ou: gcc -O2 -Wall -o pi_threads pi_threads.c -lpthread -lm
```

## Execução

```bash
./pi_threads 4                # 4 threads, 10^9 termos
time ./pi_threads 4           # com medição de tempo
./pi_threads 8 100000000      # 8 threads, 10^8 termos
```

## Medição automática

```bash
chmod +x benchmark.sh
./benchmark.sh maquina1       # todos os núcleos
./benchmark.sh mono 0         # restrito a 1 núcleo (taskset -c 0)
```

Executa 1, 2, 4, 8, 16 e 32 threads, 5 vezes cada, e gera em `resultados/`:

| Arquivo | Conteúdo |
|---|---|
| `<plat>_brutos.csv` | todas as medições (real, user, sys) |
| `<plat>_resumo.csv` | média, desvio-padrão e CV (%) |
| `<plat>_info.txt` | configuração da máquina (CPU, núcleos, memória) |

Medições com CV > 5% aparecem como `REFAZER`. Antes de medir, feche outros programas e confira a carga com `top`.

## Ambiente

Linux ou WSL (Ubuntu) com `gcc` e `make` (`sudo apt install build-essential`).
