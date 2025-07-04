#include "../include/Menu.h"
#include "../include/Game.h"
#include "../include/Settings.h"
#include "../include/Sound.h" // Para funções de som se o menu as controlar diretamente
#include <stdio.h>
#include <string.h>
#include "raylib.h"

extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern bool g_request_exit; // Definido em main.c
extern GameModeType currentGameMode;
extern int currentActivePlayers;

#define NUM_MAIN_MENU_BUTTONS 5
static MenuButton mainMenuButtons[NUM_MAIN_MENU_BUTTONS];
static bool mainMenuButtonsInitialized = false;

#define NUM_PLAYER_MODE_MENU_BUTTONS 2
static MenuButton playerModeMenuButtons[NUM_PLAYER_MODE_MENU_BUTTONS];
static bool playerModeMenuButtonsInitialized = false;

static bool s_is_in_save_mode = true;
static bool s_is_new_game_flow = false;
static int s_save_load_menu_sub_state = 0;
static int s_selected_slot_for_action = -1;
static GameState s_previous_screen_before_save_load;
static GameModeType s_save_load_context_mode = GAME_MODE_TWO_PLAYER; // Ou GAME_MODE_UNINITIALIZED


#define COLOR_BUTTON_ACTIVE MAROON
#define COLOR_BUTTON_HOVER ORANGE
#define COLOR_BUTTON_DISABLED GRAY
#define COLOR_BUTTON_TEXT WHITE

void InitializeMainMenuButtons() {
    float buttonWidth = 280.0f; float buttonHeight = 50.0f; float spacingY = 20.0f; float spacingX = 20.0f;
    float twoButtonRowWidth = 2.0f * buttonWidth + spacingX;
    // float totalButtonBlockHeight = 3.0f * buttonHeight + 2.0f * spacingY; // Não usado
    float titleHeightEstimate = (float)virtualScreenHeight / 7.0f + 70.0f;
    float startY = titleHeightEstimate + 40.0f;
    // if (startY + totalButtonBlockHeight > (float)virtualScreenHeight - 30.0f) { startY = ((float)virtualScreenHeight - totalButtonBlockHeight) / 2.0f; if (startY < titleHeightEstimate) startY = titleHeightEstimate; }
    // if (startY < (float)virtualScreenHeight * 0.3f) startY = (float)virtualScreenHeight * 0.3f; // Simplificado
    float startX_twoButtons = ((float)virtualScreenWidth - twoButtonRowWidth) / 2.0f;
    mainMenuButtons[0] = (MenuButton){{startX_twoButtons, startY, buttonWidth, buttonHeight}, "Um Jogador", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_GOTO_SINGLE_PLAYER_SETUP };
    mainMenuButtons[1] = (MenuButton){{startX_twoButtons + buttonWidth + spacingX, startY, buttonWidth, buttonHeight}, "Dois Jogadores", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_GOTO_TWO_PLAYER_SETUP };
    float currentY_main = startY + buttonHeight + spacingY;
    mainMenuButtons[2] = (MenuButton){{((float)virtualScreenWidth - buttonWidth) / 2.0f, currentY_main, buttonWidth, buttonHeight}, "Multijogador (em breve)", COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, LIGHTGRAY, false, false, BUTTON_ACTION_NONE };
    currentY_main += buttonHeight + spacingY;
    mainMenuButtons[3] = (MenuButton){{startX_twoButtons, currentY_main, buttonWidth, buttonHeight}, "Opcoes", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_SETTINGS };
    mainMenuButtons[4] = (MenuButton){{startX_twoButtons + buttonWidth + spacingX, currentY_main, buttonWidth, buttonHeight}, "Sair do Jogo", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_QUIT_GAME };
    mainMenuButtonsInitialized = true;
}

void UpdateMenuScreen(GameState *currentScreen_ptr, Vector2 virtualMousePos) { /* ... (código usando g_request_exit) ... */
    if (!currentScreen_ptr) return;
    if (!mainMenuButtonsInitialized) { InitializeMainMenuButtons(); }
    for (int i = 0; i < NUM_MAIN_MENU_BUTTONS; i++) {
        mainMenuButtons[i].is_hovered = false;
        if (mainMenuButtons[i].is_active && CheckCollisionPointRec(virtualMousePos, mainMenuButtons[i].rect)) {
            mainMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                switch (mainMenuButtons[i].action) {
                    case BUTTON_ACTION_GOTO_SINGLE_PLAYER_SETUP:
                        currentGameMode = GAME_MODE_SINGLE_PLAYER; currentActivePlayers = 1;
                        *currentScreen_ptr = GAMESTATE_PLAYER_MODE_MENU;
                        playerModeMenuButtonsInitialized = false; Menu_SetIsNewGameFlow(false);
                        break;
                    case BUTTON_ACTION_GOTO_TWO_PLAYER_SETUP:
                        currentGameMode = GAME_MODE_TWO_PLAYER; currentActivePlayers = MAX_PLAYERS_SUPPORTED;
                        *currentScreen_ptr = GAMESTATE_PLAYER_MODE_MENU;
                        playerModeMenuButtonsInitialized = false; Menu_SetIsNewGameFlow(false);
                        break;
                    case BUTTON_ACTION_SETTINGS:
                        InitializeSettingsScreen(GAMESTATE_MENU);
                        *currentScreen_ptr = GAMESTATE_SETTINGS;
                        break;
                    case BUTTON_ACTION_QUIT_GAME: g_request_exit = true; break; // Usa g_request_exit
                    default: break;
                }
            }
        }
    }
}
void DrawMenuScreen(void) { 
    if (!mainMenuButtonsInitialized) { InitializeMainMenuButtons(); }
    ClearBackground(DARKGRAY);
    DrawText("FALL WITCHES", (int)(((float)virtualScreenWidth - (float)MeasureText("FALL WITCHES", 70)) / 2.0f), (int)((float)virtualScreenHeight / 7.0f), 70, WHITE);
    for (int i = 0; i < NUM_MAIN_MENU_BUTTONS; i++) {
        Color btnC = !mainMenuButtons[i].is_active ? mainMenuButtons[i].disabled_color : (mainMenuButtons[i].is_hovered ? mainMenuButtons[i].hover_color : mainMenuButtons[i].base_color);
        Color txtC = !mainMenuButtons[i].is_active ? DARKGRAY : mainMenuButtons[i].text_color;
        DrawRectangleRec(mainMenuButtons[i].rect, btnC); DrawRectangleLinesEx(mainMenuButtons[i].rect, 2, Fade(BLACK,0.4f));
        int tw = MeasureText(mainMenuButtons[i].text,20);
        DrawText(mainMenuButtons[i].text, (int)(mainMenuButtons[i].rect.x+(mainMenuButtons[i].rect.width-(float)tw)/2.0f), (int)(mainMenuButtons[i].rect.y+(mainMenuButtons[i].rect.height-20.0f)/2.0f), 20, txtC);
    }
    DrawText("Use o mouse para selecionar.", 10, virtualScreenHeight-20, 10, LIGHTGRAY);
}

void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr) { 
    if(introFrames_ptr && currentScreen_ptr){
        (*introFrames_ptr)++;
        if (*introFrames_ptr > 180 || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON) ) {
            *currentScreen_ptr = GAMESTATE_MENU;
            *introFrames_ptr = 0;
            mainMenuButtonsInitialized = false; // Re-inicializa botões do menu na próxima vez que entrar
        }
    }
}
void DrawIntroScreen(void) { 
    ClearBackground(BLACK); 
    DrawText("TELA DE INTRODUCAO", (int)(((float)virtualScreenWidth-(float)MeasureText("TELA DE INTRODUCAO",30))/2.0f), (int)((float)virtualScreenHeight/2.0f-40.0f), 30,WHITE);
    DrawText("Fall Witches Engine v0.1",(int)(((float)virtualScreenWidth-(float)MeasureText("Fall Witches Engine v0.1",20))/2.0f),(int)((float)virtualScreenHeight/2.0f+10.0f),20,LIGHTGRAY); // Versão de exemplo
    DrawText("Pressione ENTER, ESC ou clique para continuar...",(int)(((float)virtualScreenWidth-(float)MeasureText("Pressione ENTER, ESC ou clique para continuar...",10))/2.0f),virtualScreenHeight-30,10,GRAY);
}

void InitializePlayerModeMenuButtons() { 
    float btnW=250.0f, btnH=50.0f, spY=30.0f; float totalH=2.0f*btnH+spY;
    float sY=((float)virtualScreenHeight-totalH)/2.0f; float sX=((float)virtualScreenWidth-btnW)/2.0f;
    playerModeMenuButtons[0]=(MenuButton){{sX,sY,btnW,btnH},"Novo Jogo",MAROON,COLOR_BUTTON_HOVER,COLOR_BUTTON_DISABLED,COLOR_BUTTON_TEXT,true,false,BUTTON_ACTION_NEW_GAME_SETUP_SLOT};
    playerModeMenuButtons[1]=(MenuButton){{sX,sY+btnH+spY,btnW,btnH},"Carregar Jogo",MAROON,COLOR_BUTTON_HOVER,COLOR_BUTTON_DISABLED,COLOR_BUTTON_TEXT,true,false,BUTTON_ACTION_LOAD_GAME};
    playerModeMenuButtonsInitialized=true;
}

// Parâmetro isPlaying_ptr corrigido para bool*
void UpdatePlayerModeMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, bool *isPlaying_ptr, Vector2 virtualMousePos) {
    (void)playlist; (void)currentMusicIndex; (void)currentVolume; // Silencia warnings se não usados diretamente

    if (!playerModeMenuButtonsInitialized) InitializePlayerModeMenuButtons();
    for (int i = 0; i < NUM_PLAYER_MODE_MENU_BUTTONS; i++) {
        playerModeMenuButtons[i].is_hovered = false;
        if (playerModeMenuButtons[i].is_active && CheckCollisionPointRec(virtualMousePos, playerModeMenuButtons[i].rect)) {
            playerModeMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (!currentScreen_ptr) return;
                switch (playerModeMenuButtons[i].action) {
                    case BUTTON_ACTION_NEW_GAME_SETUP_SLOT:
                        Menu_RequestSaveLoadScreen(currentScreen_ptr, true, GAMESTATE_PLAYER_MODE_MENU);
                        Menu_SetIsNewGameFlow(true);
                        break;
                    case BUTTON_ACTION_LOAD_GAME:
                        Menu_RequestSaveLoadScreen(currentScreen_ptr, false, GAMESTATE_PLAYER_MODE_MENU);
                        break;
                    default: break;
                }
            }
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (currentScreen_ptr) Menu_RequestMainMenu(currentScreen_ptr, playlist, currentMusicIndex, isPlaying_ptr); // Passa bool*
    }
}
void DrawPlayerModeMenuScreen(void) {
    if (!playerModeMenuButtonsInitialized) { InitializePlayerModeMenuButtons(); }
    ClearBackground(DARKGRAY);
    const char* titleText = (currentGameMode == GAME_MODE_SINGLE_PLAYER) ? "UM JOGADOR" : "DOIS JOGADORES";
    DrawText(titleText, (int)(((float)virtualScreenWidth - (float)MeasureText(titleText, 40)) / 2.0f), (int)((float)virtualScreenHeight / 4.0f), 40, WHITE);
    for (int i = 0; i < NUM_PLAYER_MODE_MENU_BUTTONS; i++) {
        Color btnC = !playerModeMenuButtons[i].is_active ? playerModeMenuButtons[i].disabled_color : (playerModeMenuButtons[i].is_hovered ? playerModeMenuButtons[i].hover_color : playerModeMenuButtons[i].base_color);
        Color txtC = !playerModeMenuButtons[i].is_active ? DARKGRAY : playerModeMenuButtons[i].text_color;
        DrawRectangleRec(playerModeMenuButtons[i].rect, btnC); DrawRectangleLinesEx(playerModeMenuButtons[i].rect, 2, Fade(BLACK, 0.4f));
        int tw = MeasureText(playerModeMenuButtons[i].text, 20);
        DrawText(playerModeMenuButtons[i].text, (int)(playerModeMenuButtons[i].rect.x + (playerModeMenuButtons[i].rect.width - (float)tw) / 2.0f), (int)(playerModeMenuButtons[i].rect.y + (playerModeMenuButtons[i].rect.height - 20.0f) / 2.0f), 20, txtC);
    }
    DrawText("ESC para Voltar ao Menu Principal", 10, virtualScreenHeight - 20, 10, LIGHTGRAY);
}

void Menu_RequestSaveLoadScreen(GameState *currentScreen_ptr, bool isSaving, GameState fromScreen) {
    s_is_in_save_mode = isSaving;
    s_save_load_context_mode = currentGameMode; 
    if (fromScreen == GAMESTATE_PLAYER_MODE_MENU && isSaving) { s_is_new_game_flow = true;}
    else {s_is_new_game_flow = false;}
    s_save_load_menu_sub_state = 0;
    s_previous_screen_before_save_load = fromScreen;
    if (currentScreen_ptr) *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU;
}

// Parâmetro musicIsCurrentlyPlaying_ptr corrigido para bool*
void Menu_RequestMainMenu(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, bool *musicIsCurrentlyPlaying_ptr) {
    if(currentScreen_ptr) *currentScreen_ptr = GAMESTATE_MENU;

    // Usa Sound.c para parar a música do menu se estiver tocando
    StopCurrentMusic(); // Para qualquer música gerenciada por Sound.c
    if (musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = false; // Atualiza flag externa

    // Lógica legada (pode ser removida se Sound.c for exclusivo)
    if (playlist && musicIsCurrentlyPlaying_ptr && *musicIsCurrentlyPlaying_ptr && // Usa bool*
        currentMusicIndex >= 0 && currentMusicIndex < MAX_MUSIC_PLAYLIST_SIZE && // MAX_MUSIC_PLAYLIST_SIZE de Game.h
        playlist[currentMusicIndex].stream.buffer != NULL) {
        // StopMusicStream(playlist[currentMusicIndex]); // Sound.c deve ter feito isso
        // *musicIsCurrentlyPlaying_ptr = false; // Já feito acima
    }
    // Toca a música do menu principal via Sound.c
    if(GetMusicTrackCount(MUSIC_CATEGORY_MAINMENU) > 0){
        PlayMusicTrack(MUSIC_CATEGORY_MAINMENU, 0, true);
        if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = true;
    }


    mainMenuButtonsInitialized = false;
    s_is_new_game_flow = false;
}
// Getters e Setters
bool Menu_IsInSaveMode(void) { return s_is_in_save_mode; }
bool Menu_IsNewGameFlow(void) { return s_is_new_game_flow; }
int Menu_GetSaveLoadSubState(void) { return s_save_load_menu_sub_state; }
int Menu_GetSelectedSlotForAction(void) { return s_selected_slot_for_action; }
GameState Menu_GetPreviousScreenBeforeSaveLoad(void) { return s_previous_screen_before_save_load; }
GameModeType Menu_GetSaveLoadContextMode(void) { return s_save_load_context_mode; }
void Menu_SetSaveLoadSubState(int subState) { s_save_load_menu_sub_state = subState; }
void Menu_SetSelectedSlotForAction(int slot) { s_selected_slot_for_action = slot; }
void Menu_SetIsNewGameFlow(bool isNewGameFlow) { s_is_new_game_flow = isNewGameFlow; }