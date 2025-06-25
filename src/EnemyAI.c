#include "../include/EnemyAI.h"
#include "../include/CharacterManager.h" // *** CORREÇÃO: Incluído para a definição completa de MapCharacter ***
#include "../include/Classes.h"          // *** CORREÇÃO: Incluído para a definição completa de Player ***
#include "raymath.h"
#include <stdlib.h>

#define VISION_RANGE 250.0f
#define LOSE_AGGRO_RANGE 400.0f
#define ENEMY_SPEED 2.0f
#define IDLE_MOVE_INTERVAL 2.0f

void EnemyAI_Init(MapCharacter* character) {
    if (!character || character->type != CHAR_TYPE_ENEMY) return;
    character->data.enemy.aiState = AI_STATE_IDLE;
    character->data.enemy.moveTimer = (float)GetRandomValue(0, 100) / 100.0f * IDLE_MOVE_INTERVAL;
}

void EnemyAI_Update(MapCharacter* enemy, Player* player, const WorldSection* activeSection) {
    if (!enemy || !player || !activeSection || enemy->type != CHAR_TYPE_ENEMY) return;

    Vector2 enemyPos = enemy->position;
    Vector2 playerPos = { (float)player->posx, (float)player->posy };
    float distance = Vector2Distance(enemyPos, playerPos);

    switch (enemy->data.enemy.aiState) {
        case AI_STATE_IDLE:
            if (distance < VISION_RANGE) {
                enemy->data.enemy.aiState = AI_STATE_CHASING;
                TraceLog(LOG_INFO, "Inimigo %d avistou o jogador!", enemy->id);
            } else {
                enemy->data.enemy.moveTimer += GetFrameTime();
                if (enemy->data.enemy.moveTimer >= IDLE_MOVE_INTERVAL) {
                    enemy->data.enemy.moveTimer = 0.0f;
                    int moveX = GetRandomValue(-1, 1) * 10;
                    int moveY = GetRandomValue(-1, 1) * 10;
                    
                    Rectangle futureRect = {enemy->position.x + moveX, enemy->position.y + moveY, (float)enemy->width, (float)enemy->height};
                    bool willCollide = false;
                    for(int i = 0; i < activeSection->collisionRectCount; i++){
                        if(CheckCollisionRecs(futureRect, activeSection->collisionRects[i])){
                            willCollide = true;
                            break;
                        }
                    }
                    if(!willCollide){
                        enemy->position.x += moveX;
                        enemy->position.y += moveY;
                    }
                }
            }
            break;

        case AI_STATE_CHASING:
            if (distance > LOSE_AGGRO_RANGE) {
                enemy->data.enemy.aiState = AI_STATE_IDLE;
                TraceLog(LOG_INFO, "Inimigo %d perdeu o jogador de vista.", enemy->id);
            } else {
                Vector2 direction = Vector2Normalize(Vector2Subtract(playerPos, enemyPos));
                float moveX = direction.x * ENEMY_SPEED;
                float moveY = direction.y * ENEMY_SPEED;
                
                Rectangle futureRect = {enemy->position.x + moveX, enemy->position.y + moveY, (float)enemy->width, (float)enemy->height};
                bool willCollide = false;
                for(int i = 0; i < activeSection->collisionRectCount; i++){
                    if(CheckCollisionRecs(futureRect, activeSection->collisionRects[i])){
                        willCollide = true;
                        break;
                    }
                }
                if(!willCollide){
                    enemy->position.x += moveX;
                    enemy->position.y += moveY;
                }
            }
            break;
        
        case AI_STATE_COOLDOWN:
            enemy->data.enemy.aiState = AI_STATE_IDLE;
            break;
    }
}
