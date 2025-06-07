#include "../include/CharacterManager.h"
#include "../include/MapData.h"      // Para obter dados de configuração do mapa
#include "../include/Dialogue.h"    // Para iniciar diálogos
#include "raylib.h"
#include <stdlib.h> // Para GetRandomValue
#include <string.h> // Para memset

// --- Armazenamento Global de Personagens Ativos no Mapa ---
static MapCharacter s_mapCharacters[MAX_MAP_CHARACTERS];
static float s_spawnCheckTimer = 0.0f;
#define SPAWN_CHECK_INTERVAL 5.0f // Tenta spawnar um inimigo a cada 5 segundos

// --- Funções Internas ---
static MapCharacter* FindEmptyCharacterSlot(void) {
    for (int i = 0; i < MAX_MAP_CHARACTERS; i++) {
        if (!s_mapCharacters[i].isActive) {
            s_mapCharacters[i].id = i;
            return &s_mapCharacters[i];
        }
    }
    TraceLog(LOG_WARNING, "Nao foi possivel encontrar slot de personagem vazio.");
    return NULL;
}

static void SpawnRandomEnemy(const WorldSection* activeSection) {
    int numTypes = MapData_GetEnemyTypeConfigCount();
    if (numTypes == 0) return;
    
    MapCharacter* newEnemy = FindEmptyCharacterSlot();
    if (!newEnemy) return;

    const EnemyTypeConfig* typeToSpawn = MapData_GetEnemyTypeConfig(GetRandomValue(0, numTypes - 1));

    newEnemy->isActive = true;
    newEnemy->type = CHAR_TYPE_ENEMY;
    strncpy(newEnemy->name, typeToSpawn->name, MAX_CHAR_NAME_LENGTH - 1);
    strncpy(newEnemy->spriteFolder, typeToSpawn->spriteFolder, MAX_CHAR_SPRITE_FOLDER_LENGTH - 1);
    
    char spritePath[256];
    sprintf(spritePath, "assets/characters/%s/walk_down_0.png", newEnemy->spriteFolder);
    newEnemy->sprite = LoadTexture(spritePath);
    if(newEnemy->sprite.id > 0) { newEnemy->width = newEnemy->sprite.width; newEnemy->height = newEnemy->sprite.height; }
    else { newEnemy->width = 36; newEnemy->height = 54; } // Fallback

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
    // Falhou em encontrar local, libera o slot
    newEnemy->isActive = false;
    UnloadTexture(newEnemy->sprite);
}


// --- Funções Públicas ---
void CharManager_Init(void) {
    memset(s_mapCharacters, 0, sizeof(s_mapCharacters));
}

void CharManager_UnloadAll(void) {
    for (int i = 0; i < MAX_MAP_CHARACTERS; i++) {
        if (s_mapCharacters[i].isActive) {
            UnloadTexture(s_mapCharacters[i].sprite);
        }
    }
    memset(s_mapCharacters, 0, sizeof(s_mapCharacters));
    s_spawnCheckTimer = 0.0f; // Reseta timer de spawn
}

void CharManager_LoadNpcsForMap(void) {
    int npcCount = MapData_GetNpcConfigCount();
    TraceLog(LOG_INFO, "Carregando %d NPCs para o mapa...", npcCount);
    for (int i = 0; i < npcCount; i++) {
        const NpcConfig* config = MapData_GetNpcConfig(i);
        MapCharacter* newChar = FindEmptyCharacterSlot();
        if (newChar && config) {
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
            TraceLog(LOG_INFO, "NPC '%s' spawnado em (%.0f, %.0f)", newChar->name, newChar->position.x, newChar->position.y);
        }
    }
}

void CharManager_Update(Player* player, const WorldSection* activeSection) {
    if (!player || !activeSection) return;
    const MapConfig* mapConfig = MapData_GetConfig();
    if (!mapConfig || !mapConfig->isLoaded) return;
    
    s_spawnCheckTimer += GetFrameTime();
    if (s_spawnCheckTimer >= SPAWN_CHECK_INTERVAL) {
        s_spawnCheckTimer = 0.0f;
        if (GetRandomValue(1, 100) <= mapConfig->enemySpawnChance) {
            SpawnRandomEnemy(activeSection);
        }
    }
    for (int i = 0; i < MAX_MAP_CHARACTERS; i++) {
        if (s_mapCharacters[i].isActive && s_mapCharacters[i].type == CHAR_TYPE_ENEMY) {
            MapCharacter* enemy = &s_mapCharacters[i];
            enemy->data.enemy.moveTimer += GetFrameTime();
            if (enemy->data.enemy.moveTimer > 2.0f) {
                enemy->data.enemy.moveTimer = 0.0f;
                enemy->position.x += GetRandomValue(-1, 1) * 5;
                enemy->position.y += GetRandomValue(-1, 1) * 5;
                // TODO: Adicionar colisão de inimigos com o mapa
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
            Vector2 playerCenter = { player->posx + player->width/2.0f, player->posy + player->height/2.0f };
            Vector2 npcCenter = { npc->position.x + npc->width/2.0f, npc->position.y + npc->height/2.0f };
            if (CheckCollisionPointCircle(playerCenter, npcCenter, INTERACTION_RADIUS)) {
                Dialogue_StartById(npc->data.npc.dialogueId);
                break;
            }
        }
    }
}