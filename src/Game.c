#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Singleplayer.h"
#include "../include/WalkCycle.h"
#include "../include/WorldMap.h"
#include "../include/WorldLoading.h"
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Extern global variables (defined in main.c, declared in Game.h)
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern const int gameSectionWidthMultiplier;
extern const int gameSectionHeightMultiplier;

// UpdateCameraCenteredOnPlayers, move_character, InitGameResources as in Turn 38.
void UpdateCameraCenteredOnPlayers(Camera2D *camera, Player players[], int numActivePlayers, float sectionActualWidth, float sectionActualHeight) {
    if (!camera || !players || numActivePlayers <= 0) return;
    if (sectionActualWidth <= 0 || sectionActualHeight <= 0) {
        camera->zoom = 1.0f; camera->target = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f };
        camera->offset = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f }; return;
    }
    const float USER_SPECIFIED_MAP_WIDTH_FOR_MAX_ZOOM = 320.0f; 
    float calculated_max_zoom = (USER_SPECIFIED_MAP_WIDTH_FOR_MAX_ZOOM > 0) ? ((float)virtualScreenWidth / USER_SPECIFIED_MAP_WIDTH_FOR_MAX_ZOOM) : 1.0f;
    const float MAX_CAMERA_ZOOM = fmaxf(1755, calculated_max_zoom);
    float zoomToFitWidth = (sectionActualWidth > 0) ? ((float)virtualScreenWidth / sectionActualWidth) : 1.0f;
    float zoomToFitHeight = (sectionActualHeight > 0) ? ((float)virtualScreenHeight / sectionActualHeight) : 1.0f;
    const float MIN_CAMERA_ZOOM_FIT_SECTION = fminf(1755, zoomToFitHeight);
    const float BORDER_PADDING_PERCENT = 0.15f;

    if (numActivePlayers == 1) {
        camera->target.x = (float)players[0].posx + ((float)players[0].width / 2.0f);
        camera->target.y = (float)players[0].posy + ((float)players[0].height / 2.0f);
        camera->zoom = 1.0f;
    } else if (numActivePlayers >= 2) {
        camera->target.x = (((float)players[0].posx + ((float)players[0].width / 2.0f)) + ((float)players[1].posx + ((float)players[1].width / 2.0f))) / 2.0f;
        camera->target.y = (((float)players[0].posy + ((float)players[0].height / 2.0f)) + ((float)players[1].posy + ((float)players[1].height / 2.0f))) / 2.0f;
        float p0_posx_f = (float)players[0].posx; float p1_posx_f = (float)players[1].posx;
        float p0_posy_f = (float)players[0].posy; float p1_posy_f = (float)players[1].posy;
        float p0_width_f = (float)players[0].width; float p1_width_f = (float)players[1].width;
        float p0_height_f = (float)players[0].height; float p1_height_f = (float)players[1].height;
        float minX = fminf(p0_posx_f, p1_posx_f); float maxX = fmaxf(p0_posx_f + p0_width_f, p1_posx_f + p1_width_f);
        float minY = fminf(p0_posy_f, p1_posy_f); float maxY = fmaxf(p0_posy_f + p0_height_f, p1_posy_f + p1_height_f);
        float boundingBoxWidth = maxX - minX; float boundingBoxHeight = maxY - minY;
        float paddedWidth = boundingBoxWidth * (1.0f + BORDER_PADDING_PERCENT * 2.0f);
        float paddedHeight = boundingBoxHeight * (1.0f + BORDER_PADDING_PERCENT * 2.0f);
        paddedWidth = fmaxf(paddedWidth, ((float)virtualScreenWidth / MAX_CAMERA_ZOOM) * 0.5f);
        paddedHeight = fmaxf(paddedHeight, ((float)virtualScreenHeight / MAX_CAMERA_ZOOM) * 0.5f);
        if (paddedWidth < 1.0f) { paddedWidth = 1.0f; }
        if (paddedHeight < 1.0f) { paddedHeight = 1.0f; }
        float zoomRequiredX = (float)virtualScreenWidth / paddedWidth; float zoomRequiredY = (float)virtualScreenHeight / paddedHeight;
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
    if (sectionActualWidth < visibleWorldHalfWidth * 2.0f) camera->target.x = sectionActualWidth / 2.0f;
    if (sectionActualHeight < visibleWorldHalfHeight * 2.0f) camera->target.y = sectionActualHeight / 2.0f;
    if (isnan(camera->target.x) || isinf(camera->target.x)) camera->target.x = sectionActualWidth / 2.0f;
    if (isnan(camera->target.y) || isinf(camera->target.y)) camera->target.y = sectionActualHeight / 2.0f;
    camera->offset = (Vector2){ (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f };
}

void move_character(Player *player_obj, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift, const WorldSection* activeSection) {
    // ... (código como na resposta anterior - Turn 40, com colisões iterativas) ...
    if (!player_obj || !activeSection || !activeSection->isLoaded) { return; }
    int speed = 3; int sprintSpeed = 5; int currentSpeed = IsKeyDown(keyShift) ? sprintSpeed : speed;
    float totalDeltaX = 0; float totalDeltaY = 0;
    if (IsKeyDown(keyLeft))  totalDeltaX -= (float)currentSpeed; if (IsKeyDown(keyRight)) totalDeltaX += (float)currentSpeed;
    if (IsKeyDown(keyUp))    totalDeltaY -= (float)currentSpeed; if (IsKeyDown(keyDown))  totalDeltaY += (float)currentSpeed;
    for (int i = 0; (float)i < fabsf(totalDeltaX); i++) {
        int stepX = (totalDeltaX > 0) ? 1 : -1; int oldPosX_iter = player_obj->posx; player_obj->posx += stepX;
        Rectangle playerRect = { (float)player_obj->posx, (float)player_obj->posy, (float)player_obj->width, (float)player_obj->height };
        bool collisionFoundX = false;
        for (int c = 0; c < activeSection->collisionRectCount; c++) {
            if (CheckCollisionRecs(playerRect, activeSection->collisionRects[c])) { player_obj->posx = oldPosX_iter; collisionFoundX = true; break; }
        } if (collisionFoundX) break;
    }
    for (int i = 0; (float)i < fabsf(totalDeltaY); i++) {
        int stepY = (totalDeltaY > 0) ? 1 : -1; int oldPosY_iter = player_obj->posy; player_obj->posy += stepY;
        Rectangle playerRect = { (float)player_obj->posx, (float)player_obj->posy, (float)player_obj->width, (float)player_obj->height };
        bool collisionFoundY = false;
        for (int c = 0; c < activeSection->collisionRectCount; c++) {
            if (CheckCollisionRecs(playerRect, activeSection->collisionRects[c])) { player_obj->posy = oldPosY_iter; collisionFoundY = true; break; }
        } if (collisionFoundY) break;
    }
    if (player_obj->posx < 0) player_obj->posx = 0;
    if (player_obj->posx > activeSection->width - player_obj->width) { player_obj->posx = activeSection->width - player_obj->width; }
    if (player_obj->posy < 0) player_obj->posy = 0;
    if (player_obj->posy > activeSection->height - player_obj->height) { player_obj->posy = activeSection->height - player_obj->height; }
    if (player_obj->posx < 0) player_obj->posx = 0; if (player_obj->posy < 0) player_obj->posy = 0;
}

void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]) { (void)mainPlaylist_arr; if(players_arr == NULL) return; }

/**
 * @brief Configura jogadores para uma nova sessão de jogo.
 * Prioriza os 'playerSpawns' (green patches) da WorldSection.
 * Se não houver, posiciona no centro. Aplica um 'nudge' de segurança e clamp final.
 */
void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY, int numActivePlayers, const WorldSection* worldSection) {
    if (!players_arr || !mapX || !mapY ) {
        TraceLog(LOG_ERROR, "PrepareNewGameSession: Argumentos de ponteiro invalidos.");
        return;
    }

    *mapX = 0; // Garante que o novo jogo começa no mapa 0
    *mapY = 0;
    TraceLog(LOG_INFO, "PrepareNewGameSession: Configurando para mapa (X:%d, Y:%d) com %d jogadores.", *mapX, *mapY, numActivePlayers);

    const int BORDER_TOLERANCE = 10; // Tolerância de WorldMap.c
    const int MIN_SAFE_DISTANCE_FROM_EDGE = BORDER_TOLERANCE + 1; // e.g., 11

    for (int i = 0; i < numActivePlayers && i < MAX_PLAYERS_SUPPORTED; i++) {
        // Usa dimensões do jogador (de init_player, depois atualizadas por LoadCharacterAnimations)
        // Fallback para as dimensões que você especificou para seus sprites (111x150)
        int p_width = (players_arr[i].width > 0) ? players_arr[i].width : 111;
        int p_height = (players_arr[i].height > 0) ? players_arr[i].height : 150;
        TraceLog(LOG_DEBUG, "PrepareNewGameSession: Jogador %d usando dimensoes %dx%d para calculo de spawn.", i, p_width, p_height);

        bool positionSet = false;

        // 1. Tenta usar os 'playerSpawns' (green patches) do worldSection
        if (worldSection && worldSection->isLoaded && worldSection->playerSpawnCount > 0) {
            int spawn_idx = i % worldSection->playerSpawnCount; // Cicla pelos spawns se houver mais jogadores que spawns
            players_arr[i].posx = (int)worldSection->playerSpawns[spawn_idx].x;
            players_arr[i].posy = (int)worldSection->playerSpawns[spawn_idx].y;
            positionSet = true;
            TraceLog(LOG_INFO, "Jogador %d: Posicao inicial via green patch (X: %d, Y: %d) da secao %dx%d.",
                     i, players_arr[i].posx, players_arr[i].posy, worldSection->width, worldSection->height);
        }

        // 2. Se não há green patches ou worldSection não carregou, posiciona no centro (ou fallback absoluto)
        if (!positionSet) {
            if (worldSection && worldSection->isLoaded && worldSection->width > 0 && worldSection->height > 0) {
                TraceLog(LOG_WARNING, "PrepareNewGameSession: Nenhum player spawn (green patch) definido para jogador %d na secao (0,0). Posicionando no centro.", i);
                float centerX = (float)worldSection->width / 2.0f;
                float centerY = (float)worldSection->height / 2.0f;
                if (numActivePlayers == 1) {
                    players_arr[i].posx = (int)(centerX - (float)p_width / 2.0f);
                } else {
                    players_arr[i].posx = (int)(centerX - (float)p_width - 10.0f + ((float)i * ((float)p_width + 20.0f))); // Espalha
                }
                players_arr[i].posy = (int)(centerY - (float)p_height / 2.0f);
            } else {
                TraceLog(LOG_ERROR, "PrepareNewGameSession: Secao do mundo (0,0) NAO carregada ou com dimensoes invalidas. Usando posicoes de fallback absolutas.");
                int cSectH_fallback = gameSectionHeightMultiplier * virtualScreenHeight;
                players_arr[i].posx = p_width + 50 + (i * (p_width + 20));
                players_arr[i].posy = cSectH_fallback / 2 - p_height / 2;
            }
            positionSet = true; // Marcar como definido mesmo que seja fallback
        }

        // 3. Nudge e Clamp final (APENAS se worldSection estiver carregado e válido)
        if (worldSection && worldSection->isLoaded && worldSection->width > 0 && worldSection->height > 0) {
            TraceLog(LOG_DEBUG, "P%d: Antes do nudge/clamp: pos=(%d,%d), p_size=(%d,%d), section_size=(%d,%d)",
                     i, players_arr[i].posx, players_arr[i].posy, p_width, p_height, worldSection->width, worldSection->height);

            // Nudge para longe das bordas se estiver muito perto (APENAS para o mapa inicial 0,0)
            if (*mapX == 0 && *mapY == 0) {
                if (players_arr[i].posx <= BORDER_TOLERANCE) {
                    players_arr[i].posx = MIN_SAFE_DISTANCE_FROM_EDGE;
                }
                if (players_arr[i].posy <= BORDER_TOLERANCE) {
                    players_arr[i].posy = MIN_SAFE_DISTANCE_FROM_EDGE;
                }
                // Nudge da direita e de baixo (cuidado para não empurrar para fora se o mapa for pequeno)
                if (players_arr[i].posx >= worldSection->width - p_width - BORDER_TOLERANCE) {
                    players_arr[i].posx = worldSection->width - p_width - MIN_SAFE_DISTANCE_FROM_EDGE;
                }
                if (players_arr[i].posy >= worldSection->height - p_height - BORDER_TOLERANCE) {
                    players_arr[i].posy = worldSection->height - p_height - MIN_SAFE_DISTANCE_FROM_EDGE;
                }
            }

            // Clamp final para garantir que está dentro dos limites da seção
            if (players_arr[i].posx < 0) players_arr[i].posx = 0;
            if (players_arr[i].posx > worldSection->width - p_width) {
                players_arr[i].posx = worldSection->width - p_width;
            }
            if (players_arr[i].posy < 0) players_arr[i].posy = 0;
            if (players_arr[i].posy > worldSection->height - p_height) {
                players_arr[i].posy = worldSection->height - p_height;
            }
            // Re-clamp para 0 se o cálculo acima resultar em negativo (mapa menor que jogador)
            if (players_arr[i].posx < 0) players_arr[i].posx = 0;
            if (players_arr[i].posy < 0) players_arr[i].posy = 0;
        }
        TraceLog(LOG_INFO, "Jogador %d posicao final em PrepareNewGameSession: (X: %d, Y: %d)", i, players_arr[i].posx, players_arr[i].posy);
    }
}

void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], int numActivePlayers,
                         Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, bool *isPlaying_ptr,
                         float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr,
                         int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera,
                         const WorldSection* activeSection) {
    // ... (código como na resposta anterior - Turn 38) ...
    if (!currentScreen_ptr || !players_arr || !gameCamera || !currentMapX_ptr || !currentMapY_ptr ) { if (IsKeyPressed(KEY_ESCAPE)) { *currentScreen_ptr = GAMESTATE_MENU; } return; }
    if (!activeSection || !activeSection->isLoaded) { if (IsKeyPressed(KEY_ESCAPE)) { *currentScreen_ptr = GAMESTATE_MENU; } TraceLog(LOG_WARNING, "UpdatePlayingScreen: activeSection invalida ou nao carregada."); return; }
    float currentSectionActualWidth = (float)activeSection->width; float currentSectionActualHeight = (float)activeSection->height;
    if (playlist_arr && currentMusicIndex_ptr && isPlaying_ptr && volume_ptr && musicPlayingTimer_ptr && currentMusicDuration_ptr &&
        *currentMusicIndex_ptr >= 0 && *currentMusicIndex_ptr < MAX_MUSIC_PLAYLIST_SIZE && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
        if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].frameCount > 0) { UpdateMusicStream(playlist_arr[*currentMusicIndex_ptr]); }
    }
    UpdateCameraCenteredOnPlayers(gameCamera, players_arr, numActivePlayers, currentSectionActualWidth, currentSectionActualHeight);
    int oldPosX[MAX_PLAYERS_SUPPORTED] = {0}; int oldPosY[MAX_PLAYERS_SUPPORTED] = {0};
    for (int i = 0; i < numActivePlayers && i < MAX_PLAYERS_SUPPORTED; i++) { oldPosX[i] = players_arr[i].posx; oldPosY[i] = players_arr[i].posy; }
    if (currentGameMode == GAME_MODE_SINGLE_PLAYER) {
        if (numActivePlayers > 0) { SinglePlayer_HandleMovement(&players_arr[0], activeSection); }
    } else {
        if (numActivePlayers > 0) { move_character(&players_arr[0], KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT, activeSection); }
        if (numActivePlayers > 1) { move_character(&players_arr[1], KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT, activeSection); }
    }
    TraceLog(LOG_DEBUG, "UpdatePlayingScreen: ANTES WorldMap_CheckTransition - Mapa Atual (X:%d, Y:%d), P0 (X:%d, Y:%d)", *currentMapX_ptr, *currentMapY_ptr, players_arr[0].posx, players_arr[0].posy);
    WorldMap_CheckTransition(players_arr, numActivePlayers, currentMapX_ptr, currentMapY_ptr, currentGameMode, currentSectionActualWidth, currentSectionActualHeight);
    TraceLog(LOG_DEBUG, "UpdatePlayingScreen: APOS WorldMap_CheckTransition - Mapa Atual (X:%d, Y:%d)", *currentMapX_ptr, *currentMapY_ptr);
    for (int i = 0; i < numActivePlayers && i < MAX_PLAYERS_SUPPORTED; i++) {
        bool isMovingNow = (players_arr[i].posx != oldPosX[i] || players_arr[i].posy != oldPosY[i]);
        float mX = (float)players_arr[i].posx - (float)oldPosX[i]; float mY = (float)players_arr[i].posy - (float)oldPosY[i];
        UpdateWalkCycle(&players_arr[i], isMovingNow, mX, mY);
        TraceLog(LOG_INFO, "UpdatePlayingScreen: Jogador %d - Posicao final (X: %d, Y: %d) no Mapa (X:%d, Y:%d)", i, players_arr[i].posx, players_arr[i].posy, *currentMapX_ptr, *currentMapY_ptr);
    }
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = GAMESTATE_PAUSE;
        if (isPlaying_ptr && *isPlaying_ptr && playlist_arr && currentMusicIndex_ptr &&
            *currentMusicIndex_ptr >= 0 && *currentMusicIndex_ptr < MAX_MUSIC_PLAYLIST_SIZE &&
            playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
            PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]);
        }
    }
    if (IsKeyPressed(KEY_E)) { *currentScreen_ptr = GAMESTATE_INVENTORY; }
}

void DrawPlayingScreen(Player players_arr[], int numActivePlayers, float currentVolume, int currentMusicIndex, bool isPlaying, int currentMapX, int currentMapY) {
    // ... (código como na resposta anterior - Turn 38) ...
    if (players_arr != NULL) {
        for (int i = 0; i < numActivePlayers && i < MAX_PLAYERS_SUPPORTED; i++) {
            Texture2D currentSprite = GetCurrentCharacterSprite(&players_arr[i]);
            if (currentSprite.id > 0) {
                DrawTexturePro(currentSprite, (Rectangle){ 0.0f, 0.0f, (float)currentSprite.width, (float)currentSprite.height },
                               (Rectangle){ (float)players_arr[i].posx, (float)players_arr[i].posy, (float)players_arr[i].width, (float)players_arr[i].height },
                               (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
            } else { DrawRectangle(players_arr[i].posx, players_arr[i].posy, players_arr[i].width, players_arr[i].height, GRAY); }
        }
    }
    char mapCoordsText[64]; sprintf(mapCoordsText, "Mapa (X: %d, Y: %d)", currentMapX, currentMapY);
    DrawText(mapCoordsText, 10, 10, 20, YELLOW);
    DrawText(TextFormat("Volume Jogo (Ex): %.0f%%", currentVolume * 100.0f), 10, 30, 10, LIGHTGRAY);
    DrawText(TextFormat("Musica Legada Idx: %d (Tocando: %s)", currentMusicIndex + 1, isPlaying ? "Sim" : "Nao"), 10, 45, 10, LIGHTGRAY);
    DrawText("Controles: P/Esc=Pausa, E=Inventario", 10, 60, 10, RAYWHITE);
}