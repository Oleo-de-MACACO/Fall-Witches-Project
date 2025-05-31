#include <stddef.h>     // Para NULL
// #include <unistd.h>  // Removido se não estritamente necessário, geralmente stddef.h é suficiente para NULL
#include "raylib.h"
#include "../include/Game.h"    // Para GameState e protótipos de Game.c
#include "../include/Classes.h" // Para a struct Player

// --- Constantes e Variáveis Globais de main.c (ou no escopo de main) ---
const int SCREEN_WIDTH = 800;   //
const int SCREEN_HEIGHT = 450;  //

// Dados do Jogo
Player players[MAX_PLAYERS];        //
Texture2D player1Texture;
Music gamePlaylist[MAX_SIZE];       //

GameState currentScreen;
int introScreenFramesCounter = 0;   // Contador de frames para a tela de intro

// Estado da Música
int currentPlaylistIndex = 0;       //
int musicIsPlaying = 1;             //
float musicVolume = 0.5f;           //
float musicPlayTimer = 0.0f;        //
float currentTrackDuration = 0.0f;  //


int main(void) { //
    // Inicialização
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_BORDERLESS_WINDOWED_MODE | FLAG_WINDOW_RESIZABLE); //
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fall Witches - Nome temp"); //
    InitAudioDevice(); //

    // Carrega recursos e inicializa dados do jogo (função de Game.c)
    InitGameResources(players, &player1Texture, gamePlaylist);

    currentScreen = GAMESTATE_INTRO; // Define a cena inicial

    // Inicia a primeira música, se houver
    if (gamePlaylist[currentPlaylistIndex].frameCount > 0 && gamePlaylist[currentPlaylistIndex].stream.buffer != NULL) { // Checa se a música é válida
        PlayMusicStream(gamePlaylist[currentPlaylistIndex]); //
        SetMusicVolume(gamePlaylist[currentPlaylistIndex], musicVolume); //
        musicIsPlaying = 1; //
        currentTrackDuration = GetMusicTimeLength(gamePlaylist[currentPlaylistIndex]); //
        musicPlayTimer = 0.0f; //
    } else {
        musicIsPlaying = 0; //
    }

    SetTargetFPS(60); //

    // --- Laço Principal do Jogo ---
    while (!WindowShouldClose()) { //
        // --- Lógica de Atualização (Update) ---
        switch (currentScreen) {
            case GAMESTATE_MENU:
                UpdateMenuScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, musicVolume, &musicIsPlaying);
                break;
            case GAMESTATE_INTRO:
                UpdateIntroScreen(&currentScreen, &introScreenFramesCounter);
                break;
            case GAMESTATE_PLAYING:
                UpdatePlayingScreen(&currentScreen, players, player1Texture, gamePlaylist, &currentPlaylistIndex, &musicVolume, &musicIsPlaying, &musicPlayTimer, &currentTrackDuration);
                break;
            case GAMESTATE_PAUSE:
                UpdatePauseScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, musicIsPlaying);
                break;
            default:
                break;
        }

        // --- Lógica de Desenho (Draw) ---
        BeginDrawing(); //
            ClearBackground(RAYWHITE); // Limpa a tela com uma cor base (pode ser sobrescrito pela cena)

            switch (currentScreen) {
                case GAMESTATE_MENU:
                    DrawMenuScreen();
                    break;
                case GAMESTATE_INTRO:
                    DrawIntroScreen();
                    break;
                case GAMESTATE_PLAYING:
                    DrawPlayingScreen(players, player1Texture, musicVolume, currentPlaylistIndex, musicIsPlaying);
                    break;
                case GAMESTATE_PAUSE:
                    DrawPauseScreen(players, player1Texture, musicVolume, currentPlaylistIndex, musicIsPlaying);
                    break;
                default:
                    DrawText("ESTADO DESCONHECIDO!", 20, 20, 20, RED);
                    break;
            }
        EndDrawing(); //
    }

    // --- Desinicialização ---
    if (player1Texture.id > 0) { UnloadTexture(player1Texture); } //
    // Se player[1].txr for uma textura separada, descarregue-a também.
    // No InitGameResources, player[1].txr pode receber player1Texture, então descarregar player1Texture já basta.

    for (int i = 0; i < MAX_SIZE; i++) { //
        if (gamePlaylist[i].stream.buffer != NULL) { UnloadMusicStream(gamePlaylist[i]); } //
    }

    CloseAudioDevice(); //
    CloseWindow();      //
    return 0; //
}