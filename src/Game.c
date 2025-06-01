#include "../include/Game.h"    
#include "../include/Classes.h" 
#include "../include/Inventory.h" // Necessário se AddItemToInventory fosse chamado aqui
#include <raylib.h>          
#include <stdbool.h>         
#include <stdio.h>           
#include <stddef.h>          
#include <stdlib.h>          
#include <string.h>          

// Variáveis globais externas dos limites do mapa (declaradas em main.c)
extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;

// Enum para detecção de borda
typedef enum {
    BORDER_NONE = 0, BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM
} BorderDirection;

// --- Implementação da Função de Movimentação de Personagem ---
void move_character(int *posx_ptr, int *posy_ptr, int pWidth, int pHeight, int screenWidth, int screenHeight,
                    int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift){
    int speed = 3; // Velocidade normal
    int sprintSpeed = 5; // Velocidade correndo
    int currentSpeed = IsKeyDown(keyShift) ? sprintSpeed : speed;

    if (IsKeyDown(keyLeft))  { *posx_ptr -= currentSpeed; }
    if (IsKeyDown(keyRight)) { *posx_ptr += currentSpeed; }
    if (IsKeyDown(keyUp))    { *posy_ptr -= currentSpeed; }
    if (IsKeyDown(keyDown))  { *posy_ptr += currentSpeed; }

    // Verificações de limites da tela
    if (*posx_ptr < 0) *posx_ptr = 0;
    if (*posx_ptr > screenWidth - pWidth) *posx_ptr = screenWidth - pWidth;
    if (*posy_ptr < 0) *posy_ptr = 0;
    if (*posy_ptr > screenHeight - pHeight) *posy_ptr = screenHeight - pHeight;
}

// --- Inicializa os Recursos Principais do Jogo (Texturas, Músicas) ---
void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]) {
    // Define dimensões padrão para os jogadores (stats e nomes serão definidos na criação/load)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players_arr[i].width = 80; 
        players_arr[i].height = 80;
        // Texturas são carregadas aqui, mas init_player não deve limpá-las.
    }

    const char *player0ImagePath = "./assets/images/player2.png"; 
    if (MAX_PLAYERS > 0) {
        players_arr[0].txr = LoadTexture(player0ImagePath); 
        if (players_arr[0].txr.id == 0) { 
            TraceLog(LOG_WARNING, "[InitGameResources] Falha ao carregar textura para Jogador 0: %s", player0ImagePath); 
        }
    }

    const char *player1ImagePath = "./assets/images/player1.png"; 
    if (MAX_PLAYERS > 1) {
        players_arr[1].txr = LoadTexture(player1ImagePath); 
        if (players_arr[1].txr.id == 0) { 
            TraceLog(LOG_WARNING, "[InitGameResources] Falha ao carregar textura para Jogador 1: %s", player1ImagePath); 
        }
    }

    // Carregamento de músicas
    const char *musicFiles[MAX_SIZE] = { 
        "./assets/songs/Desmeon_-_My_Sunshine.ogg", 
        "./assets/songs/19 William Tell Overture.ogg", 
        "./assets/songs/last-summer-by-ikson.mp3", 
        "./assets/songs/Floatinurboat - Spirit Of Things.mp3" 
    };
    for (int i = 0; i < MAX_SIZE; i++) { 
        mainPlaylist_arr[i] = LoadMusicStream(musicFiles[i]); 
        if (mainPlaylist_arr[i].stream.buffer == NULL) { 
            TraceLog(LOG_WARNING, "[InitGameResources] Falha ao carregar música %d: %s", i + 1, musicFiles[i]); 
        }
    }
}

/**
 * @brief Prepara o estado básico para uma nova sessão de jogo, ANTES da criação de personagem.
 * Reseta as coordenadas do mapa. As structs dos jogadores são apenas preparadas com posições
 * e dimensões iniciais; init_player será chamado após a criação de personagem.
 */
void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY, int screenWidth, int screenHeight) {
    TraceLog(LOG_INFO, "Preparando nova sessão de jogo (antes da criação de personagem)...");

    *mapX = 0; // Reseta mapa para a origem
    *mapY = 0;

    // Define posições e dimensões iniciais. Nomes, classes e stats serão definidos na criação.
    for (int i = 0; i < MAX_PLAYERS; i++) {
        // Limpa o nome para a tela de criação
        strcpy(players_arr[i].nome, ""); 
        
        players_arr[i].width = 80; 
        players_arr[i].height = 80;
        // Posições iniciais padrão na tela de jogo (podem ser ajustadas após criação)
        if (i == 0) {
            players_arr[i].posx = players_arr[i].width + 50; // Afasta um pouco da borda
            players_arr[i].posy = screenHeight / 2 - players_arr[i].height / 2; 
        } else if (i == 1) {
            players_arr[i].posx = screenWidth - players_arr[i].width * 2 - 50; // Afasta da outra borda
            players_arr[i].posy = screenHeight / 2 - players_arr[i].height / 2;
        }
        // Outros campos (nível, exp, vida, etc.) serão definidos por init_player na tela de criação.
    }
}


// Determina se um jogador está na borda da tela para transições de mapa.
BorderDirection GetPlayerBorderCondition(Player player_obj, int sWidth, int sHeight) {
    // (Lógica mantida)
    int tolerance = 5; 
    if (player_obj.posx <= tolerance) return BORDER_LEFT; 
    if (player_obj.posx >= sWidth - player_obj.width - tolerance) return BORDER_RIGHT; 
    if (player_obj.posy <= tolerance) return BORDER_TOP; 
    if (player_obj.posy >= sHeight - player_obj.height - tolerance) return BORDER_BOTTOM; 
    return BORDER_NONE; 
}

// Atualiza a lógica principal da tela de jogo.
void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr, int *currentMapX_ptr, int *currentMapY_ptr) {
    // (Lógica de música e movimentação mantida)
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { UpdateMusicStream(playlist_arr[*currentMusicIndex_ptr]); } 
    if (IsKeyPressed(KEY_COMMA)) { if (*isPlaying_ptr) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 0; } else { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { ResumeMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 1; } } } 
    if (IsKeyPressed(KEY_PERIOD)) { if (*isPlaying_ptr) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 0; } else { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { ResumeMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 1; } } } // Ponto também como toggle
    if (IsKeyPressed(KEY_PAGE_DOWN)) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } } 
    if (IsKeyPressed(KEY_PAGE_UP)) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); *currentMusicIndex_ptr = (*currentMusicIndex_ptr - 1 + MAX_SIZE) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } } 
    if (IsKeyPressed(KEY_KP_0) || IsKeyPressed(KEY_ZERO)) { if (*isPlaying_ptr) { *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } } 
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) { if (*isPlaying_ptr) { *volume_ptr -= 0.05f; if (*volume_ptr < 0.0f) *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } } 
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) { if (*isPlaying_ptr) { *volume_ptr += 0.05f; if (*volume_ptr > 1.0f) *volume_ptr = 1.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } } 
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { *musicPlayingTimer_ptr += GetFrameTime(); if (*currentMusicDuration_ptr > 0 && *musicPlayingTimer_ptr >= *currentMusicDuration_ptr) { StopMusicStream(playlist_arr[*currentMusicIndex_ptr]);  *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE;  if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]);  SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]);  *musicPlayingTimer_ptr = 0.0f;  } else { *isPlaying_ptr = 0; } } } 

    if (MAX_PLAYERS > 0) {
        move_character(&players_arr[0].posx, &players_arr[0].posy, players_arr[0].width, players_arr[0].height, GetScreenWidth(), GetScreenHeight(),
                    KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT); 
    }
    if (MAX_PLAYERS > 1) {
        move_character(&players_arr[1].posx, &players_arr[1].posy, players_arr[1].width, players_arr[1].height, GetScreenWidth(), GetScreenHeight(),
                       KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT); 
    }

    // Lógica de transição de mapa (mantida)
    if (MAX_PLAYERS > 1) { 
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
            }
        }
        if (performTransition) { 
            *currentMapX_ptr = nextMapX; 
            *currentMapY_ptr = nextMapY; 
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
    }

    if (IsKeyPressed(KEY_P)) { 
        *currentScreen_ptr = GAMESTATE_PAUSE; 
        if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { 
            PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); 
        }
    }
    if (IsKeyPressed(KEY_E)) {
        *currentScreen_ptr = GAMESTATE_INVENTORY;
    }
}

void DrawPlayingScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY) {
    // (Lógica de desenho mantida)
    Color bgColor; 
    int colorIndex = (abs(currentMapX) + abs(currentMapY)) % 4; // Adicionada uma cor extra
    if (colorIndex == 0) bgColor = BLACK; 
    else if (colorIndex == 1) bgColor = DARKBROWN; 
    else if (colorIndex == 2) bgColor = DARKPURPLE; 
    else bgColor = (Color){20, 40, 60, 255}; // Azul escuro acinzentado
    ClearBackground(bgColor); 

    DrawFPS(5, 5); 
    char mapCoordsText[64]; 
    sprintf(mapCoordsText, "Mapa (X: %d, Y: %d)", currentMapX, currentMapY); 
    int mapCoordsTextWidth = MeasureText(mapCoordsText, 20); 
    DrawText(mapCoordsText, GetScreenWidth() - mapCoordsTextWidth - 10, 5, 20, YELLOW); 

    for (int i = 0; i < MAX_PLAYERS; i++) { 
        if (players_arr[i].txr.id > 0) { 
            DrawTextureRec(players_arr[i].txr,
                           (Rectangle){0, 0, (float)players_arr[i].txr.width, (float)players_arr[i].txr.height},
                           (Vector2){(float)players_arr[i].posx, (float)players_arr[i].posy},
                           WHITE); 
        } else { 
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

    DrawText(TextFormat("Volume: %.0f%%", currentVolume * 100), 5, GetScreenHeight() - 60, 10, LIGHTGRAY); 
    DrawText(TextFormat("Música %d (Tocando: %s)", currentMusicIndex + 1, isPlaying ? "Sim" : "Não"), 5, GetScreenHeight() - 45, 10, LIGHTGRAY); 
    DrawText("Controles: P=Pausa, E=Inventário, Setas/WASD=Mover, Shift=Correr", 5, GetScreenHeight() - 30, 10, RAYWHITE); 
    DrawText(",/.=Play/Pause Música, PgUp/Dn=Mudar Música, +/-/0=Volume", 5, GetScreenHeight() - 15, 10, RAYWHITE); 
}

void *pixels() { return NULL; }