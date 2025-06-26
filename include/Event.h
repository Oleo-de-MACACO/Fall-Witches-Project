#ifndef EVENT_H
#define EVENT_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_EVENT_ACTIONS 5
#define MAX_NAME_LENGTH 64
#define MAX_PATH_LENGTH 128

// --- Enum para os tipos de gatilhos de evento ---
typedef enum {
    TRIGGER_NONE,
    TRIGGER_ON_MAP_ENTRY_FIRST_TIME // Dispara na primeira vez que o jogador entra no mapa
} EventTriggerType;

// --- Estruturas para os dados das ações ---
typedef struct {
    char name[MAX_NAME_LENGTH];
    char spriteFiles[MAX_PATH_LENGTH]; // Caminho para a pasta dos sprites do NPC
    Vector2 spawnCoords;
} NpcSpawnData;

// *** CORRIGIDO: O enum para tipos de ação agora tem um nome (ActionType) ***
typedef enum {
    ACTION_NONE,
    ACTION_PLAY_MUSIC,
    ACTION_SPAWN_NPC,       // Para implementação futura
    ACTION_GIVE_ITEM,       // Para implementação futura
    ACTION_SHOW_DIALOGUE    // Para implementação futura
} ActionType;

// Estrutura para os dados de uma ação de evento
typedef struct {
    ActionType type; // Usa o novo tipo 'ActionType'

    // Union para armazenar dados específicos de cada tipo de ação
    union {
        struct {
            char category[MAX_NAME_LENGTH];
            char songName[MAX_NAME_LENGTH]; // Deixar em branco para aleatório
            bool loop;
        } musicData;
        
        NpcSpawnData npcSpawnData; // Renomeado para clareza

        struct {
            char itemName[MAX_NAME_LENGTH];
            int quantity;
        } itemData;

        int dialogueId;
    } data;
} EventAction;

// Estrutura principal de um evento do jogo
typedef struct {
    int id;
    int mapX;
    int mapY;
    EventTriggerType trigger;
    EventAction actions[MAX_EVENT_ACTIONS]; // Array de ações a serem executadas
    int actionCount;
    bool hasFired;
    bool isRepeatable;
} GameEvent;

// --- Funções do Módulo de Eventos ---
void Event_LoadAll(const char* eventsFilePath);
void Event_UnloadAll(void);
void Event_CheckAndRun(int currentMapX, int currentMapY);

#endif // EVENT_H