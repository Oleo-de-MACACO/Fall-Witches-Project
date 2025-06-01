#include "../include/Menu.h"    // Para MenuButton, MenuButtonAction, g_request_exit, etc.
#include "../include/Game.h"    // Para GameState, Player, DrawPlayingScreen, MAX_PLAYERS
#include "../include/Classes.h"   // Para Player
#include "../include/SaveLoad.h"  // Para SaveGame, LoadGame, DoesSaveSlotExist, GetSaveFileName, MAX_SAVE_SLOTS
#include <stdio.h>
#include <math.h>               // Para fmaxf, fminf (se necessário para clamp, ou usar if/else)
#include "raylib.h"

// === Configurações do Menu Principal ===
#define NUM_MAIN_MENU_BUTTONS 5
static MenuButton mainMenuButtons[NUM_MAIN_MENU_BUTTONS];
static bool mainMenuButtonsInitialized = false;

// === Configurações do Menu de Pausa ===
#define NUM_PAUSE_MENU_BUTTONS 5
static MenuButton pauseMenuButtons[NUM_PAUSE_MENU_BUTTONS];
static bool pauseMenuButtonsInitialized = false;

// === Configurações da Tela de Slots de Save/Load ===
// MAX_SAVE_SLOTS é definido em SaveLoad.h
#define SLOTS_PER_ROW_SAVE_LOAD 6 // Usado para calcular numRows, mas os slots são em coluna única
#define SLOT_PADDING_SAVE_LOAD 10
#define SLOT_HEIGHT_SAVE_LOAD 70 // Altura de cada slot de save/load
static MenuButton saveLoadSlotButtons[MAX_SAVE_SLOTS]; 
static bool saveLoadSlotsInitialized = false;

// Variáveis de estado para o scroll da lista de saves
static float saveLoadScrollOffset = 0.0f;
static float saveLoadTotalContentHeight = 0.0f;
static Rectangle saveLoadSlotsViewArea = {0};

// Estado para a tela de save/load e confirmação de sobrescrita
static bool s_is_in_save_mode = true;
static int s_save_load_menu_sub_state = 0;
static int s_selected_slot_for_action = -1;
static GameState s_previous_screen_before_save_load;

static MenuButton s_confirmationButtons[2];
static bool s_confirmationButtonsInitialized = false;


// Cores dos botões
#define COLOR_BUTTON_ACTIVE MAROON
#define COLOR_BUTTON_HOVER ORANGE
#define COLOR_BUTTON_DISABLED GRAY
#define COLOR_BUTTON_TEXT WHITE

// extern bool g_request_exit; // Já declarado em Menu.h

// --- Funções do Menu Principal ---
void InitializeMainMenuButtons(int screenWidth, int screenHeight) {
    float buttonWidth = 280; float buttonHeight = 50; float spacingY = 20; float spacingX = 20;
    float twoButtonRowWidth = 2 * buttonWidth + spacingX; float totalButtonBlockHeight = 3 * buttonHeight + 2 * spacingY;
    float titleHeightEstimate = screenHeight / 7.0f + 70; float startY = titleHeightEstimate + 40;
    if (startY + totalButtonBlockHeight > screenHeight - 30) { startY = (screenHeight - totalButtonBlockHeight) / 2.0f; if (startY < titleHeightEstimate) startY = titleHeightEstimate; }
    if (startY < screenHeight * 0.3f) startY = screenHeight * 0.3f;
    float startX_twoButtons = (screenWidth - twoButtonRowWidth) / 2.0f; float startX_oneButton = (screenWidth - buttonWidth) / 2.0f;
    mainMenuButtons[0] = (MenuButton){{startX_twoButtons, startY, buttonWidth, buttonHeight}, "Single Player (em breve)", COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, LIGHTGRAY, false, false, BUTTON_ACTION_NONE };
    mainMenuButtons[1] = (MenuButton){{startX_twoButtons + buttonWidth + spacingX, startY, buttonWidth, buttonHeight}, "2 Players", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_START_2PLAYERS };
    float currentY_main = startY + buttonHeight + spacingY;
    mainMenuButtons[2] = (MenuButton){{startX_oneButton, currentY_main, buttonWidth, buttonHeight}, "Multiplayer (em breve)", COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, LIGHTGRAY, false, false, BUTTON_ACTION_NONE };
    currentY_main += buttonHeight + spacingY;
    mainMenuButtons[3] = (MenuButton){{startX_twoButtons, currentY_main, buttonWidth, buttonHeight}, "Configurações", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_SETTINGS };
    mainMenuButtons[4] = (MenuButton){{startX_twoButtons + buttonWidth + spacingX, currentY_main, buttonWidth, buttonHeight}, "Sair do Jogo", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_QUIT_GAME };
    mainMenuButtonsInitialized = true;
}
void UpdateMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr) {
    if (!mainMenuButtonsInitialized) InitializeMainMenuButtons(GetScreenWidth(), GetScreenHeight());
    Vector2 mousePoint = GetMousePosition();
    for (int i = 0; i < NUM_MAIN_MENU_BUTTONS; i++) {
        mainMenuButtons[i].is_hovered = false;
        if (mainMenuButtons[i].is_active && CheckCollisionPointRec(mousePoint, mainMenuButtons[i].rect)) {
            mainMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                switch (mainMenuButtons[i].action) {
                    case BUTTON_ACTION_START_2PLAYERS:
                        *currentScreen_ptr = GAMESTATE_PLAYING;
                        if (playlist[currentMusicIndex].frameCount>0 && playlist[currentMusicIndex].stream.buffer!=NULL) {
                            if (!(*isPlaying_ptr)) { PlayMusicStream(playlist[currentMusicIndex]); *isPlaying_ptr = 1; }
                            SetMusicVolume(playlist[currentMusicIndex], currentVolume); }
                        TraceLog(LOG_INFO, "Main Menu: 2 Players"); break;
                    case BUTTON_ACTION_SETTINGS: TraceLog(LOG_INFO, "Main Menu: Settings"); break;
                    case BUTTON_ACTION_QUIT_GAME: g_request_exit = true; TraceLog(LOG_INFO, "Main Menu: Quit Game"); break;
                    default: break;
                }
            }
        }
    }
}
void DrawMenuScreen(void) {
    InitializeMainMenuButtons(GetScreenWidth(), GetScreenHeight());
    ClearBackground(DARKGRAY);
    DrawText("FALL WITCHES", GetScreenWidth()/2 - MeasureText("FALL WITCHES", 70)/2, GetScreenHeight()/7, 70, WHITE);
    for (int i = 0; i < NUM_MAIN_MENU_BUTTONS; i++) {
        Color btnC = !mainMenuButtons[i].is_active?mainMenuButtons[i].disabled_color:(mainMenuButtons[i].is_hovered?mainMenuButtons[i].hover_color:mainMenuButtons[i].base_color);
        Color txtC = !mainMenuButtons[i].is_active?DARKGRAY:mainMenuButtons[i].text_color;
        DrawRectangleRec(mainMenuButtons[i].rect, btnC);
        DrawRectangleLinesEx(mainMenuButtons[i].rect, 2, Fade(BLACK,0.4f));
        int tw = MeasureText(mainMenuButtons[i].text,20);
        DrawText(mainMenuButtons[i].text, mainMenuButtons[i].rect.x+(mainMenuButtons[i].rect.width-tw)/2, mainMenuButtons[i].rect.y+(mainMenuButtons[i].rect.height-20)/2, 20, txtC);
    }
    DrawText("Use o mouse para selecionar.", 10, GetScreenHeight()-20, 10, LIGHTGRAY);
}

// --- Funções da Tela de Introdução ---
void UpdateIntroScreen(GameState *currentScreen_ptr, int *introFrames_ptr) {
    (*introFrames_ptr)++;
    if (*introFrames_ptr > 180 || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = GAMESTATE_MENU; *introFrames_ptr = 0; }
}
void DrawIntroScreen(void) {
    ClearBackground(BLACK);
    DrawText("TELA DE INTRODUÇÃO", GetScreenWidth()/2 - MeasureText("TELA DE INTRODUÇÃO", 30)/2, GetScreenHeight()/2 - 40, 30, WHITE);
    DrawText("Comi o CU de quem tá lendo", GetScreenWidth()/2 - MeasureText("Comi o CU de quem tá lendo", 20)/2, GetScreenHeight()/2, 20, LIGHTGRAY);
}

// --- Funções do Menu de Pausa ---
void InitializePauseMenuButtons(int screenWidth, int screenHeight) {
    float btnContinueWidth = 300; float btnContinueHeight = 60;
    float btnWidth = 220; float btnHeight = 45;
    float spacingY = 15; float spacingX = 20;
    float titlePauseFontSize = 50; float titlePauseY = screenHeight / 2.0f - 120;
    float startY = titlePauseY + titlePauseFontSize + 20;
    pauseMenuButtons[0] = (MenuButton){{(screenWidth-btnContinueWidth)/2.0f, startY, btnContinueWidth, btnContinueHeight}, "Continuar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_RESUME_GAME};
    float currentY = startY + btnContinueHeight + spacingY;
    float twoButtonRowWidthPause = 2 * btnWidth + spacingX;
    float startX_twoButtonsPause = (screenWidth - twoButtonRowWidthPause) / 2.0f;
    pauseMenuButtons[1] = (MenuButton){{startX_twoButtonsPause, currentY, btnWidth, btnHeight}, "Save", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_SAVE_GAME};
    pauseMenuButtons[2] = (MenuButton){{startX_twoButtonsPause + btnWidth + spacingX, currentY, btnWidth, btnHeight}, "Load", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_LOAD_GAME};
    currentY += btnHeight + spacingY;
    pauseMenuButtons[3] = (MenuButton){{startX_twoButtonsPause, currentY, btnWidth, btnHeight}, "Configurações", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_PAUSE_SETTINGS};
    pauseMenuButtons[4] = (MenuButton){{startX_twoButtonsPause + btnWidth + spacingX, currentY, btnWidth, btnHeight}, "Sair para Menu", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_GOTO_MAIN_MENU};
    pauseMenuButtonsInitialized = true;
}

void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int isPlaying_beforePause, int *musicIsCurrentlyPlaying_ptr) {
    (void)players; 
    if (!pauseMenuButtonsInitialized) InitializePauseMenuButtons(GetScreenWidth(), GetScreenHeight());
    Vector2 mousePoint = GetMousePosition();
    for (int i = 0; i < NUM_PAUSE_MENU_BUTTONS; i++) {
        pauseMenuButtons[i].is_hovered = false;
        if (pauseMenuButtons[i].is_active && CheckCollisionPointRec(mousePoint, pauseMenuButtons[i].rect)) {
            pauseMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                switch (pauseMenuButtons[i].action) {
                    case BUTTON_ACTION_RESUME_GAME:
                        *currentScreen_ptr = GAMESTATE_PLAYING;
                        if(isPlaying_beforePause && playlist[currentMusicIndex].stream.buffer!=NULL) { ResumeMusicStream(playlist[currentMusicIndex]); if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 1; }
                        break;
                    case BUTTON_ACTION_SAVE_GAME:
                        s_is_in_save_mode = true; s_save_load_menu_sub_state = 0;
                        s_previous_screen_before_save_load = GAMESTATE_PAUSE;
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU;
                        saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f; 
                        TraceLog(LOG_INFO, "Pause Menu: Save Game -> Entrando na tela de Slots.");
                        break;
                    case BUTTON_ACTION_LOAD_GAME:
                        s_is_in_save_mode = false; s_save_load_menu_sub_state = 0;
                        s_previous_screen_before_save_load = GAMESTATE_PAUSE;
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU;
                        saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f;
                        TraceLog(LOG_INFO, "Pause Menu: Load Game -> Entrando na tela de Slots.");
                        break;
                    case BUTTON_ACTION_PAUSE_SETTINGS: TraceLog(LOG_INFO, "Pause Menu: Settings (Placeholder)"); break;
                    case BUTTON_ACTION_GOTO_MAIN_MENU:
                        *currentScreen_ptr = GAMESTATE_MENU;
                        if(playlist[currentMusicIndex].stream.buffer!=NULL) StopMusicStream(playlist[currentMusicIndex]);
                        if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 0;
                        break;
                    default: break;
                }
            }
        }
    }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
        *currentScreen_ptr = GAMESTATE_PLAYING;
        if(isPlaying_beforePause && playlist[currentMusicIndex].stream.buffer!=NULL) { ResumeMusicStream(playlist[currentMusicIndex]); if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 1; }
    }
}

void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying_when_game_paused, int currentMapX, int currentMapY) {
    InitializePauseMenuButtons(GetScreenWidth(), GetScreenHeight());
    DrawPlayingScreen(players_arr, currentVolume, currentMusicIndex, isPlaying_when_game_paused, currentMapX, currentMapY);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
    DrawText("PAUSADO", GetScreenWidth()/2 - MeasureText("PAUSADO", 50)/2, GetScreenHeight()/2.0f - 120, 50, GRAY);
    for (int i = 0; i < NUM_PAUSE_MENU_BUTTONS; i++) {
        Color btnC = !pauseMenuButtons[i].is_active ? pauseMenuButtons[i].disabled_color : (pauseMenuButtons[i].is_hovered ? pauseMenuButtons[i].hover_color : pauseMenuButtons[i].base_color);
        Color txtC = !pauseMenuButtons[i].is_active ? DARKGRAY : pauseMenuButtons[i].text_color;
        DrawRectangleRec(pauseMenuButtons[i].rect, btnC);
        DrawRectangleLinesEx(pauseMenuButtons[i].rect, 2, Fade(BLACK,0.4f));
        int tw = MeasureText(pauseMenuButtons[i].text,20);
        DrawText(pauseMenuButtons[i].text, pauseMenuButtons[i].rect.x+(pauseMenuButtons[i].rect.width-tw)/2, pauseMenuButtons[i].rect.y+(pauseMenuButtons[i].rect.height-20)/2, 20, txtC);
    }
}

// --- Funções para a Tela de Slots de Save/Load ---
void InitializeSaveLoadSlotsMenuButtons(int screenWidth, int screenHeight) {
    float panelPadding = 20;
    float topOffsetForTitle = 70; 
    float bottomOffsetForEsc = 30; 
    saveLoadSlotsViewArea = (Rectangle){ panelPadding, topOffsetForTitle, (float)screenWidth - 2 * panelPadding, (float)screenHeight - topOffsetForTitle - bottomOffsetForEsc };
    float slotWidth = saveLoadSlotsViewArea.width - 2 * SLOT_PADDING_SAVE_LOAD;
    // int numRows = (MAX_SAVE_SLOTS + SLOTS_PER_ROW_SAVE_LOAD - 1) / SLOTS_PER_ROW_SAVE_LOAD; // SLOTS_PER_ROW_SAVE_LOAD é para grid, aqui é coluna única

    saveLoadTotalContentHeight = 0;
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        saveLoadSlotButtons[i].rect.x = SLOT_PADDING_SAVE_LOAD; 
        saveLoadSlotButtons[i].rect.y = i * (SLOT_HEIGHT_SAVE_LOAD + SLOT_PADDING_SAVE_LOAD);
        saveLoadSlotButtons[i].rect.width = slotWidth;
        saveLoadSlotButtons[i].rect.height = SLOT_HEIGHT_SAVE_LOAD;
        saveLoadSlotButtons[i].text = ""; 
        saveLoadSlotButtons[i].base_color = DARKGRAY;
        saveLoadSlotButtons[i].hover_color = GRAY;
        saveLoadSlotButtons[i].disabled_color = DARKGRAY; 
        saveLoadSlotButtons[i].text_color = WHITE;
        saveLoadSlotButtons[i].is_active = true;
        saveLoadSlotButtons[i].is_hovered = false;
        saveLoadSlotButtons[i].action = BUTTON_ACTION_NONE; 
        if (i < MAX_SAVE_SLOTS -1) {
            saveLoadTotalContentHeight += SLOT_HEIGHT_SAVE_LOAD + SLOT_PADDING_SAVE_LOAD;
        } else {
            saveLoadTotalContentHeight += SLOT_HEIGHT_SAVE_LOAD;
        }
    }
    saveLoadSlotsInitialized = true;
}

void InitializeSaveLoadConfirmationButtons(int screenWidth, int screenHeight) {
    float btnWidth = 120; float btnHeight = 40;
    float boxWidth = 450; float boxHeight = 150;
    float boxX = (screenWidth - boxWidth) / 2.0f; float boxY = (screenHeight - boxHeight) / 2.0f;
    float spacing = 20; float totalBtnWidth = 2 * btnWidth + spacing;
    float buttonY = boxY + boxHeight - btnHeight - 20;
    s_confirmationButtons[0] = (MenuButton){ 
        {boxX + (boxWidth - totalBtnWidth)/2.0f, buttonY, btnWidth, btnHeight},
        "Sim", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, GRAY, WHITE, true, false, BUTTON_ACTION_NONE };
    s_confirmationButtons[1] = (MenuButton){ 
        {boxX + (boxWidth - totalBtnWidth)/2.0f + btnWidth + spacing, buttonY, btnWidth, btnHeight},
        "Não", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, GRAY, WHITE, true, false, BUTTON_ACTION_NONE };
    s_confirmationButtonsInitialized = true;
}

void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int *musicIsCurrentlyPlaying_ptr) {
    if (!saveLoadSlotsInitialized) InitializeSaveLoadSlotsMenuButtons(GetScreenWidth(), GetScreenHeight());
    // Não precisa mais: if (s_save_load_menu_sub_state == 1 && !s_confirmationButtonsInitialized) InitializeSaveLoadConfirmationButtons(GetScreenWidth(),GetScreenHeight());
    // InitializeSaveLoadConfirmationButtons é chamado em DrawSaveLoadMenuScreen quando necessário

    Vector2 mousePoint = GetMousePosition();
    float wheelMove = GetMouseWheelMove();

    if (s_save_load_menu_sub_state == 0) { 
        if (CheckCollisionPointRec(mousePoint, saveLoadSlotsViewArea) && wheelMove != 0) {
            saveLoadScrollOffset += wheelMove * SLOT_HEIGHT_SAVE_LOAD; 
            float maxScroll = 0.0f;
            if (saveLoadTotalContentHeight > saveLoadSlotsViewArea.height) { // Só calcula maxScroll se há o que scrollar
                maxScroll = saveLoadTotalContentHeight - saveLoadSlotsViewArea.height;
            }
            if (saveLoadScrollOffset > 0) saveLoadScrollOffset = 0;
            if (maxScroll > 0) { // Só aplica limite inferior se há scroll
                 if (saveLoadScrollOffset < -maxScroll) saveLoadScrollOffset = -maxScroll;
            } else {
                 saveLoadScrollOffset = 0; // Sem scroll se não precisa
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            *currentScreen_ptr = s_previous_screen_before_save_load;
            saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f; return;
        }
        for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
            Rectangle onScreenSlotRect = {
                saveLoadSlotsViewArea.x + saveLoadSlotButtons[i].rect.x,
                saveLoadSlotsViewArea.y + saveLoadSlotButtons[i].rect.y + saveLoadScrollOffset,
                saveLoadSlotButtons[i].rect.width,
                saveLoadSlotButtons[i].rect.height
            };
            saveLoadSlotButtons[i].is_hovered = false;
            if (CheckCollisionRecs(onScreenSlotRect, saveLoadSlotsViewArea)) { // Só interage com slots visíveis
                 if (CheckCollisionPointRec(mousePoint, onScreenSlotRect)) {
                    saveLoadSlotButtons[i].is_hovered = true;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        s_selected_slot_for_action = i;
                        if (s_is_in_save_mode) {
                            if (DoesSaveSlotExist(s_selected_slot_for_action)) {
                                s_save_load_menu_sub_state = 1; 
                                s_confirmationButtonsInitialized = false; // Força recálculo da posição dos botões Sim/Não
                            } else { 
                                if (SaveGame(players, MAX_PLAYERS, s_selected_slot_for_action)) TraceLog(LOG_INFO, "Jogo salvo no slot %d.", s_selected_slot_for_action + 1);
                                else TraceLog(LOG_ERROR, "Falha ao salvar no slot %d.", s_selected_slot_for_action + 1);
                                *currentScreen_ptr = s_previous_screen_before_save_load;
                            }
                        } else { 
                            if (DoesSaveSlotExist(s_selected_slot_for_action)) {
                                if (LoadGame(players, MAX_PLAYERS, s_selected_slot_for_action)) {
                                    TraceLog(LOG_INFO, "Jogo carregado do slot %d.", s_selected_slot_for_action + 1);
                                    *currentScreen_ptr = GAMESTATE_PLAYING;
                                    if (playlist[currentMusicIndex].stream.buffer != NULL) {
                                         if(IsMusicStreamPlaying(playlist[currentMusicIndex])) StopMusicStream(playlist[currentMusicIndex]);
                                         PlayMusicStream(playlist[currentMusicIndex]);
                                         if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 1;
                                    } else { if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 0; }
                                } else { TraceLog(LOG_ERROR, "Falha ao carregar do slot %d.", s_selected_slot_for_action + 1); }
                            } else { TraceLog(LOG_INFO, "Slot %d está vazio.", s_selected_slot_for_action + 1); }
                        }
                        if (*currentScreen_ptr != GAMESTATE_SAVE_LOAD_MENU || s_save_load_menu_sub_state == 1) {
                           saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f;
                        } return;
                    }
                }
            }
        }
    } else if (s_save_load_menu_sub_state == 1) { 
        if (!s_confirmationButtonsInitialized) InitializeSaveLoadConfirmationButtons(GetScreenWidth(), GetScreenHeight());
        if (IsKeyPressed(KEY_ESCAPE)) {
            s_save_load_menu_sub_state = 0; s_selected_slot_for_action = -1; return;
        }
        for (int i = 0; i < 2; i++) {
            s_confirmationButtons[i].is_hovered = false;
            if (CheckCollisionPointRec(mousePoint, s_confirmationButtons[i].rect)) {
                s_confirmationButtons[i].is_hovered = true;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (i == 0) { 
                        if (SaveGame(players, MAX_PLAYERS, s_selected_slot_for_action)) TraceLog(LOG_INFO, "Jogo sobrescrito no slot %d.", s_selected_slot_for_action + 1);
                        else TraceLog(LOG_ERROR, "Falha ao sobrescrever slot %d.", s_selected_slot_for_action + 1);
                        *currentScreen_ptr = s_previous_screen_before_save_load;
                    } else { 
                        TraceLog(LOG_INFO, "Sobrescrita cancelada para slot %d.", s_selected_slot_for_action + 1);
                    }
                    s_save_load_menu_sub_state = 0; s_selected_slot_for_action = -1;
                    saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f;
                    return; 
                }
            }
        }
    }
}

void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, int musicIsPlaying, float musicVolume, int mapX, int mapY) {
    (void)playlist; 

    DrawPauseScreen(players, musicVolume, currentMusicIndex, musicIsPlaying, mapX, mapY);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f)); 

    if (!saveLoadSlotsInitialized) InitializeSaveLoadSlotsMenuButtons(GetScreenWidth(), GetScreenHeight());
    // A inicialização dos botões de confirmação agora é feita em UpdateSaveLoadMenuScreen quando o sub-estado muda para 1
    // ou aqui, se ainda não inicializado e no sub-estado 1.
    if (s_save_load_menu_sub_state == 1 && !s_confirmationButtonsInitialized) {
        InitializeSaveLoadConfirmationButtons(GetScreenWidth(), GetScreenHeight());
    }


    char title[64];
    sprintf(title, "ESCOLHA UM SLOT PARA %s", s_is_in_save_mode ? "SALVAR" : "CARREGAR");
    DrawText(title, (GetScreenWidth() - MeasureText(title, 30)) / 2, 30, 30, WHITE);

    if (s_save_load_menu_sub_state == 0) { 
        BeginScissorMode((int)saveLoadSlotsViewArea.x, (int)saveLoadSlotsViewArea.y, (int)saveLoadSlotsViewArea.width, (int)saveLoadSlotsViewArea.height);
            for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                float onScreenSlotY = saveLoadSlotsViewArea.y + saveLoadSlotButtons[i].rect.y + saveLoadScrollOffset;
                Rectangle onScreenSlotRect = {
                    saveLoadSlotsViewArea.x + saveLoadSlotButtons[i].rect.x,
                    onScreenSlotY,
                    saveLoadSlotButtons[i].rect.width,
                    saveLoadSlotButtons[i].rect.height
                };
                if ((onScreenSlotRect.y + onScreenSlotRect.height > saveLoadSlotsViewArea.y) && (onScreenSlotRect.y < saveLoadSlotsViewArea.y + saveLoadSlotsViewArea.height)) {
                    Color slotColor = saveLoadSlotButtons[i].is_hovered ? LIGHTGRAY : DARKGRAY;
                    if (!s_is_in_save_mode && !DoesSaveSlotExist(i)) { 
                        slotColor = Fade(DARKGRAY, 0.5f); 
                    }
                    DrawRectangleRec(onScreenSlotRect, slotColor);
                    DrawRectangleLinesEx(onScreenSlotRect, 2, GRAY);
                    char slotText[64];
                    if (DoesSaveSlotExist(i)) {
                        sprintf(slotText, "Slot %d - Ocupado", i + 1);
                        DrawText(slotText, onScreenSlotRect.x + 10, onScreenSlotRect.y + (onScreenSlotRect.height - 20)/2, 20, WHITE);
                    } else {
                        sprintf(slotText, "Slot %d - Vazio", i + 1);
                        DrawText(slotText, onScreenSlotRect.x + 10, onScreenSlotRect.y + (onScreenSlotRect.height - 20)/2, 20, LIGHTGRAY);
                    }
                }
            }
        EndScissorMode();

        float maxScroll = 0.0f;
        if (saveLoadTotalContentHeight > saveLoadSlotsViewArea.height) {
            maxScroll = saveLoadTotalContentHeight - saveLoadSlotsViewArea.height;
        }

        if (maxScroll > 0) {
            Rectangle scrollBarArea = { saveLoadSlotsViewArea.x + saveLoadSlotsViewArea.width + 2, saveLoadSlotsViewArea.y, 8, saveLoadSlotsViewArea.height };
            DrawRectangleRec(scrollBarArea, LIGHTGRAY);
            float thumbHeight = (saveLoadSlotsViewArea.height / saveLoadTotalContentHeight) * scrollBarArea.height;
            if (thumbHeight < 20) thumbHeight = 20; 
            float thumbY = scrollBarArea.y;
            if (maxScroll > 0) { // Evita divisão por zero se maxScroll for 0
                 thumbY += (-saveLoadScrollOffset / maxScroll) * (scrollBarArea.height - thumbHeight);
            }
            DrawRectangle((int)scrollBarArea.x, (int)thumbY, (int)scrollBarArea.width, (int)thumbHeight, DARKGRAY);
        }
        DrawText("ESC para Voltar | Roda do Mouse para Scroll", 10, GetScreenHeight() - 20, 10, WHITE);

    } else if (s_save_load_menu_sub_state == 1) { 
        float boxWidth = 450; float boxHeight = 150;
        Rectangle confBoxRect = {(GetScreenWidth() - boxWidth)/2.0f, (GetScreenHeight() - boxHeight)/2.0f, boxWidth, boxHeight};
        DrawRectangleRec(confBoxRect, Fade(BLACK, 0.95f)); 
        DrawRectangleLinesEx(confBoxRect, 2, WHITE);
        char confText[128];
        sprintf(confText, "Você quer sobrescrever o Save %d?", s_selected_slot_for_action + 1);
        DrawText(confText, confBoxRect.x + (boxWidth - MeasureText(confText, 20)) / 2.0f, confBoxRect.y + 20, 20, WHITE);
        for (int i = 0; i < 2; i++) {
            Color btnColor = s_confirmationButtons[i].is_hovered ? ORANGE : MAROON;
            DrawRectangleRec(s_confirmationButtons[i].rect, btnColor);
            DrawText(s_confirmationButtons[i].text,
                     s_confirmationButtons[i].rect.x + (s_confirmationButtons[i].rect.width - MeasureText(s_confirmationButtons[i].text, 20)) / 2.0f,
                     s_confirmationButtons[i].rect.y + (s_confirmationButtons[i].rect.height - 20) / 2.0f, 20, WHITE);
        }
        DrawText("ESC para Cancelar", confBoxRect.x + 10, confBoxRect.y + confBoxRect.height - 20, 10, GRAY);
    }
}