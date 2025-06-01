#include "../include/Game.h"    // Para estruturas específicas do jogo, enums (como GameState) e protótipos de funções
#include "../include/Classes.h" // Para a struct Player e a função init_player
#include <raylib.h>          // Para funções e tipos da Raylib
#include <stdbool.h>         // Para o tipo bool
#include <stdio.h>           // Para E/S padrão, como sprintf
#include <stddef.h>          // Para NULL, size_t etc.
#include <stdlib.h>          // Para abs()
#include <string.h>          // Para strncpy, strlen (usado em InitGameResources para nomes, se necessário)

// Declarações externas para os limites globais do mapa definidos em main.c
extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;

// Enum para detecção de borda, usado por GetPlayerBorderCondition
typedef enum {
    BORDER_NONE = 0, BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM
} BorderDirection;

// --- Implementação da Função de Movimentação de Personagem ---
void move_character(int *posx_ptr, int *posy_ptr, int pWidth, int pHeight, int screenWidth, int screenHeight,
                    int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift){
    int speed = 3;
    int sprintSpeed = 5;
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

// --- Implementações das Funções de Gerenciamento de Jogo ---

/**
 * @brief Inicializa os recursos do jogo. Chama init_player para cada jogador
 * e então define posições, texturas e carrega músicas.
 * @param players_arr Array de estruturas Player para inicializar.
 * @param mainPlaylist_arr Array de streams de Música para carregar.
 */
void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        char player_default_name[MAX_PLAYER_NAME_LENGTH];
        Classe player_default_class;

        if (i == 0) {
            player_default_class = GUERREIRO;
            sprintf(player_default_name, "Jogador %d", i + 1); // Nome padrão
             // A função init_player em Classes.c já define o nome "Guerrilheiro Nato"
             // Se quiser usar nomes diferentes aqui, pode passar para init_player.
             // Por ora, vamos usar os nomes definidos em init_player.
            init_player(&players_arr[i], "Guerrilheiro Nato", player_default_class); //
        } else if (i == 1) { // Assumindo MAX_PLAYERS >= 2
            player_default_class = MAGO;
            sprintf(player_default_name, "Jogador %d", i + 1); // Nome padrão
            init_player(&players_arr[i], "Feiticeira Suprema", player_default_class); //
        } else {
            // Fallback para outros jogadores, se houver
            player_default_class = ARQUEIRO; // Exemplo
            sprintf(player_default_name, "Aventureiro %d", i + 1);
            init_player(&players_arr[i], player_default_name, player_default_class);
        }


        // init_player já cuida de: nome, classe, nível, vida/max_vida, mana/max_mana,
        // ataque, defesa, current_inventory_tab e chama init_player_inventory.

        // Agora, InitGameResources define/sobrescreve APENAS o que é específico do setup do jogo:
        // como posições iniciais e dimensões de colisão/desenho.
        players_arr[i].width = 80;  // Largura para colisão e desenho da sprite.
        players_arr[i].height = 80; // Altura para colisão e desenho da sprite.

        if (i == 0) {
            players_arr[i].posx = players_arr[i].width; //
            players_arr[i].posy = GetScreenHeight() - players_arr[i].height * 2; //
        } else if (i == 1) {
            players_arr[i].posx = GetScreenWidth() - players_arr[i].width * 2; //
            players_arr[i].posy = GetScreenHeight() - players_arr[i].height * 2; //
        }
    }

    // Carrega a textura para o Jogador 0
    const char *player0ImagePath = "./assets/images/player2.png"; //
    if (MAX_PLAYERS > 0) {
        players_arr[0].txr = LoadTexture(player0ImagePath); //
        if (players_arr[0].txr.id == 0) { //
            TraceLog(LOG_WARNING, "Falha ao carregar textura para Jogador 0: %s", player0ImagePath); //
        } else {
            TraceLog(LOG_INFO, "Textura para Jogador 0 carregada: %s", player0ImagePath); //
        }
    }

    // Carrega a textura para o Jogador 1
    const char *player1ImagePath = "./assets/images/player1.png"; //
    if (MAX_PLAYERS > 1) {
        players_arr[1].txr = LoadTexture(player1ImagePath); //
        if (players_arr[1].txr.id == 0) { //
            TraceLog(LOG_WARNING, "Falha ao carregar textura para Jogador 1: %s", player1ImagePath); //
        } else {
            TraceLog(LOG_INFO, "Textura para Jogador 1 carregada: %s", player1ImagePath); //
        }
    }

    // Carregamento de músicas
    const char *musicFiles[MAX_SIZE] = { //
        "./assets/songs/Desmeon_-_My_Sunshine.ogg", //
        "./assets/songs/19 William Tell Overture.ogg", //
        "./assets/songs/last-summer-by-ikson.mp3", //
        "./assets/songs/Floatinurboat - Spirit Of Things.mp3" //
    };
    for (int i = 0; i < MAX_SIZE; i++) { //
        mainPlaylist_arr[i] = LoadMusicStream(musicFiles[i]); //
        if (mainPlaylist_arr[i].stream.buffer == NULL) { //
            TraceLog(LOG_WARNING, "Falha ao carregar música %d: %s", i + 1, musicFiles[i]); //
        }
    }
}

/**
 * @brief Determina se um jogador está na borda da tela para transições de mapa.
 * @param player_obj O objeto do jogador a ser verificado.
 * @param sWidth Largura da tela.
 * @param sHeight Altura da tela.
 * @return Enum BorderDirection indicando qual borda, ou BORDER_NONE.
 */
BorderDirection GetPlayerBorderCondition(Player player_obj, int sWidth, int sHeight) {
    int tolerance = 5; // Pequena tolerância para facilitar o acionamento
    if (player_obj.posx <= tolerance) return BORDER_LEFT; //
    if (player_obj.posx >= sWidth - player_obj.width - tolerance) return BORDER_RIGHT; //
    if (player_obj.posy <= tolerance) return BORDER_TOP; //
    if (player_obj.posy >= sHeight - player_obj.height - tolerance) return BORDER_BOTTOM; //
    return BORDER_NONE; //
}

/**
 * @brief Atualiza a lógica principal da tela de jogo.
 * Gerencia música, movimento dos jogadores, transições de mapa, pausa e abertura do inventário.
 * @param currentScreen_ptr Ponteiro para o estado atual do jogo.
 * @param players_arr Array de estruturas Player.
 * @param playlist_arr Array de streams de Música.
 * @param currentMusicIndex_ptr Ponteiro para o índice da faixa de música atual.
 * @param volume_ptr Ponteiro para o volume atual da música.
 * @param isPlaying_ptr Ponteiro para uma flag indicando se a música está tocando.
 * @param musicPlayingTimer_ptr Ponteiro para um timer para a reprodução da faixa de música atual.
 * @param currentMusicDuration_ptr Ponteiro para a duração da faixa de música atual.
 * @param currentMapX_ptr Ponteiro para a coordenada X atual do mapa.
 * @param currentMapY_ptr Ponteiro para a coordenada Y atual do mapa.
 */
void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr, int *currentMapX_ptr, int *currentMapY_ptr) {
    // --- Lógica da Música ---
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { UpdateMusicStream(playlist_arr[*currentMusicIndex_ptr]); } //
    if (IsKeyPressed(KEY_SLASH)) { if (*isPlaying_ptr) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 0; } else { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { ResumeMusicStream(playlist_arr[*currentMusicIndex_ptr]); *isPlaying_ptr = 1; } } } //
    if (IsKeyPressed(KEY_PAGE_DOWN)) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } } //
    if (IsKeyPressed(KEY_PAGE_UP)) { if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); *currentMusicIndex_ptr = (*currentMusicIndex_ptr - 1 + MAX_SIZE) % MAX_SIZE; if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); *musicPlayingTimer_ptr = 0.0f; } else { *isPlaying_ptr = 0; } } //
    if (IsKeyPressed(KEY_ZERO)) { if (*isPlaying_ptr) { *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } } //
    if (IsKeyPressed(KEY_MINUS)) { if (*isPlaying_ptr) { *volume_ptr -= 0.05f; if (*volume_ptr < 0.0f) *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } } //
    if (IsKeyPressed(KEY_EQUAL)) { if (*isPlaying_ptr) { *volume_ptr += 0.05f; if (*volume_ptr > 1.0f) *volume_ptr = 1.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } } //
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { *musicPlayingTimer_ptr += GetFrameTime(); if (*currentMusicDuration_ptr > 0 && *musicPlayingTimer_ptr >= *currentMusicDuration_ptr) { StopMusicStream(playlist_arr[*currentMusicIndex_ptr]);  *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE;  if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]);  SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); *isPlaying_ptr = 1; *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]);  *musicPlayingTimer_ptr = 0.0f;  } else { *isPlaying_ptr = 0; } } } //

    // --- Movimentação dos Jogadores ---
    move_character(&players_arr[0].posx, &players_arr[0].posy, players_arr[0].width, players_arr[0].height, GetScreenWidth(), GetScreenHeight(),
                   KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT); //
    if (MAX_PLAYERS > 1) {
        move_character(&players_arr[1].posx, &players_arr[1].posy, players_arr[1].width, players_arr[1].height, GetScreenWidth(), GetScreenHeight(),
                       KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT); //
    }


    // --- Lógica de Transição de Mapa ---
    if (MAX_PLAYERS > 1) { // Lógica de transição de mapa requer 2 jogadores neste design
        BorderDirection p0_border = GetPlayerBorderCondition(players_arr[0], GetScreenWidth(), GetScreenHeight()); //
        BorderDirection p1_border = GetPlayerBorderCondition(players_arr[1], GetScreenWidth(), GetScreenHeight()); //
        bool performTransition = false;      //
        BorderDirection transitionTo = BORDER_NONE; //
        int nextMapX = *currentMapX_ptr; //
        int nextMapY = *currentMapY_ptr; //
        if (p0_border != BORDER_NONE && p0_border == p1_border) { //
            transitionTo = p0_border; //
            switch (transitionTo) { //
                case BORDER_LEFT:   nextMapX = *currentMapX_ptr - 1; break; //
                case BORDER_RIGHT:  nextMapX = *currentMapX_ptr + 1; break; //
                case BORDER_TOP:    nextMapY = *currentMapY_ptr - 1; break; //
                case BORDER_BOTTOM: nextMapY = *currentMapY_ptr + 1; break; //
                case BORDER_NONE:   break; //
            }
            if (nextMapX >= WORLD_MAP_MIN_X && nextMapX <= WORLD_MAP_MAX_X && nextMapY >= WORLD_MAP_MIN_Y && nextMapY <= WORLD_MAP_MAX_Y) { //
                performTransition = true; //
            } else {
                TraceLog(LOG_INFO, "Bloqueio de transição: Tentativa de sair dos limites do mapa para (%d, %d)", nextMapX, nextMapY); //
            }
        }
        if (performTransition) { //
            *currentMapX_ptr = nextMapX; //
            *currentMapY_ptr = nextMapY; //
            TraceLog(LOG_INFO, "Transição para Mapa Coords: (%d, %d) pela borda %d", *currentMapX_ptr, *currentMapY_ptr, transitionTo); //
            for (int i = 0; i < MAX_PLAYERS; i++) { //
                switch (transitionTo) { //
                    case BORDER_RIGHT: players_arr[i].posx = players_arr[i].width / 2; players_arr[i].posy = GetScreenHeight() / 2 - players_arr[i].height / 2; break; //
                    case BORDER_LEFT: players_arr[i].posx = GetScreenWidth() - players_arr[i].width - (players_arr[i].width / 2); players_arr[i].posy = GetScreenHeight() / 2 - players_arr[i].height / 2; break; //
                    case BORDER_TOP: players_arr[i].posx = GetScreenWidth() / 2 - players_arr[i].width / 2; players_arr[i].posy = GetScreenHeight() - players_arr[i].height - (players_arr[i].height/2); break; //
                    case BORDER_BOTTOM: players_arr[i].posx = GetScreenWidth() / 2 - players_arr[i].width / 2; players_arr[i].posy = players_arr[i].height / 2; break; //
                    case BORDER_NONE: break; //
                }
            }
        }
    }


    // --- Transição para Pausa ---
    if (IsKeyPressed(KEY_P)) { //
        *currentScreen_ptr = GAMESTATE_PAUSE; //
        if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { //
            PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
        }
    }

    // --- Transição para Inventário ---
    if (IsKeyPressed(KEY_E)) {
        *currentScreen_ptr = GAMESTATE_INVENTORY;
        // A lógica de pausar/retomar música será tratada por UpdateInventoryScreen.
    }
}

/**
 * @brief Desenha a tela principal de jogo.
 * Renderiza o fundo, jogadores, FPS, coordenadas do mapa e outras informações do HUD.
 * @param players_arr Array de estruturas Player.
 * @param currentVolume Volume atual da música.
 * @param currentMusicIndex Índice da faixa de música atual.
 * @param isPlaying Flag indicando se a música está tocando.
 * @param currentMapX Coordenada X atual do mapa.
 * @param currentMapY Coordenada Y atual do mapa.
 */
void DrawPlayingScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY) {
    Color bgColor; //
    int colorIndex = (abs(currentMapX) + abs(currentMapY)) % 3; //
    if (colorIndex == 0) bgColor = BLACK; //
    else if (colorIndex == 1) bgColor = DARKBLUE; //
    else bgColor = DARKGREEN; //
    ClearBackground(bgColor); //

    DrawFPS(5, 5); //

    char mapCoordsText[64]; //
    sprintf(mapCoordsText, "Mapa (X: %d, Y: %d)", currentMapX, currentMapY); //
    int mapCoordsTextWidth = MeasureText(mapCoordsText, 20); //
    DrawText(mapCoordsText, GetScreenWidth() - mapCoordsTextWidth - 10, 5, 20, YELLOW); //

    Vector2 mouse = GetMousePosition(); //
    DrawText(TextFormat("Mouse X: %d, Y: %d", (int)mouse.x, (int)mouse.y), 5, GetScreenHeight() - 80, 18, LIGHTGRAY); //

    for (int i = 0; i < MAX_PLAYERS; i++) { //
        if (players_arr[i].txr.id > 0) { //
            DrawTextureRec(players_arr[i].txr,
                           (Rectangle){0, 0, (float)players_arr[i].txr.width, (float)players_arr[i].txr.height},
                           (Vector2){(float)players_arr[i].posx, (float)players_arr[i].posy},
                           WHITE); //
        } else {
            Color playerColor; //
            switch (players_arr[i].classe) { //
                case GUERREIRO: playerColor = (i == 0) ? RED : MAROON; break; //
                case MAGO:    playerColor = (i == 0) ? BLUE : DARKBLUE; break; //
                case ARQUEIRO:playerColor = (i == 0) ? GREEN : DARKGREEN; break; //
                default:      playerColor = (i == 0) ? PURPLE : DARKPURPLE; break; //
            }
            DrawRectangle(players_arr[i].posx, players_arr[i].posy, players_arr[i].width, players_arr[i].height, playerColor); //
        }
    }

    DrawText(TextFormat("Volume: %.0f%%", currentVolume * 100), 5, GetScreenHeight() - 60, 10, LIGHTGRAY); //
    DrawText(TextFormat("Música %d (Tocando: %s)", currentMusicIndex + 1, isPlaying ? "Sim" : "Não"), 5, GetScreenHeight() - 45, 10, LIGHTGRAY); //
    DrawText("Controles: P=Pause, E=Inventário, Setas/WASD=Mover, Shift=Correr", 5, GetScreenHeight() - 30, 10, RAYWHITE); // Atualizado
    DrawText("Slash=Play/Pause Música, PgUp/Dn=Mudar Música, +/-/0=Volume", 5, GetScreenHeight() - 15, 10, RAYWHITE); //
}

// Função placeholder (existente no código original)
void *pixels() { return NULL; } //