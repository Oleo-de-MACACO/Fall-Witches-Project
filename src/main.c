#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// --- Inclusão de Todos os Headers de Módulos do Jogo ---
// Incluir os headers aqui dá a main.c acesso às funções de cada módulo.
#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Menu.h"
#include "../include/Inventory.h"
#include "../include/CharacterCreation.h"
#include "../include/PauseMenu.h"
#include "../include/LoadSaveUI.h"
#include "../include/Settings.h"
#include "../include/WalkCycle.h"
#include "../include/Sound.h"
#include "../include/WorldLoading.h"
#include "../include/Event.h"
#include "../include/Dialogue.h"
#include "../include/GameProgress.h"

// --- Constantes Globais ---
// Estas constantes definem propriedades fundamentais do jogo.
const int virtualScreenWidth = 800;
const int virtualScreenHeight = 450;
const int WORLD_MAP_MIN_X = -10;
const int WORLD_MAP_MAX_X = 10;
const int WORLD_MAP_MIN_Y = -10;
const int WORLD_MAP_MAX_Y = 10;
const int gameSectionWidthMultiplier = 3;   // Usado como fallback ou para lógica legada
const int gameSectionHeightMultiplier = 3;  // Usado como fallback ou para lógica legada

// --- Variáveis Globais de Estado do Jogo ---
Player players[MAX_PLAYERS_SUPPORTED];          // Array para os dados dos jogadores
GameState currentScreen;                        // A tela/estado atual do jogo
GameModeType currentGameMode = GAME_MODE_UNINITIALIZED; // Modo de jogo (1P ou 2P)
int currentActivePlayers = 1;                   // Número de jogadores ativos
int g_currentMapX = 0;                          // Coordenada X global do mapa
int g_currentMapY = 0;                          // Coordenada Y global do mapa
bool g_request_exit = false;                    // Sinaliza o fechamento do jogo a partir de um menu

// --- Variáveis Globais de Volume ---
// Estas são controladas pela tela de Configurações e usadas pelo módulo de Som.
float masterVolume = 1.0f;
float mainMenuMusicVolume = 0.7f;
float gameplayMusicVolume = 0.5f;
float battleMusicVolume = 0.5f;
float cutsceneMusicVolume = 0.6f;
float ambientNatureVolume = 0.4f;
float ambientCityVolume = 0.4f;
float ambientCaveVolume = 0.45f;
float sfxVolume = 0.5f;

// --- Variáveis Legadas de Música ---
// Mantidas para compatibilidade com funções mais antigas, se necessário.
Music gamePlaylist[MAX_MUSIC_PLAYLIST_SIZE];
bool isMusicPlaying = true;
int currentPlaylistIndex = 0;
float musicPlayTimer = 0.0f;
float currentTrackDuration = 0.0f;

// --- Variáveis Globais de Renderização e Câmera ---
RenderTexture2D targetRenderTexture;
Camera2D gameCamera = {0};

// Ponteiro para a seção do mundo atualmente carregada e ativa
WorldSection* currentActiveWorldSection = NULL;

/**
 * @brief Calcula a posição do mouse na tela virtual.
 * @param actualMousePos Posição real do mouse na janela do sistema.
 * @return Vector2 Posição do mouse na tela virtual do jogo.
 */
Vector2 GetVirtualMousePosition(Vector2 actualMousePos) {
    Vector2 virtualMouse = { 0.0f, 0.0f };
    float scale = fminf((float)GetScreenWidth() / (float)virtualScreenWidth, (float)GetScreenHeight() / (float)virtualScreenHeight);
    float letterboxX = ((float)GetScreenWidth() - ((float)virtualScreenWidth * scale)) * 0.5f;
    float letterboxY = ((float)GetScreenHeight() - ((float)virtualScreenHeight * scale)) * 0.5f;
    virtualMouse.x = (actualMousePos.x - letterboxX) / scale;
    virtualMouse.y = (actualMousePos.y - letterboxY) / scale;
    return virtualMouse;
}

// --- Função Principal (main) ---
int main(void) {
    // --- Inicialização da Janela e Dispositivos ---
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_RESIZABLE);
    InitWindow(virtualScreenWidth, virtualScreenHeight, "Fall Witches");
    SetExitKey(KEY_NULL); // Desabilita ESC para fechar; g_request_exit fará isso
    InitAudioDevice();

    // --- Carregamento de Todos os Recursos do Jogo ---
    TraceLog(LOG_INFO, "Carregando todos os recursos do jogo...");
    InitGameResources(players, gamePlaylist);
    Progress_Reset();                           // Limpa o progresso para uma nova sessão
    LoadGameAudio("assets/audio");              // Carrega todos os sons e músicas
    Event_LoadAll("assets/Events/Events.txt");      // Carrega eventos
    Dialogue_LoadAll("assets/Dialogues/dialogues.txt");// Carrega diálogos
    
    UpdateCurrentlyPlayingMusicVolume(); // Aplica volumes iniciais
    ApplySfxVolume();

    // Cria a textura de renderização para a resolução virtual
    targetRenderTexture = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    SetTextureFilter(targetRenderTexture.texture, TEXTURE_FILTER_BILINEAR);

    // --- Configuração Inicial do Jogo ---
    gameCamera = (Camera2D){
        .offset = { (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f },
        .target = { (float)virtualScreenWidth / 2.0f, (float)virtualScreenHeight / 2.0f },
        .rotation = 0.0f, .zoom = 1.0f
    };
    currentScreen = GAMESTATE_INTRO; // Define a tela inicial

    if (GetMusicTrackCount(MUSIC_CATEGORY_MAINMENU) > 0) {
        PlayMusicTrack(MUSIC_CATEGORY_MAINMENU, 0, true);
        isMusicPlaying = true;
    }
    SetTargetFPS(60);

    // --- Loop Principal do Jogo ---
    while (!WindowShouldClose() && !g_request_exit) {
        if(IsAudioDeviceReady()) UpdateAudioStreams(); // Essencial para músicas em streaming

        Vector2 virtualMousePosition = GetVirtualMousePosition(GetMousePosition());
        int previousMapX = g_currentMapX;
        int previousMapY = g_currentMapY;

        // --- Lógica de Atualização ---
        if (Dialogue_IsActive()) {
            Dialogue_Update(); // Se um diálogo está ativo, ele tem prioridade e pausa o resto
        } else {
            // Se não há diálogo, atualiza o estado de jogo normal
            switch (currentScreen) {
                case GAMESTATE_INTRO: UpdateIntroScreen(&currentScreen, &introScreenFramesCounter); break;
                case GAMESTATE_MENU: UpdateMenuScreen(&currentScreen, virtualMousePosition); break;
                case GAMESTATE_PLAYER_MODE_MENU: UpdatePlayerModeMenuScreen(&currentScreen, gamePlaylist, currentPlaylistIndex, gameplayMusicVolume * masterVolume, &isMusicPlaying, virtualMousePosition); break;
                case GAMESTATE_CHARACTER_CREATION: UpdateCharacterCreationScreen(&currentScreen, players, &g_currentMapX, &g_currentMapY, gamePlaylist, currentPlaylistIndex, gameplayMusicVolume * masterVolume, &isMusicPlaying); break;
                case GAMESTATE_SAVE_LOAD_MENU: UpdateSaveLoadMenuScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, gameplayMusicVolume * masterVolume, &isMusicPlaying, &g_currentMapX, &g_currentMapY, virtualMousePosition, &currentActiveWorldSection); break;
                case GAMESTATE_SETTINGS: UpdateSettingsScreen(&currentScreen); break;
                case GAMESTATE_PAUSE: UpdatePauseScreen(&currentScreen, players, gamePlaylist, currentPlaylistIndex, isMusicPlaying, &isMusicPlaying, virtualMousePosition); break;
                case GAMESTATE_INVENTORY: UpdateInventoryScreen(&currentScreen, players, &isMusicPlaying, gamePlaylist, &currentPlaylistIndex); break;
                case GAMESTATE_PLAYING:
                    UpdatePlayingScreen(&currentScreen, players, currentActivePlayers, gamePlaylist, &currentPlaylistIndex, &gameplayMusicVolume, &isMusicPlaying, &musicPlayTimer, &currentTrackDuration, &g_currentMapX, &g_currentMapY, &gameCamera, currentActiveWorldSection);
                    Event_CheckAndRun(g_currentMapX, g_currentMapY);
                    break;
                default: break;
            }
        }
        
        // Se uma transição de mapa ocorreu, recarrega a seção do mundo e seus personagens
        if (g_currentMapX != previousMapX || g_currentMapY != previousMapY) {
            TraceLog(LOG_INFO, "Mapa mudou. Recarregando secao e personagens para (%d,%d).", g_currentMapX, g_currentMapY);
            if (currentActiveWorldSection) { UnloadWorldSection(currentActiveWorldSection); }
            Characters_UnloadCurrentMap();
            currentActiveWorldSection = LoadWorldSection(g_currentMapX, g_currentMapY);
            Characters_LoadForMap(g_currentMapX, g_currentMapY);
            if (!currentActiveWorldSection) { /* Tratar erro de carregamento */ }
        }

        // --- Lógica de Desenho ---
        BeginDrawing();
            ClearBackground(BLACK);
            BeginTextureMode(targetRenderTexture);
                ClearBackground(DARKGRAY);
                // Desenha o estado de jogo atual
                if (currentScreen == GAMESTATE_PLAYING || currentScreen == GAMESTATE_PAUSE || currentScreen == GAMESTATE_INVENTORY) {
                    BeginMode2D(gameCamera);
                        if (currentActiveWorldSection) DrawWorldSectionBackground(currentActiveWorldSection);
                        else { ClearBackground(PURPLE); DrawText("ERRO: SECAO NAO CARREGADA", 10,10,20,WHITE); }
                        Characters_Draw(); // Desenha NPCs e Inimigos
                        DrawPlayingScreen(players, currentActivePlayers, gameplayMusicVolume * masterVolume, currentPlaylistIndex, isMusicPlaying, g_currentMapX, g_currentMapY);
                        DrawWorldSectionDebug(currentActiveWorldSection); // Para depuração
                    EndMode2D();
                    if(currentScreen == GAMESTATE_PAUSE) DrawPauseMenuElements();
                    if(currentScreen == GAMESTATE_INVENTORY) DrawInventoryUIElements(players);
                } else { // Desenha telas que não usam a câmera do jogo
                    switch (currentScreen) {
                        case GAMESTATE_MENU: DrawMenuScreen(); break;
                        case GAMESTATE_INTRO: DrawIntroScreen(); break;
                        case GAMESTATE_PLAYER_MODE_MENU: DrawPlayerModeMenuScreen(); break;
                        case GAMESTATE_CHARACTER_CREATION: DrawCharacterCreationScreen(players); break;
                        case GAMESTATE_SAVE_LOAD_MENU: DrawSaveLoadMenuScreen(players, gamePlaylist, currentPlaylistIndex, isMusicPlaying, gameplayMusicVolume * masterVolume, g_currentMapX, g_currentMapY); break;
                        case GAMESTATE_SETTINGS: DrawSettingsScreen(); break;
                        default: DrawText("ESTADO DESCONHECIDO", 190, 200, 20, LIGHTGRAY); break;
                    }
                }
            EndTextureMode();
            float scale = fminf((float)GetScreenWidth()/(float)virtualScreenWidth, (float)GetScreenHeight()/(float)virtualScreenHeight);
            Rectangle destRect = { ((float)GetScreenWidth() - ((float)virtualScreenWidth * scale)) * 0.5f, ((float)GetScreenHeight() - ((float)virtualScreenHeight * scale)) * 0.5f, (float)virtualScreenWidth * scale, (float)virtualScreenHeight * scale };
            DrawTexturePro(targetRenderTexture.texture, (Rectangle){0,0,(float)targetRenderTexture.texture.width, (float)-targetRenderTexture.texture.height}, destRect, (Vector2){0,0}, 0.0f, WHITE);

            // Desenha a UI do diálogo POR CIMA de tudo, em coordenadas de tela real
            Dialogue_Draw();
            DrawFPS(10, 10);
        EndDrawing();
    }

    // --- Descarregamento de Recursos ---
    TraceLog(LOG_INFO, "Descarregando todos os recursos...");
    if (currentActiveWorldSection) { UnloadWorldSection(currentActiveWorldSection); }
    Characters_UnloadCurrentMap();
    Dialogue_UnloadAll();
    Event_UnloadAll();
    UnloadGameAudio();
    UnloadRenderTexture(targetRenderTexture);
    for (int i=0; i<MAX_PLAYERS_SUPPORTED; i++) { UnloadCharacterAnimations(&players[i]); }
    if(IsAudioDeviceReady()) CloseAudioDevice();
    CloseWindow();
    return 0;
}