#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Inventory.h"
#include <raylib.h>
#include <raymath.h> // Para Vector2Add, Vector2Scale, fminf, fmaxf etc.
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Para fminf, fmaxf, abs

// --- Variáveis Globais Externas ---
extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;

// --- Multiplicadores para Dimensões da Seção do Mapa ---
const int gameSectionWidthMultiplier = 3;
const int gameSectionHeightMultiplier = 3;

// Enum para detecção de borda da seção do mapa
typedef enum {
    BORDER_NONE = 0, BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM
} BorderDirection;


// --- Função de Atualização da Câmera ---
void UpdateCameraCenteredOnPlayers(Camera2D *camera, Player players[], int numPlayers, float vWidth, float vHeight) {
    if (numPlayers <= 0) return;

    const float MIN_CAMERA_ZOOM = 1.0f / fmaxf(gameSectionWidthMultiplier, gameSectionHeightMultiplier);
    const float MAX_CAMERA_ZOOM = 1.0f; 
    const float BORDER_PADDING_PERCENT = 0.20f;

    if (numPlayers == 1) {
        camera->target.x = players[0].posx + players[0].width / 2.0f;
        camera->target.y = players[0].posy + players[0].height / 2.0f;
        camera->zoom = MAX_CAMERA_ZOOM; 
    } else if (numPlayers >= 2) {
        camera->target.x = (players[0].posx + players[0].width / 2.0f + players[1].posx + players[1].width / 2.0f) / 2.0f;
        camera->target.y = (players[0].posy + players[0].height / 2.0f + players[1].posy + players[1].height / 2.0f) / 2.0f;

        float minX = fminf(players[0].posx, players[1].posx);
        float maxX = fmaxf(players[0].posx + players[0].width, players[1].posx + players[1].width);
        float minY = fminf(players[0].posy, players[1].posy);
        float maxY = fmaxf(players[0].posy + players[0].height, players[1].posy + players[1].height);

        float boundingBoxWidth = maxX - minX;
        float boundingBoxHeight = maxY - minY;

        float paddedWidth = boundingBoxWidth * (1.0f + BORDER_PADDING_PERCENT * 2.0f);
        float paddedHeight = boundingBoxHeight * (1.0f + BORDER_PADDING_PERCENT * 2.0f);
        
        float minPracticalPaddingW = players[0].width * 0.5f;
        float minPracticalPaddingH = players[0].height * 0.5f;
        if (paddedWidth < minPracticalPaddingW) paddedWidth = minPracticalPaddingW;
        if (paddedHeight < minPracticalPaddingH) paddedHeight = minPracticalPaddingH;

        if (paddedWidth < (vWidth / MAX_CAMERA_ZOOM)) paddedWidth = (vWidth / MAX_CAMERA_ZOOM);
        if (paddedHeight < (vHeight / MAX_CAMERA_ZOOM)) paddedHeight = (vHeight / MAX_CAMERA_ZOOM);

        float zoomRequiredX = vWidth / paddedWidth;
        float zoomRequiredY = vHeight / paddedHeight;
        camera->zoom = fminf(zoomRequiredX, zoomRequiredY);
    }

    if (camera->zoom < MIN_CAMERA_ZOOM) camera->zoom = MIN_CAMERA_ZOOM;
    if (camera->zoom > MAX_CAMERA_ZOOM) camera->zoom = MAX_CAMERA_ZOOM;

    float currentSectionTotalWidth = gameSectionWidthMultiplier * virtualScreenWidth;
    float currentSectionTotalHeight = gameSectionHeightMultiplier * virtualScreenHeight;
    float visibleWorldHalfWidth = (vWidth / camera->zoom) / 2.0f;
    float visibleWorldHalfHeight = (vHeight / camera->zoom) / 2.0f;

    camera->target.x = fmaxf(visibleWorldHalfWidth, camera->target.x);
    camera->target.x = fminf(currentSectionTotalWidth - visibleWorldHalfWidth, camera->target.x);
    camera->target.y = fmaxf(visibleWorldHalfHeight, camera->target.y);
    camera->target.y = fminf(currentSectionTotalHeight - visibleWorldHalfHeight, camera->target.y);

    camera->offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
}


// Função de movimentação de personagem
void move_character(int *posx_ptr, int *posy_ptr, int pWidth, int pHeight,
                    int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift){
    int speed = 3;
    int sprintSpeed = 5;
    int currentSpeed = IsKeyDown(keyShift) ? sprintSpeed : speed;

    if (IsKeyDown(keyLeft))  { *posx_ptr -= currentSpeed; }
    if (IsKeyDown(keyRight)) { *posx_ptr += currentSpeed; }
    if (IsKeyDown(keyUp))    { *posy_ptr -= currentSpeed; }
    if (IsKeyDown(keyDown))  { *posy_ptr += currentSpeed; }

    int currentSectionTotalWidth = gameSectionWidthMultiplier * virtualScreenWidth;
    int currentSectionTotalHeight = gameSectionHeightMultiplier * virtualScreenHeight;

    if (*posx_ptr < 0) *posx_ptr = 0;
    if (*posx_ptr > currentSectionTotalWidth - pWidth) *posx_ptr = currentSectionTotalWidth - pWidth;
    if (*posy_ptr < 0) *posy_ptr = 0;
    if (*posy_ptr > currentSectionTotalHeight - pHeight) *posy_ptr = currentSectionTotalHeight - pHeight;
}

// Inicializa recursos do jogo
void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]) {
    // Define dimensões padrão para os jogadores.
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players_arr[i].width = 80; 
        players_arr[i].height = 80;
    }

    // Carrega a textura para o Jogador 0 (P1)
    const char *player0ImagePath = "./assets/images/player2.png"; 
    if (MAX_PLAYERS > 0) {
        players_arr[0].txr = LoadTexture(player0ImagePath); 
        if (players_arr[0].txr.id == 0) { // Verifica se o carregamento da textura falhou
            TraceLog(LOG_WARNING, "[InitGameResources] Falha ao carregar textura para Jogador 0: %s", player0ImagePath); 
        }
    }

    // Carrega a textura para o Jogador 1 (P2)
    const char *player1ImagePath = "./assets/images/player1.png"; 
    if (MAX_PLAYERS > 1) {
        players_arr[1].txr = LoadTexture(player1ImagePath); 
        if (players_arr[1].txr.id == 0) { // Verifica se o carregamento da textura falhou
            TraceLog(LOG_WARNING, "[InitGameResources] Falha ao carregar textura para Jogador 1: %s", player1ImagePath); 
        }
    }

    // Carregamento de músicas
    const char *musicFiles[MAX_SIZE] = {
        "./assets/songs/Desmeon_-_My_Sunshine.ogg", "./assets/songs/19 William Tell Overture.ogg",
        "./assets/songs/last-summer-by-ikson.mp3", "./assets/songs/Floatinurboat - Spirit Of Things.mp3"
    };
    for (int i = 0; i < MAX_SIZE; i++) {
        mainPlaylist_arr[i] = LoadMusicStream(musicFiles[i]);
        if (mainPlaylist_arr[i].stream.buffer == NULL) {
            TraceLog(LOG_WARNING, "[InitGameResources] Falha música %d: %s", i + 1, musicFiles[i]);
        }
    }
}

// Prepara nova sessão de jogo
void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY) {
    TraceLog(LOG_INFO, "Preparando nova sessão de jogo...");
    *mapX = 0; *mapY = 0;
    int currentSectionTotalHeight = gameSectionHeightMultiplier * virtualScreenHeight;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        strcpy(players_arr[i].nome, "");
        players_arr[i].width = 80; players_arr[i].height = 80;
        if (i == 0) {
            players_arr[i].posx = players_arr[i].width + 50;
            players_arr[i].posy = currentSectionTotalHeight / 2 - players_arr[i].height / 2;
        } else if (i == 1) {
            players_arr[i].posx = players_arr[i].width + 150;
            players_arr[i].posy = currentSectionTotalHeight / 2 - players_arr[i].height / 2;
        }
    }
}

// Detecção de borda para transição de mapa
BorderDirection GetPlayerBorderCondition(Player player_obj) {
    int tolerance = 10;
    int currentSectionTotalWidth = gameSectionWidthMultiplier * virtualScreenWidth;
    int currentSectionTotalHeight = gameSectionHeightMultiplier * virtualScreenHeight;

    if (player_obj.posx <= tolerance) return BORDER_LEFT;
    if (player_obj.posx >= currentSectionTotalWidth - player_obj.width - tolerance) return BORDER_RIGHT;
    if (player_obj.posy <= tolerance) return BORDER_TOP;
    if (player_obj.posy >= currentSectionTotalHeight - player_obj.height - tolerance) return BORDER_BOTTOM;
    return BORDER_NONE;
}

// --- Atualização da Tela de Jogo ---
void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], Music playlist_arr[],
                         int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr,
                         float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr,
                         int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera) {

    UpdateCameraCenteredOnPlayers(gameCamera, players_arr, MAX_PLAYERS, virtualScreenWidth, virtualScreenHeight);

    // Lógica de Controle da Música (Completa)
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { UpdateMusicStream(playlist_arr[*currentMusicIndex_ptr]); }
    if (IsKeyPressed(KEY_COMMA) || IsKeyPressed(KEY_PERIOD)) { if (*isPlaying_ptr) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 0; } else { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { ResumeMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 1; } } }
    if (IsKeyPressed(KEY_PAGE_DOWN)) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } }
    if (IsKeyPressed(KEY_PAGE_UP)) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); *currentMusicIndex_ptr = (*currentMusicIndex_ptr - 1 + MAX_SIZE) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } }
    if (IsKeyPressed(KEY_KP_0) || IsKeyPressed(KEY_ZERO)) { if (*isPlaying_ptr) { *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } }
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) { if (*isPlaying_ptr) { *volume_ptr -= 0.05f; if (*volume_ptr < 0.0f) *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } }
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) { if (*isPlaying_ptr) { *volume_ptr += 0.05f; if (*volume_ptr > 1.0f) *volume_ptr = 1.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } }
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { *musicPlayingTimer_ptr += GetFrameTime(); if (*currentMusicDuration_ptr > 0 && *musicPlayingTimer_ptr >= *currentMusicDuration_ptr) { StopMusicStream(playlist_arr[*currentMusicIndex_ptr]);  *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]);  *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } } }

    // Movimentação dos Jogadores
    if (MAX_PLAYERS > 0) {
        move_character(&players_arr[0].posx, &players_arr[0].posy, players_arr[0].width, players_arr[0].height,
                    KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT);
    }
    if (MAX_PLAYERS > 1) {
        move_character(&players_arr[1].posx, &players_arr[1].posy, players_arr[1].width, players_arr[1].height,
                       KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT);
    }

    // --- Lógica de Transição de Mapa ---
    if (MAX_PLAYERS > 1) {
        BorderDirection p0_border = GetPlayerBorderCondition(players_arr[0]);
        BorderDirection p1_border = GetPlayerBorderCondition(players_arr[1]);
        bool performTransition = false;
        BorderDirection transitionTo = BORDER_NONE;
        int nextMapX = *currentMapX_ptr;
        int nextMapY = *currentMapY_ptr;

        if (p0_border != BORDER_NONE && p0_border == p1_border) {
            transitionTo = p0_border;
            switch (transitionTo) {
                case BORDER_LEFT:   nextMapX = *currentMapX_ptr - 1; break;
                case BORDER_RIGHT:  nextMapX = *currentMapX_ptr + 1; break;
                case BORDER_TOP:    nextMapY = *currentMapY_ptr - 1; break;
                case BORDER_BOTTOM: nextMapY = *currentMapY_ptr + 1; break;
                case BORDER_NONE: default: break;
            }
            if (nextMapX >= WORLD_MAP_MIN_X && nextMapX <= WORLD_MAP_MAX_X &&
                nextMapY >= WORLD_MAP_MIN_Y && nextMapY <= WORLD_MAP_MAX_Y) {
                performTransition = true;
            }
        }
        if (performTransition) {
            *currentMapX_ptr = nextMapX;
            *currentMapY_ptr = nextMapY;
            
            int currentSectionTotalWidth = gameSectionWidthMultiplier * virtualScreenWidth;
            int currentSectionTotalHeight = gameSectionHeightMultiplier * virtualScreenHeight;

            for (int i = 0; i < MAX_PLAYERS; i++) {
                int playerExitPosX = players_arr[i].posx;
                int playerExitPosY = players_arr[i].posy;

                switch (transitionTo) {
                    case BORDER_RIGHT: 
                        players_arr[i].posx = players_arr[i].width / 2; 
                        players_arr[i].posy = playerExitPosY;          
                        break;
                    case BORDER_LEFT: 
                        players_arr[i].posx = currentSectionTotalWidth - players_arr[i].width - (players_arr[i].width / 2); 
                        players_arr[i].posy = playerExitPosY;            
                        break;
                    case BORDER_TOP: 
                        players_arr[i].posx = playerExitPosX;            
                        players_arr[i].posy = currentSectionTotalHeight - players_arr[i].height - (players_arr[i].height / 2); 
                        break;
                    case BORDER_BOTTOM: 
                        players_arr[i].posx = playerExitPosX;           
                        players_arr[i].posy = players_arr[i].height / 2; 
                        break;
                    case BORDER_NONE:
                    default:
                        break;
                }

                if (players_arr[i].posx < 0) players_arr[i].posx = 0;
                if (players_arr[i].posx > currentSectionTotalWidth - players_arr[i].width) players_arr[i].posx = currentSectionTotalWidth - players_arr[i].width;
                if (players_arr[i].posy < 0) players_arr[i].posy = 0;
                if (players_arr[i].posy > currentSectionTotalHeight - players_arr[i].height) players_arr[i].posy = currentSectionTotalHeight - players_arr[i].height;
            }
        }
    }

    // Transição para outras telas
    if (IsKeyPressed(KEY_P)) { *currentScreen_ptr = GAMESTATE_PAUSE; if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); }
    if (IsKeyPressed(KEY_E)) { *currentScreen_ptr = GAMESTATE_INVENTORY; }
}

// Desenha a tela de jogo
void DrawPlayingScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY) {
    Color bgColor;
    int colorIndex = (abs(currentMapX) + abs(currentMapY)) % 4;
    if (colorIndex == 0) bgColor = BLACK;
    else if (colorIndex == 1) bgColor = DARKBROWN;
    else if (colorIndex == 2) bgColor = DARKPURPLE;
    else bgColor = (Color){20, 40, 60, 255};
    ClearBackground(bgColor);

    // Desenha os jogadores
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players_arr[i].txr.id > 0) { // Se tem textura, desenha a textura
            DrawTextureRec(players_arr[i].txr,
                           (Rectangle){0, 0, (float)players_arr[i].txr.width, (float)players_arr[i].txr.height},
                           (Vector2){(float)players_arr[i].posx, (float)players_arr[i].posy}, WHITE);
        } else { // Senão, desenha um retângulo colorido
            Color playerColor;
            switch (players_arr[i].classe) {
                 case GUERREIRO: playerColor = (i == 0) ? RED : MAROON; break;
                 case MAGO:    playerColor = (i == 0) ? BLUE : DARKBLUE; break;
                 case ARQUEIRO:playerColor = (i == 0) ? GREEN : DARKGREEN; break;
                 case BARBARO: playerColor = (i == 0) ? ORANGE : BROWN; break;
                 case LADINO:  playerColor = (i == 0) ? GRAY : DARKGRAY; break;
                 case CLERIGO: playerColor = (i == 0) ? GOLD : YELLOW; break;
                 default:      playerColor = (i == 0) ? PURPLE : DARKPURPLE; break;
            }
            DrawRectangle(players_arr[i].posx, players_arr[i].posy, players_arr[i].width, players_arr[i].height, playerColor);
        }
    }

    // UI (Textos de informação)
    // Estes textos são desenhados em coordenadas relativas à tela virtual,
    // mas como DrawPlayingScreen é chamada dentro de BeginMode2D/EndMode2D em main.c,
    // eles serão afetados pela câmera (escalonados e movidos com o mundo).
    // Para UI fixa na tela (HUD), o desenho deveria ocorrer em main.c, após EndMode2D().
    char mapCoordsText[64];
    sprintf(mapCoordsText, "Mapa (X: %d, Y: %d)", currentMapX, currentMapY);
    DrawText(mapCoordsText, 10, 10, 20, YELLOW);

    DrawText(TextFormat("Volume: %.0f%%", currentVolume * 100), 10, virtualScreenHeight - 60, 10, LIGHTGRAY);
    DrawText(TextFormat("Música %d (Tocando: %s)", currentMusicIndex + 1, isPlaying ? "Sim" : "Não"), 10, virtualScreenHeight - 45, 10, LIGHTGRAY);
    DrawText("Controles: P=Pausa, E=Inventário, Setas/WASD=Mover, Shift=Correr", 10, virtualScreenHeight - 30, 10, RAYWHITE);
    DrawText(",/.=Play/Pause Música, PgUp/Dn=Mudar Música, +/-/0=Volume", 10, virtualScreenHeight - 15, 10, RAYWHITE);

    // DrawFPS da Raylib geralmente desenha em coordenadas de tela fixas, ignorando a câmera.
    // Se precisar que seja no mundo, teria que usar GetScreenToWorld2D ou desenhar texto manualmente.
    DrawFPS(10, virtualScreenHeight - 75); 
}

// Função placeholder
void *pixels() { return NULL; }