/**
 * Ferramenta de Leaderboard para o jogo Fall Witches (Versão Corrigida)
 *
 * Autor: Bernardo Machado de Souza
 * Descrição: Este programa lê todos os arquivos de save (.sav), compila um
 * placar de líderes (leaderboard), permite ordenar os jogadores por vários
 * critérios e salva o resultado em "leaderboard.sav".
 *
 * Como compilar (exemplo com GCC):
 * gcc leaderboard_tool.c -o leaderboard_tool -lm
 *
 * Como usar:
 * ./leaderboard_tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h> // Para tolower

// --- Definições e Estruturas Adaptadas do Projeto Original ---
#define MAX_PLAYER_NAME_LENGTH 50
#define MAX_ITEM_NAME_LENGTH 50
#define MAX_INVENTORY_SLOTS 10
#define MAX_EQUIP_SLOTS 3
#define SAVE_BASE_DIRECTORY "Saves"
#define SAVE_SUBDIR_SINGLEPLAYER "SinglePlayer"
#define SAVE_SUBDIR_TWOPLAYER    "TwoPlayer"

// --- CORREÇÃO: A estrutura do Player deve ser idêntica à do jogo ---
// Removidos campos que não estão no save (width, height, etc.) para evitar desalinhamento.
// A ordem dos campos é crucial e deve ser a mesma da função SaveGame do jogo.
typedef enum { GUERREIRO, MAGO, ARQUEIRO, BARBARO, LADINO, CLERIGO, CLASSE_COUNT } Classe;
typedef enum { SPRITE_TYPE_HUMANO, SPRITE_TYPE_DEMONIO, NUM_SPRITE_TYPES } SpriteType;

typedef struct {
    char name[MAX_ITEM_NAME_LENGTH];
    int quantity;
} InventoryItem;

typedef struct Player {
  char nome[MAX_PLAYER_NAME_LENGTH];
  Classe classe;
  SpriteType spriteType;
  int nivel;
  int exp;
  int vida;
  int max_vida;
  int mana;
  int max_mana;
  int stamina;
  int max_stamina;
  int ataque;
  int defesa;
  int magic_attack;
  int magic_defense;
  int forca;
  int percepcao;
  int resistencia;
  int carisma;
  int inteligencia;
  int agilidade;
  int sorte;
  int moedas;
  int posx;
  int posy;
  InventoryItem inventory[MAX_INVENTORY_SLOTS];
  int inventory_item_count;
  InventoryItem equipped_items[MAX_EQUIP_SLOTS];
} Player;

// Estrutura para o progresso do jogo, usada para pular os dados no arquivo de save.
typedef struct {
    bool visitedMaps[21][21];
} GameProgress;


// --- Protótipos das Funções ---

typedef int (*CompareFunc)(const void*, const void*);

int compareByName(const void* a, const void* b);
int compareByLevel(const void* a, const void* b);
int compareByExp(const void* a, const void* b);
int compareByStrength(const void* a, const void* b);
int compareByAgility(const void* a, const void* b);

void quickSort(Player arr[], int low, int high, CompareFunc compare);
void printMenu(void);
void printLeaderboard(Player players[], int count);
void searchPlayer(Player players[], int count);
bool saveLeaderboard(Player players[], int count, const char* filename);
void loadAllPlayers(Player** allPlayers, int* totalPlayerCount);


// --- Função Principal ---

int main() {
    Player* allPlayers = NULL;
    int totalPlayerCount = 0;

    printf("Iniciando Ferramenta de Leaderboard Fall Witches...\n");
    printf("Lendo arquivos de save...\n");

    loadAllPlayers(&allPlayers, &totalPlayerCount);

    printf("%d jogador(es) encontrado(s) em todos os saves.\n\n", totalPlayerCount);

    if (totalPlayerCount == 0) {
        printf("Nenhum jogador para exibir. Encerrando.\n");
        return 0;
    }

    int choice;
    do {
        printMenu();
        // --- CORREÇÃO: Melhorado o tratamento de entrada ---
        if (scanf("%d", &choice) != 1) {
            // Limpa o buffer de entrada em caso de falha
            while (getchar() != '\n');
            choice = -1;
        }
        printf("\n");

        CompareFunc sortFunc = NULL;

        switch (choice) {
            case 1: sortFunc = compareByName; break;
            case 2: sortFunc = compareByLevel; break;
            case 3: sortFunc = compareByExp; break;
            case 4: sortFunc = compareByStrength; break;
            case 5: sortFunc = compareByAgility; break;
            case 6: searchPlayer(allPlayers, totalPlayerCount); continue; // `continue` para redesenhar o menu
            case 7:
                if (saveLeaderboard(allPlayers, totalPlayerCount, "leaderboard.sav")) {
                    printf("Leaderboard salvo com sucesso em 'leaderboard.sav'!\n");
                } else {
                    fprintf(stderr, "Erro ao salvar o leaderboard.\n");
                }
                continue; // `continue` para redesenhar o menu
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opcao invalida. Tente novamente.\n"); continue;
        }

        if (sortFunc) {
            quickSort(allPlayers, 0, totalPlayerCount - 1, sortFunc);
            printLeaderboard(allPlayers, totalPlayerCount);
        }

    } while (choice != 0);

    free(allPlayers);
    return 0;
}


// --- Implementação da Lógica Principal ---

void loadAllPlayers(Player** allPlayers, int* totalPlayerCount) {
    const char* directories[] = {SAVE_SUBDIR_SINGLEPLAYER, SAVE_SUBDIR_TWOPLAYER};

    for (int i = 0; i < 2; i++) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", SAVE_BASE_DIRECTORY, directories[i]);

        DIR* d = opendir(path);
        if (!d) {
            // Se o diretório não existe, apenas avisa e continua
            printf("Aviso: Diretório '%s' não encontrado.\n", path);
            continue;
        }

        struct dirent* dir;
        while ((dir = readdir(d)) != NULL) {
            // Verifica se o arquivo termina com .sav
            if (strstr(dir->d_name, ".sav")) {
                char fullPath[512];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", path, dir->d_name);

                FILE* file = fopen(fullPath, "rb");
                if(!file) continue;

                // --- CORREÇÃO: Leitura de dados alinhada com a função SaveGame ---
                unsigned int version;
                fread(&version, sizeof(unsigned int), 1, file); // Lê a versão

                // Pula mapX e mapY
                fseek(file, sizeof(int) * 2, SEEK_CUR);

                int playerCount;
                if (fread(&playerCount, sizeof(int), 1, file) != 1 || playerCount <= 0 || playerCount > 2) {
                    fclose(file);
                    continue;
                }

                // Pula a estrutura GameProgress
                fseek(file, sizeof(GameProgress), SEEK_CUR);

                for (int p = 0; p < playerCount; ++p) {
                    Player newPlayer = {0}; // Inicializa a estrutura com zeros

                    // --- CORREÇÃO CRÍTICA: Lê cada campo individualmente, na ordem correta ---
                    fread(newPlayer.nome, sizeof(char), MAX_PLAYER_NAME_LENGTH, file);
                    fread(&newPlayer.classe, sizeof(Classe), 1, file);
                    fread(&newPlayer.spriteType, sizeof(SpriteType), 1, file);
                    fread(&newPlayer.nivel, sizeof(int), 1, file);
                    fread(&newPlayer.exp, sizeof(int), 1, file);
                    fread(&newPlayer.vida, sizeof(int), 1, file);
                    fread(&newPlayer.max_vida, sizeof(int), 1, file);
                    fread(&newPlayer.mana, sizeof(int), 1, file);
                    fread(&newPlayer.max_mana, sizeof(int), 1, file);
                    fread(&newPlayer.stamina, sizeof(int), 1, file);
                    fread(&newPlayer.max_stamina, sizeof(int), 1, file);
                    fread(&newPlayer.magic_attack, sizeof(int), 1, file);
                    fread(&newPlayer.magic_defense, sizeof(int), 1, file);
                    fread(&newPlayer.ataque, sizeof(int), 1, file);
                    fread(&newPlayer.defesa, sizeof(int), 1, file);
                    fread(&newPlayer.forca, sizeof(int), 1, file);
                    fread(&newPlayer.percepcao, sizeof(int), 1, file);
                    fread(&newPlayer.resistencia, sizeof(int), 1, file);
                    fread(&newPlayer.carisma, sizeof(int), 1, file);
                    fread(&newPlayer.inteligencia, sizeof(int), 1, file);
                    fread(&newPlayer.agilidade, sizeof(int), 1, file);
                    fread(&newPlayer.sorte, sizeof(int), 1, file);
                    fread(&newPlayer.moedas, sizeof(int), 1, file);
                    fread(&newPlayer.posx, sizeof(int), 1, file);
                    fread(&newPlayer.posy, sizeof(int), 1, file);
                    fread(newPlayer.inventory, sizeof(InventoryItem), MAX_INVENTORY_SLOTS, file);
                    fread(&newPlayer.inventory_item_count, sizeof(int), 1, file);
                    fread(newPlayer.equipped_items, sizeof(InventoryItem), MAX_EQUIP_SLOTS, file);

                    // Adiciona o jogador à lista
                    (*totalPlayerCount)++;
                    *allPlayers = (Player*)realloc(*allPlayers, (*totalPlayerCount) * sizeof(Player));
                    if (!*allPlayers) {
                        perror("Falha ao realocar memoria para jogadores");
                        exit(EXIT_FAILURE);
                    }
                    (*allPlayers)[(*totalPlayerCount) - 1] = newPlayer;
                }
                fclose(file);
            }
        }
        closedir(d);
    }
}

void printMenu() {
    printf("\n--- MENU DO LEADERBOARD ---\n");
    printf("Ordenar por:\n");
    printf("1. Nome\n");
    printf("2. Nivel (Maior primeiro)\n");
    printf("3. XP (Maior primeiro)\n");
    printf("4. Forca (Maior primeiro)\n");
    printf("5. Agilidade (Maior primeiro)\n");
    printf("---------------------------\n");
    printf("6. Buscar Jogador por Nome\n");
    printf("7. Salvar Leaderboard Atual\n");
    printf("0. Sair\n");
    printf("Sua escolha: ");
}

void printLeaderboard(Player players[], int count) {
    printf("\n================================================ LEADERBOARD =================================================\n");
    printf("%-20s | %-5s | %-10s | %-4s | %-4s | %-4s | %-4s | %-4s | %-4s | %-6s | %-6s\n",
           "Nome", "Nivel", "XP", "FOR", "PER", "RES", "CAR", "INT", "AGI", "ATK", "DEF");
    printf("-----------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-20s | %-5d | %-10d | %-4d | %-4d | %-4d | %-4d | %-4d | %-4d | %-6d | %-6d\n",
               players[i].nome, players[i].nivel, players[i].exp,
               players[i].forca, players[i].percepcao, players[i].resistencia,
               players[i].carisma, players[i].inteligencia, players[i].agilidade,
               players[i].ataque, players[i].defesa);
    }
    printf("===========================================================================================================\n");
}

// --- CORREÇÃO: Função de busca melhorada ---
void searchPlayer(Player players[], int count) {
    char nameToSearch[MAX_PLAYER_NAME_LENGTH];
    printf("Digite o nome do jogador a ser buscado: ");
    scanf("%49s", nameToSearch); // Limita a leitura para evitar overflow

    bool found = false;
    for (int i = 0; i < count; i++) {
        // Compara ignorando maiúsculas/minúsculas
        if (strcasecmp(players[i].nome, nameToSearch) == 0) {
            if (!found) {
                printf("\n--- Jogador(es) Encontrado(s) ---\n");
            }
            printLeaderboard(&players[i], 1); // Imprime apenas o jogador encontrado
            found = true;
        }
    }

    if (!found) {
        printf("Nenhum jogador encontrado com o nome '%s'.\n", nameToSearch);
    }
}

bool saveLeaderboard(Player players[], int count, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Erro ao abrir arquivo para salvar leaderboard");
        return false;
    }

    unsigned int leaderboard_version = 1;
    fwrite(&leaderboard_version, sizeof(unsigned int), 1, file);
    fwrite(&count, sizeof(int), 1, file);

    size_t written = fwrite(players, sizeof(Player), count, file);

    fclose(file);

    if (written != (size_t)count) {
        fprintf(stderr, "Erro: Nem todos os dados dos jogadores foram escritos no arquivo.\n");
        return false;
    }
    return true;
}


// --- Funções de Ordenação (Quicksort) ---

int partition(Player arr[], int low, int high, CompareFunc compare) {
    Player pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (compare(&arr[j], &pivot) < 0) {
            i++;
            Player temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    Player temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

void quickSort(Player arr[], int low, int high, CompareFunc compare) {
    if (low < high) {
        int pi = partition(arr, low, high, compare);
        quickSort(arr, low, pi - 1, compare);
        quickSort(arr, pi + 1, high, compare);
    }
}


// --- Funções de Comparação ---

int compareByName(const void* a, const void* b) {
    const Player* pA = (const Player*)a;
    const Player* pB = (const Player*)b;
    // --- CORREÇÃO: Usa strcasecmp para ignorar maiúsculas/minúsculas ---
    return strcasecmp(pA->nome, pB->nome);
}

int compareByLevel(const void* a, const void* b) {
    const Player* pA = (const Player*)a;
    const Player* pB = (const Player*)b;
    if (pB->nivel != pA->nivel) return (pB->nivel - pA->nivel);
    return (pB->exp - pA->exp); // Desempate por XP
}

int compareByExp(const void* a, const void* b) {
    const Player* pA = (const Player*)a;
    const Player* pB = (const Player*)b;
    return (pB->exp - pA->exp);
}

int compareByStrength(const void* a, const void* b) {
    const Player* pA = (const Player*)a;
    const Player* pB = (const Player*)b;
    return (pB->forca - pA->forca);
}

int compareByAgility(const void* a, const void* b) {
    const Player* pA = (const Player*)a;
    const Player* pB = (const Player*)b;
    return (pB->agilidade - pA->agilidade);
}