#include "../include/MapData.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Armazenamento Estático para os Dados do Mapa Atual ---
// *** CORRIGIDO: Usa {0} para inicializar a struct inteira, uma forma padrão e segura em C. ***
static MapConfig s_currentMapConfig = {0};
static NpcConfig s_npcConfigs[MAX_NPC_CONFIGS_PER_MAP];
static int s_npcConfigCount = 0;
static EnemyTypeConfig s_enemyTypeConfigs[MAX_ENEMY_TYPES_PER_MAP];
static int s_enemyTypeConfigCount = 0;

// --- Funções Auxiliares de Parsing ---

/**
 * @brief Remove espaços em branco do início e do fim de uma string.
 */
static char* TrimWhitespace(char* str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

/**
 * @brief Converte uma string de nome de categoria de música para o enum MusicCategory.
 */
static MusicCategory StringToMusicCategory(const char* categoryName) {
    if (strcmp(categoryName, "MainMenu") == 0) return MUSIC_CATEGORY_MAINMENU;
    if (strcmp(categoryName, "Game") == 0) return MUSIC_CATEGORY_GAME;
    if (strcmp(categoryName, "Battle") == 0) return MUSIC_CATEGORY_BATTLE;
    if (strcmp(categoryName, "Cutscene") == 0) return MUSIC_CATEGORY_CUTSCENE;
    if (strcmp(categoryName, "Nature") == 0) return MUSIC_CATEGORY_AMBIENT_NATURE;
    if (strcmp(categoryName, "City") == 0) return MUSIC_CATEGORY_AMBIENT_CITY;
    if (strcmp(categoryName, "Cave") == 0) return MUSIC_CATEGORY_AMBIENT_CAVE;
    return MUSIC_CATEGORY_GAME;
}

// --- Funções Públicas ---

void MapData_UnloadCurrent(void) {
    memset(&s_currentMapConfig, 0, sizeof(MapConfig));
    memset(s_npcConfigs, 0, sizeof(s_npcConfigs));
    memset(s_enemyTypeConfigs, 0, sizeof(s_enemyTypeConfigs));
    s_npcConfigCount = 0;
    s_enemyTypeConfigCount = 0;
}

void MapData_LoadForMap(int mapX, int mapY) {
    MapData_UnloadCurrent();

    char filePath[256];
    sprintf(filePath, "assets/MapVariables/%d-%d.txt", mapX, mapY);

    if (!FileExists(filePath)) {
        TraceLog(LOG_INFO, "MapData: Nenhum arquivo de variaveis encontrado para o mapa (%d,%d).", mapX, mapY);
        return;
    }

    FILE* file = fopen(filePath, "rt");
    if (!file) {
        TraceLog(LOG_WARNING, "MapData: Nao foi possivel abrir o arquivo %s", filePath);
        return;
    }

    char lineBuffer[256];
    int parseState = 0;

    while (fgets(lineBuffer, sizeof(lineBuffer), file)) {
        char* line = TrimWhitespace(lineBuffer);
        if (strlen(line) == 0 || line[0] == '#') continue;

        if (strcmp(line, "npc_start") == 0) { parseState = 1; continue; }
        if (strcmp(line, "enemy_type_start") == 0) { parseState = 2; continue; }
        if (strcmp(line, "allowed_music_start") == 0) { parseState = 3; continue; }
        
        if (strcmp(line, "npc_end") == 0 && parseState == 1) { s_npcConfigCount++; parseState = 0; continue; }
        if (strcmp(line, "enemy_type_end") == 0 && parseState == 2) { s_enemyTypeConfigCount++; parseState = 0; continue; }
        if (strcmp(line, "allowed_music_end") == 0 && parseState == 3) { parseState = 0; continue; }
        
        if (parseState == 3) {
            if (s_currentMapConfig.numAllowedMusicCategories < MAX_ALLOWED_MUSIC_CATEGORIES) {
                s_currentMapConfig.allowedMusic[s_currentMapConfig.numAllowedMusicCategories++] = StringToMusicCategory(line);
            }
            continue;
        }

        char* key = strtok(line, ":");
        char* value = strtok(NULL, "\n");
        if (!key || !value) continue;
        key = TrimWhitespace(key);
        value = TrimWhitespace(value);

        switch(parseState) {
            case 0:
                if (strcmp(key, "enemy_spawn_chance") == 0) s_currentMapConfig.enemySpawnChance = atoi(value);
                break;
            case 1:
                if (s_npcConfigCount < MAX_NPC_CONFIGS_PER_MAP) {
                    if (strcmp(key, "name") == 0) strncpy(s_npcConfigs[s_npcConfigCount].name, value, MAX_CHAR_NAME_LENGTH - 1);
                    else if (strcmp(key, "dialogue_id") == 0) s_npcConfigs[s_npcConfigCount].dialogueId = atoi(value);
                    else if (strcmp(key, "sprite_folder") == 0) strncpy(s_npcConfigs[s_npcConfigCount].spriteFolder, value, MAX_CHAR_SPRITE_FOLDER_LENGTH - 1);
                    else if (strcmp(key, "spawn_coords") == 0) sscanf(value, "%f, %f", &s_npcConfigs[s_npcConfigCount].spawnCoords.x, &s_npcConfigs[s_npcConfigCount].spawnCoords.y);
                }
                break;
            case 2:
                if (s_enemyTypeConfigCount < MAX_ENEMY_TYPES_PER_MAP) {
                    if (strcmp(key, "name") == 0) strncpy(s_enemyTypeConfigs[s_enemyTypeConfigCount].name, value, MAX_CHAR_NAME_LENGTH - 1);
                    else if (strcmp(key, "sprite_folder") == 0) strncpy(s_enemyTypeConfigs[s_enemyTypeConfigCount].spriteFolder, value, MAX_CHAR_SPRITE_FOLDER_LENGTH - 1);
                }
                break;
        }
    }
    fclose(file);
    s_currentMapConfig.isLoaded = true;
    TraceLog(LOG_INFO, "MapData para (%d,%d) carregado. NPCs:%d, Tipos de Inimigos:%d, Chance Spawn:%d%%", mapX, mapY, s_npcConfigCount, s_enemyTypeConfigCount, s_currentMapConfig.enemySpawnChance);
}

// --- Getters ---
const MapConfig* MapData_GetConfig(void) { return &s_currentMapConfig; }
int MapData_GetNpcConfigCount(void) { return s_npcConfigCount; }
const NpcConfig* MapData_GetNpcConfig(int index) {
    if (index >= 0 && index < s_npcConfigCount) return &s_npcConfigs[index];
    return NULL;
}
int MapData_GetEnemyTypeConfigCount(void) { return s_enemyTypeConfigCount; }
const EnemyTypeConfig* MapData_GetEnemyTypeConfig(int index) {
    if (index >= 0 && index < s_enemyTypeConfigCount) return &s_enemyTypeConfigs[index];
    return NULL;
}