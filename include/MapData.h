#ifndef MAP_DATA_H
#define MAP_DATA_H

#include "raylib.h"
#include "Sound.h" // Para o enum MusicCategory
#include "Classes.h" // Para definições de constantes como MAX_CHAR_NAME_LENGTH
#include <stdbool.h>

#define MAX_ALLOWED_MUSIC_CATEGORIES 5
#define MAX_NPC_CONFIGS_PER_MAP 10
#define MAX_ENEMY_TYPES_PER_MAP 10

/**
 * @brief Configurações de um NPC lidas do arquivo de variáveis do mapa.
 */
typedef struct {
    char name[MAX_CHAR_NAME_LENGTH];
    int dialogueId;
    char spriteFolder[MAX_CHAR_SPRITE_FOLDER_LENGTH];
    Vector2 spawnCoords;
} NpcConfig;

/**
 * @brief Definição de um tipo de inimigo que pode spawnar no mapa.
 */
typedef struct {
    char name[MAX_CHAR_NAME_LENGTH];
    char spriteFolder[MAX_CHAR_SPRITE_FOLDER_LENGTH];
    // Futuramente, pode incluir stats base como HP, ataque, etc.
} EnemyTypeConfig;

/**
 * @brief Configurações gerais para uma seção de mapa.
 */
typedef struct {
    bool isLoaded;
    int enemySpawnChance; // Chance de 0 a 100
    MusicCategory allowedMusic[MAX_ALLOWED_MUSIC_CATEGORIES];
    int numAllowedMusicCategories;
} MapConfig;


// --- Funções do Módulo ---

/**
 * @brief Carrega as configurações para uma seção de mapa específica.
 * Lê o arquivo /assets/MapVariables/X-Y.txt correspondente.
 * @param mapX A coordenada X do mapa.
 * @param mapY A coordenada Y do mapa.
 */
void MapData_LoadForMap(int mapX, int mapY);

/**
 * @brief Limpa os dados de configuração do mapa atual.
 * Chamada antes de carregar uma nova seção.
 */
void MapData_UnloadCurrent(void);

// --- Getters para Acessar os Dados Carregados ---

const MapConfig* MapData_GetConfig(void);
int MapData_GetNpcConfigCount(void);
const NpcConfig* MapData_GetNpcConfig(int index);
int MapData_GetEnemyTypeConfigCount(void);
const EnemyTypeConfig* MapData_GetEnemyTypeConfig(int index);

#endif // MAP_DATA_H