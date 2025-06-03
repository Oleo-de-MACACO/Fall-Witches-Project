#include "../include/WalkCycle.h"
#include <stdio.h>  
#include <string.h> 
#include "raylib.h" 

const char* CHARACTER_ASSETS_BASE_PATH = "assets/characters/";

static void SafeUnloadTexture(Texture2D texture) {
    if (texture.id > 0) {
        UnloadTexture(texture);
    }
}

void LoadCharacterAnimations(Player *player) {
    if (!player) {
        TraceLog(LOG_ERROR, "LoadCharacterAnimations: Player pointer is NULL.");
        return;
    }

    // Primeiro, descarrega quaisquer animações antigas para evitar memory leaks se chamado múltiplas vezes
    UnloadCharacterAnimations(player); // Garante que não há texturas antigas

    const char* folderName = NULL;
    switch (player->spriteType) {
        case SPRITE_TYPE_HUMANO:  folderName = "player1"; break; 
        case SPRITE_TYPE_DEMONIO: folderName = "player2"; break; 
        default:                  
            folderName = "player1"; // Fallback para humano se spriteType for inválido
            TraceLog(LOG_WARNING, "SpriteType desconhecido (%d) para jogador %s, usando assets de '%s' como fallback.", 
                     player->spriteType, player->nome, folderName);
            player->spriteType = SPRITE_TYPE_HUMANO; // Corrige o spriteType para o fallback
            break;
    }

    char filePath[256];

    for (int i = 0; i < NUM_WALK_UP_FRAMES; i++) {
        sprintf(filePath, "%s%s/walk_up_%d.png", CHARACTER_ASSETS_BASE_PATH, folderName, i);
        player->walkUpFrames[i] = LoadTexture(filePath);
        if (player->walkUpFrames[i].id == 0) TraceLog(LOG_WARNING, "Falha ao carregar animacao: %s", filePath);
    }
    for (int i = 0; i < NUM_WALK_DOWN_FRAMES; i++) {
        sprintf(filePath, "%s%s/walk_down_%d.png", CHARACTER_ASSETS_BASE_PATH, folderName, i);
        player->walkDownFrames[i] = LoadTexture(filePath);
        if (player->walkDownFrames[i].id == 0) TraceLog(LOG_WARNING, "Falha ao carregar animacao: %s", filePath);
    }
    for (int i = 0; i < NUM_WALK_LEFT_FRAMES; i++) {
        sprintf(filePath, "%s%s/walk_left_%d.png", CHARACTER_ASSETS_BASE_PATH, folderName, i);
        player->walkLeftFrames[i] = LoadTexture(filePath);
        if (player->walkLeftFrames[i].id == 0) TraceLog(LOG_WARNING, "Falha ao carregar animacao: %s", filePath);
    }
    for (int i = 0; i < NUM_WALK_RIGHT_FRAMES; i++) {
        sprintf(filePath, "%s%s/walk_right_%d.png", CHARACTER_ASSETS_BASE_PATH, folderName, i);
        player->walkRightFrames[i] = LoadTexture(filePath);
        if (player->walkRightFrames[i].id == 0) TraceLog(LOG_WARNING, "Falha ao carregar animacao: %s", filePath);
    }
    
    // init_player já define currentAnimFrame, frameTimer, frameDuration, facingDir, isMoving.
    // Não é necessário resetar aqui se LoadCharacterAnimations é chamado logo após init_player
    // ou após carregar um save que já tem esses valores.

    TraceLog(LOG_INFO, "Animacoes carregadas para: %s (Pasta: %s)", player->nome, folderName);
}

void UnloadCharacterAnimations(Player *player) {
    if (!player) return;
    for (int i = 0; i < NUM_WALK_UP_FRAMES; i++) SafeUnloadTexture(player->walkUpFrames[i]);
    for (int i = 0; i < NUM_WALK_DOWN_FRAMES; i++) SafeUnloadTexture(player->walkDownFrames[i]);
    for (int i = 0; i < NUM_WALK_LEFT_FRAMES; i++) SafeUnloadTexture(player->walkLeftFrames[i]);
    for (int i = 0; i < NUM_WALK_RIGHT_FRAMES; i++) SafeUnloadTexture(player->walkRightFrames[i]);
    // Para garantir que os arrays não apontem para texturas descarregadas:
    memset(player->walkUpFrames, 0, sizeof(player->walkUpFrames));
    memset(player->walkDownFrames, 0, sizeof(player->walkDownFrames));
    memset(player->walkLeftFrames, 0, sizeof(player->walkLeftFrames));
    memset(player->walkRightFrames, 0, sizeof(player->walkRightFrames));
}

void UpdateWalkCycle(Player *player, bool isMovingAhora, float moveX, float moveY) {
    if (!player) return;
    player->isMoving = isMovingAhora;
    if (player->isMoving) {
        if (moveY < 0) player->facingDir = DIR_UP;
        else if (moveY > 0) player->facingDir = DIR_DOWN;
        else if (moveX < 0) player->facingDir = DIR_LEFT;
        else if (moveX > 0) player->facingDir = DIR_RIGHT;
        player->frameTimer += GetFrameTime();
        if (player->frameTimer >= player->frameDuration) {
            player->frameTimer = 0.0f;
            player->currentAnimFrame++;
            int numFramesForCurrentDir = 0;
            switch (player->facingDir) {
                case DIR_UP:   numFramesForCurrentDir = NUM_WALK_UP_FRAMES; break;
                case DIR_DOWN: numFramesForCurrentDir = NUM_WALK_DOWN_FRAMES; break;
                case DIR_LEFT: numFramesForCurrentDir = NUM_WALK_LEFT_FRAMES; break;
                case DIR_RIGHT:numFramesForCurrentDir = NUM_WALK_RIGHT_FRAMES; break;
                default:       numFramesForCurrentDir = NUM_WALK_DOWN_FRAMES; break; 
            }
            if (numFramesForCurrentDir > 0 && player->currentAnimFrame >= numFramesForCurrentDir) {
                player->currentAnimFrame = 0; 
            } else if (numFramesForCurrentDir == 0) { player->currentAnimFrame = 0; }
        }
    } else { player->currentAnimFrame = 0; player->frameTimer = 0.0f; }
}

Texture2D GetCurrentCharacterSprite(Player *player) {
    Texture2D emptyTexture = { .id = 0, .width = 0, .height = 0, .mipmaps = 0, .format = 0 };
    if (!player) return emptyTexture; 
    if (!player->isMoving) {
        switch (player->facingDir) {
            case DIR_UP:    return (NUM_WALK_UP_FRAMES > 0 && player->walkUpFrames[0].id > 0) ? player->walkUpFrames[0] : emptyTexture;
            case DIR_DOWN:  return (NUM_WALK_DOWN_FRAMES > 0 && player->walkDownFrames[0].id > 0) ? player->walkDownFrames[0] : emptyTexture;
            case DIR_LEFT:  return (NUM_WALK_LEFT_FRAMES > 0 && player->walkLeftFrames[0].id > 0) ? player->walkLeftFrames[0] : emptyTexture;
            case DIR_RIGHT: return (NUM_WALK_RIGHT_FRAMES > 0 && player->walkRightFrames[0].id > 0) ? player->walkRightFrames[0] : emptyTexture;
            default:        return (NUM_WALK_DOWN_FRAMES > 0 && player->walkDownFrames[0].id > 0) ? player->walkDownFrames[0] : emptyTexture; 
        }
    } else { 
        switch (player->facingDir) {
            case DIR_UP:    return (NUM_WALK_UP_FRAMES > 0 && player->walkUpFrames[player->currentAnimFrame % NUM_WALK_UP_FRAMES].id > 0) ? player->walkUpFrames[player->currentAnimFrame % NUM_WALK_UP_FRAMES] : emptyTexture;
            case DIR_DOWN:  return (NUM_WALK_DOWN_FRAMES > 0 && player->walkDownFrames[player->currentAnimFrame % NUM_WALK_DOWN_FRAMES].id > 0) ? player->walkDownFrames[player->currentAnimFrame % NUM_WALK_DOWN_FRAMES] : emptyTexture;
            case DIR_LEFT:  return (NUM_WALK_LEFT_FRAMES > 0 && player->walkLeftFrames[player->currentAnimFrame % NUM_WALK_LEFT_FRAMES].id > 0) ? player->walkLeftFrames[player->currentAnimFrame % NUM_WALK_LEFT_FRAMES] : emptyTexture;
            case DIR_RIGHT: return (NUM_WALK_RIGHT_FRAMES > 0 && player->walkRightFrames[player->currentAnimFrame % NUM_WALK_RIGHT_FRAMES].id > 0) ? player->walkRightFrames[player->currentAnimFrame % NUM_WALK_RIGHT_FRAMES] : emptyTexture;
            default:        return (NUM_WALK_DOWN_FRAMES > 0 && player->walkDownFrames[0].id > 0) ? player->walkDownFrames[0] : emptyTexture; 
        }
    }
}