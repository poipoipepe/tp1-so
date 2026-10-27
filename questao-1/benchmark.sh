#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# Mede o tempo de execucao de pi_threads para 1, 2, 4, 8, 16 e 32 threads,
# 5 repeticoes cada (comando time do UNIX), e calcula media e coeficiente de
# variacao (CV = desvio-padrao amostral / media * 100%).
#
# Uso:
#   ./benchmark.sh <nome_plataforma> [cpus]
#
# Exemplos:
#   ./benchmark.sh maquina1              # usa todos os nucleos
#   ./benchmark.sh mono 0                # restringe ao nucleo 0 (taskset)
#
# Saidas (pasta resultados/):
#   <plataforma>_brutos.csv   -> todas as medicoes
#   <plataforma>_resumo.csv   -> media, desvio e CV por numero de threads
#   <plataforma>_info.txt     -> configuracao da maquina
# ---------------------------------------------------------------------------

set -euo pipefail

PLAT="${1:?Informe o nome da plataforma. Ex: ./benchmark.sh maquina1}"
CPUS="${2:-}"
THREADS=(1 2 4 8 16 32)
REPS=5
BIN=./pi_threads
OUT=resultados

[ -x "$BIN" ] || make
mkdir -p "$OUT"

if [ -n "$CPUS" ]; then
    PREFIX="taskset -c $CPUS"
else
    PREFIX=""
fi

# ---- Informacoes da maquina ----
{
    echo "Plataforma : $PLAT"
    echo "Data       : $(date)"
    echo "Restricao  : ${CPUS:-nenhuma (todos os nucleos)}"
    echo "Kernel     : $(uname -srm)"
    echo "Nucleos    : $(nproc --all)"
    echo
    lscpu 2>/dev/null | grep -E "Model name|^CPU\(s\)|Thread\(s\) per core|Core\(s\) per socket|Socket\(s\)|MHz|L1d|L2|L3" || true
    echo
    grep MemTotal /proc/meminfo 2>/dev/null || true
    echo "Carga inicial: $(uptime)"
} > "$OUT/${PLAT}_info.txt"

echo ">> Carga atual da maquina: $(uptime | sed 's/.*load average/load average/')"
echo ">> Se a carga estiver alta, feche outros programas (confira com 'top')."
echo

BRUTO="$OUT/${PLAT}_brutos.csv"
echo "plataforma,threads,repeticao,real_s,user_s,sys_s" > "$BRUTO"

TIMEFORMAT='%R %U %S'
for t in "${THREADS[@]}"; do
    for r in $(seq 1 $REPS); do
        # 'time' (builtin do bash) escreve em stderr; descartamos a saida do programa
        tempos=$( { time $PREFIX "$BIN" "$t" > /dev/null; } 2>&1 )
        read -r real user sys <<< "$tempos"
        echo "$PLAT,$t,$r,$real,$user,$sys" >> "$BRUTO"
        printf "threads=%-2d rep=%d  real=%ss user=%ss sys=%ss\n" "$t" "$r" "$real" "$user" "$sys"
    done
done

# ---- Resumo: media, desvio-padrao amostral e CV ----
RESUMO="$OUT/${PLAT}_resumo.csv"
awk -F, 'NR > 1 {
    n[$2]++; s[$2] += $4; q[$2] += $4 * $4
} END {
    print "threads,media_s,desvio_s,cv_pct,status"
    for (t in n) {
        m  = s[t] / n[t]
        v  = (q[t] - n[t] * m * m) / (n[t] - 1); if (v < 0) v = 0
        dp = sqrt(v)
        cv = (m > 0) ? dp / m * 100 : 0
        printf "%d,%.4f,%.4f,%.2f,%s\n", t, m, dp, cv, (cv <= 5 ? "OK" : "REFAZER")
    }
}' "$BRUTO" | (read -r h; echo "$h"; sort -t, -k1 -n) > "$RESUMO"

echo
echo "===== Resumo ($PLAT) ====="
column -t -s, "$RESUMO" 2>/dev/null || cat "$RESUMO"
echo
echo "Arquivos gerados em $OUT/"
if grep -q REFAZER "$RESUMO"; then
    echo "ATENCAO: ha medicoes com CV > 5%. Feche outros programas e rode novamente."
fi
