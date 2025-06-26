#include "../include/CharacterManager.h"
#include "../include/MapData.h"
#include "../include/Dialogue.h"
#include "../include/EnemyAI.h"
#include "../include/BattleSystem.h"
#include "../include/Game.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

extern GameState* g_currentScreen_ptr; 
extern Player* g_players_ptr;
extern int* g_currentActivePlayers_ptr;

typedef struct {
    char name[MAX_CHAR_NAME_LENGTH];
    Texture2D sprite;
} EnemySpriteCacheEntry;

static MapCharacter s_mapCharacters[MAX_MAP_CHARACTERS];
static float s_spawnCheckTimer = 0.0f;
#define SPAWN_CHECK_INTERVAL 5.0f
static EnemySpriteCacheEntry s_enemySpriteCache[MAX_ENEMY_TYPES_PER_MAP];
static int s_enemySpriteCacheCount = 0;


static Texture2D FindSpriteInCache(const char* enemyTypeName) {
    for (int i = 0; i < s_enemySpriteCacheCount; i++) {
        if (strcmp(s_enemySpriteCache[i].name, enemyTypeName) == 0) return s_enemySpriteCache[i].sprite;
    }
    return (Texture2D){0};
}

static MapCharacter* FindEmptyCharacterSlot(void) {
    for (int i = 0; i < MAX_MAP_CHARACTERS; i++) {
        if (!s_mapCharacters[i].isActive) {
            s_mapCharacters[i].id = i;
            return &s_mapCharacters[i];
        }
    }
    return NULL;
}

static void SpawnRandomEnemy(const WorldSection* activeSection) {
    if (s_enemySpriteCacheCount == 0) return;
    MapCharacter* newEnemy = FindEmptyCharacterSlot();
    if (!newEnemy) return;

    int enemyTypeIndex = GetRandomValue(0, MapData_GetEnemyTypeConfigCount() - 1);
    const EnemyTypeConfig* typeToSpawn = MapData_GetEnemyTypeConfig(enemyTypeIndex);
    if (!typeToSpawn) { return; }

    memset(newEnemy, 0, sizeof(MapCharacter));
    newEnemy->isActive = true;
    newEnemy->type = CHAR_TYPE_ENEMY;
    strncpy(newEnemy->name, typeToSpawn->name, MAX_CHAR_NAME_LENGTH - 1);
    newEnemy->sprite = FindSpriteInCache(typeToSpawn->name);
    
    if (newEnemy->sprite.id > 0) {
        newEnemy->width = newEnemy->sprite.width; newEnemy->height = newEnemy->sprite.height;
    } else { newEnemy->width = 36; newEnemy->height = 54; }
    
    EnemyAI_Init(newEnemy);

    int attempts = 0;
    while(attempts < 50) {
        int spawnX = GetRandomValue(0, activeSection->width - newEnemy->width);
        int spawnY = GetRandomValue(0, activeSection->height - newEnemy->height);
        Rectangle enemyRect = {(float)spawnX, (float)spawnY, (float)newEnemy->width, (float)newEnemy->height};
        bool collides = false;
        for (int i = 0; i < activeSection->collisionRectCount; i++) {
            if (CheckCollisionRecs(enemyRect, activeSection->collisionRects[i])) { collides = true; break; }
        }
        if (!collides) { newEnemy->position = (Vector2){(float)spawnX, (float)spawnY}; return; }
        attempts++;
    }
    newEnemy->isActive = false;
}

void CharManager_Init(void) {
    memset(s_mapCharacters, 0, sizeof(s_mapCharacters));
    s_enemySpriteCacheCount = 0;
}

void CharManager_UnloadAll(void) {
    for (int i = 0; i < s_enemySpriteCacheCount; i++) {
        if (s_enemySpriteCache[i].sprite.id > 0) UnloadTexture(s_enemySpriteCache[i].sprite);
    }
    s_enemySpriteCacheCount = 0;
    for (int i = 0; i < MAX_MAP_CHARACTERS; i++) {
        if (s_mapCharacters[i].isActive && s_mapCharacters[i].type == CHAR_TYPE_NPC) {
            UnloadTexture(s_mapCharacters[i].sprite);
        }
    }
    memset(s_mapCharacters, 0, sizeof(s_mapCharacters));
    s_spawnCheckTimer = 0.0f;
}

void CharManager_CacheSpritesForMap(void) {
    s_enemySpriteCacheCount = MapData_GetEnemyTypeConfigCount();
    for (int i = 0; i < s_enemySpriteCacheCount; i++) {
        const EnemyTypeConfig* config = MapData_GetEnemyTypeConfig(i);
        if (config) {
            strncpy(s_enemySpriteCache[i].name, config->name, MAX_CHAR_NAME_LENGTH - 1);
            char spritePath[256];
            sprintf(spritePath, "assets/characters/%s/walk_down_0.png", config->spriteFolder);
            s_enemySpriteCache[i].sprite = LoadTexture(spritePath);
        }
    }
}

void CharManager_LoadNpcsForMap(void) {
    int npcCount = MapData_GetNpcConfigCount();
    for (int i = 0; i < npcCount; i++) {
        const NpcConfig* config = MapData_GetNpcConfig(i);
        MapCharacter* newChar = FindEmptyCharacterSlot();
        if (newChar && config) {
            memset(newChar, 0, sizeof(MapCharacter));
            newChar->isActive = true;
            newChar->type = CHAR_TYPE_NPC;
            strncpy(newChar->name, config->name, MAX_CHAR_NAME_LENGTH - 1);
            strncpy(newChar->spriteFolder, config->spriteFolder, MAX_CHAR_SPRITE_FOLDER_LENGTH - 1);
            newChar->position = config->spawnCoords;
            newChar->data.npc.dialogueId = config->dialogueId;
            char spritePath[256];
            sprintf(spritePath, "assets/characters/%s/walk_down_0.png", newChar->spriteFolder);
            newChar->sprite = LoadTexture(spritePath);
            if(newChar->sprite.id > 0) { newChar->width = newChar->sprite.width; newChar->height = newChar->sprite.height; }
            else { newChar->width = 36; newChar->height = 54; }
        }
    }
}

/**
 * @brief ADICIONADO: Realiza o spawn inicial de inimigos ao carregar o mapa.
 */
void CharManager_InitialSpawn(const WorldSection* activeSection) {
    if (!activeSection) return;
    const MapConfig* mapConfig = MapData_GetConfig();
    // Verifica se o mapa foi carregado e se há uma chance de spawn.
    if (!mapConfig || !mapConfig->isLoaded || mapConfig->enemySpawnChance <= 0) {
        return;
    }

    // Define um número aleatório de inimigos para o spawn inicial.
    int numToSpawn = GetRandomValue(1, 3);
    TraceLog(LOG_INFO, "Tentando spawn inicial de %d inimigos.", numToSpawn);

    for (int i = 0; i < numToSpawn; i++) {
        // Usa a chance de spawn para cada tentativa de criação de inimigo.
        if (GetRandomValue(1, 100) <= mapConfig->enemySpawnChance) {
            SpawnRandomEnemy(activeSection);
        }
    }
}

void CharManager_Update(Player* player, const WorldSection* activeSection) {
    if (!player || !activeSection) return;
    const MapConfig* mapConfig = MapData_GetConfig();
    if (!mapConfig || !mapConfig->isLoaded) return;
    
    // O timer continua a funcionar para respawns periódicos.
    s_spawnCheckTimer += GetFrameTime();
    if (s_spawnCheckTimer >= SPAWN_CHECK_INTERVAL) {
        s_spawnCheckTimer = 0.0f;
        if (GetRandomValue(1, 100) <= mapConfig->enemySpawnChance) { SpawnRandomEnemy(activeSection); }
    }

    for (int i = 0; i < MAX_MAP_CHARACTERS; i++) {
        if (s_mapCharacters[i].isActive && s_mapCharacters[i].type == CHAR_TYPE_ENEMY) {
            MapCharacter* enemy = &s_mapCharacters[i];
            EnemyAI_Update(enemy, player, activeSection);
            Rectangle playerRect = {(float)player->posx, (float)player->posy, (float)player->width, (float)player->height};
            Rectangle enemyRect = {enemy->position.x, enemy->position.y, (float)enemy->width, (float)enemy->height};
            if (CheckCollisionRecs(playerRect, enemyRect)) {
                CharManager_TriggerBattle(g_players_ptr, *g_currentActivePlayers_ptr, enemy, g_currentScreen_ptr);
                enemy->isActive = false; 
                break;
            }
        }
    }
}

void CharManager_Draw(void) {
    for (int i = 0; i < MAX_MAP_CHARACTERS; i++) {
        if (s_mapCharacters[i].isActive) {
            MapCharacter* character = &s_mapCharacters[i];
            if (character->sprite.id > 0) {
                DrawTexture(character->sprite, (int)character->position.x, (int)character->position.y, WHITE);
            } else {
                DrawRectangleRec((Rectangle){character->position.x, character->position.y, (float)character->width, (float)character->height}, (character->type == CHAR_TYPE_ENEMY) ? RED : BLUE);
            }
        }
    }
}

void CharManager_CheckInteraction(Player* player) {
    if (!player || !IsKeyPressed(KEY_SPACE)) return;
    const float INTERACTION_RADIUS = 40.0f;
    for (int i = 0; i < MAX_MAP_CHARACTERS; i++) {
        if (s_mapCharacters[i].isActive && s_mapCharacters[i].type == CHAR_TYPE_NPC) {
            MapCharacter* npc = &s_mapCharacters[i];
            Vector2 playerCenter = { (float)player->posx + (float)player->width/2.0f, (float)player->posy + (float)player->height/2.0f };
            Vector2 npcCenter = { npc->position.x + (float)npc->width/2.0f, npc->position.y + (float)npc->height/2.0f };
            if (CheckCollisionPointCircle(playerCenter, npcCenter, INTERACTION_RADIUS)) {
                Dialogue_StartById(npc->data.npc.dialogueId);
                break;
            }
        }
    }
}

void CharManager_TriggerBattle(Player* players, int numPlayers, MapCharacter* enemy, GameState* screen_ptr) {
    if (!players || !enemy || !screen_ptr) return;
    BattleSystem_Start(players, numPlayers, enemy);
    *screen_ptr = GAMESTATE_BATTLE;
}