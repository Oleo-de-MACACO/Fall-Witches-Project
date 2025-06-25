#include "../include/WalkCycle.h"
#include <stdio.h>
#include <string.h>
#include <math.h> // *** ADICIONADO PARA fabsf ***
#include "raylib.h"


const char* CHARACTER_ASSETS_BASE_PATH = "assets/characters/";

static void SafeUnloadTexture(Texture2D texture) {
    if (texture.id > 0) {
        UnloadTexture(texture);
    }
}

void LoadCharacterAnimations(Player *player) {
    if (!player) {
        TraceLog(LOG_ERROR, "LoadCharacterAnimations: Ponteiro do jogador e NULL.");
        return;
    }
    UnloadCharacterAnimations(player);
    const char* folderName = NULL;
    switch (player->spriteType) {
        case SPRITE_TYPE_HUMANO:  folderName = "player1"; break;
        case SPRITE_TYPE_DEMONIO: folderName = "player2"; break;
        default:
            folderName = "player1";
            TraceLog(LOG_WARNING, "SpriteType desconhecido (%d) para jogador %s, usando assets de '%s' como fallback.",
                     player->spriteType, player->nome, folderName);
            player->spriteType = SPRITE_TYPE_HUMANO;
            break;
    }
    char filePath[256];
    bool sizeSet = false;

    for (int i = 0; i < NUM_WALK_UP_FRAMES; i++) {
        sprintf(filePath, "%s%s/walk_up_%d.png", CHARACTER_ASSETS_BASE_PATH, folderName, i);
        player->walkUpFrames[i] = LoadTexture(filePath);
        if (player->walkUpFrames[i].id == 0) { TraceLog(LOG_WARNING, "Falha ao carregar animacao: %s", filePath); }
        else if (!sizeSet) { player->width = player->walkUpFrames[i].width; player->height = player->walkUpFrames[i].height; sizeSet = true; TraceLog(LOG_INFO, "Dimensoes de '%s': %dx%d (de %s)", player->nome, player->width, player->height, filePath);}
    }
    for (int i = 0; i < NUM_WALK_DOWN_FRAMES; i++) {
        sprintf(filePath, "%s%s/walk_down_%d.png", CHARACTER_ASSETS_BASE_PATH, folderName, i);
        player->walkDownFrames[i] = LoadTexture(filePath);
        if (player->walkDownFrames[i].id == 0) { TraceLog(LOG_WARNING, "Falha ao carregar animacao: %s", filePath); }
        else if (!sizeSet) { player->width = player->walkDownFrames[i].width; player->height = player->walkDownFrames[i].height; sizeSet = true; TraceLog(LOG_INFO, "Dimensoes de '%s': %dx%d (de %s)", player->nome, player->width, player->height, filePath);}
    }
    for (int i = 0; i < NUM_WALK_LEFT_FRAMES; i++) {
        sprintf(filePath, "%s%s/walk_left_%d.png", CHARACTER_ASSETS_BASE_PATH, folderName, i);
        player->walkLeftFrames[i] = LoadTexture(filePath);
        if (player->walkLeftFrames[i].id == 0) { TraceLog(LOG_WARNING, "Falha ao carregar animacao: %s", filePath); }
        else if (!sizeSet) { player->width = player->walkLeftFrames[i].width; player->height = player->walkLeftFrames[i].height; sizeSet = true; TraceLog(LOG_INFO, "Dimensoes de '%s': %dx%d (de %s)", player->nome, player->width, player->height, filePath);}
    }
    for (int i = 0; i < NUM_WALK_RIGHT_FRAMES; i++) {
        sprintf(filePath, "%s%s/walk_right_%d.png", CHARACTER_ASSETS_BASE_PATH, folderName, i);
        player->walkRightFrames[i] = LoadTexture(filePath);
        if (player->walkRightFrames[i].id == 0) { TraceLog(LOG_WARNING, "Falha ao carregar animacao: %s", filePath); }
        else if (!sizeSet) { player->width = player->walkRightFrames[i].width; player->height = player->walkRightFrames[i].height; sizeSet = true; TraceLog(LOG_INFO, "Dimensoes de '%s': %dx%d (de %s)", player->nome, player->width, player->height, filePath);}
    }
    if (!sizeSet) {
        TraceLog(LOG_ERROR, "Nenhuma textura carregada para '%s'. Usando dimensoes fallback de init_player.", player->nome);
        // init_player já definiu um fallback para width/height
    }
    TraceLog(LOG_INFO, "Animacoes carregadas para: %s. Dimensoes Finais: %dx%d", player->nome, player->width, player->height);
}

void UnloadCharacterAnimations(Player *player) {
    if (!player) return;
    for (int i = 0; i < NUM_WALK_UP_FRAMES; i++) SafeUnloadTexture(player->walkUpFrames[i]);
    for (int i = 0; i < NUM_WALK_DOWN_FRAMES; i++) SafeUnloadTexture(player->walkDownFrames[i]);
    for (int i = 0; i < NUM_WALK_LEFT_FRAMES; i++) SafeUnloadTexture(player->walkLeftFrames[i]);
    for (int i = 0; i < NUM_WALK_RIGHT_FRAMES; i++) SafeUnloadTexture(player->walkRightFrames[i]);
    memset(player->walkUpFrames, 0, sizeof(player->walkUpFrames));
    memset(player->walkDownFrames, 0, sizeof(player->walkDownFrames));
    memset(player->walkLeftFrames, 0, sizeof(player->walkLeftFrames));
    memset(player->walkRightFrames, 0, sizeof(player->walkRightFrames));
}

void UpdateWalkCycle(Player *player, bool isMovingAgora, float moveX, float moveY) {
    if (!player) return;
    player->isMoving = isMovingAgora;

    if (player->isMoving) {
        if (fabsf(moveX) > fabsf(moveY)) { // fabsf de math.h
            if (moveX < 0) player->facingDir = DIR_LEFT;
            else if (moveX > 0) player->facingDir = DIR_RIGHT;
        } else if (fabsf(moveY) > 0) { // Só muda direção vertical se houver movimento vertical
             if (moveY < 0) player->facingDir = DIR_UP;
             else if (moveY > 0) player->facingDir = DIR_DOWN;
        } // Se moveX e moveY são 0, isMoving seria false. Se moveX==moveY e não são zero, a lógica acima prioriza X.

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
    Texture2D* targetFrames = NULL;
    int numFramesInDir = 0;
    int frameIndexToUse = player->isMoving ? player->currentAnimFrame : 0;

    switch (player->facingDir) {
        case DIR_UP:    targetFrames = player->walkUpFrames;    numFramesInDir = NUM_WALK_UP_FRAMES;    break;
        case DIR_DOWN:  targetFrames = player->walkDownFrames;  numFramesInDir = NUM_WALK_DOWN_FRAMES;  break;
        case DIR_LEFT:  targetFrames = player->walkLeftFrames;  numFramesInDir = NUM_WALK_LEFT_FRAMES;  break;
        case DIR_RIGHT: targetFrames = player->walkRightFrames; numFramesInDir = NUM_WALK_RIGHT_FRAMES; break;
        default:        targetFrames = player->walkDownFrames;  numFramesInDir = NUM_WALK_DOWN_FRAMES;  break;
    }
    if (targetFrames != NULL && numFramesInDir > 0) {
        int safeFrameIndex = frameIndexToUse % numFramesInDir;
        if (targetFrames[safeFrameIndex].id > 0) { return targetFrames[safeFrameIndex]; }
    }
    if (NUM_WALK_DOWN_FRAMES > 0 && player->walkDownFrames[0].id > 0) { return player->walkDownFrames[0]; }
    return emptyTexture;
}