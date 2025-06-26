#ifndef CHARACTER_MANAGER_H
#define CHARACTER_MANAGER_H

#include "Classes.h"
#include "WorldLoading.h"
#include "EnemyAI.h"      
#include "Game.h"         

#define MAX_MAP_CHARACTERS 16

typedef enum {
    CHAR_TYPE_NPC,
    CHAR_TYPE_ENEMY
} CharacterType;

typedef struct MapCharacter {
    int id;
    CharacterType type;
    char name[MAX_CHAR_NAME_LENGTH];
    char spriteFolder[MAX_CHAR_SPRITE_FOLDER_LENGTH];

    Vector2 position;
    int width;
    int height;
    
    Texture2D sprite;

    union {
        struct {
            int dialogueId;
        } npc;
        struct {
            int hp;
            int attack;
            float moveTimer;
            EnemyAIState aiState; 
        } enemy;
    } data;
    
    bool isActive;
} MapCharacter;


void CharManager_Init(void);
void CharManager_UnloadAll(void);
void CharManager_CacheSpritesForMap(void);
void CharManager_LoadNpcsForMap(void);
void CharManager_Update(Player* player, const WorldSection* activeSection);
void CharManager_Draw(void);
void CharManager_CheckInteraction(Player* player);

/**
 * @brief Tenta gerar um inimigo imediatamente, respeitando a chance de spawn do mapa.
 * Ideal para ser chamada ao entrar em um novo mapa.
 * @param activeSection A seção do mundo onde o inimigo deve ser gerado.
 */
void CharManager_TryInitialSpawn(const WorldSection* activeSection);

/**
 * @brief Inicia uma batalha.
 * @param players Ponteiro para o array de jogadores.
 * @param numPlayers Número de jogadores ativos.
 * @param enemy O inimigo encontrado.
 * @param screen_ptr Ponteiro para o estado de tela atual do jogo.
 */
void CharManager_TriggerBattle(Player* players, int numPlayers, MapCharacter* enemy, GameState* screen_ptr);

#endif // CHARACTER_MANAGER_H