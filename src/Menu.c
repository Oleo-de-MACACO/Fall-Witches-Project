#include "../include/Menu.h"    
#include "../include/Game.h"    
#include "../include/Classes.h"   
#include "../include/SaveLoad.h"
#include "../include/Inventory.h" 
#include <stdio.h>
#include <string.h> 
#include "raylib.h" 

// === Configurações e Variáveis Estáticas para Menus ===
#define NUM_MAIN_MENU_BUTTONS 5
static MenuButton mainMenuButtons[NUM_MAIN_MENU_BUTTONS];
static bool mainMenuButtonsInitialized = false;

#define NUM_PLAYER_MODE_MENU_BUTTONS 2
static MenuButton playerModeMenuButtons[NUM_PLAYER_MODE_MENU_BUTTONS];
static bool playerModeMenuButtonsInitialized = false;

#define NUM_PAUSE_MENU_BUTTONS 5
static MenuButton pauseMenuButtons[NUM_PAUSE_MENU_BUTTONS];
static bool pauseMenuButtonsInitialized = false;

#define SLOTS_PER_ROW_SAVE_LOAD 6 
#define SLOT_PADDING_SAVE_LOAD 10
#define SLOT_HEIGHT_SAVE_LOAD 70 
static MenuButton saveLoadSlotButtons[MAX_SAVE_SLOTS]; 
static bool saveLoadSlotsInitialized = false;

static float saveLoadScrollOffset = 0.0f;
static float saveLoadTotalContentHeight = 0.0f;
static Rectangle saveLoadSlotsViewArea = {0};

static bool s_is_in_save_mode = true;      
static bool s_is_new_game_flow = false;    
static int s_save_load_menu_sub_state = 0; 
static int s_selected_slot_for_action = -1;
static GameState s_previous_screen_before_save_load; 

static MenuButton s_confirmationButtons[2]; 
static bool s_confirmationButtonsInitialized = false;

static int creation_current_player_idx = 0; 
static int creation_step = 0; 
static char creation_player_names[MAX_PLAYERS][MAX_PLAYER_NAME_LENGTH];
static int creation_name_letter_counts[MAX_PLAYERS] = {0};
static Classe creation_player_classes[MAX_PLAYERS];
static int creation_class_selection_idx[MAX_PLAYERS] = {0}; 
static bool creation_name_edit_mode[MAX_PLAYERS] = {false};

const Classe AVAILABLE_CLASSES[] = {GUERREIRO, MAGO, ARQUEIRO, BARBARO, LADINO, CLERIGO};
const int NUM_AVAILABLE_CLASSES = sizeof(AVAILABLE_CLASSES) / sizeof(AVAILABLE_CLASSES[0]);
const char* CLASS_DISPLAY_NAMES[] = {"Guerreiro", "Mago", "Arqueiro", "Bárbaro", "Ladino", "Clérigo"};


#define COLOR_BUTTON_ACTIVE MAROON
#define COLOR_BUTTON_HOVER ORANGE
#define COLOR_BUTTON_DISABLED GRAY
#define COLOR_BUTTON_TEXT WHITE


// --- Funções do Menu Principal ---
void InitializeMainMenuButtons(int screenWidth, int screenHeight) { 
    float buttonWidth = 280; float buttonHeight = 50; float spacingY = 20; float spacingX = 20;
    float twoButtonRowWidth = 2 * buttonWidth + spacingX; float totalButtonBlockHeight = 3 * buttonHeight + 2 * spacingY;
    float titleHeightEstimate = screenHeight / 7.0f + 70; float startY = titleHeightEstimate + 40;
    if (startY + totalButtonBlockHeight > screenHeight - 30) { startY = (screenHeight - totalButtonBlockHeight) / 2.0f; if (startY < titleHeightEstimate) startY = titleHeightEstimate; }
    if (startY < screenHeight * 0.3f) startY = screenHeight * 0.3f;
    float startX_twoButtons = (screenWidth - twoButtonRowWidth) / 2.0f; float startX_oneButton = (screenWidth - buttonWidth) / 2.0f;
    
    mainMenuButtons[0] = (MenuButton){{startX_twoButtons, startY, buttonWidth, buttonHeight}, "Um Jogador (em breve)", COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, LIGHTGRAY, false, false, BUTTON_ACTION_NONE };
    mainMenuButtons[1] = (MenuButton){{startX_twoButtons + buttonWidth + spacingX, startY, buttonWidth, buttonHeight}, "Dois Jogadores", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_GOTO_PLAYER_MODE_MENU };
    float currentY_main = startY + buttonHeight + spacingY;
    mainMenuButtons[2] = (MenuButton){{startX_oneButton, currentY_main, buttonWidth, buttonHeight}, "Multijogador (em breve)", COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, COLOR_BUTTON_DISABLED, LIGHTGRAY, false, false, BUTTON_ACTION_NONE };
    currentY_main += buttonHeight + spacingY;
    mainMenuButtons[3] = (MenuButton){{startX_twoButtons, currentY_main, buttonWidth, buttonHeight}, "Opções", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_SETTINGS };
    mainMenuButtons[4] = (MenuButton){{startX_twoButtons + buttonWidth + spacingX, currentY_main, buttonWidth, buttonHeight}, "Sair do Jogo", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_QUIT_GAME };
    mainMenuButtonsInitialized = true;
}
void UpdateMenuScreen(GameState *currentScreen_ptr) { 
    if (!mainMenuButtonsInitialized) InitializeMainMenuButtons(GetScreenWidth(), GetScreenHeight());
    Vector2 mousePoint = GetMousePosition();
    for (int i = 0; i < NUM_MAIN_MENU_BUTTONS; i++) {
        mainMenuButtons[i].is_hovered = false;
        if (mainMenuButtons[i].is_active && CheckCollisionPointRec(mousePoint, mainMenuButtons[i].rect)) {
            mainMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                switch (mainMenuButtons[i].action) {
                    case BUTTON_ACTION_GOTO_PLAYER_MODE_MENU:
                        *currentScreen_ptr = GAMESTATE_PLAYER_MODE_MENU; 
                        playerModeMenuButtonsInitialized = false; 
                        s_is_new_game_flow = false; 
                        TraceLog(LOG_INFO, "[Menu Principal] -> Menu de Modo de Jogador");
                        break;
                    case BUTTON_ACTION_SETTINGS: 
                        TraceLog(LOG_INFO, "[Menu Principal] Opções (Placeholder)"); 
                        break;
                    case BUTTON_ACTION_QUIT_GAME: 
                        g_request_exit = true; 
                        TraceLog(LOG_INFO, "[Menu Principal] Sair do Jogo"); 
                        break;
                    default: break;
                }
            }
        }
    }
 }
void DrawMenuScreen(void) { 
    if (!mainMenuButtonsInitialized) InitializeMainMenuButtons(GetScreenWidth(), GetScreenHeight());
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
    if (*introFrames_ptr > 180 || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON) ) { 
        *currentScreen_ptr = GAMESTATE_MENU; 
        *introFrames_ptr = 0; 
        mainMenuButtonsInitialized = false; 
    }
 }
void DrawIntroScreen(void) { 
    ClearBackground(BLACK);
    DrawText("TELA DE INTRODUÇÃO", GetScreenWidth()/2 - MeasureText("TELA DE INTRODUÇÃO", 30)/2, GetScreenHeight()/2 - 40, 30, WHITE);
    DrawText("Fall Witches Engine v0.0.6", GetScreenWidth()/2 - MeasureText("Fall Witches Engine v0.0.6", 20)/2, GetScreenHeight()/2 + 10, 20, LIGHTGRAY);
    DrawText("Pressione ENTER, ESC ou clique para continuar...", GetScreenWidth()/2 - MeasureText("Pressione ENTER, ESC ou clique para continuar...", 10)/2, GetScreenHeight() - 30, 10, GRAY);
}


// --- Funções do Menu de Modo de Jogador (Novo Jogo / Carregar) ---
void InitializePlayerModeMenuButtons(int screenWidth, int screenHeight) { 
    float buttonWidth = 250;
    float buttonHeight = 50;
    float spacingY = 30;
    float totalHeight = NUM_PLAYER_MODE_MENU_BUTTONS * buttonHeight + (NUM_PLAYER_MODE_MENU_BUTTONS - 1) * spacingY;
    float startY = (screenHeight - totalHeight) / 2.0f;
    float startX = (screenWidth - buttonWidth) / 2.0f;

    playerModeMenuButtons[0] = (MenuButton){
        {startX, startY, buttonWidth, buttonHeight},
        "Novo Jogo", 
        MAROON, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT,
        true, false, BUTTON_ACTION_NEW_GAME_SETUP_SLOT 
    };
    playerModeMenuButtons[1] = (MenuButton){
        {startX, startY + buttonHeight + spacingY, buttonWidth, buttonHeight},
        "Carregar Jogo", 
        MAROON, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT,
        true, false, BUTTON_ACTION_LOAD_GAME 
    };
    playerModeMenuButtonsInitialized = true;
}
void UpdatePlayerModeMenuScreen(GameState *currentScreen_ptr, Music playlist[], int currentMusicIndex, float currentVolume, int *isPlaying_ptr) { 
    (void)playlist; (void)currentMusicIndex; (void)currentVolume; (void)isPlaying_ptr;

    if (!playerModeMenuButtonsInitialized) InitializePlayerModeMenuButtons(GetScreenWidth(), GetScreenHeight());

    Vector2 mousePoint = GetMousePosition();
    for (int i = 0; i < NUM_PLAYER_MODE_MENU_BUTTONS; i++) {
        playerModeMenuButtons[i].is_hovered = false;
        if (playerModeMenuButtons[i].is_active && CheckCollisionPointRec(mousePoint, playerModeMenuButtons[i].rect)) {
            playerModeMenuButtons[i].is_hovered = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                switch (playerModeMenuButtons[i].action) {
                    case BUTTON_ACTION_NEW_GAME_SETUP_SLOT: 
                        s_is_in_save_mode = true;    
                        s_is_new_game_flow = true;   
                        s_save_load_menu_sub_state = 0; 
                        s_previous_screen_before_save_load = GAMESTATE_PLAYER_MODE_MENU; 
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU; 
                        saveLoadSlotsInitialized = false; 
                        s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f;
                        TraceLog(LOG_INFO, "[Menu Modo Jogador] Novo Jogo -> Selecionar Slot");
                        break;
                    case BUTTON_ACTION_LOAD_GAME:
                        s_is_in_save_mode = false;   
                        s_is_new_game_flow = false;  
                        s_save_load_menu_sub_state = 0;
                        s_previous_screen_before_save_load = GAMESTATE_PLAYER_MODE_MENU; 
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU;
                        saveLoadSlotsInitialized = false; 
                        s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f;
                        TraceLog(LOG_INFO, "[Menu Modo Jogador] Carregar Jogo -> Tela de Slots");
                        break;
                    default:
                        break;
                }
            }
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = GAMESTATE_MENU; 
        mainMenuButtonsInitialized = false; 
        s_is_new_game_flow = false; 
    }
}
void DrawPlayerModeMenuScreen(void) { 
    if (!playerModeMenuButtonsInitialized) InitializePlayerModeMenuButtons(GetScreenWidth(), GetScreenHeight());

    ClearBackground(DARKGRAY); 
    DrawText("DOIS JOGADORES", GetScreenWidth() / 2 - MeasureText("DOIS JOGADORES", 40) / 2, GetScreenHeight() / 4, 40, WHITE);

    for (int i = 0; i < NUM_PLAYER_MODE_MENU_BUTTONS; i++) {
        Color btnC = !playerModeMenuButtons[i].is_active ? playerModeMenuButtons[i].disabled_color : (playerModeMenuButtons[i].is_hovered ? playerModeMenuButtons[i].hover_color : playerModeMenuButtons[i].base_color);
        Color txtC = !playerModeMenuButtons[i].is_active ? DARKGRAY : playerModeMenuButtons[i].text_color;
        DrawRectangleRec(playerModeMenuButtons[i].rect, btnC);
        DrawRectangleLinesEx(playerModeMenuButtons[i].rect, 2, Fade(BLACK, 0.4f));
        int tw = MeasureText(playerModeMenuButtons[i].text, 20);
        DrawText(playerModeMenuButtons[i].text, playerModeMenuButtons[i].rect.x + (playerModeMenuButtons[i].rect.width - tw) / 2, playerModeMenuButtons[i].rect.y + (playerModeMenuButtons[i].rect.height - 20) / 2, 20, txtC);
    }
    DrawText("ESC para Voltar ao Menu Principal", 10, GetScreenHeight() - 20, 10, LIGHTGRAY);
}

// --- Tela de Criação de Personagem ---
void InitializeCharacterCreation(void) {
    creation_current_player_idx = 0; 
    creation_step = 0;               

    for (int i = 0; i < MAX_PLAYERS; i++) {
        strcpy(creation_player_names[i], ""); 
        creation_name_letter_counts[i] = 0;
        creation_player_classes[i] = GUERREIRO; 
        creation_class_selection_idx[i] = 0;    
        creation_name_edit_mode[i] = false;     
    }
    if (MAX_PLAYERS > 0) creation_name_edit_mode[0] = true; 
    TraceLog(LOG_INFO, "Tela de Criação de Personagem Inicializada.");
}

void UpdateCharacterCreationScreen(GameState *currentScreen_ptr, Player players[], int *mapX, int *mapY, Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsPlaying_ptr) {
    int player_idx_configuring = creation_step / 2; // 0 para P1 (passos 0,1), 1 para P2 (passos 2,3)

    if (creation_step % 2 == 0) { // Passo de input de nome
        creation_name_edit_mode[player_idx_configuring] = true;
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        int key = GetCharPressed();
        while (key > 0) { 
            if ((key >= 32) && (key <= 125) && (creation_name_letter_counts[player_idx_configuring] < MAX_PLAYER_NAME_LENGTH - 1)) {
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = (char)key;
                creation_name_letter_counts[player_idx_configuring]++;
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = '\0'; 
            }
            key = GetCharPressed(); 
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (creation_name_letter_counts[player_idx_configuring] > 0) {
                creation_name_letter_counts[player_idx_configuring]--;
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = '\0';
            }
        }
        if (IsKeyPressed(KEY_ENTER)) { 
            if (creation_name_letter_counts[player_idx_configuring] > 0) { 
                creation_name_edit_mode[player_idx_configuring] = false;
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                creation_step++; 
                TraceLog(LOG_INFO, "Jogador %d Nome: '%s' confirmado.", player_idx_configuring + 1, creation_player_names[player_idx_configuring]);
            } else {
                // Adicionar feedback visual aqui (ex: caixa de nome pisca em vermelho)
                TraceLog(LOG_INFO, "Nome do Jogador %d não pode ser vazio.", player_idx_configuring + 1);
            }
        }
    } else { // Passo de seleção de classe 
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        creation_name_edit_mode[player_idx_configuring] = false; 

        if (IsKeyPressed(KEY_UP)) {
            creation_class_selection_idx[player_idx_configuring]--;
            if (creation_class_selection_idx[player_idx_configuring] < 0) creation_class_selection_idx[player_idx_configuring] = NUM_AVAILABLE_CLASSES - 1;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            creation_class_selection_idx[player_idx_configuring]++;
            if (creation_class_selection_idx[player_idx_configuring] >= NUM_AVAILABLE_CLASSES) creation_class_selection_idx[player_idx_configuring] = 0;
        }
        creation_player_classes[player_idx_configuring] = AVAILABLE_CLASSES[creation_class_selection_idx[player_idx_configuring]];

        if (IsKeyPressed(KEY_ENTER)) { 
            TraceLog(LOG_INFO, "Jogador %d Classe: %s confirmada.", player_idx_configuring + 1, CLASS_DISPLAY_NAMES[creation_class_selection_idx[player_idx_configuring]]);
            if (player_idx_configuring < MAX_PLAYERS - 1) { 
                creation_step++; 
                // creation_current_player_idx não é mais necessário, usamos player_idx_configuring que é derivado de creation_step
                creation_name_edit_mode[player_idx_configuring + 1] = true; // Ativa edição de nome para o próximo
            } else {
                creation_step++; // Vai para o passo de confirmação final
            }
        }
    }

    int final_confirmation_step = MAX_PLAYERS * 2;
    if (creation_step == final_confirmation_step) {
        if (IsKeyPressed(KEY_ENTER)) { 
            for (int i = 0; i < MAX_PLAYERS; i++) {
                init_player(&players[i], creation_player_names[i], creation_player_classes[i]);
                // Posições iniciais (já definidas em PrepareNewGameSession, mas pode ajustar aqui se necessário)
                if (i == 0) {
                    players[i].posx = players[i].width + 50; 
                    players[i].posy = GetScreenHeight() / 2 - players[i].height / 2;
                } else if (i == 1) {
                    players[i].posx = GetScreenWidth() - players[i].width * 2 - 50;
                    players[i].posy = GetScreenHeight() / 2 - players[i].height / 2;
                }
                // Adicionar itens iniciais se desejado, por exemplo:
                // if (players[i].classe == GUERREIRO) AddItemToInventory(&players[i], "Espada de Treino", 1);
            }
            
            *mapX = 0; 
            *mapY = 0;

            if (playlist[currentMusicIndex].stream.buffer != NULL) { 
                if(IsMusicStreamPlaying(playlist[currentMusicIndex])) StopMusicStream(playlist[currentMusicIndex]);
                PlayMusicStream(playlist[currentMusicIndex]);
                SetMusicVolume(playlist[currentMusicIndex], currentVolume); 
                if(musicIsPlaying_ptr) *musicIsPlaying_ptr = 1;
            } else { if(musicIsPlaying_ptr) *musicIsPlaying_ptr = 0; }

            *currentScreen_ptr = GAMESTATE_PLAYING; 
            TraceLog(LOG_INFO, "[Criação Personagem Completa] -> Iniciando Jogo!");
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (creation_step > 0) { 
            creation_name_edit_mode[player_idx_configuring] = false; // Desativa edição de nome ao voltar
            creation_step--;
            int new_player_idx_configuring = creation_step / 2;
            if (creation_step % 2 == 0) { // Voltando para um passo de nome
                 creation_name_edit_mode[new_player_idx_configuring] = true;
            }
        } else { 
            *currentScreen_ptr = GAMESTATE_PLAYER_MODE_MENU; 
            playerModeMenuButtonsInitialized = false; 
            TraceLog(LOG_INFO, "[Criação Personagem] -> Menu Modo Jogador (ESC no primeiro passo)");
        }
    }
}

void DrawCharacterCreationScreen(Player players[], int currentPlayerFocus) {
    (void)players; 
    (void)currentPlayerFocus; 

    ClearBackground(DARKBLUE); 
    
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    // O jogador sendo configurado é derivado de creation_step
    int player_idx_drawing = creation_step / 2; 

    DrawText("CRIAÇÃO DE PERSONAGEM", screenWidth/2 - MeasureText("CRIAÇÃO DE PERSONAGEM", 30)/2, 30, 30, WHITE);

    int boxX = 50;
    int boxY = 80;
    int boxWidth = screenWidth - 100;
    // int boxHeight = screenHeight - 150; // Removido - não usado

    if (creation_step < MAX_PLAYERS * 2) { 
        DrawText(TextFormat("CONFIGURANDO JOGADOR %d", player_idx_drawing + 1), boxX + 20, boxY + 20, 20, RAYWHITE);

        if (creation_step % 2 == 0) { 
            DrawText("Digite o Nome:", boxX + 20, boxY + 60, 20, LIGHTGRAY);
            DrawRectangleLines(boxX + 20, boxY + 90, boxWidth - 40, 40, WHITE);
            DrawText(creation_player_names[player_idx_drawing], boxX + 25, boxY + 100, 20, YELLOW);
            if (creation_name_edit_mode[player_idx_drawing]) {
                if (((int)(GetTime()*2.0f)) % 2 == 0) { 
                    DrawText("_", boxX + 25 + MeasureText(creation_player_names[player_idx_drawing], 20), boxY + 100, 20, YELLOW);
                }
            }
            DrawText("Pressione ENTER para confirmar o nome.", boxX + 20, boxY + 140, 10, GRAY);
        } 
        else { 
            DrawText(TextFormat("Nome: %s", creation_player_names[player_idx_drawing]), boxX + 20, boxY + 60, 20, YELLOW);
            DrawText("Escolha a Classe:", boxX + 20, boxY + 90, 20, LIGHTGRAY);
            
            int classOptionY = boxY + 120;
            for (int i = 0; i < NUM_AVAILABLE_CLASSES; i++) {
                Color textColor = (i == creation_class_selection_idx[player_idx_drawing]) ? ORANGE : WHITE;
                DrawText(TextFormat("%s %s", (i == creation_class_selection_idx[player_idx_drawing]) ? "->" : "  ", CLASS_DISPLAY_NAMES[i]), 
                         boxX + 40, classOptionY + i * 25, 20, textColor);
            }
            DrawText("Use SETAS CIMA/BAIXO e ENTER para selecionar.", boxX + 20, classOptionY + NUM_AVAILABLE_CLASSES * 25 + 20, 10, GRAY);

            Player previewPlayer;
            init_player(&previewPlayer, "", AVAILABLE_CLASSES[creation_class_selection_idx[player_idx_drawing]]);
            int previewX = boxX + (boxWidth / 2) + 20; 
            int previewY = boxY + 90;
            DrawText("Atributos Base:", previewX, previewY, 10, RAYWHITE); previewY += 15;
            DrawText(TextFormat("HP:%d MP:%d ATK:%d DEF:%d", previewPlayer.max_vida, previewPlayer.max_mana, previewPlayer.ataque, previewPlayer.defesa), previewX, previewY, 10, LIGHTGRAY); previewY += 15;
            DrawText(TextFormat("FOR:%d PER:%d RES:%d", previewPlayer.forca, previewPlayer.percepcao, previewPlayer.resistencia), previewX, previewY, 10, LIGHTGRAY); previewY += 15;
            DrawText(TextFormat("CAR:%d INT:%d AGI:%d SOR:%d", previewPlayer.carisma, previewPlayer.inteligencia, previewPlayer.agilidade, previewPlayer.sorte), previewX, previewY, 10, LIGHTGRAY);
        }
    } else { 
        DrawText("CONFIRMAR EQUIPE?", screenWidth/2 - MeasureText("CONFIRMAR EQUIPE?", 20)/2, boxY + 20, 20, YELLOW);
        int summaryY = boxY + 60;
        for(int i=0; i < MAX_PLAYERS; i++) {
            DrawText(TextFormat("Jogador %d: %s (%s)", i+1, creation_player_names[i], CLASS_DISPLAY_NAMES[creation_class_selection_idx[i]]), 
                     boxX + 20, summaryY, 20, WHITE);
            summaryY += 30;
        }
        DrawText("Pressione ENTER para Iniciar o Jogo", screenWidth/2 - MeasureText("Pressione ENTER para Iniciar o Jogo", 20)/2, screenHeight - 80, 20, LIME);
    }
    DrawText("Pressione ESC para Voltar/Cancelar", 20, screenHeight - 30, 10, GRAY);
}


// --- Funções do Menu de Pausa ---
void InitializePauseMenuButtons(int screenWidth, int screenHeight) { /* ... (código mantido) ... */ 
    float btnContinueWidth = 300; float btnContinueHeight = 60;
    float btnWidth = 220; float btnHeight = 45;
    float spacingY = 15; float spacingX = 20;
    float titlePauseFontSize = 50; float titlePauseY = screenHeight / 2.0f - 120;
    float startY = titlePauseY + titlePauseFontSize + 20;
    pauseMenuButtons[0] = (MenuButton){{(screenWidth-btnContinueWidth)/2.0f, startY, btnContinueWidth, btnContinueHeight}, "Continuar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_RESUME_GAME};
    float currentY = startY + btnContinueHeight + spacingY;
    float twoButtonRowWidthPause = 2 * btnWidth + spacingX;
    float startX_twoButtonsPause = (screenWidth - twoButtonRowWidthPause) / 2.0f;
    pauseMenuButtons[1] = (MenuButton){{startX_twoButtonsPause, currentY, btnWidth, btnHeight}, "Salvar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_SAVE_GAME};
    pauseMenuButtons[2] = (MenuButton){{startX_twoButtonsPause + btnWidth + spacingX, currentY, btnWidth, btnHeight}, "Carregar", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_LOAD_GAME};
    currentY += btnHeight + spacingY;
    pauseMenuButtons[3] = (MenuButton){{startX_twoButtonsPause, currentY, btnWidth, btnHeight}, "Opções", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_PAUSE_SETTINGS};
    pauseMenuButtons[4] = (MenuButton){{startX_twoButtonsPause + btnWidth + spacingX, currentY, btnWidth, btnHeight}, "Sair para Menu", COLOR_BUTTON_ACTIVE, COLOR_BUTTON_HOVER, COLOR_BUTTON_DISABLED, COLOR_BUTTON_TEXT, true, false, BUTTON_ACTION_GOTO_MAIN_MENU};
    pauseMenuButtonsInitialized = true;
}
void UpdatePauseScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, int isPlaying_beforePause, int *musicIsCurrentlyPlaying_ptr) { /* ... (código mantido) ... */
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
                        s_is_in_save_mode = true; 
                        s_is_new_game_flow = false; 
                        s_save_load_menu_sub_state = 0;
                        s_previous_screen_before_save_load = GAMESTATE_PAUSE; 
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU;
                        saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f; 
                        TraceLog(LOG_INFO, "[Menu Pausa] Salvar Jogo -> Tela de Slots");
                        break;
                    case BUTTON_ACTION_LOAD_GAME:
                        s_is_in_save_mode = false; 
                        s_is_new_game_flow = false; 
                        s_save_load_menu_sub_state = 0;
                        s_previous_screen_before_save_load = GAMESTATE_PAUSE; 
                        *currentScreen_ptr = GAMESTATE_SAVE_LOAD_MENU;
                        saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false;
                        saveLoadScrollOffset = 0.0f;
                        TraceLog(LOG_INFO, "[Menu Pausa] Carregar Jogo -> Tela de Slots");
                        break;
                    case BUTTON_ACTION_PAUSE_SETTINGS: TraceLog(LOG_INFO, "[Menu Pausa] Opções (Placeholder)"); break;
                    case BUTTON_ACTION_GOTO_MAIN_MENU:
                        *currentScreen_ptr = GAMESTATE_MENU;
                        if(playlist[currentMusicIndex].stream.buffer!=NULL) StopMusicStream(playlist[currentMusicIndex]);
                        if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 0;
                        mainMenuButtonsInitialized = false; 
                        s_is_new_game_flow = false; 
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
void DrawPauseScreen(Player players_arr[], float currentVolume, int currentMusicIndex, int isPlaying_when_game_paused, int currentMapX, int currentMapY) { /* ... (código mantido) ... */ 
    if (!pauseMenuButtonsInitialized) InitializePauseMenuButtons(GetScreenWidth(), GetScreenHeight());
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
void InitializeSaveLoadSlotsMenuButtons(int screenWidth, int screenHeight) { /* ... (código mantido) ... */ 
    float panelPadding = 20;
    float topOffsetForTitle = 70; 
    float bottomOffsetForEsc = 30; 
    saveLoadSlotsViewArea = (Rectangle){ panelPadding, topOffsetForTitle, (float)screenWidth - 2 * panelPadding, (float)screenHeight - topOffsetForTitle - bottomOffsetForEsc };
    float slotWidth = saveLoadSlotsViewArea.width - 2 * SLOT_PADDING_SAVE_LOAD;
    
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
void InitializeSaveLoadConfirmationButtons(int screenWidth, int screenHeight) { /* ... (código mantido) ... */
    float btnWidth = 120; float btnHeight = 40;
    float boxWidth = s_is_new_game_flow ? 550 : 450; 
    float boxHeight = s_is_new_game_flow ? 180 : 150;

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

void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsCurrentlyPlaying_ptr, int *currentMapX_ptr, int *currentMapY_ptr) {
    // (Lógica atualizada para transicionar para GAMESTATE_CHARACTER_CREATION)
    if (!saveLoadSlotsInitialized) InitializeSaveLoadSlotsMenuButtons(GetScreenWidth(), GetScreenHeight());
    
    Vector2 mousePoint = GetMousePosition();
    float wheelMove = GetMouseWheelMove();

    if (s_save_load_menu_sub_state == 0) { 
        if (CheckCollisionPointRec(mousePoint, saveLoadSlotsViewArea) && wheelMove != 0) {
            saveLoadScrollOffset += wheelMove * SLOT_HEIGHT_SAVE_LOAD; 
            float maxScroll = 0.0f;
            if (saveLoadTotalContentHeight > saveLoadSlotsViewArea.height) { 
                maxScroll = saveLoadTotalContentHeight - saveLoadSlotsViewArea.height;
            }
            if (saveLoadScrollOffset > 0) saveLoadScrollOffset = 0;
            if (maxScroll > 0 && saveLoadScrollOffset < -maxScroll) saveLoadScrollOffset = -maxScroll;
            else if (maxScroll <=0) saveLoadScrollOffset = 0; 
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            *currentScreen_ptr = s_previous_screen_before_save_load; 
            if (s_previous_screen_before_save_load == GAMESTATE_PLAYER_MODE_MENU) playerModeMenuButtonsInitialized = false;
            else if (s_previous_screen_before_save_load == GAMESTATE_PAUSE) pauseMenuButtonsInitialized = false;
            s_is_new_game_flow = false; 
            saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f; 
            return;
        }

        for (int i = 0; i < MAX_SAVE_SLOTS; i++) { 
            Rectangle onScreenSlotRect = { 
                saveLoadSlotsViewArea.x + saveLoadSlotButtons[i].rect.x,
                saveLoadSlotsViewArea.y + saveLoadSlotButtons[i].rect.y + saveLoadScrollOffset,
                saveLoadSlotButtons[i].rect.width,
                saveLoadSlotButtons[i].rect.height
            };
            saveLoadSlotButtons[i].is_hovered = false;
            if (CheckCollisionRecs(onScreenSlotRect, saveLoadSlotsViewArea)) { 
                 if (CheckCollisionPointRec(mousePoint, onScreenSlotRect)) {
                    saveLoadSlotButtons[i].is_hovered = true;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        s_selected_slot_for_action = i;
                        if (s_is_in_save_mode) { 
                            if (DoesSaveSlotExist(s_selected_slot_for_action)) {
                                s_save_load_menu_sub_state = 1; 
                                s_confirmationButtonsInitialized = false; 
                            } else { 
                                if (s_is_new_game_flow) {
                                    PrepareNewGameSession(players, currentMapX_ptr, currentMapY_ptr, GetScreenWidth(), GetScreenHeight());
                                    InitializeCharacterCreation(); 
                                    *currentScreen_ptr = GAMESTATE_CHARACTER_CREATION;
                                } else { 
                                    if (SaveGame(players, MAX_PLAYERS, s_selected_slot_for_action, *currentMapX_ptr, *currentMapY_ptr)) { /*...*/ }
                                    *currentScreen_ptr = s_previous_screen_before_save_load; 
                                }
                            }
                        } else { 
                            if (DoesSaveSlotExist(s_selected_slot_for_action)) {
                                if (LoadGame(players, MAX_PLAYERS, s_selected_slot_for_action, currentMapX_ptr, currentMapY_ptr)) {
                                    *currentScreen_ptr = GAMESTATE_PLAYING; 
                                    if (playlist[currentMusicIndex].stream.buffer != NULL) {
                                         if(IsMusicStreamPlaying(playlist[currentMusicIndex])) StopMusicStream(playlist[currentMusicIndex]);
                                         PlayMusicStream(playlist[currentMusicIndex]);
                                         SetMusicVolume(playlist[currentMusicIndex], currentVolume); 
                                         if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 1; 
                                    } else { if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 0; }
                                } else { /*...*/ }
                            } else { /*...*/ }
                        }
                        if (*currentScreen_ptr != GAMESTATE_SAVE_LOAD_MENU || s_save_load_menu_sub_state == 1) {
                           saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f;
                        } 
                        if (*currentScreen_ptr == s_previous_screen_before_save_load) { 
                             if (s_previous_screen_before_save_load == GAMESTATE_PLAYER_MODE_MENU) playerModeMenuButtonsInitialized = false;
                             else if (s_previous_screen_before_save_load == GAMESTATE_PAUSE) pauseMenuButtonsInitialized = false;
                        }
                        return; 
                    }
                }
            }
        }
    } else if (s_save_load_menu_sub_state == 1) { 
        if (!s_confirmationButtonsInitialized) InitializeSaveLoadConfirmationButtons(GetScreenWidth(), GetScreenHeight());
        if (IsKeyPressed(KEY_ESCAPE)) {
            s_save_load_menu_sub_state = 0; 
            s_selected_slot_for_action = -1; 
            saveLoadSlotsInitialized = false; 
            return;
        }
        for (int i = 0; i < 2; i++) { 
            s_confirmationButtons[i].is_hovered = false;
            if (CheckCollisionPointRec(mousePoint, s_confirmationButtons[i].rect)) {
                s_confirmationButtons[i].is_hovered = true;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (i == 0) { 
                        if (s_is_new_game_flow) { 
                            PrepareNewGameSession(players, currentMapX_ptr, currentMapY_ptr, GetScreenWidth(), GetScreenHeight());
                            InitializeCharacterCreation(); 
                            *currentScreen_ptr = GAMESTATE_CHARACTER_CREATION;
                        } else { 
                            if (SaveGame(players, MAX_PLAYERS, s_selected_slot_for_action, *currentMapX_ptr, *currentMapY_ptr)) { /*...*/ }
                            *currentScreen_ptr = s_previous_screen_before_save_load; 
                        }
                    } else { 
                         s_save_load_menu_sub_state = 0; 
                    }
                    
                    if (*currentScreen_ptr != GAMESTATE_SAVE_LOAD_MENU || s_save_load_menu_sub_state == 0){
                        saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f;
                    }
                    s_selected_slot_for_action = -1; 
                    if(*currentScreen_ptr == s_previous_screen_before_save_load) { 
                        if (s_previous_screen_before_save_load == GAMESTATE_PLAYER_MODE_MENU) playerModeMenuButtonsInitialized = false;
                        else if (s_previous_screen_before_save_load == GAMESTATE_PAUSE) pauseMenuButtonsInitialized = false;
                        if (!s_is_new_game_flow && (*currentScreen_ptr != GAMESTATE_CHARACTER_CREATION)) s_is_new_game_flow = false; 
                    }
                    return; 
                }
            }
        }
    }
}

void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, int musicIsPlaying, float musicVolume, int mapX, int mapY) {
    // (Desenho da tela de Save/Load como na resposta anterior, com a correção do texto de confirmação)
    (void)playlist; 

    if (s_previous_screen_before_save_load == GAMESTATE_PAUSE) {
        DrawPauseScreen(players, musicVolume, currentMusicIndex, musicIsPlaying, mapX, mapY);
    } else if (s_previous_screen_before_save_load == GAMESTATE_PLAYER_MODE_MENU) {
        ClearBackground(DARKGRAY); 
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
    } else { 
        ClearBackground(DARKGRAY);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
    }
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f)); 

    if (!saveLoadSlotsInitialized) InitializeSaveLoadSlotsMenuButtons(GetScreenWidth(), GetScreenHeight());
    if (s_save_load_menu_sub_state == 1 && !s_confirmationButtonsInitialized) {
        InitializeSaveLoadConfirmationButtons(GetScreenWidth(), GetScreenHeight()); 
    }

    char title[128]; 
    if (s_is_new_game_flow) {
        sprintf(title, "SELECIONAR SLOT PARA NOVO JOGO");
    } else {
        sprintf(title, "ESCOLHA UM SLOT PARA %s", s_is_in_save_mode ? "SALVAR" : "CARREGAR");
    }
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
                        DrawText(slotText, (int)(onScreenSlotRect.x + 10), (int)(onScreenSlotRect.y + (onScreenSlotRect.height - 20)/2), 20, WHITE);
                    } else {
                        sprintf(slotText, "Slot %d - Vazio", i + 1);
                        DrawText(slotText, (int)(onScreenSlotRect.x + 10), (int)(onScreenSlotRect.y + (onScreenSlotRect.height - 20)/2), 20, LIGHTGRAY);
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
            if (maxScroll > 0) thumbY += (-saveLoadScrollOffset / maxScroll) * (scrollBarArea.height - thumbHeight);
            DrawRectangle((int)scrollBarArea.x, (int)thumbY, (int)scrollBarArea.width, (int)thumbHeight, DARKGRAY);
        }
        DrawText("ESC para Voltar | Roda do Mouse para Scroll", 10, GetScreenHeight() - 20, 10, WHITE);

    } else if (s_save_load_menu_sub_state == 1) { 
        float boxWidth = s_is_new_game_flow ? 550 : 450; 
        float boxHeight = s_is_new_game_flow ? 180 : 150; 
        Rectangle confBoxRect = {(GetScreenWidth() - boxWidth)/2.0f, (GetScreenHeight() - boxHeight)/2.0f, boxWidth, boxHeight};
        DrawRectangleRec(confBoxRect, Fade(BLACK, 0.95f)); 
        DrawRectangleLinesEx(confBoxRect, 2, WHITE);
        
        float textY = confBoxRect.y + 20;
        float textPadding = 15; 

        if (s_is_new_game_flow) {
            char line1[128];
            char line2[128];
            char line3[128];
            sprintf(line1, "Slot %d contém dados salvos.", s_selected_slot_for_action + 1);
            sprintf(line2, "Iniciar novo jogo aqui?"); 
            sprintf(line3, "(O jogo anterior será perdido se não salvo em outro slot)");

            DrawText(line1, (int)(confBoxRect.x + (boxWidth - MeasureText(line1, 20)) / 2.0f), (int)textY, 20, WHITE);
            textY += 25;
            DrawText(line2, (int)(confBoxRect.x + (boxWidth - MeasureText(line2, 20)) / 2.0f), (int)textY, 20, WHITE);
            textY += 25 + 5; 
            DrawText(line3, (int)(confBoxRect.x + (boxWidth - MeasureText(line3, 10)) / 2.0f), (int)textY, 10, LIGHTGRAY);

        } else { 
            char confText[128];
            sprintf(confText, "Você quer sobrescrever o Save %d?", s_selected_slot_for_action + 1);
            DrawText(confText, (int)(confBoxRect.x + (boxWidth - MeasureText(confText, 20)) / 2.0f), (int)(confBoxRect.y + (boxHeight - s_confirmationButtons[0].rect.height - 20 - 20 - 10) / 2.0f), 20, WHITE); 
        }

        for (int i = 0; i < 2; i++) { 
            Color btnColor = s_confirmationButtons[i].is_hovered ? ORANGE : MAROON;
            DrawRectangleRec(s_confirmationButtons[i].rect, btnColor); 
            DrawText(s_confirmationButtons[i].text,
                     (int)(s_confirmationButtons[i].rect.x + (s_confirmationButtons[i].rect.width - MeasureText(s_confirmationButtons[i].text, 20)) / 2.0f),
                     (int)(s_confirmationButtons[i].rect.y + (s_confirmationButtons[i].rect.height - 20) / 2.0f), 20, WHITE);
        }
        DrawText("ESC para Cancelar", (int)(confBoxRect.x + textPadding), (int)(confBoxRect.y + confBoxRect.height - 20), 10, GRAY);
    }
}