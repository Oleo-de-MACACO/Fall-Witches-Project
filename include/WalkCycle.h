#ifndef WALKCICLE_H // Corrigido para WALKCICLE_H para corresponder ao ifndef
#define WALKCICLE_H

#include "raylib.h"
#include "Classes.h" 

/**
 * @brief Carrega as texturas de animação para um jogador baseado no seu player->spriteType.
 * Assume que as texturas estão em basePath/spriteTypeName/direction_frame.png
 * Exemplo: ./assets/characters/humano/walk_up_0.png
 * @param player Ponteiro para o jogador para carregar as animações.
 */
void LoadCharacterAnimations(Player *player); // Removido characterFolderName

void UnloadCharacterAnimations(Player *player);
void UpdateWalkCycle(Player *player, bool isMovingAgora, float moveX, float moveY);
Texture2D GetCurrentCharacterSprite(Player *player);

#endif // WALKCICLE_H