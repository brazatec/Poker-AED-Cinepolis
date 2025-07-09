#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "pilha.h"
#include "arvoreavl.h"

// Cores ANSI para personalização do terminal
#define ANSI_RESET   "\x1b[0m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_BOLD    "\x1b[1m"
#define ANSI_UNDERLINE "\x1b[4m"

#define MAX_LOG 500

typedef struct {
    float saldo;
    char nome[20];
    cartas *mao;
} jogador;

typedef struct {
    cartas *mesa;
} mesa;

char log_jogo[MAX_LOG][200];
int log_index = 0;
FILE *arquivo_log;

void adicionaLog(const char *mensagem) {
    if (log_index < MAX_LOG) {
        strcpy(log_jogo[log_index], mensagem);
        log_index++;
    }
    if (arquivo_log != NULL) {
        fprintf(arquivo_log, "%s\n", mensagem);
    }
}

void iniciaBaralho(cartas baralho[52]) {
    int cont = 0;
    char valores[13][3] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
    char naipes[4][11] = {"Copas", "Ouros", "Espadas", "Paus"};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 13; j++) {
            strcpy(baralho[cont].naipe, naipes[i]);
            strcpy(baralho[cont].valor, valores[j]);
            cont++;
        }
    }
}

void embaralha(cartas baralho[52], tp_pilha *p) {
    srand(time(NULL));
    inicializaPilha(p);
    for (int i = 51; i >= 0; i--) {
        int num = rand() % (i + 1);
        push(p, baralho[num]);
        cartas temp = baralho[i];
        baralho[i] = baralho[num];
        baralho[num] = temp;
    }
}

void distribuiCartas(tp_pilha *p, jogador *jogadores, int num_jogadores, mesa *m) {
    for (int i = 0; i < num_jogadores; i++) {
        jogadores[i].mao = (cartas *)malloc(2 * sizeof(cartas));
        for (int j = 0; j < 2; j++) {
            pop(p, &jogadores[i].mao[j]);
        }
    }
    // As cartas da mesa são sacadas, mas inicialmente "ocultas"
    m->mesa = (cartas *)malloc(5 * sizeof(cartas));
    for (int i = 0; i < 5; i++) {
        pop(p, &m->mesa[i]);
    }
}

// Funções de verificação de combinação precisarão de um parâmetro `num_cartas_mesa_reveladas`
int verificarPar(cartas mao[2], cartas mesa[5], int num_cartas_mesa_reveladas) {
    int contadores[13] = {0};

    int valorParaIndice(const char *valor) {
        if (strcmp(valor, "A") == 0) return 12; // Ás como o maior valor
        if (strcmp(valor, "K") == 0) return 11;
        if (strcmp(valor, "Q") == 0) return 10;
        if (strcmp(valor, "J") == 0) return 9;
        if (strcmp(valor, "10") == 0) return 8;
        if (strcmp(valor, "9") == 0) return 7;
        if (strcmp(valor, "8") == 0) return 6;
        if (strcmp(valor, "7") == 0) return 5;
        if (strcmp(valor, "6") == 0) return 4;
        if (strcmp(valor, "5") == 0) return 3;
        if (strcmp(valor, "4") == 0) return 2;
        if (strcmp(valor, "3") == 0) return 1;
        if (strcmp(valor, "2") == 0) return 0;
        return -1;
    }

    // Contar valores das cartas da mão
    for (int i = 0; i < 2; i++) {
        int idx = valorParaIndice(mao[i].valor);
        if (idx != -1) contadores[idx]++;
    }

    // Contar valores das cartas da mesa (apenas as reveladas)
    for (int i = 0; i < num_cartas_mesa_reveladas; i++) {
        int idx = valorParaIndice(mesa[i].valor);
        if (idx != -1) contadores[idx]++;
    }

    // Verificar se existe pelo menos um par (2 cartas com mesmo valor)
    for (int i = 0; i < 13; i++) {
        if (contadores[i] >= 2) return 1;
    }

    return 0;
}

int verificarDoisPares(cartas mao[2], cartas mesa[5], int num_cartas_mesa_reveladas) {
    int cont[13] = {0}, pares = 0;
    // Criar um array temporário com as cartas visíveis (mão + mesa revelada)
    cartas todas[7];
    int total_cartas_visiveis = 0;

    for (int i = 0; i < 2; i++) {
        todas[total_cartas_visiveis++] = mao[i];
    }
    for (int i = 0; i < num_cartas_mesa_reveladas; i++) {
        todas[total_cartas_visiveis++] = mesa[i];
    }

    char *valores[] = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"};
    for (int i = 0; i < total_cartas_visiveis; i++)
        for (int j = 0; j < 13; j++)
            if (strcmp(todas[i].valor, valores[j]) == 0) cont[j]++;
    for (int i = 0; i < 13; i++)
        if (cont[i] >= 2) pares++;
    return (pares >= 2);
}

int verificarTrinca(cartas mao[2], cartas mesa[5], int num_cartas_mesa_reveladas) {
    int cont[13] = {0};
    cartas todas[7];
    int total_cartas_visiveis = 0;

    for (int i = 0; i < 2; i++) {
        todas[total_cartas_visiveis++] = mao[i];
    }
    for (int i = 0; i < num_cartas_mesa_reveladas; i++) {
        todas[total_cartas_visiveis++] = mesa[i];
    }

    char *valores[] = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"};
    for (int i = 0; i < total_cartas_visiveis; i++)
        for (int j = 0; j < 13; j++)
            if (strcmp(todas[i].valor, valores[j]) == 0) cont[j]++;
    for (int i = 0; i < 13; i++)
        if (cont[i] >= 3) return 1;
    return 0;
}

int verificarSequencia(cartas mao[2], cartas mesa[5], int num_cartas_mesa_reveladas) {
    int cont[14] = {0}; // 14 para Ás ser 1 ou 14 (A-2-3-4-5 ou 10-J-Q-K-A)
    cartas todas[7];
    int total_cartas_visiveis = 0;

    for (int i = 0; i < 2; i++) {
        todas[total_cartas_visiveis++] = mao[i];
    }
    for (int i = 0; i < num_cartas_mesa_reveladas; i++) {
        todas[total_cartas_visiveis++] = mesa[i];
    }

    char *valores[] = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"}; // A é o último no array
    for (int i = 0; i < total_cartas_visiveis; i++) {
        for (int j = 0; j < 13; j++) {
            if (strcmp(todas[i].valor, valores[j]) == 0) {
                cont[j + 1]++; // Ajuste de índice para 1-13
                if (j == 12) cont[0]++; // Ás como 1 (para sequência A-2-3-4-5)
                break;
            }
        }
    }
    // Reajustar para uma ordem onde 2=0, ..., A=12 para verificar sequências normalmente
    // E então verificar a sequência A-2-3-4-5 separadamente se necessário.

    // Convertendo valores para inteiros para facilitar a ordenação e verificação de sequência
    int valores_numericos[7];
    int k = 0;
    for(int i=0; i<total_cartas_visiveis; i++) {
        if (strcmp(todas[i].valor, "2") == 0) valores_numericos[k++] = 2;
        else if (strcmp(todas[i].valor, "3") == 0) valores_numericos[k++] = 3;
        else if (strcmp(todas[i].valor, "4") == 0) valores_numericos[k++] = 4;
        else if (strcmp(todas[i].valor, "5") == 0) valores_numericos[k++] = 5;
        else if (strcmp(todas[i].valor, "6") == 0) valores_numericos[k++] = 6;
        else if (strcmp(todas[i].valor, "7") == 0) valores_numericos[k++] = 7;
        else if (strcmp(todas[i].valor, "8") == 0) valores_numericos[k++] = 8;
        else if (strcmp(todas[i].valor, "9") == 0) valores_numericos[k++] = 9;
        else if (strcmp(todas[i].valor, "10") == 0) valores_numericos[k++] = 10;
        else if (strcmp(todas[i].valor, "J") == 0) valores_numericos[k++] = 11;
        else if (strcmp(todas[i].valor, "Q") == 0) valores_numericos[k++] = 12;
        else if (strcmp(todas[i].valor, "K") == 0) valores_numericos[k++] = 13;
        else if (strcmp(todas[i].valor, "A") == 0) {
            valores_numericos[k++] = 14; // Ás pode ser 14 (para K-A)
            valores_numericos[k++] = 1;  // Ás pode ser 1 (para A-2-3-4-5)
        }
    }

    // Ordenar os valores
    for (int i = 0; i < k - 1; i++) {
        for (int j = i + 1; j < k; j++) {
            if (valores_numericos[i] > valores_numericos[j]) {
                int temp = valores_numericos[i];
                valores_numericos[i] = valores_numericos[j];
                valores_numericos[j] = temp;
            }
        }
    }

    // Remover duplicatas e verificar sequência
    int valores_distintos[7];
    int num_distintos = 0;
    if (k > 0) {
        valores_distintos[num_distintos++] = valores_numericos[0];
        for (int i = 1; i < k; i++) {
            if (valores_numericos[i] != valores_numericos[i-1]) {
                valores_distintos[num_distintos++] = valores_numericos[i];
            }
        }
    }

    if (num_distintos < 5) return 0; // Não há cartas suficientes para uma sequência

    for (int i = 0; i <= num_distintos - 5; i++) {
        int e_sequencia = 1;
        for (int j = 0; j < 4; j++) {
            if (valores_distintos[i+j+1] != valores_distintos[i+j] + 1) {
                e_sequencia = 0;
                break;
            }
        }
        if (e_sequencia) return 1;
    }
    return 0;
}

int verificarFlush(cartas mao[2], cartas mesa[5], int num_cartas_mesa_reveladas) {
    int copas = 0, ouros = 0, paus = 0, espadas = 0;
    cartas todas[7];
    int total_cartas_visiveis = 0;

    for (int i = 0; i < 2; i++) {
        todas[total_cartas_visiveis++] = mao[i];
    }
    for (int i = 0; i < num_cartas_mesa_reveladas; i++) {
        todas[total_cartas_visiveis++] = mesa[i];
    }

    for (int i = 0; i < total_cartas_visiveis; i++) {
        if (strcmp(todas[i].naipe, "Copas") == 0) copas++;
        else if (strcmp(todas[i].naipe, "Ouros") == 0) ouros++;
        else if (strcmp(todas[i].naipe, "Paus") == 0) paus++;
        else if (strcmp(todas[i].naipe, "Espadas") == 0) espadas++;
    }
    return (copas >= 5 || ouros >= 5 || paus >= 5 || espadas >= 5);
}

int verificarFullHouse(cartas mao[2], cartas mesa[5], int num_cartas_mesa_reveladas) {
    int cont[13] = {0};
    cartas todas[7];
    int total_cartas_visiveis = 0;

    for (int i = 0; i < 2; i++) {
        todas[total_cartas_visiveis++] = mao[i];
    }
    for (int i = 0; i < num_cartas_mesa_reveladas; i++) {
        todas[total_cartas_visiveis++] = mesa[i];
    }

    char *valores[] = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"};
    for (int i = 0; i < total_cartas_visiveis; i++)
        for (int j = 0; j < 13; j++)
            if (strcmp(todas[i].valor, valores[j]) == 0) cont[j]++;
    int temTrinca = -1, temPar = 0;
    for (int i = 0; i < 13; i++)
        if (cont[i] >= 3) temTrinca = i;
    for (int i = 0; i < 13; i++)
        if (cont[i] >= 2 && i != temTrinca) temPar = 1; // Pelo menos um par diferente da trinca
    return (temTrinca != -1 && temPar);
}

// Para as combinações mais altas como Straight Flush e Royal Flush, você precisaria de funções adicionais
// que combinam a lógica de Sequência e Flush.

int verificarCombinacoes(jogador *jogadores, int num_jogadores, cartas mesa[5], int num_cartas_mesa_reveladas , ArvAVL * raiz) {
    char buffer[200];
    printf("\n--- Suas Combinações Possíveis (com %d cartas na mesa) ---\n", num_cartas_mesa_reveladas);
    for (int i = 0; i < num_jogadores; i++) {
        printf("Jogador %s (Mão: %s de %s, %s de %s): ", jogadores[i].nome,
            jogadores[i].mao[0].valor, jogadores[i].mao[0].naipe,
            jogadores[i].mao[1].valor, jogadores[i].mao[1].naipe);

        // Somente verifique combinações se houver cartas suficientes na mesa
        if (num_cartas_mesa_reveladas >= 5 && verificarFullHouse(jogadores[i].mao, mesa, num_cartas_mesa_reveladas)) {
            printf("Full House!\n");
            sprintf(buffer, "Jogador %s tem Full House", jogadores[i].nome);
            adicionaLog(buffer);
            inserir(raiz , 7);
        }
        else if (num_cartas_mesa_reveladas >= 5 && verificarFlush(jogadores[i].mao, mesa, num_cartas_mesa_reveladas)) {
            printf("Flush!\n");
            sprintf(buffer, "Jogador %s tem Flush", jogadores[i].nome);
            adicionaLog(buffer);
            inserir(raiz , 6);
        }
        else if (num_cartas_mesa_reveladas >= 5 && verificarSequencia(jogadores[i].mao, mesa, num_cartas_mesa_reveladas)) {
            printf("Sequência!\n");
            sprintf(buffer, "Jogador %s tem Sequência", jogadores[i].nome);
            adicionaLog(buffer);
            inserir(raiz , 5);
        }
        else if (num_cartas_mesa_reveladas >= 3 && verificarTrinca(jogadores[i].mao, mesa, num_cartas_mesa_reveladas)) {
            printf("Trinca!\n");
            sprintf(buffer, "Jogador %s tem Trinca", jogadores[i].nome);
            adicionaLog(buffer);
            inserir(raiz , 4);
        }
        else if (num_cartas_mesa_reveladas >= 3 && verificarDoisPares(jogadores[i].mao, mesa, num_cartas_mesa_reveladas)) {
            printf("Dois Pares!\n");
            sprintf(buffer, "Jogador %s tem Dois Pares", jogadores[i].nome);
            adicionaLog(buffer);
            inserir(raiz , 3);
        }
        else if (verificarPar(jogadores[i].mao, mesa, num_cartas_mesa_reveladas)) { // Par pode ser com cartas da mão apenas, ou mão + flop
            printf("Par!\n");
            sprintf(buffer, "Jogador %s tem Par", jogadores[i].nome);
            adicionaLog(buffer);
            inserir(raiz , 2);
        }
        else {
            printf("Nenhuma combinação válida encontrada.\n");
            sprintf(buffer, "Jogador %s: Nenhuma combinação válida encontrada.", jogadores[i].nome);
            adicionaLog(buffer);
        }
    }
    return 0; // Este retorno não é usado, mas pode ser para indicar algo no futuro
}

int rankComb(cartas mao[2], cartas mesa[5], int num_cartas_mesa_reveladas) {
    // Retorna o ranking da combinação, considerando apenas as cartas reveladas
    if (num_cartas_mesa_reveladas >= 5 && verificarFullHouse(mao, mesa, num_cartas_mesa_reveladas)) return 6;
    if (num_cartas_mesa_reveladas >= 5 && verificarFlush(mao, mesa, num_cartas_mesa_reveladas)) return 5;
    if (num_cartas_mesa_reveladas >= 5 && verificarSequencia(mao, mesa, num_cartas_mesa_reveladas)) return 4;
    if (num_cartas_mesa_reveladas >= 3 && verificarTrinca(mao, mesa, num_cartas_mesa_reveladas)) return 3;
    if (num_cartas_mesa_reveladas >= 3 && verificarDoisPares(mao, mesa, num_cartas_mesa_reveladas)) return 2;
    if (verificarPar(mao, mesa, num_cartas_mesa_reveladas)) return 1;
    return 0; // Carta alta
}


void rodadaAposta(jogador *jogadores, int num_jogadores, int *apostaram) {
    float aposta = 100.0; // Valor fixo da aposta
    printf("\n--- Rodada de Apostas ---\n");
    for (int i = 0; i < num_jogadores; i++) {
        apostaram[i] = 0; // Resetar para cada rodada de aposta
        if (jogadores[i].saldo < aposta) {
            printf("Jogador %s (R$ %.2f): Saldo insuficiente para apostar.\n", jogadores[i].nome, jogadores[i].saldo);
            adicionaLog("Jogador com saldo insuficiente para apostar.");
            continue; // Pular este jogador se ele não tem saldo
        }
        printf("Jogador %s (R$ %.2f), apostar R$ %.2f? (1=sim, 0=não): ", jogadores[i].nome, jogadores[i].saldo, aposta);
        int r;
        scanf("%d", &r);
        if (r == 1) {
            jogadores[i].saldo -= aposta;
            apostaram[i] = 1;
            char buffer[200];
            sprintf(buffer, "Jogador %s apostou R$ %.2f", jogadores[i].nome, aposta);
            adicionaLog(buffer);
        } else {
            char buffer[200];
            sprintf(buffer, "Jogador %s não apostou", jogadores[i].nome);
            adicionaLog(buffer);
        }
    }
}

int determinarVencedor(jogador *jogadores, int num_jogadores, mesa *m, int *apostaram, int num_cartas_mesa_reveladas) {
    int vencedor = -1;
    int melhor_ranking = -1;

    for (int i = 0; i < num_jogadores; i++) {
        if (!apostaram[i]) { // Apenas considere jogadores que apostaram
            continue;
        }
        int rank_atual = rankComb(jogadores[i].mao, m->mesa, num_cartas_mesa_reveladas);

        if (rank_atual > melhor_ranking) {
            melhor_ranking = rank_atual;
            vencedor = i;
        }
        // Lógica para desempate (ex: kicker, maior carta da combinação) precisaria ser implementada aqui.
        // Por enquanto, o primeiro jogador encontrado com o melhor rank ganha em caso de empate de rank.
    }

    if (vencedor != -1) {
        char buffer[200];
        sprintf(buffer, "Vencedor potencial: %s com ranking %d", jogadores[vencedor].nome, melhor_ranking);
        adicionaLog(buffer);
    }
    return vencedor;
}

void exibeCartasMesa(cartas mesa[5], int num_cartas_reveladas) {
    printf("\nCartas da mesa (%d reveladas):\n", num_cartas_reveladas);
    for (int i = 0; i < num_cartas_reveladas; i++) {
        printf("%s de %s\n", mesa[i].valor, mesa[i].naipe);
    }
    for (int i = num_cartas_reveladas; i < 5; i++) {
        printf("[Carta Oculta]\n");
    }
}

void jogarRodadaPoker(jogador *jogadores, int num_jogadores, mesa *m , ArvAVL *raiz) {
    int apostaram[4] = {0}; // Para controlar quem apostou em cada rodada de aposta

    // Pré-flop: Apenas as cartas da mão
    printf("\n--- Pré-flop ---\n");
    for (int i = 0; i < num_jogadores; i++) {
        printf("Jogador %s (R$ %.2f): Mão: %s de %s, %s de %s\n", jogadores[i].nome, jogadores[i].saldo,
            jogadores[i].mao[0].valor, jogadores[i].mao[0].naipe,
            jogadores[i].mao[1].valor, jogadores[i].mao[1].naipe);
    }
    exibeCartasMesa(m->mesa, 0); // Nenhuma carta da mesa revelada
    verificarCombinacoes(jogadores, num_jogadores, m->mesa, 0 , raiz); // Verificar combinações com 0 cartas na mesa
    rodadaAposta(jogadores, num_jogadores, apostaram);

    // Flop: 3 cartas da mesa são reveladas
    printf("\n--- Flop ---\n");
    exibeCartasMesa(m->mesa, 3);
    verificarCombinacoes(jogadores, num_jogadores, m->mesa, 3 , raiz); // Verificar combinações com 3 cartas na mesa
    rodadaAposta(jogadores, num_jogadores, apostaram);

    // Turn: A 4ª carta da mesa é revelada
    printf("\n--- Turn ---\n");
    exibeCartasMesa(m->mesa, 4);
    verificarCombinacoes(jogadores, num_jogadores, m->mesa, 4 , raiz); // Verificar combinações com 4 cartas na mesa
    rodadaAposta(jogadores, num_jogadores, apostaram);

    // River: A 5ª carta da mesa é revelada
    printf("\n--- River ---\n");
    exibeCartasMesa(m->mesa, 5);
    verificarCombinacoes(jogadores, num_jogadores, m->mesa, 5 , raiz); // Verificar combinações com 5 cartas na mesa
    rodadaAposta(jogadores, num_jogadores, apostaram);

    // Determinar vencedor final e distribuir o pote
    int vencedor_final = determinarVencedor(jogadores, num_jogadores, m, apostaram, 5);
    if (vencedor_final != -1) {
        float total_pote = 0;
        for (int i = 0; i < num_jogadores; i++) {
            if (apostaram[i]) { // Se o jogador apostou em alguma rodada
                total_pote += 100; // Simplesmente somando o valor da aposta por rodada
            }
        }
        jogadores[vencedor_final].saldo += total_pote;
        printf("\nO VENCEDOR É: %s com a melhor mão (ranking %d) e ganhou R$ %.2f!\n",
               jogadores[vencedor_final].nome, rankComb(jogadores[vencedor_final].mao, m->mesa, 5), total_pote);
        char buffer[200];
        sprintf(buffer, "Vencedor final: %s ganhou R$ %.2f", jogadores[vencedor_final].nome, total_pote);
        adicionaLog(buffer);
    } else {
        printf("\nNinguém apostou ou houve um empate não resolvido. Pote dividido ou sem vencedor claro.\n");
        adicionaLog("Ninguém apostou ou houve um empate não resolvido.");
    }
}


int main() {
    int num_jogadores;
    cartas baralho[52];
    jogador *jogadores;
    mesa m;
    tp_pilha monte;
    ArvAVL* raiz = criarAVL();
    
    

    arquivo_log = fopen("historico_jogo.txt", "w");
    if (arquivo_log == NULL) {
        printf("Erro ao abrir arquivo de log.\n");
        return 1;
    }
    
        // Personalização do terminal com tema de poker
    printf("♠️ ♥️ ♣️ ♦️ ♠️ ♥️ ♣️ ♦️ ♠️ ♥️ ♣️ ♦️ ♠️ ♥️ ♣️ ♦️ ♠️ ♥️ ♣️ ♦️\n");
    printf("         🂡  🂱  🃁  🃑  BEM-VINDO AO 🂡  🂱  🃁  🃑\n");
    printf("                  ╔═══════════════════╗\n");
    printf("                  ║     P O K E R     ║\n");
    printf("                  ╚═══════════════════╝\n");
    printf("♠️ ♥️ ♣️ ♦️ ♠️ ♥️ ♣️ ♦️ ♠️ ♥️ ♣️ ♦️ ♠️ ♥️ ♣️ ♦️ ♠️ ♥️ ♣️ ♦️\n\n");
    
    iniciaBaralho(baralho);
    embaralha(baralho, &monte);

    printf("---------------------------\n");
    printf("       🂡  🂱  🃁  🃑   👑 VAMOS COMEÇAR 👑   🂡  🂱  🃁  🃑\n\n");
    printf("Informe o número de jogadores (2-4): ");
    scanf("%d", &num_jogadores);
    while (num_jogadores < 2 || num_jogadores > 4) {
        printf("Inválido. Digite novamente (2-4): ");
        scanf("%d", &num_jogadores);
    }

    jogadores = (jogador *)malloc(num_jogadores * sizeof(jogador));
    for (int i = 0; i < num_jogadores; i++) {
        printf("Nome do jogador %d: ", i + 1);
        scanf(" %s", jogadores[i].nome);
        jogadores[i].saldo = 1000.0;
    }

    distribuiCartas(&monte, jogadores, num_jogadores, &m);
    jogarRodadaPoker(jogadores, num_jogadores, &m , raiz); // Chamada da nova função de rodada

    printf("\n===📊 Histórico de Jogadas ===\n"ANSI_RESET);
    for (int i = 0; i < log_index; i++) {
        printf("%s\n", log_jogo[i]);
    }
    
    printf("\n✔️ Histórico de combinações por round em ordem:\n" ANSI_RESET);
    emOrd(raiz);

    fclose(arquivo_log);

    for (int i = 0; i < num_jogadores; i++) {
        if (jogadores[i].mao != NULL) { // Verificação para evitar free em ponteiro nulo
            free(jogadores[i].mao);
        }
    }
    free(jogadores);
    if (m.mesa != NULL) { // Verificação para evitar free em ponteiro nulo
        free(m.mesa);
    }

    return 0;
}