#ifndef CHARACTER_MANAGER_H
#define CHARACTER_MANAGER_H

#include "Classes.h"      // Para a struct Player
#include "WorldLoading.h" // Para colisões de spawn

#define MAX_MAP_CHARACTERS 16 // Máximo de NPCs + Inimigos em uma seção de mapa por vez

/**
 * @brief Define se um personagem é um NPC amigável ou um Inimigo hostil.
 */
typedef enum {
    CHAR_TYPE_NPC,
    CHAR_TYPE_ENEMY
} CharacterType;

/**
 * @brief Estrutura para um personagem não-jogador (NPC ou Inimigo) no mundo.
 */
typedef struct {
    int id; // ID único na instância do mapa
    CharacterType type;
    char name[MAX_CHAR_NAME_LENGTH];
    char spriteFolder[MAX_CHAR_SPRITE_FOLDER_LENGTH];

    Vector2 position;
    int width;
    int height;
    
    Texture2D sprite; // Sprite atual (simplificado, para animações completas, usar sistema do Player)

    // Dados específicos do tipo de personagem
    union {
        struct { // Dados se for um NPC
            int dialogueId;
        } npc;
        struct { // Dados se for um Inimigo
            int hp;
            int attack;
            float moveTimer; // Para controlar a frequência de movimento da IA
        } enemy;
    } data;
    
    bool isActive; // Para controlar se este slot no array está em uso
} MapCharacter;


// --- Funções do Módulo ---
void CharManager_Init(void); // Inicializa o sistema de gerenciamento de personagens
void CharManager_UnloadAll(void); // Descarrega todos os personagens
void CharManager_LoadNpcsForMap(void); // Lê dados do MapData e spawna NPCs estáticos
void CharManager_Update(Player* player, const WorldSection* activeSection); // Atualiza lógica (IA, spawn)
void CharManager_Draw(void); // Desenha os personagens
void CharManager_CheckInteraction(Player* player); // Verifica interação com NPCs

#endif // CHARACTER_MANAGER_H