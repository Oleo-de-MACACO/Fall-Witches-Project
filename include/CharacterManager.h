#ifndef CHARACTER_MANAGER_H
#define CHARACTER_MANAGER_H

#include "Classes.h"
#include "WorldLoading.h"
#include "EnemyAI.h"      // *** CORREÇÃO: Inclui a definição de EnemyAIState em vez de redefini-la. ***
#include "Game.h"         // *** CORREÇÃO: Incluído para o tipo GameState. ***

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
 * @brief Inicia uma batalha.
 * @param players Ponteiro para o array de jogadores.
 * @param numPlayers Número de jogadores ativos.
 * @param enemy O inimigo encontrado.
 * @param screen_ptr Ponteiro para o estado de tela atual do jogo.
 */
// *** CORREÇÃO: A assinatura da função foi atualizada para corresponder à sua chamada. ***
void CharManager_TriggerBattle(Player* players, int numPlayers, MapCharacter* enemy, GameState* screen_ptr);

#endif // CHARACTER_MANAGER_H
