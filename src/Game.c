#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Singleplayer.h"
#include "../include/WalkCycle.h"
#include "../include/WorldMap.h"
#include "../include/WorldLoading.h"
#include "../include/Event.h"
#include "../include/Dialogue.h"
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Globais de main.c, declaradas extern em Game.h
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern const int gameSectionWidthMultiplier;
extern const int gameSectionHeightMultiplier;

/**
 * @brief Atualiza a câmera para centralizar nos jogadores, com limites de zoom dinâmicos.
 */
void UpdateCameraCenteredOnPlayers(Camera2D *camera, Player players[], int numActivePlayers, float sectionActualWidth, float sectionActualHeight) {
    if (!camera || !players || numActivePlayers <= 0 || sectionActualWidth <= 0 || sectionActualHeight <= 0) return;

    // --- Limites de Zoom ---
    // Máximo Zoom In (mais perto): Definido por uma largura de mundo desejada.
    const float TARGET_VISIBLE_WIDTH_AT_MAX_ZOOM = 320.0f; // Ex: Largura da sua imagem de referência
    const float MAX_CAMERA_ZOOM = (TARGET_VISIBLE_WIDTH_AT_MAX_ZOOM > 0) ? ((float)virtualScreenWidth / TARGET_VISIBLE_WIDTH_AT_MAX_ZOOM) : 1.0f;

    // Mínimo Zoom Out (mais longe): Definido pela necessidade de caber a seção atual na tela.
    float zoomToFitWidth = (float)virtualScreenWidth / sectionActualWidth;
    float zoomToFitHeight = (float)virtualScreenHeight / sectionActualHeight;
    const float MIN_CAMERA_ZOOM_FIT_SECTION = fminf(zoomToFitWidth, zoomToFitHeight);

    // --- Cálculo do Zoom e Target ---
    if (numActivePlayers == 1) {
        camera->target.x = (float)players[0].posx + ((float)players[0].width / 2.0f);
        camera->target.y = (float)players[0].posy + ((float)players[0].height / 2.0f);
        camera->zoom = 1.2f; // Um bom zoom padrão para 1 jogador
    } else if (numActivePlayers >= 2) {
        // Calcula o zoom dinâmico para manter ambos os jogadores na tela
        camera->target.x = (((float)players[0].posx + ((float)players[0].width / 2.0f)) + ((float)players[1].posx + ((float)players[1].width / 2.0f))) / 2.0f;
        camera->target.y = (((float)players[0].posy + ((float)players[0].height / 2.0f)) + ((float)players[1].posy + ((float)players[1].height / 2.0f))) / 2.0f;
        float minX = fminf((float)players[0].posx, (float)players[1].posx);
        float maxX = fmaxf((float)players[0].posx + (float)players[0].width, (float)players[1].posx + (float)players[1].width);
        float minY = fminf((float)players[0].posy, (float)players[1].posy);
        float maxY = fmaxf((float)players[0].posy + (float)players[0].height, (float)players[1].posy + (float)players[1].height);
        float paddedWidth = (maxX - minX) * 1.2f;
        float paddedHeight = (maxY - minY) * 1.2f;
        float zoomRequiredX = (paddedWidth > 0) ? ((float)virtualScreenWidth / paddedWidth) : MAX_CAMERA_ZOOM;
        float zoomRequiredY = (paddedHeight > 0) ? ((float)virtualScreenHeight / paddedHeight) : MAX_CAMERA_ZOOM;
        camera->zoom = fminf(zoomRequiredX, zoomRequiredY);
    }

    // Aplica os limites de zoom
    if (camera->zoom < MIN_CAMERA_ZOOM_FIT_SECTION) camera->zoom = MIN_CAMERA_ZOOM_FIT_SECTION;
    if (camera->zoom > MAX_CAMERA_ZOOM) camera->zoom = MAX_CAMERA_ZOOM;
    
    // --- Limites de Posição da Câmera ---
    float visibleWorldHalfWidth = ((float)virtualScreenWidth / camera->zoom) / 2.0f;
    float visibleWorldHalfHeight = ((float)virtualScreenHeight / camera->zoom) / 2.0f;
    camera->target.x = fmaxf(visibleWorldHalfWidth, camera->target.x);
    camera->target.x = fminf(sectionActualWidth - visibleWorldHalfWidth, camera->target.x);
    camera->target.y = fmaxf(visibleWorldHalfHeight, camera->target.y);
    camera->target.y = fminf(sectionActualHeight - visibleWorldHalfHeight, camera->target.y);
    
    camera->offset = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f };
}

/**
 * @brief Movimenta um personagem, checando colisões iterativamente para evitar "tunneling".
 */
void move_character(Player *player_obj, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift, const WorldSection* activeSection) {
    if (!player_obj || !activeSection || !activeSection->isLoaded) return;
    int speed = 3; int sprintSpeed = 5;
    int currentSpeed = IsKeyDown(keyShift) ? sprintSpeed : speed;
    float totalDeltaX = 0; float totalDeltaY = 0;
    if (IsKeyDown(keyLeft))  totalDeltaX -= currentSpeed;
    if (IsKeyDown(keyRight)) totalDeltaX += currentSpeed;
    if (IsKeyDown(keyUp))    totalDeltaY -= currentSpeed;
    if (IsKeyDown(keyDown))  totalDeltaY += currentSpeed;

    // Movimento X, passo a passo
    for (int i = 0; i < fabsf(totalDeltaX); i++) {
        int stepX = (totalDeltaX > 0) ? 1 : -1; int oldPosX = player_obj->posx;
        player_obj->posx += stepX;
        Rectangle playerRect = { (float)player_obj->posx, (float)player_obj->posy, (float)player_obj->width, (float)player_obj->height };
        bool collisionFound = false;
        for (int c = 0; c < activeSection->collisionRectCount; c++) {
            if (CheckCollisionRecs(playerRect, activeSection->collisionRects[c])) {
                player_obj->posx = oldPosX; collisionFound = true; break;
            }
        } if (collisionFound) break;
    }
    // Movimento Y, passo a passo
    for (int i = 0; i < fabsf(totalDeltaY); i++) {
        int stepY = (totalDeltaY > 0) ? 1 : -1; int oldPosY = player_obj->posy;
        player_obj->posy += stepY;
        Rectangle playerRect = { (float)player_obj->posx, (float)player_obj->posy, (float)player_obj->width, (float)player_obj->height };
        bool collisionFound = false;
        for (int c = 0; c < activeSection->collisionRectCount; c++) {
            if (CheckCollisionRecs(playerRect, activeSection->collisionRects[c])) {
                player_obj->posy = oldPosY; collisionFound = true; break;
            }
        } if (collisionFound) break;
    }
    // Clamp aos limites da seção
    if (player_obj->posx < 0) player_obj->posx = 0;
    if (player_obj->posx > activeSection->width - player_obj->width) player_obj->posx = activeSection->width - player_obj->width;
    if (player_obj->posy < 0) player_obj->posy = 0;
    if (player_obj->posy > activeSection->height - player_obj->height) player_obj->posy = activeSection->height - player_obj->height;
}

void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]) { (void)mainPlaylist_arr; if(players_arr == NULL) return; }

void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY, int numActivePlayers, const WorldSection* worldSection) {
    if (!players_arr || !mapX || !mapY ) return;
    *mapX = 0; *mapY = 0;
    for (int i = 0; i < numActivePlayers; i++) {
        int p_width = (players_arr[i].width > 0) ? players_arr[i].width : 111;
        int p_height = (players_arr[i].height > 0) ? players_arr[i].height : 150;
        if (worldSection && worldSection->isLoaded && worldSection->width > 0) {
            float centerX = (float)worldSection->width / 2.0f;
            float centerY = (float)worldSection->height / 2.0f;
            if (numActivePlayers == 1) { players_arr[i].posx = (int)(centerX - (float)p_width / 2.0f); }
            else { players_arr[i].posx = (int)(centerX - (float)p_width + (i * ((float)p_width + 20.0f))); }
            players_arr[i].posy = (int)(centerY - (float)p_height / 2.0f);
        } else {
            int cSectH_fallback = gameSectionHeightMultiplier * virtualScreenHeight;
            players_arr[i].posx = p_width + 50 + (i * 100);
            players_arr[i].posy = cSectH_fallback / 2 - p_height / 2;
        }
    }
}

void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], int numActivePlayers,
                         Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, bool *isPlaying_ptr,
                         float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr,
                         int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera,
                         const WorldSection* activeSection) {
    if (!currentScreen_ptr || !activeSection || !activeSection->isLoaded) { return; }
    
    // Armazena posições antigas para animação
    int oldPosX[MAX_PLAYERS_SUPPORTED] = {0}; int oldPosY[MAX_PLAYERS_SUPPORTED] = {0};
    for (int i = 0; i < numActivePlayers; i++) { oldPosX[i] = players_arr[i].posx; oldPosY[i] = players_arr[i].posy; }
    
    // Lógica de movimento e colisão
    if (currentGameMode == GAME_MODE_SINGLE_PLAYER) {
        if (numActivePlayers > 0) { SinglePlayer_HandleMovement(&players_arr[0], activeSection); }
    } else {
        if (numActivePlayers > 0) { move_character(&players_arr[0], KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT, activeSection); }
        if (numActivePlayers > 1) { move_character(&players_arr[1], KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT, activeSection); }
    }
    
    // Atualiza animações
    for (int i = 0; i < numActivePlayers; i++) {
        bool isMovingNow = (players_arr[i].posx != oldPosX[i] || players_arr[i].posy != oldPosY[i]);
        float mX = (float)players_arr[i].posx - (float)oldPosX[i];
        float mY = (float)players_arr[i].posy - (float)oldPosY[i];
        UpdateWalkCycle(&players_arr[i], isMovingNow, mX, mY);
    }

    // Checa transição de mapa
    WorldMap_CheckTransition(players_arr, numActivePlayers, currentMapX_ptr, currentMapY_ptr, currentGameMode, (float)activeSection->width, (float)activeSection->height);
    
    // Checa input de pausa/inventário
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { *currentScreen_ptr = GAMESTATE_PAUSE; }
    if (IsKeyPressed(KEY_E)) { *currentScreen_ptr = GAMESTATE_INVENTORY; }
}

void DrawPlayingScreen(Player players_arr[], int numActivePlayers, float currentVolume, int currentMusicIndex, bool isPlaying, int currentMapX, int currentMapY) {
    // A função que chama (em main.c) já limpou o fundo com DrawWorldSectionBackground
    for (int i = 0; i < numActivePlayers; i++) {
        Texture2D currentSprite = GetCurrentCharacterSprite(&players_arr[i]);
        if (currentSprite.id > 0) {
            DrawTexturePro(currentSprite, (Rectangle){ 0.0f, 0.0f, (float)currentSprite.width, (float)currentSprite.height },
                           (Rectangle){ (float)players_arr[i].posx, (float)players_arr[i].posy, (float)players_arr[i].width, (float)players_arr[i].height },
                           (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
        } else {
            DrawRectangle(players_arr[i].posx, players_arr[i].posy, players_arr[i].width, players_arr[i].height, GRAY);
        }
    }
    // O HUD idealmente é desenhado em main.c após EndMode2D para ser fixo na tela
}