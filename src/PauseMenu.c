#include "../include/PauseMenu.h"
#include "../include/Menu.h"     // For MenuButton, MenuButtonAction, Menu_Request... funcs
#include "../include/Game.h"     // For DrawPlayingScreen, GameState, Player, Music, MAX_MUSIC_PLAYLIST_SIZE
#include <stddef.h>              // For NULL

// Extern global variables (defined in main.c)
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern GameModeType currentGameMode; // Used by Menu_RequestSaveLoadScreen (indirectly via Menu.c)
extern int currentActivePlayers;    // Used when calling DrawPlayingScreen

#define NUM_PAUSE_MENU_BUTTONS 5
static MenuButton pauseMenuButtons[NUM_PAUSE_MENU_BUTTONS];
static bool pauseMenuButtonsInitialized = false;

// Define colors locally or use a common header if shared across more UI files
#define COLOR_BUTTON_ACTIVE MAROON    
#define COLOR_BUTTON_HOVER ORANGE
#define COLOR_BUTTON_DISABLED GRAY
#define COLOR_BUTTON_TEXT WHITE

static void InitializePauseMenuButtons(void) {
    float btnContinueWidth = 300.0f; float btnContinueHeight = 60.0f;
    float btnWidth = 220.0f; float btnHeight = 45.0f;
    float spacingY = 15.0f; float spacingX = 20.0f;
    float titlePauseFontSize = 50.0f;
    float titlePauseY = (float)virtualScreenHeight / 2.0f - 120.0f;
    float startY = titlePauseY + titlePauseFontSize + 20.0f;

    // Order: rect, text, base_color, hover_color, disabled_color, text_color, is_active, is_hovered, action
    pauseMenuButtons[0] = (MenuButton){
        {(float)(virtualScreenWidth - btnContinueWidth) / 2.0f, startY, btnContinueWidth, btnContinueHeight}, 
        "Continuar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, 
        true, false, BUTTON_ACTION_RESUME_GAME
    };
    float currentY = startY + btnContinueHeight + spacingY;
    float twoButtonRowWidthPause = 2.0f * btnWidth + spacingX;
    float startX_twoButtonsPause = ((float)virtualScreenWidth - twoButtonRowWidthPause) / 2.0f;
    pauseMenuButtons[1] = (MenuButton){
        {startX_twoButtonsPause, currentY, btnWidth, btnHeight}, 
        "Salvar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, 
        true, false, BUTTON_ACTION_SAVE_GAME
    };
    pauseMenuButtons[2] = (MenuButton){
        {startX_twoButtonsPause + btnWidth + spacingX, currentY, btnWidth, btnHeight}, 
        "Carregar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, 
        true, false, BUTTON_ACTION_LOAD_GAME
    };
    currentY += btnHeight + spacingY;
    pauseMenuButtons[3] = (MenuButton){
        {startX_twoButtonsPause, currentY, btnWidth, btnHeight}, 
        "Opcoes", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, 
        true, false, BUTTON_ACTION_PAUSE_SETTINGS
    };
    pauseMenuButtons[4] = (MenuButton){
        {startX_twoButtonsPause + btnWidth + spacingX, currentY, btnWidth, btnHeight}, 
        "Sair para Menu", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, 
        true, false, BUTTON_ACTION_GOTO_MAIN_MENU
    };
    pauseMenuButtonsInitialized = true;
}

void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int isPlaying_beforePause, int *musicIsCurrentlyPlaying_ptr, Vector2 virtualMousePos) {
    (void)players; // players array not directly used in this update logic for now

    if (!pauseMenuButtonsInitialized) {
        InitializePauseMenuButtons();
    }

    for (int i = 0; i < NUM_PAUSE_MENU_BUTTONS; i++) {
        pauseMenuButtons[i].is_hovered = false;
        if (pauseMenuButtons[i].is_active && CheckCollisionPointRec(virtualMousePos, pauseMenuButtons[i].rect)) {
            pauseMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                switch (pauseMenuButtons[i].action) {
                    case BUTTON_ACTION_RESUME_GAME:
                        if (currentScreen_ptr) *currentScreen_ptr = GAMESTATE_PLAYING;
                        if (playlist && musicIsCurrentlyPlaying_ptr && isPlaying_beforePause && 
                            currentMusicIndex >= 0 && currentMusicIndex < MAX_MUSIC_PLAYLIST_SIZE && /* Safety check */
                            playlist[currentMusicIndex].stream.buffer != NULL) {
                            ResumeMusicStream(playlist[currentMusicIndex]);
                            *musicIsCurrentlyPlaying_ptr = 1;
                        }
                        break;
                    case BUTTON_ACTION_SAVE_GAME:
                        Menu_RequestSaveLoadScreen(currentScreen_ptr, true, GAMESTATE_PAUSE);
                        break;
                    case BUTTON_ACTION_LOAD_GAME:
                        Menu_RequestSaveLoadScreen(currentScreen_ptr, false, GAMESTATE_PAUSE);
                        break;
                    case BUTTON_ACTION_PAUSE_SETTINGS: 
                        TraceLog(LOG_INFO, "[PauseMenu] Opcoes (Placeholder)"); 
                        break;
                    case BUTTON_ACTION_GOTO_MAIN_MENU: 
                        Menu_RequestMainMenu(currentScreen_ptr, playlist, currentMusicIndex, musicIsCurrentlyPlaying_ptr); 
                        break;
                    default: break;
                }
            }
        }
    }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
        if (currentScreen_ptr) *currentScreen_ptr = GAMESTATE_PLAYING;
        if (playlist && musicIsCurrentlyPlaying_ptr && isPlaying_beforePause && 
            currentMusicIndex >= 0 && currentMusicIndex < MAX_MUSIC_PLAYLIST_SIZE && 
            playlist[currentMusicIndex].stream.buffer != NULL) {
            ResumeMusicStream(playlist[currentMusicIndex]);
            *musicIsCurrentlyPlaying_ptr = 1;
        }
    }
 }

void DrawPauseMenuElements(void) {
    if (!pauseMenuButtonsInitialized) {
        InitializePauseMenuButtons();
    }
    DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.85f));
    DrawText("PAUSADO", (int)(((float)virtualScreenWidth - (float)MeasureText("PAUSADO", 50)) / 2.0f), (int)((float)virtualScreenHeight/2.0f - 120.0f), 50, GRAY);
    for (int i = 0; i < NUM_PAUSE_MENU_BUTTONS; i++) {
        Color btnC = !pauseMenuButtons[i].is_active ? pauseMenuButtons[i].disabled_color : (pauseMenuButtons[i].is_hovered ? pauseMenuButtons[i].hover_color : pauseMenuButtons[i].base_color);
        Color txtC = !pauseMenuButtons[i].is_active ? DARKGRAY : pauseMenuButtons[i].text_color;
        DrawRectangleRec(pauseMenuButtons[i].rect, btnC);
        DrawRectangleLinesEx(pauseMenuButtons[i].rect, 2, Fade(BLACK,0.4f));
        int tw = MeasureText(pauseMenuButtons[i].text,20);
        DrawText(pauseMenuButtons[i].text, (int)(pauseMenuButtons[i].rect.x+(pauseMenuButtons[i].rect.width-(float)tw)/2.0f), (int)(pauseMenuButtons[i].rect.y+(pauseMenuButtons[i].rect.height-20.0f)/2.0f), 20, txtC);
    }
}

void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying_when_game_paused, int currentMapX, int currentMapY) {
    // Desenha o fundo do jogo (mundo do jogo) com o número de jogadores ativos atual.
    DrawPlayingScreen(players_arr, currentActivePlayers, currentVolume, currentMusicIndex, isPlaying_when_game_paused, currentMapX, currentMapY);
    // Desenha os elementos da UI de pausa por cima.
    DrawPauseMenuElements();
}