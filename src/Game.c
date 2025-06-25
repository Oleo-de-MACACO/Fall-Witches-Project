#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Singleplayer.h"
#include "../include/WalkCycle.h"
#include "../include/WorldMap.h"
#include "../include/WorldLoading.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern const int gameSectionWidthMultiplier;
extern const int gameSectionHeightMultiplier;

void UpdateCameraCenteredOnPlayers(Camera2D *camera, Player players[], int numActivePlayers, float sectionActualWidth, float sectionActualHeight) {
    if (!camera || !players || numActivePlayers <= 0 || sectionActualWidth <= 0 || sectionActualHeight <= 0) return;

    const float TARGET_VISIBLE_WIDTH_AT_MAX_ZOOM = 320.0f;
    const float MAX_CAMERA_ZOOM = (TARGET_VISIBLE_WIDTH_AT_MAX_ZOOM > 0) ? ((float)virtualScreenWidth / TARGET_VISIBLE_WIDTH_AT_MAX_ZOOM) : 1.0f;

    float zoomToFitWidth = (float)virtualScreenWidth / sectionActualWidth;
    float zoomToFitHeight = (float)virtualScreenHeight / sectionActualHeight;
    const float MIN_CAMERA_ZOOM_FIT_SECTION = fminf(zoomToFitWidth, zoomToFitHeight);

    if (numActivePlayers == 1) {
        camera->target.x = (float)players[0].posx + ((float)players[0].width / 2.0f);
        camera->target.y = (float)players[0].posy + ((float)players[0].height / 2.0f);
        camera->zoom = 1.2f;
    } else if (numActivePlayers >= 2) {
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

    if (camera->zoom < MIN_CAMERA_ZOOM_FIT_SECTION) camera->zoom = MIN_CAMERA_ZOOM_FIT_SECTION;
    if (camera->zoom > MAX_CAMERA_ZOOM) camera->zoom = MAX_CAMERA_ZOOM;
    
    float visibleWorldHalfWidth = ((float)virtualScreenWidth / camera->zoom) / 2.0f;
    float visibleWorldHalfHeight = ((float)virtualScreenHeight / camera->zoom) / 2.0f;
    camera->target.x = fmaxf(visibleWorldHalfWidth, camera->target.x);
    camera->target.x = fminf(sectionActualWidth - visibleWorldHalfWidth, camera->target.x);
    camera->target.y = fmaxf(visibleWorldHalfHeight, camera->target.y);
    camera->target.y = fminf(sectionActualHeight - visibleWorldHalfHeight, camera->target.y);
    
    camera->offset = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f };
}

/**
 * @brief Movimenta um personagem, checando colisões.
 * @note Esta é uma versão reescrita e simplificada para corrigir o bug de movimento.
 * Move o personagem de uma só vez por eixo e verifica a colisão, em vez de um loop pixel a pixel.
 */
void move_character(Player *player_obj, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift, const WorldSection* activeSection) {
    if (!player_obj || !activeSection || !activeSection->isLoaded) return;

    int speed = 3;
    int sprintSpeed = 5;
    int currentSpeed = IsKeyDown(keyShift) ? sprintSpeed : speed;

    float deltaX = 0.0f;
    float deltaY = 0.0f;

    if (IsKeyDown(keyLeft))  deltaX -= (float)currentSpeed;
    if (IsKeyDown(keyRight)) deltaX += (float)currentSpeed;
    if (IsKeyDown(keyUp))    deltaY -= (float)currentSpeed;
    if (IsKeyDown(keyDown))  deltaY += (float)currentSpeed;

    // --- Movimento no Eixo X ---
    if (deltaX != 0) {
        int originalX = player_obj->posx;
        player_obj->posx += (int)deltaX;

        // *** CORREÇÃO: Usava 'originalY' antes de ser declarado. Deve usar a posição Y atual do jogador. ***
        Rectangle playerRect = { (float)player_obj->posx, (float)player_obj->posy, (float)player_obj->width, (float)player_obj->height };
        for (int i = 0; i < activeSection->collisionRectCount; i++) {
            if (CheckCollisionRecs(playerRect, activeSection->collisionRects[i])) {
                player_obj->posx = originalX; // Reverte o movimento se houver colisão
                break;
            }
        }
    }

    // --- Movimento no Eixo Y ---
    if (deltaY != 0) {
        int originalY = player_obj->posy;
        player_obj->posy += (int)deltaY;

        // Checa colisão após o movimento em Y
        Rectangle playerRect = { (float)player_obj->posx, (float)player_obj->posy, (float)player_obj->width, (float)player_obj->height };
        for (int i = 0; i < activeSection->collisionRectCount; i++) {
            if (CheckCollisionRecs(playerRect, activeSection->collisionRects[i])) {
                player_obj->posy = originalY; // Reverte o movimento se houver colisão
                break;
            }
        }
    }

    // Garante que o jogador permaneça dentro dos limites do mapa
    if (player_obj->posx < 0) player_obj->posx = 0;
    if (player_obj->posx > activeSection->width - player_obj->width) player_obj->posx = activeSection->width - player_obj->width;
    if (player_obj->posy < 0) player_obj->posy = 0;
    if (player_obj->posy > activeSection->height - player_obj->height) player_obj->posy = activeSection->height - player_obj->height;
}


void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]) { (void)mainPlaylist_arr; if(players_arr == NULL) return; }

void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY, int numActivePlayers, const WorldSection* worldSection) {
    if (!players_arr || !mapX || !mapY ) return;
    *mapX = 0; *mapY = 0;

    if (worldSection && worldSection->isLoaded && worldSection->playerSpawnCount > 0) {
        TraceLog(LOG_INFO, "Pontos de spawn do jogador encontrados. Usando coordenadas do mapa.");
        for (int i = 0; i < numActivePlayers; i++) {
            int spawnIndex = (i < worldSection->playerSpawnCount) ? i : 0;
            players_arr[i].posx = (int)worldSection->playerSpawns[spawnIndex].x;
            players_arr[i].posy = (int)worldSection->playerSpawns[spawnIndex].y;
            TraceLog(LOG_INFO, "Jogador %d posicionado em (%.0f, %.0f)", i, worldSection->playerSpawns[spawnIndex].x, worldSection->playerSpawns[spawnIndex].y);
        }
    } else if (worldSection && worldSection->isLoaded) {
        TraceLog(LOG_WARNING, "Nenhum ponto de spawn do jogador definido no mapa! Usando o centro geométrico como fallback.");
        for (int i = 0; i < numActivePlayers; i++) {
            int p_width = (players_arr[i].width > 0) ? players_arr[i].width : 111;
            int p_height = (players_arr[i].height > 0) ? players_arr[i].height : 150;
            float centerX = (float)worldSection->width / 2.0f;
            float centerY = (float)worldSection->height / 2.0f;
            if (numActivePlayers == 1) {
                players_arr[i].posx = (int)(centerX - (float)p_width / 2.0f);
            } else {
                players_arr[i].posx = (int)(centerX - (float)p_width + ((float)i * ((float)p_width + 20.0f)));
            }
            players_arr[i].posy = (int)(centerY - (float)p_height / 2.0f);
        }
    } else {
        TraceLog(LOG_ERROR, "PrepareNewGameSession chamada com uma secao de mundo invalida!");
    }
}

void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], int numActivePlayers,
                         Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, bool *isPlaying_ptr,
                         float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr,
                         int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera,
                         const WorldSection* activeSection) {
    (void)playlist_arr; (void)currentMusicIndex_ptr; (void)volume_ptr; (void)isPlaying_ptr;
    (void)musicPlayingTimer_ptr; (void)currentMusicDuration_ptr;

    if (!currentScreen_ptr || !activeSection || !activeSection->isLoaded) { return; }
    
    int oldPosX[MAX_PLAYERS_SUPPORTED] = {0}; int oldPosY[MAX_PLAYERS_SUPPORTED] = {0};
    for (int i = 0; i < numActivePlayers; i++) { oldPosX[i] = players_arr[i].posx; oldPosY[i] = players_arr[i].posy; }
    
    if (currentGameMode == GAME_MODE_SINGLE_PLAYER) {
        if (numActivePlayers > 0) { SinglePlayer_HandleMovement(&players_arr[0], activeSection); }
    } else {
        if (numActivePlayers > 0) { move_character(&players_arr[0], KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT, activeSection); }
        if (numActivePlayers > 1) { move_character(&players_arr[1], KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT, activeSection); }
    }
    
    for (int i = 0; i < numActivePlayers; i++) {
        bool isMovingNow = (players_arr[i].posx != oldPosX[i] || players_arr[i].posy != oldPosY[i]);
        float mX = (float)(players_arr[i].posx - oldPosX[i]);
        float mY = (float)(players_arr[i].posy - oldPosY[i]);
        UpdateWalkCycle(&players_arr[i], isMovingNow, mX, mY);
    }

    // *** CORREÇÃO: A chamada para atualizar a câmera foi restaurada. ***
    UpdateCameraCenteredOnPlayers(gameCamera, players_arr, numActivePlayers, (float)activeSection->width, (float)activeSection->height);
    WorldMap_CheckTransition(players_arr, numActivePlayers, currentMapX_ptr, currentMapY_ptr, currentGameMode, (float)activeSection->width, (float)activeSection->height);
    
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { *currentScreen_ptr = GAMESTATE_PAUSE; }
    if (IsKeyPressed(KEY_E)) { *currentScreen_ptr = GAMESTATE_INVENTORY; }
}

void DrawPlayingScreen(Player players_arr[], int numActivePlayers, float currentVolume, int currentMusicIndex, bool isPlaying, int currentMapX, int currentMapY) {
    (void)currentVolume; (void)currentMusicIndex; (void)isPlaying; (void)currentMapX; (void)currentMapY;

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
}