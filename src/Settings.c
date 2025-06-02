#include "../include/Settings.h"
#include "raylib.h"
#include <stdio.h>  // For sprintf
#include <math.h>   // For fmaxf

// Variáveis globais de main.c que serão modificadas/lidas
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern float musicVolume; 
extern float sfxVolume;   

// Estado interno da tela de Configurações
static GameState screenToReturnTo = GAMESTATE_MENU; 
static bool settingsScreenInitialized = false;

// Abas da Tela de Configurações
typedef enum {
    SETTINGS_TAB_AUDIO,
    SETTINGS_TAB_KEYBINDS,
    SETTINGS_TAB_PERSONALIZATION,
    SETTINGS_TAB_COUNT
} SettingsTabType;

static SettingsTabType currentSettingsTab = SETTINGS_TAB_AUDIO;
static const char* settingsTabNames[SETTINGS_TAB_COUNT] = {"AUDIO", "ATALHOS", "PERSONALIZACAO"};
static Rectangle settingsTabRects[SETTINGS_TAB_COUNT];

// --- Elementos da UI de Configurações ---
static const char* titleSettings = "CONFIGURACOES";

// Sliders de Som
static Rectangle musicVolumeSliderArea;
static Rectangle musicVolumeKnob;
static bool musicKnobDragging = false;

static Rectangle sfxVolumeSliderArea;
static Rectangle sfxVolumeKnob;
static bool sfxKnobDragging = false;

// Botão Voltar
static Rectangle backButtonRect;
static const char* backButtonText = "Voltar";
static bool backButtonHover = false;

// Dimensões e espaçamentos comuns
#define SLIDER_WIDTH 250 
#define SLIDER_HEIGHT 20
#define KNOB_WIDTH 10
#define KNOB_HEIGHT 30
#define V_SPACING_LABEL_SLIDER 25  
#define V_SPACING_SECTION_TITLE 30 
#define V_SPACING_SECTION 50       
#define SECTION_START_Y_OFFSET 60  
#define TEXT_FONT_SIZE 20
#define TITLE_FONT_SIZE 30
#define SUBTITLE_FONT_SIZE 24 
#define KEYBIND_ITEM_FONT_SIZE 18
#define KEYBIND_ITEM_SPACING 5 
#define TAB_PADDING 10          // << DEFINIDO
#define TAB_HEIGHT 30           // << DEFINIDO

typedef struct {
    const char* actionName;
    const char* keyAssigned;
} KeybindDisplayInfo;

static KeybindDisplayInfo keybindsToDisplay[] = {
    {"Mover Cima (P1):", "W"}, {"Mover Baixo (P1):", "S"},
    {"Mover Esquerda (P1):", "A"}, {"Mover Direita (P1):", "D"},
    {"Correr (P1):", "Shift Direito"},
    {"Mover Cima (P2):", "Seta Cima"}, {"Mover Baixo (P2):", "Seta Baixo"},
    {"Mover Esquerda (P2):", "Seta Esquerda"}, {"Mover Direita (P2):", "Seta Direita"},
    {"Correr (P2):", "Shift Esquerdo"},
    {"Pausar Jogo:", "P / Esc"}, {"Abrir Inventario:", "E"},
    {"Acao Especial 1:", "Q"}, {"Acao Especial 2:", "R"},
    {"Proxima Musica:", "PageDown"}, {"Musica Anterior:", "PageUp"},
    {"Volume +:", "+ / Num+"}, {"Volume -:", "- / Num-"},
};
static int numKeybindsToDisplay = sizeof(keybindsToDisplay) / sizeof(keybindsToDisplay[0]);

static float keybindScrollOffset = 0.0f;
static float keybindTotalContentHeight = 0.0f;
static Rectangle keybindViewAreaRect;
static float keybindScrollSpeedFactor = 3.0f;


void InitializeSettingsScreen(GameState previousScreen) {
    screenToReturnTo = previousScreen; 
    currentSettingsTab = SETTINGS_TAB_AUDIO; 

    int topMargin = 40 + TITLE_FONT_SIZE + 20; 
    
    int tabTotalWidth = 0;
    for(int i=0; i < SETTINGS_TAB_COUNT; i++) {
        tabTotalWidth += MeasureText(settingsTabNames[i], TEXT_FONT_SIZE) + (TAB_PADDING * 2);
    }
    tabTotalWidth += TAB_PADDING * (SETTINGS_TAB_COUNT -1); 

    int tabStartX = (virtualScreenWidth - tabTotalWidth) / 2;
    for(int i=0; i < SETTINGS_TAB_COUNT; i++) {
        int tabWidth = MeasureText(settingsTabNames[i], TEXT_FONT_SIZE) + (TAB_PADDING * 2);
        settingsTabRects[i] = (Rectangle){ (float)tabStartX, (float)topMargin, (float)tabWidth, (float)TAB_HEIGHT };
        tabStartX += tabWidth + TAB_PADDING;
    }

    int contentStartY = topMargin + TAB_HEIGHT + 30; 
    int audioLabelStartX = virtualScreenWidth / 2 - 220; // Posição X para labels de áudio
    int audioSliderStartX = audioLabelStartX + 200;    // Posição X para os sliders de áudio

    musicVolumeSliderArea = (Rectangle){ (float)audioSliderStartX, (float)contentStartY, (float)SLIDER_WIDTH, (float)SLIDER_HEIGHT };
    musicVolumeKnob.width = KNOB_WIDTH; musicVolumeKnob.height = KNOB_HEIGHT;
    musicVolumeKnob.y = musicVolumeSliderArea.y + musicVolumeSliderArea.height / 2.0f - musicVolumeKnob.height / 2.0f;
    
    sfxVolumeSliderArea = (Rectangle){ (float)audioSliderStartX, musicVolumeSliderArea.y + musicVolumeSliderArea.height + (float)V_SPACING_LABEL_SLIDER, (float)SLIDER_WIDTH, (float)SLIDER_HEIGHT };
    sfxVolumeKnob.width = KNOB_WIDTH; sfxVolumeKnob.height = KNOB_HEIGHT;
    sfxVolumeKnob.y = sfxVolumeSliderArea.y + sfxVolumeSliderArea.height / 2.0f - sfxVolumeKnob.height / 2.0f;

    int keybindListTitleY = contentStartY; // Y do título "Atalhos Atuais..."
    int keybindListStartY = keybindListTitleY + TEXT_FONT_SIZE + V_SPACING_LABEL_SLIDER / 2; // Y onde a lista começa
    int keybindListX = virtualScreenWidth / 2 - 200; 
    int keybindListWidth = 400; 
    
    keybindViewAreaRect = (Rectangle){
        (float)keybindListX - (float)TAB_PADDING, // Adiciona um pequeno padding lateral à área de visão
        (float)keybindListStartY, 
        (float)keybindListWidth + (float)(2 * TAB_PADDING), 
        (float)virtualScreenHeight - (float)keybindListStartY - 80.0f 
    };
    keybindTotalContentHeight = (float)numKeybindsToDisplay * ((float)KEYBIND_ITEM_FONT_SIZE + (float)KEYBIND_ITEM_SPACING);
    if (numKeybindsToDisplay > 0) keybindTotalContentHeight -= (float)KEYBIND_ITEM_SPACING;
    keybindScrollOffset = 0.0f;

    int backButtonWidth = 150; int backButtonHeight = 40;
    backButtonRect = (Rectangle){ 
        (float)(virtualScreenWidth / 2 - backButtonWidth / 2), 
        (float)virtualScreenHeight - 70.0f, 
        (float)backButtonWidth, (float)backButtonHeight 
    };

    musicKnobDragging = false; sfxKnobDragging = false;
    settingsScreenInitialized = true;
}

void UpdateSettingsScreen(GameState *currentScreen_ptr) {
    if (!currentScreen_ptr) return;
    if (!settingsScreenInitialized) {
        InitializeSettingsScreen(screenToReturnTo); 
    }

    Vector2 mousePoint = GetMousePosition(); // Para UI que não está em render texture, GetMousePosition() é o correto.

    for (int i = 0; i < SETTINGS_TAB_COUNT; i++) {
        if (CheckCollisionPointRec(mousePoint, settingsTabRects[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (currentSettingsTab != (SettingsTabType)i) {
                 keybindScrollOffset = 0.0f; 
            }
            currentSettingsTab = (SettingsTabType)i;
            break; 
        }
    }

    if (currentSettingsTab == SETTINGS_TAB_AUDIO) {
        if (CheckCollisionPointRec(mousePoint, musicVolumeSliderArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) musicKnobDragging = true;
        if (musicKnobDragging) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                musicVolume = (mousePoint.x - musicVolumeSliderArea.x) / musicVolumeSliderArea.width;
                if (musicVolume < 0.0f) musicVolume = 0.0f; // Clamping
                else if (musicVolume > 1.0f) musicVolume = 1.0f; // Clamping
            } else { musicKnobDragging = false; }
        }
        if (CheckCollisionPointRec(mousePoint, sfxVolumeSliderArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) sfxKnobDragging = true;
        if (sfxKnobDragging) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                sfxVolume = (mousePoint.x - sfxVolumeSliderArea.x) / sfxVolumeSliderArea.width;
                if (sfxVolume < 0.0f) sfxVolume = 0.0f; // Clamping
                else if (sfxVolume > 1.0f) sfxVolume = 1.0f; // Clamping
            } else { sfxKnobDragging = false; }
        }
    } else if (currentSettingsTab == SETTINGS_TAB_KEYBINDS) {
        if (CheckCollisionPointRec(mousePoint, keybindViewAreaRect)) { // Scroll apenas se o mouse estiver sobre a área
            float wheelMove = GetMouseWheelMove();
            if (wheelMove != 0.0f) { 
                keybindScrollOffset += wheelMove * ((float)KEYBIND_ITEM_FONT_SIZE + (float)KEYBIND_ITEM_SPACING) * keybindScrollSpeedFactor;
                float maxScroll = 0.0f;
                if (keybindTotalContentHeight > keybindViewAreaRect.height) {
                    maxScroll = keybindTotalContentHeight - keybindViewAreaRect.height;
                }
                if (keybindScrollOffset > 0.0f) keybindScrollOffset = 0.0f;
                if (maxScroll > 0.0f) { 
                    if (keybindScrollOffset < -maxScroll) keybindScrollOffset = -maxScroll;
                } else { 
                     keybindScrollOffset = 0.0f;
                }
            }
        }
    }
    
    backButtonHover = CheckCollisionPointRec(mousePoint, backButtonRect);
    if (backButtonHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *currentScreen_ptr = screenToReturnTo; 
        settingsScreenInitialized = false; 
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = screenToReturnTo; 
        settingsScreenInitialized = false; 
    }
}

void DrawSettingsScreen(void) {
    if (!settingsScreenInitialized) { InitializeSettingsScreen(screenToReturnTo); }
    ClearBackground(DARKGRAY); 

    int topY = 40; 
    DrawText(titleSettings, (int)(((float)virtualScreenWidth - (float)MeasureText(titleSettings, TITLE_FONT_SIZE)) / 2.0f), topY, TITLE_FONT_SIZE, WHITE);
    topY += TITLE_FONT_SIZE + 20; 

    for (int i = 0; i < SETTINGS_TAB_COUNT; i++) {
        Color tabFgColor = (currentSettingsTab == (SettingsTabType)i) ? ORANGE : LIGHTGRAY; // Cast para silenciar -Wsign-compare
        Color tabBgColor = (currentSettingsTab == (SettingsTabType)i) ? Fade(GRAY, 0.7f) : Fade(DARKGRAY, 0.9f);
        DrawRectangleRec(settingsTabRects[i], tabBgColor );
        DrawText(settingsTabNames[i], 
                 (int)(settingsTabRects[i].x + (settingsTabRects[i].width - (float)MeasureText(settingsTabNames[i], TEXT_FONT_SIZE)) / 2.0f),
                 (int)(settingsTabRects[i].y + (settingsTabRects[i].height - (float)TEXT_FONT_SIZE) / 2.0f), 
                 TEXT_FONT_SIZE, tabFgColor);
        DrawRectangleLinesEx(settingsTabRects[i], 1, (currentSettingsTab == (SettingsTabType)i) ? ORANGE : GRAY); // Cast
    }
    
    int contentStartY = topY + TAB_HEIGHT + 30; 
    int audioLabelStartX = (int)((float)virtualScreenWidth / 2.0f - (float)SLIDER_WIDTH - 30.0f); // Alinha labels à esquerda dos sliders

    if (currentSettingsTab == SETTINGS_TAB_AUDIO) {
        char volText[32];
        sprintf(volText, "Volume da Musica: %d%%", (int)(musicVolume * 100.0f));
        DrawText(volText, audioLabelStartX, (int)(musicVolumeSliderArea.y + musicVolumeSliderArea.height / 2.0f - (float)TEXT_FONT_SIZE / 2.0f), TEXT_FONT_SIZE, WHITE);
        DrawRectangleRec(musicVolumeSliderArea, LIGHTGRAY); DrawRectangleLinesEx(musicVolumeSliderArea, 1, DARKGRAY);
        musicVolumeKnob.x = musicVolumeSliderArea.x + musicVolume * (musicVolumeSliderArea.width - musicVolumeKnob.width);
        DrawRectangleRec(musicVolumeKnob, musicKnobDragging ? ORANGE : MAROON);
        
        sprintf(volText, "Volume dos Efeitos: %d%%", (int)(sfxVolume * 100.0f));
        DrawText(volText, audioLabelStartX, (int)(sfxVolumeSliderArea.y + sfxVolumeSliderArea.height / 2.0f - (float)TEXT_FONT_SIZE / 2.0f), TEXT_FONT_SIZE, WHITE);
        DrawRectangleRec(sfxVolumeSliderArea, LIGHTGRAY); DrawRectangleLinesEx(sfxVolumeSliderArea, 1, DARKGRAY);
        sfxVolumeKnob.x = sfxVolumeSliderArea.x + sfxVolume * (sfxVolumeSliderArea.width - sfxVolumeKnob.width);
        DrawRectangleRec(sfxVolumeKnob, sfxKnobDragging ? ORANGE : MAROON);

    } else if (currentSettingsTab == SETTINGS_TAB_KEYBINDS) {
        int keybindTitleY = contentStartY;
        DrawText("Atalhos Atuais (Nao Editaveis Ainda):", (int)(((float)virtualScreenWidth - (float)MeasureText("Atalhos Atuais (Nao Editaveis Ainda):", TEXT_FONT_SIZE)) / 2.0f), keybindTitleY, TEXT_FONT_SIZE, RAYWHITE);
        
        // Usa keybindViewAreaRect.y que foi calculado em InitializeSettingsScreen
        DrawRectangleLinesEx(keybindViewAreaRect, 1, GRAY); 

        BeginScissorMode((int)keybindViewAreaRect.x, (int)keybindViewAreaRect.y, (int)keybindViewAreaRect.width, (int)keybindViewAreaRect.height);
            int keybindActionX = (int)(keybindViewAreaRect.x + (float)TAB_PADDING); 
            int keybindKeyX = (int)(keybindViewAreaRect.x + keybindViewAreaRect.width / 2.0f + (float)TAB_PADDING);  
            float currentItemDrawY = keybindScrollOffset; 

            for (int i = 0; i < numKeybindsToDisplay; i++) {
                float itemAbsoluteY = keybindViewAreaRect.y + currentItemDrawY; // Posição absoluta na tela para checagem de visibilidade
                if (itemAbsoluteY + (float)KEYBIND_ITEM_FONT_SIZE >= keybindViewAreaRect.y && itemAbsoluteY <= keybindViewAreaRect.y + keybindViewAreaRect.height) {
                     DrawText(keybindsToDisplay[i].actionName, keybindActionX, (int)itemAbsoluteY, KEYBIND_ITEM_FONT_SIZE, WHITE);
                     DrawText(keybindsToDisplay[i].keyAssigned, keybindKeyX, (int)itemAbsoluteY, KEYBIND_ITEM_FONT_SIZE, YELLOW);
                }
                currentItemDrawY += (float)KEYBIND_ITEM_FONT_SIZE + (float)KEYBIND_ITEM_SPACING;
            }
        EndScissorMode();

        if (keybindTotalContentHeight > keybindViewAreaRect.height) {
            Rectangle scrollBarBg = { keybindViewAreaRect.x + keybindViewAreaRect.width + 2.0f, keybindViewAreaRect.y, 8.0f, keybindViewAreaRect.height };
            DrawRectangleRec(scrollBarBg, LIGHTGRAY);
            float scrollPercentage = 0.0f;
            if (keybindTotalContentHeight - keybindViewAreaRect.height > 0.001f) { // Evita divisão por zero
                scrollPercentage = -keybindScrollOffset / (keybindTotalContentHeight - keybindViewAreaRect.height);
            }
            float scrollThumbHeight = fmaxf(20.0f, (keybindViewAreaRect.height / keybindTotalContentHeight) * scrollBarBg.height); // Usa fmaxf
            Rectangle scrollThumb = { scrollBarBg.x, scrollBarBg.y + scrollPercentage * (scrollBarBg.height - scrollThumbHeight), scrollBarBg.width, scrollThumbHeight };
            DrawRectangleRec(scrollThumb, DARKGRAY);
        }

    } else if (currentSettingsTab == SETTINGS_TAB_PERSONALIZATION) {
        DrawText("Opcoes de Personalizacao", (int)(((float)virtualScreenWidth - (float)MeasureText("Opcoes de Personalizacao", TEXT_FONT_SIZE)) / 2.0f), contentStartY, TEXT_FONT_SIZE, RAYWHITE);
        DrawText("Em breve...", (int)(((float)virtualScreenWidth - (float)MeasureText("Em breve...", TEXT_FONT_SIZE)) / 2.0f), contentStartY + TEXT_FONT_SIZE + 20, TEXT_FONT_SIZE, GRAY);
    }

    Color backBtnBg = backButtonHover ? ORANGE : MAROON;
    DrawRectangleRec(backButtonRect, backBtnBg);
    DrawText(backButtonText, 
             (int)(backButtonRect.x + (backButtonRect.width - (float)MeasureText(backButtonText, TEXT_FONT_SIZE)) / 2.0f), 
             (int)(backButtonRect.y + (backButtonRect.height - (float)TEXT_FONT_SIZE) / 2.0f), TEXT_FONT_SIZE, WHITE);
    DrawRectangleLinesEx(backButtonRect, 2, Fade(BLACK, 0.5f));
    DrawText("ESC para Voltar", 20, virtualScreenHeight - 30, 10, GRAY);
}