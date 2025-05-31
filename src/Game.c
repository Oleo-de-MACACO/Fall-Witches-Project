#include "../include/Game.h"    
#include "../include/Classes.h" 
#include <raylib.h>
#include <stdbool.h>         
#include <stdio.h>           
#include <stddef.h>          
#include <stdlib.h> // Para abs()

extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;

typedef enum {
    BORDER_NONE = 0, BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM       
} BorderDirection;

// --- Implementação da Função de Movimentação de Personagem ---
void move_character(int *posx_ptr, int *posy_ptr, int pWidth, int pHeight, int screenWidth, int screenHeight, 
                    int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift){
    // ... (código igual, sem alterações)
    int speed = 3;          
    int sprintSpeed = 5;    
    int currentSpeed = IsKeyDown(keyShift) ? sprintSpeed : speed;

    if (IsKeyDown(keyLeft))  { *posx_ptr -= currentSpeed; }
    if (IsKeyDown(keyRight)) { *posx_ptr += currentSpeed; }
    if (IsKeyDown(keyUp))    { *posy_ptr -= currentSpeed; }
    if (IsKeyDown(keyDown))  { *posy_ptr += currentSpeed; }

    if (*posx_ptr < 0) *posx_ptr = 0; 
    if (*posx_ptr > screenWidth - pWidth) *posx_ptr = screenWidth - pWidth; 
    if (*posy_ptr < 0) *posy_ptr = 0; 
    if (*posy_ptr > screenHeight - pHeight) *posy_ptr = screenHeight - pHeight; 
}

// --- Implementações das Funções de Gerenciamento de Jogo e Cenas ---

// CORRIGIDO: Removido Texture2D *player1Tex_ptr dos parâmetros
void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]) {
    // Configurações e posições iniciais dos jogadores
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players_arr[i].width = 80;  // Largura para colisão e desenho da sprite.
        players_arr[i].height = 80; // Altura para colisão e desenho da sprite.
                                    // AJUSTE estes valores se a colisão parecer prematura
                                    // devido a espaços transparentes na sua arte.
                                    
        if (i == 0) players_arr[i].classe = GUERREIRO; 
        else players_arr[i].classe = MAGO;    

        if (i == 0) { // Jogador 0 (Setas)
            players_arr[i].posx = players_arr[i].width; 
            players_arr[i].posy = GetScreenHeight() - players_arr[i].height * 2; 
        }
        if (i == 1) { // Jogador 1 (WASD)
            players_arr[i].posx = GetScreenWidth() - players_arr[i].width * 2; 
            players_arr[i].posy = GetScreenHeight() - players_arr[i].height * 2;
        }
    }

    // Carrega a textura para o Jogador 0 (Setas) - assumindo que você tem "player2.png"
    const char *player0ImagePath = "./assets/images/player2.png"; 
    if (MAX_PLAYERS > 0) { // Garante que o array de jogadores tem pelo menos um elemento
        players_arr[0].txr = LoadTexture(player0ImagePath); 
        if (players_arr[0].txr.id == 0) {
            TraceLog(LOG_WARNING, "Falha ao carregar textura para Jogador 0: %s", player0ImagePath);
        } else {
            TraceLog(LOG_INFO, "Textura para Jogador 0 carregada: %s", player0ImagePath);
        }
    }

    // Carrega a textura para o Jogador 1 (WASD)
    const char *player1ImagePath = "./assets/images/player1.png"; //
    if (MAX_PLAYERS > 1) { // Garante que o array de jogadores tem pelo menos dois elementos
        players_arr[1].txr = LoadTexture(player1ImagePath); //
        if (players_arr[1].txr.id == 0) {
            TraceLog(LOG_WARNING, "Falha ao carregar textura para Jogador 1: %s", player1ImagePath);
        } else {
            TraceLog(LOG_INFO, "Textura para Jogador 1 carregada: %s", player1ImagePath);
        }
    }
    
    // ... (carregamento de músicas permanece igual) ...
    const char *musicFiles[MAX_SIZE] = {
        "./assets/songs/Desmeon_-_My_Sunshine.ogg",        
        "./assets/songs/19 William Tell Overture.ogg",  
        "./assets/songs/last-summer-by-ikson.mp3",      
        "./assets/songs/Floatinurboat - Spirit Of Things.mp3" 
    };
    for (int i = 0; i < MAX_SIZE; i++) {
        mainPlaylist_arr[i] = LoadMusicStream(musicFiles[i]); 
        if (mainPlaylist_arr[i].stream.buffer == NULL) {
            TraceLog(LOG_WARNING, "Falha ao carregar música %d: %s", i + 1, musicFiles[i]); 
        }
    }
}

// ... (UpdateMenuScreen, DrawMenuScreen, UpdateIntroScreen, DrawIntroScreen - sem alterações) ...
void UpdateMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr) {
    if (IsKeyPressed(KEY_ENTER)) { *currentScreen_ptr = GAMESTATE_PLAYING; if (playlist[currentMusicIndex].frameCount > 0 && playlist[currentMusicIndex].stream.buffer != NULL) { if (!(*isPlaying_ptr)) { PlayMusicStream(playlist[currentMusicIndex]); *isPlaying_ptr = 1; } SetMusicVolume(playlist[currentMusicIndex], currentVolume); } }
    if (IsKeyPressed(KEY_Q)) *currentScreen_ptr = GAMESTATE_INTRO;
}
void DrawMenuScreen(void) { ClearBackground(DARKGRAY); DrawText("FALL WITCHES", GetScreenWidth() / 2 - MeasureText("FALL WITCHES", 40) / 2, GetScreenHeight() / 4, 40, WHITE); DrawText("PRESSIONE ENTER para INICIAR", GetScreenWidth() / 2 - MeasureText("PRESSIONE ENTER para INICIAR", 20) / 2, GetScreenHeight() / 2, 20, LIGHTGRAY); DrawText("Pressione Q para Intro (Demo)", 10, GetScreenHeight() - 30, 10, RAYWHITE); }
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr) { (*introFrames_ptr)++; if (*introFrames_ptr > 180 || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) { *currentScreen_ptr = GAMESTATE_MENU; *introFrames_ptr = 0; } }
void DrawIntroScreen(void) { ClearBackground(BLACK); DrawText("TELA DE INTRODUÇÃO", GetScreenWidth() / 2 - MeasureText("TELA DE INTRODUÇÃO", 30) / 2, GetScreenHeight() / 2 - 40, 30, WHITE); DrawText("Comi o CU de quem tá lendo", GetScreenWidth() / 2 - MeasureText("Comi o CU de quem tá lendo", 20) / 2, GetScreenHeight() / 2, 20, LIGHTGRAY); }


BorderDirection GetPlayerBorderCondition(Player player_obj, int sWidth, int sHeight) {
    // ... (código igual) ...
    int tolerance = 5; 
    if (player_obj.posx <= tolerance) return BORDER_LEFT;
    if (player_obj.posx >= sWidth - player_obj.width - tolerance) return BORDER_RIGHT;
    if (player_obj.posy <= tolerance) return BORDER_TOP;
    if (player_obj.posy >= sHeight - player_obj.height - tolerance) return BORDER_BOTTOM;
    return BORDER_NONE;
}

// UpdatePlayingScreen não precisa mais do parâmetro Texture2D player1Tex,
// pois as texturas são acessadas de players_arr[i].txr
void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr, int *currentMapX_ptr, int *currentMapY_ptr) {
    // ... (Lógica de música - igual à anterior, omitida para brevidade) ...
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { UpdateMusicStream(playlist_arr[*currentMusicIndex_ptr]); }
    if (IsKeyPressed(KEY_SLASH)) { if (*isPlaying_ptr) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 0; } else { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { ResumeMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 1; } } }
    if (IsKeyPressed(KEY_PAGE_DOWN)) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } }
    if (IsKeyPressed(KEY_PAGE_UP)) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); *currentMusicIndex_ptr = (*currentMusicIndex_ptr - 1 + MAX_SIZE) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } }
    if (IsKeyPressed(KEY_ZERO)) { if (*isPlaying_ptr) { *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } }
    if (IsKeyPressed(KEY_MINUS)) { if (*isPlaying_ptr) { *volume_ptr -= 0.05f; if (*volume_ptr < 0.0f) *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } }
    if (IsKeyPressed(KEY_EQUAL)) { if (*isPlaying_ptr) { *volume_ptr += 0.05f; if (*volume_ptr > 1.0f) *volume_ptr = 1.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } }
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { *musicPlayingTimer_ptr += GetFrameTime(); if (*currentMusicDuration_ptr > 0 && *musicPlayingTimer_ptr >= *currentMusicDuration_ptr) { StopMusicStream(playlist_arr[*currentMusicIndex_ptr]);  *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE;  if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]);  SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]);  *musicPlayingTimer_ptr = 0.0f;  } else { *isPlaying_ptr = 0; } } }


    // --- Movimentação dos Jogadores ---
    // Jogador 0 (Setas) usa RIGHT_SHIFT para correr.
    move_character(&players_arr[0].posx, &players_arr[0].posy, players_arr[0].width, players_arr[0].height, GetScreenWidth(), GetScreenHeight(), 
                   KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, 
                   KEY_RIGHT_SHIFT);
    // Jogador 1 (WASD) usa LEFT_SHIFT para correr.
    move_character(&players_arr[1].posx, &players_arr[1].posy, players_arr[1].width, players_arr[1].height, GetScreenWidth(), GetScreenHeight(), 
                   KEY_A, KEY_D, KEY_W, KEY_S, 
                   KEY_LEFT_SHIFT);

    // --- Lógica de Transição de Mapa com Coordenadas X, Y e Limites ---
    // (Lógica igual à anterior, que já usa os limites MIN/MAX corretos, omitida para brevidade)
    BorderDirection p0_border = GetPlayerBorderCondition(players_arr[0], GetScreenWidth(), GetScreenHeight());
    BorderDirection p1_border = GetPlayerBorderCondition(players_arr[1], GetScreenWidth(), GetScreenHeight());
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
            case BORDER_NONE:   break; 
        }
        if (nextMapX >= WORLD_MAP_MIN_X && nextMapX <= WORLD_MAP_MAX_X && nextMapY >= WORLD_MAP_MIN_Y && nextMapY <= WORLD_MAP_MAX_Y) {
            performTransition = true; 
        } else {
            TraceLog(LOG_INFO, "Bloqueio de transição: Tentativa de sair dos limites do mapa para (%d, %d)", nextMapX, nextMapY);
        }
    }
    if (performTransition) {
        *currentMapX_ptr = nextMapX; 
        *currentMapY_ptr = nextMapY; 
        TraceLog(LOG_INFO, "Transição para Mapa Coords: (%d, %d) pela borda %d", *currentMapX_ptr, *currentMapY_ptr, transitionTo);
        for (int i = 0; i < MAX_PLAYERS; i++) { 
            switch (transitionTo) {
                case BORDER_RIGHT: players_arr[i].posx = players_arr[i].width / 2; players_arr[i].posy = GetScreenHeight() / 2 - players_arr[i].height / 2; break;
                case BORDER_LEFT: players_arr[i].posx = GetScreenWidth() - players_arr[i].width - (players_arr[i].width / 2); players_arr[i].posy = GetScreenHeight() / 2 - players_arr[i].height / 2; break;
                case BORDER_TOP: players_arr[i].posx = GetScreenWidth() / 2 - players_arr[i].width / 2; players_arr[i].posy = GetScreenHeight() - players_arr[i].height - (players_arr[i].height/2); break;
                case BORDER_BOTTOM: players_arr[i].posx = GetScreenWidth() / 2 - players_arr[i].width / 2; players_arr[i].posy = players_arr[i].height / 2; break;
                case BORDER_NONE: break;
            }
        }
    }


    // --- Transição para Pausa ---
    if (IsKeyPressed(KEY_P)) {
        *currentScreen_ptr = GAMESTATE_PAUSE;
        if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
            PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]);
        }
    }
}

// Desenha os elementos da tela principal do jogo (gameplay).
// Removido o parâmetro Texture2D player1Tex, pois as texturas são acessadas de players_arr[i].txr
void DrawPlayingScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY) {
    Color bgColor;
    int colorIndex = (abs(currentMapX) + abs(currentMapY)) % 3; 
    if (colorIndex == 0) bgColor = BLACK;       
    else if (colorIndex == 1) bgColor = DARKBLUE;  
    else bgColor = DARKGREEN; 
    ClearBackground(bgColor);
    
    DrawFPS(5, 5);
    
    char mapCoordsText[64]; 
    sprintf(mapCoordsText, "Mapa (X: %d, Y: %d)", currentMapX, currentMapY); 
    int mapCoordsTextWidth = MeasureText(mapCoordsText, 20); 
    DrawText(mapCoordsText, GetScreenWidth() - mapCoordsTextWidth - 10, 5, 20, YELLOW);

    Vector2 mouse = GetMousePosition();
    DrawText(TextFormat("Mouse X: %d, Y: %d", (int)mouse.x, (int)mouse.y), 5, GetScreenHeight() - 80, 18, LIGHTGRAY);

    // --- Desenha os Jogadores ---
    // Itera sobre os jogadores e desenha cada um.
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players_arr[i].txr.id > 0) { 
            // Se o jogador tem uma textura válida em sua struct (players_arr[i].txr), desenha a textura.
            // As dimensões para o sourceRect da textura são tiradas da própria textura,
            // e as dimensões para colisão (players_arr[i].width/height) são usadas para posicionamento
            // e podem ser diferentes das dimensões da textura se necessário (ex: para spritesheets).
            // Para uma sprite simples que ocupa toda a textura, players_arr[i].txr.width/height seria igual a players_arr[i].width/height.
            DrawTextureRec(players_arr[i].txr, 
                           (Rectangle){0, 0, (float)players_arr[i].txr.width, (float)players_arr[i].txr.height}, 
                           (Vector2){(float)players_arr[i].posx, (float)players_arr[i].posy}, 
                           WHITE); // Cor de tonalidade (WHITE = sem alteração de cor)
        } else {
            // Se o jogador não tem uma textura carregada, desenha como um retângulo colorido.
            Color playerColor;
            switch (players_arr[i].classe) { // Define a cor baseada na classe do jogador
                case GUERREIRO: playerColor = (i == 0) ? RED : MAROON; break; // Cor diferente para P0 e P1 se ambos Guerreiros
                case MAGO:    playerColor = (i == 0) ? BLUE : DARKBLUE; break;
                case ARQUEIRO:playerColor = (i == 0) ? GREEN : DARKGREEN; break;
                default:      playerColor = (i == 0) ? PURPLE : DARKPURPLE; break;
            }
            DrawRectangle(players_arr[i].posx, players_arr[i].posy, players_arr[i].width, players_arr[i].height, playerColor);
        }
    }

    // Informações de música e controles.
    DrawText(TextFormat("Volume: %.0f%%", currentVolume * 100), 5, GetScreenHeight() - 60, 10, LIGHTGRAY);
    DrawText(TextFormat("Música %d (Tocando: %s)", currentMusicIndex + 1, isPlaying ? "Sim" : "Não"), 5, GetScreenHeight() - 45, 10, LIGHTGRAY);
    DrawText("Controles: P=Pause, Setas/WASD=Mover, Right Shift/ Left Shift=Correr P=Pause", 5, GetScreenHeight() - 30, 10, RAYWHITE);
    DrawText("Slash=Play/Pause Música, PgUp/Dn=Mudar Música, +/-/0=Volume", 5, GetScreenHeight() - 15, 10, RAYWHITE);
}

void UpdatePauseScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, int isPlaying_beforePause) {
    // ... (código igual)
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER)) { *currentScreen_ptr = GAMESTATE_PLAYING; if (isPlaying_beforePause && playlist[currentMusicIndex].frameCount > 0 && playlist[currentMusicIndex].stream.buffer != NULL) { ResumeMusicStream(playlist[currentMusicIndex]); } }
    if (IsKeyPressed(KEY_M)) *currentScreen_ptr = GAMESTATE_MENU;
}

// Removido o parâmetro Texture2D player1Tex
void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY) {
    // Passa os parâmetros corretos para DrawPlayingScreen
    DrawPlayingScreen(players_arr, currentVolume, currentMusicIndex, isPlaying, currentMapX, currentMapY); 
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.8f));
    DrawText("PAUSADO", GetScreenWidth() / 2 - MeasureText("PAUSADO", 40) / 2, GetScreenHeight() / 2 - 60, 40, GRAY);
    DrawText("Pressione P ou ENTER para Continuar", GetScreenWidth() / 2 - MeasureText("Pressione P ou ENTER para Continuar", 20) / 2, GetScreenHeight() / 2, 20, LIGHTGRAY);
    DrawText("Pressione M para Voltar ao Menu", GetScreenWidth() / 2 - MeasureText("Pressione M para Voltar ao Menu", 20) / 2, GetScreenHeight() / 2 + 30, 20, LIGHTGRAY);
}

void *pixels() { return NULL; }