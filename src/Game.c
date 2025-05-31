#include "../include/Game.h"    // Para protótipos, GameState, Player, etc.
#include "../include/Classes.h" // Para Classe enum (GUERREIRO, MAGO, ARQUEIRO)
#include <stddef.h>
// #include <string.h> // Se usar strcpy para nomes de jogadores em init_player de Classes.c

// --- Implementação da Função de Movimentação de Personagem ---
// (Sua função move_character original, ajustada para usar parâmetros de largura/altura da tela)
void move_character(int *posx, int *posy, int screenWidth, int screenHeight, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift){ //
    int speed = 3;
    int sprintSpeed = 5;
    int currentSpeed = IsKeyDown(keyShift) ? sprintSpeed : speed; //

    if(IsKeyDown(keyLeft)){ *posx -= currentSpeed; } //
    if(IsKeyDown(keyRight) ){ *posx += currentSpeed; } //
    if(IsKeyDown(keyUp)){ *posy -= currentSpeed; } //
    if(IsKeyDown(keyDown)){ *posy += currentSpeed; } //

    // Para colisão com bordas, idealmente passaria o tamanho do jogador.
    // Usando um valor fixo de 80x80 para este exemplo, assumindo que player.width/height são 80.
    int pWidth = 80;
    int pHeight = 80;

    if(*posx < 0) *posx = 0; //
    if(*posx > screenWidth - pWidth) *posx = screenWidth - pWidth; //
    if(*posy < 0) *posy = 0; //
    if(*posy > screenHeight - pHeight) *posy = screenHeight - pHeight; //
}


// --- Implementações das Funções de Cena ---

void InitGameResources(Player players_arr[], Texture2D *player1Tex_ptr, Music mainPlaylist_arr[]) {
    // Inicializa jogadores
    for (int i = 0; i < MAX_PLAYERS; i++) { //
        players_arr[i].width = 80; //
        players_arr[i].height = 80; //
        // Atribui uma classe para diferenciação (exemplo)
        if (i == 0) players_arr[i].classe = GUERREIRO;
        else players_arr[i].classe = MAGO;


        if (i == 0) { //
            players_arr[i].posx = players_arr[i].width; //
            players_arr[i].posy = GetScreenHeight() - players_arr[i].height * 2; // Ajustado para ficar mais acima
        }
        if (i == 1) { //
            players_arr[i].posx = GetScreenWidth() - players_arr[i].width * 2; //
            players_arr[i].posy = GetScreenHeight() - players_arr[i].height * 2;
        }
         // init_player(&players_arr[i], "NomeExemplo", ARQUEIRO); // Se for usar sua função de Classes.c
    }

    // Carrega textura do jogador 1
    const char *playerImage = "./assets/images/player1.png"; //
    *player1Tex_ptr = LoadTexture(playerImage); //
    if (player1Tex_ptr->id == 0) {
        TraceLog(LOG_WARNING, "Falha ao carregar textura: %s", playerImage);
    }
    // Atribui a textura carregada à struct do jogador 1 (se o campo txr existir e for usado)
    if (MAX_PLAYERS > 1) {
         players_arr[1].txr = *player1Tex_ptr; //
    }


    // Carrega músicas
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

void UpdateMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr) {
    if (IsKeyPressed(KEY_ENTER)) {
        *currentScreen_ptr = GAMESTATE_PLAYING;
        if (playlist[currentMusicIndex].frameCount > 0 && playlist[currentMusicIndex].stream.buffer != NULL) {
            if (!(*isPlaying_ptr)) {
                PlayMusicStream(playlist[currentMusicIndex]);
                *isPlaying_ptr = 1;
            }
            SetMusicVolume(playlist[currentMusicIndex], currentVolume);
        }
    }
    if (IsKeyPressed(KEY_Q)) { // Tecla Q para ir para a intro (exemplo)
        *currentScreen_ptr = GAMESTATE_INTRO;
    }
}

void DrawMenuScreen(void) {
    ClearBackground(DARKGRAY);
    DrawText("FALL WITCHES", GetScreenWidth() / 2 - MeasureText("FALL WITCHES", 40) / 2, GetScreenHeight() / 4, 40, WHITE);
    DrawText("PRESSIONE ENTER para INICIAR", GetScreenWidth() / 2 - MeasureText("PRESSIONE ENTER para INICIAR", 20) / 2, GetScreenHeight() / 2, 20, LIGHTGRAY);
    DrawText("Pressione Q para Intro (Demo)", 10, GetScreenHeight() - 30, 10, RAYWHITE);
}

void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr) {
    (*introFrames_ptr)++;
    if (*introFrames_ptr > 180 || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = GAMESTATE_MENU;
        *introFrames_ptr = 0; // Reseta o contador para a próxima vez
    }
}

void DrawIntroScreen(void) {
    ClearBackground(BLACK);
    DrawText("TELA DE INTRODUÇÃO", GetScreenWidth() / 2 - MeasureText("TELA DE INTRODUÇÃO", 30) / 2, GetScreenHeight() / 2 - 40, 30, WHITE);
    DrawText("Comi o cu de quem tá lendo.", GetScreenWidth() / 2 - MeasureText("Comi o cu de quem tá lendo", 20) / 2, GetScreenHeight() / 2, 20, LIGHTGRAY);
}

void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], Texture2D player1Tex, Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr) {
    // Atualiza stream da música
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) { //
        UpdateMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
    }

    // --- Controles de Música --- (adaptado do seu código original)
    if (IsKeyPressed(KEY_SLASH)) { //
        if (*isPlaying_ptr) {
            if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
            *isPlaying_ptr = 0; //
        } else {
            if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
                ResumeMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
                *isPlaying_ptr = 1; //
            }
        }
    }
    if (IsKeyPressed(KEY_PAGE_DOWN)) { //
        if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
        *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE; //
        if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
            PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
            SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr);
            *isPlaying_ptr = 1; //
            *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); //
            *musicPlayingTimer_ptr = 0.0f; //
        } else { *isPlaying_ptr = 0; }
    }
    if (IsKeyPressed(KEY_PAGE_UP)) { //
        if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
        *currentMusicIndex_ptr = (*currentMusicIndex_ptr - 1 + MAX_SIZE) % MAX_SIZE; //
        if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
            PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
            SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr);
            *isPlaying_ptr = 1; //
            *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); //
            *musicPlayingTimer_ptr = 0.0f; //
        } else { *isPlaying_ptr = 0; }
    }
    if (IsKeyPressed(KEY_ZERO)) { //
        if (*isPlaying_ptr) { *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } //
    }
    if (IsKeyPressed(KEY_MINUS)) { //
        if (*isPlaying_ptr) { *volume_ptr -= 0.05f; if (*volume_ptr < 0.0f) *volume_ptr = 0.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } //
    }
    if (IsKeyPressed(KEY_EQUAL)) { //
        if (*isPlaying_ptr) { *volume_ptr += 0.05f; if (*volume_ptr > 1.0f) *volume_ptr = 1.0f; SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr); } //
    }

    // Avança música automaticamente
    if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
        *musicPlayingTimer_ptr += GetFrameTime(); //
        if (*currentMusicDuration_ptr > 0 && *musicPlayingTimer_ptr >= *currentMusicDuration_ptr) { //
            StopMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
            *currentMusicIndex_ptr = (*currentMusicIndex_ptr + 1) % MAX_SIZE; //
            if (playlist_arr[*currentMusicIndex_ptr].frameCount > 0 && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
                PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]); //
                SetMusicVolume(playlist_arr[*currentMusicIndex_ptr], *volume_ptr);
                *isPlaying_ptr = 1;
                *currentMusicDuration_ptr = GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]); //
                *musicPlayingTimer_ptr = 0.0f; //
            } else { *isPlaying_ptr = 0; }
        }
    }

    // Movimentação dos Jogadores
    // Usa GetScreenWidth() e GetScreenHeight() diretamente, já que são globais da Raylib
    move_character(&players_arr[1].posx, &players_arr[1].posy, GetScreenWidth(), GetScreenHeight(), KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT); //
    move_character(&players_arr[0].posx, &players_arr[0].posy, GetScreenWidth(), GetScreenHeight(), KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT); //

    // Transição para Pausa
    if (IsKeyPressed(KEY_P)) {
        *currentScreen_ptr = GAMESTATE_PAUSE;
        if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
            PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]);
            // Não muda isPlaying_ptr para 0, pois a música está pausada, não parada.
        }
    }
}

void DrawPlayingScreen(Player players_arr[], Texture2D player1Tex, float currentVolume, int currentMusicIndex, int isPlaying) {
    ClearBackground(BLACK); //
    // UpdateTexture(player1Tex, pixels()); // Se usar textura dinâmica

    DrawFPS(5, 5); //
    Vector2 mouse = GetMousePosition(); //
    DrawText(TextFormat("Mouse X: %d, Y: %d", (int)mouse.x, (int)mouse.y), 5, 25, 18, LIGHTGRAY); //

    // Desenha Jogador 1 (com textura)
    // Checa se a textura atribuída ao player[1] na sua struct é válida.
    if (players_arr[1].txr.id > 0) { //
        DrawTextureRec(players_arr[1].txr, //
                       (Rectangle){0, 0, (float)players_arr[1].txr.width, (float)players_arr[1].txr.height},
                       (Vector2){(float)players_arr[1].posx, (float)players_arr[1].posy}, RAYWHITE); //
    } else if (player1Tex.id > 0) { // Senão, tenta usar a textura global player1Tex
        DrawTextureRec(player1Tex,
                       (Rectangle){0, 0, (float)player1Tex.width, (float)player1Tex.height},
                       (Vector2){(float)players_arr[1].posx, (float)players_arr[1].posy}, WHITE);
    }


    // Desenha Jogador 0 (retângulo com cor baseada na classe)
    Color player0Color;
    switch (players_arr[0].classe) {
        case GUERREIRO: player0Color = RED; break;
        case MAGO: player0Color = BLUE; break;
        case ARQUEIRO: player0Color = GREEN; break;
        default: player0Color = DARKPURPLE; break; // Cor padrão
    }
    DrawRectangle(players_arr[0].posx, players_arr[0].posy, players_arr[0].width, players_arr[0].height, player0Color); //

    // Informações na tela
    DrawText(TextFormat("Volume: %.0f%%", currentVolume * 100), 5, GetScreenHeight() - 60, 10, LIGHTGRAY);
    DrawText(TextFormat("Música %d (Tocando: %s)", currentMusicIndex + 1, isPlaying ? "Sim" : "Não"), 5, GetScreenHeight() - 45, 10, LIGHTGRAY);
    DrawText("Controles: P=Pause, Setas/WASD=Mover, Shift=Correr", 5, GetScreenHeight() - 30, 10, RAYWHITE);
    DrawText("Slash=Play/Pause Música, PgUp/Dn=Mudar Música, +/-/0=Volume", 5, GetScreenHeight() - 15, 10, RAYWHITE);
}

void UpdatePauseScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, int isPlaying) {
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER)) {
        *currentScreen_ptr = GAMESTATE_PLAYING;
        // Se a música estava tocando (isPlaying = 1 mas pausada), Resume
        // Se isPlaying = 0 (estava parada), não faz nada com música.
        // A flag isPlaying reflete se a música *deveria* estar tocando se não pausada.
        if (isPlaying && playlist[currentMusicIndex].frameCount > 0 && playlist[currentMusicIndex].stream.buffer != NULL) {
            ResumeMusicStream(playlist[currentMusicIndex]);
        }
    }
    if (IsKeyPressed(KEY_M)) {
        *currentScreen_ptr = GAMESTATE_MENU;
        // Opcional: parar música do jogo aqui
        // if (isPlaying && playlist[currentMusicIndex].stream.buffer != NULL) {
        // StopMusicStream(playlist[currentMusicIndex]);
        // isPlaying_ptr pode precisar ser passado e setado para 0 se a música for parada.
        // }
    }
}

void DrawPauseScreen(Player players_arr[], Texture2D player1Tex, float currentVolume, int currentMusicIndex, int isPlaying) {
    DrawPlayingScreen(players_arr, player1Tex, currentVolume, currentMusicIndex, isPlaying); // Desenha a cena de jogo por baixo
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.8f)); // Camada escura
    DrawText("PAUSADO", GetScreenWidth() / 2 - MeasureText("PAUSADO", 40) / 2, GetScreenHeight() / 2 - 60, 40, GRAY);
    DrawText("Pressione P ou ENTER para Continuar", GetScreenWidth() / 2 - MeasureText("Pressione P ou ENTER para Continuar", 20) / 2, GetScreenHeight() / 2, 20, LIGHTGRAY);
    DrawText("Pressione M para Voltar ao Menu", GetScreenWidth() / 2 - MeasureText("Pressione M para Voltar ao Menu", 20) / 2, GetScreenHeight() / 2 + 30, 20, LIGHTGRAY);
}

// Função stub 'pixels', se for usar UpdateTexture com dados da CPU
void *pixels() { //
    return NULL;
}