#include <stddef.h>
#include <stdbool.h>
#include "raylib.h"
#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Menu.h"
#include "../include/Inventory.h"
#include "../include/SaveLoad.h"
#include <math.h> // Para fminf, fmaxf e outras funções matemáticas

// --- Constantes Globais para Resolução Virtual ---
const int virtualScreenWidth = 800;
const int virtualScreenHeight = 450;

// --- Variáveis Globais do Jogo ---
Player players[MAX_PLAYERS];
Music gamePlaylist[MAX_SIZE];
GameState currentScreen;
int introScreenFramesCounter = 0;
int currentPlaylistIndex = 0;
int musicIsPlaying = 1;
float musicVolume = 0.5f;
float musicPlayTimer = 0.0f;
float currentTrackDuration = 0.0f;
int g_currentMapX = 0;
int g_currentMapY = 0;
const int WORLD_MAP_MIN_X = -10;
const int WORLD_MAP_MAX_X = 10;
const int WORLD_MAP_MIN_Y = -10;
const int WORLD_MAP_MAX_Y = 10;
bool g_request_exit = false;

RenderTexture2D targetRenderTexture; // Textura de renderização para o jogo

// --- Câmera 2D Global ---
// Usada para controlar o zoom e a posição da visão do jogo.
Camera2D gameCamera = { 0 };

// Função auxiliar para coordenadas do mouse virtuais
Vector2 GetVirtualMousePosition(Vector2 actualMousePos) {
    Vector2 virtualMouse = { 0 };
    float scale = fminf((float)GetScreenWidth() / virtualScreenWidth, (float)GetScreenHeight() / virtualScreenHeight);
    float scaledRenderWidth = virtualScreenWidth * scale;
    float scaledRenderHeight = virtualScreenHeight * scale;
    float letterboxX = (GetScreenWidth() - scaledRenderWidth) * 0.5f;
    float letterboxY = (GetScreenHeight() - scaledRenderHeight) * 0.5f;
    virtualMouse.x = (actualMousePos.x - letterboxX) / scale;
    virtualMouse.y = (actualMousePos.y - letterboxY) / scale;
    return virtualMouse;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_RESIZABLE);
    InitWindow(virtualScreenWidth, virtualScreenHeight, "Fall Witches - 0.0.7");
    SetExitKey(KEY_NULL); // ESC não fecha mais o jogo globalmente
    InitAudioDevice();

    InitGameResources(players, gamePlaylist);

    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_BILINEAR);

    // --- Inicialização da Câmera 2D ---
    // O 'offset' é o ponto na tela virtual onde o 'target' da câmera será centralizado.
    // Geralmente, é o centro da tela virtual.
    gameCamera.offset = (Vector2){ virtualScreenWidth / 2.0f, virtualScreenHeight / 2.0f };
    gameCamera.rotation = 0.0f; // Sem rotação da câmera
    // Zoom inicial. Um valor > 1.0f zooma para dentro (menos área visível).
    // Se a seção é 9x maior que o FOV no zoom máximo, zoom = 3.0f.
    // Vamos começar com um zoom mais moderado ou 1.0f e deixar UpdateCamera ajustar.
    gameCamera.zoom = 1.5f; // Zoom inicial (pode ser 3.0f para o requisito de "9x")
    // gameCamera.target será atualizado dinamicamente no loop do jogo.
    // Define um target inicial para evitar problemas no primeiro frame, caso UpdateCamera não seja chamado.
    gameCamera.target = (Vector2){ virtualScreenWidth / 2.0f, virtualScreenHeight / 2.0f };


    currentScreen = GAMESTATE_INTRO;
    introScreenFramesCounter = 0;

    if (gamePlaylist[currentPlaylistIndex].frameCount > 0 && gamePlaylist[currentPlaylistIndex].stream.buffer != NULL) {
        PlayMusicStream(gamePlaylist[currentPlaylistIndex]);
        SetMusicVolume(gamePlaylist[currentPlaylistIndex], musicVolume);
        musicIsPlaying = 1;
        currentTrackDuration = GetMusicTimeLength(gamePlaylist[currentPlaylistIndex]);
        musicPlayTimer = 0.0f;
    } else {
        musicIsPlaying = 0;
    }

    SetTargetFPS(60);

    while (!WindowShouldClose() && !g_request_exit) {
        Vector2 mousePosition = GetMousePosition();
        Vector2 virtualMousePosition = GetVirtualMousePosition(mousePosition);

        // --- Atualização da Câmera ---
        // A câmera só precisa ser atualizada se estivermos em um estado de jogo que a utiliza (ex: JOGANDO)
        // E se houver jogadores para focar.
        // A função UpdateCameraCenteredOnPlayers será chamada de dentro de UpdatePlayingScreen.
        // Se outros estados precisarem da câmera, eles também devem atualizá-la.

        switch (currentScreen) {
            case GAMESTATE_MENU:
                UpdateMenuScreen(&currentScreen, virtualMousePosition);
                break;
            case GAMESTATE_INTRO:
                UpdateIntroScreen(&currentScreen, &introScreenFramesCounter);
                break;
            case GAMESTATE_PLAYER_MODE_MENU:
                UpdatePlayerModeMenuScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying, virtualMousePosition);
                break;
            case GAMESTATE_CHARACTER_CREATION:
                UpdateCharacterCreationScreen(&currentScreen, players, &g_currentMapX, &g_currentMapY, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying);
                break;
            case GAMESTATE_PLAYING:
                // UpdatePlayingScreen agora também será responsável por chamar a atualização da câmera.
                UpdatePlayingScreen(&currentScreen, players, gamePlaylist, &currentPlaylistIndex, &musicVolume,
                                    &musicIsPlaying, &musicPlayTimer, &currentTrackDuration,
                                    &g_currentMapX, &g_currentMapY, &gameCamera); // Passa a câmera
                break;
            case GAMESTATE_PAUSE:
                // A tela de Pausa pode precisar saber da câmera se o fundo do jogo (afetado pela câmera)
                // ainda está sendo atualizado ou se a câmera deve permanecer estática.
                // Por enquanto, não atualizamos a câmera aqui; ela manterá o último estado de GAMESTATE_PLAYING.
                UpdatePauseScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, musicIsPlaying, &musicIsPlaying, virtualMousePosition);
                break;
            case GAMESTATE_INVENTORY:
                UpdateInventoryScreen(&currentScreen, players, &musicIsPlaying, gamePlaylist, &currentPlaylistIndex);
                break;
            case GAMESTATE_SAVE_LOAD_MENU:
                UpdateSaveLoadMenuScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying, &g_currentMapX, &g_currentMapY, virtualMousePosition);
                break;
            default:
                break;
        }

        BeginTextureMode(targetRenderTexture);
            ClearBackground(RAYWHITE); // Limpa a textura de renderização

            // Aplica a transformação da câmera APENAS para os estados de jogo que renderizam o mundo do jogo.
            // Menus e outras telas de UI geralmente não são afetadas pela câmera do mundo.
            if (currentScreen == GAMESTATE_PLAYING || currentScreen == GAMESTATE_PAUSE || currentScreen == GAMESTATE_INVENTORY) {
                BeginMode2D(gameCamera); // Inicia o modo de desenho com a câmera 2D
            }

            // Desenha a tela atual
            switch (currentScreen) {
                case GAMESTATE_MENU: DrawMenuScreen(); break;
                case GAMESTATE_INTRO: DrawIntroScreen(); break;
                case GAMESTATE_PLAYER_MODE_MENU: DrawPlayerModeMenuScreen(); break;
                case GAMESTATE_CHARACTER_CREATION: DrawCharacterCreationScreen(players, 0); break;
                case GAMESTATE_PLAYING:
                    DrawPlayingScreen(players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY);
                    break;
                case GAMESTATE_PAUSE:
                    // DrawPauseScreen desenha a tela de jogo (que será afetada pela câmera se BeginMode2D estiver ativo)
                    // e depois sua UI de pausa. Se a UI de pausa não deve ser afetada pela câmera,
                    // DrawPauseScreen precisaria ser refatorada para desenhar a UI após EndMode2D.
                    // Por enquanto, toda a DrawPauseScreen será afetada se BeginMode2D estiver ativo.
                    DrawPauseScreen(players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY);
                    break;
                case GAMESTATE_INVENTORY:
                    DrawInventoryScreen(players, players, musicVolume, currentPlaylistIndex, musicIsPlaying, g_currentMapX, g_currentMapY);
                    break;
                case GAMESTATE_SAVE_LOAD_MENU:
                    // A tela de Save/Load geralmente é uma sobreposição e não usa a câmera do mundo.
                    // Se ela desenhar a tela de jogo por baixo (como a de Pausa faz), essa parte precisaria de atenção especial.
                    // Por simplicidade, assumimos que ela é desenhada sem a câmera do mundo aplicada aqui.
                    DrawSaveLoadMenuScreen(players, gamePlaylist, currentPlaylistIndex, musicIsPlaying, musicVolume, g_currentMapX, g_currentMapY);
                    break;
                default:
                    DrawText("ESTADO DESCONHECIDO!", virtualScreenWidth / 2 - MeasureText("ESTADO DESCONHECIDO!", 20) / 2, virtualScreenHeight / 2 - 10, 20, RED);
                    break;
            }

            if (currentScreen == GAMESTATE_PLAYING || currentScreen == GAMESTATE_PAUSE || currentScreen == GAMESTATE_INVENTORY) {
                EndMode2D(); // Finaliza o modo de desenho com a câmera 2D
            }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            float scale = fminf((float)GetScreenWidth() / virtualScreenWidth, (float)GetScreenHeight() / virtualScreenHeight);
            float destWidth = virtualScreenWidth * scale;
            float destHeight = virtualScreenHeight * scale;
            float destX = (GetScreenWidth() - destWidth) * 0.5f;
            float destY = (GetScreenHeight() - destHeight) * 0.5f;
            DrawTexturePro(targetRenderTexture.texture,
                           (Rectangle){ 0.0f, 0.0f, (float)targetRenderTexture.texture.width, (float)-targetRenderTexture.texture.height },
                           (Rectangle){ destX, destY, destWidth, destHeight },
                           (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(targetRenderTexture);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].txr.id > 0) { UnloadTexture(players[i].txr); }
    }
    for (int i = 0; i < MAX_SIZE; i++) {
        if (gamePlaylist[i].stream.buffer != NULL) { UnloadMusicStream(gamePlaylist[i]); }
    }
    CloseAudioDevice();
    CloseWindow();
    return 0;
}