#include "../include/Settings.h"
#include "../include/Sound.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>

// --- Variáveis Globais Externas de Volume (definidas em main.c) ---
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;

extern float masterVolume;
extern float mainMenuMusicVolume;
extern float gameplayMusicVolume;
extern float battleMusicVolume;
extern float cutsceneMusicVolume;
extern float ambientNatureVolume;
extern float ambientCityVolume;
extern float ambientCaveVolume;
extern float sfxVolume;

// --- Estado Interno e UI ---
static GameState screenToReturnTo = GAMESTATE_MENU;
static bool settingsScreenInitialized = false;
typedef enum { SETTINGS_TAB_AUDIO, SETTINGS_TAB_KEYBINDS, SETTINGS_TAB_PERSONALIZATION, SETTINGS_TAB_COUNT } SettingsTabType;
static SettingsTabType currentSettingsTab = SETTINGS_TAB_AUDIO;
static const char* settingsTabNames[SETTINGS_TAB_COUNT] = {"AUDIO", "ATALHOS", "PERSONALIZACAO"};
static Rectangle settingsTabRects[SETTINGS_TAB_COUNT];
static const char* titleSettings = "CONFIGURACOES";

// Sliders
static Rectangle masterVolumeSliderArea;
static Rectangle masterVolumeKnob;
static bool masterVolumeKnobDragging;

static Rectangle mainMenuMusicVolumeSliderArea, gameplayMusicVolumeSliderArea, battleMusicVolumeSliderArea, cutsceneMusicVolumeSliderArea,
                 ambientNatureVolumeSliderArea, ambientCityVolumeSliderArea, ambientCaveVolumeSliderArea, sfxVolumeSliderArea;
static Rectangle mainMenuMusicVolumeKnob, gameplayMusicVolumeKnob, battleMusicVolumeKnob, cutsceneMusicVolumeKnob,
                 ambientNatureVolumeKnob, ambientCityVolumeKnob, ambientCaveVolumeKnob, sfxVolumeKnob;
static bool mainMenuMusicKnobDragging, gameplayMusicKnobDragging, battleMusicKnobDragging, cutsceneMusicKnobDragging,
            ambientNatureKnobDragging, ambientCityKnobDragging, ambientCaveKnobDragging, sfxKnobDragging;

static Rectangle backButtonRect; static const char* backButtonText = "Voltar"; static bool backButtonHover = false;

#define SLIDER_WIDTH_SETTINGS 200
#define SLIDER_HEIGHT_SETTINGS 18
#define KNOB_WIDTH_SETTINGS 10
#define KNOB_HEIGHT_SETTINGS 28
#define V_SPACING_SLIDER_GROUP_SETTINGS 28
#define TEXT_FONT_SIZE_SETTINGS 18
#define TITLE_FONT_SIZE_SETTINGS 30
#define TAB_PADDING_SETTINGS 10
#define TAB_HEIGHT_SETTINGS 30

typedef struct { const char* actionName; const char* keyAssigned; } KeybindDisplayInfo;
static KeybindDisplayInfo keybindsToDisplay[] = {
    {"Mover Cima (P1):", "W"}, {"Mover Baixo (P1):", "S"}, {"Mover Esquerda (P1):", "A"}, {"Mover Direita (P1):", "D"},
    {"Correr (P1):", "Shift Esquerdo"}, {"Mover Cima (P2):", "Seta Cima"}, {"Mover Baixo (P2):", "Seta Baixo"},
    {"Mover Esquerda (P2):", "Seta Esquerda"}, {"Mover Direita (P2):", "Seta Direita"}, {"Correr (P2):", "Shift Direito"},
    {"Pausar Jogo:", "P / Esc"}, {"Abrir Inventario:", "E"},
};
static int numKeybindsToDisplay = sizeof(keybindsToDisplay) / sizeof(keybindsToDisplay[0]);
static float keybindScrollOffset = 0.0f; static float keybindTotalContentHeight = 0.0f;
static Rectangle keybindViewAreaRect; static float keybindScrollSpeedFactor = 3.0f;

// --- Funções Auxiliares Estáticas ---
static void Helper_SetupSliderGeometry(Rectangle* area, Rectangle* knob, float yPos, int sliderStartX) {
    *area = (Rectangle){ (float)sliderStartX, yPos, (float)SLIDER_WIDTH_SETTINGS, (float)SLIDER_HEIGHT_SETTINGS };
    knob->width = KNOB_WIDTH_SETTINGS; knob->height = KNOB_HEIGHT_SETTINGS;
    knob->y = area->y + area->height / 2.0f - knob->height / 2.0f;
}

static void Helper_UpdateVolumeSlider(Vector2 mousePoint, Rectangle sliderArea, bool* knobIsDragging, float* targetVolumeVariable, bool isMusicType, bool isMasterVolume) {
    if (CheckCollisionPointRec(mousePoint, sliderArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { *knobIsDragging = true; }
    if (*knobIsDragging) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            *targetVolumeVariable = (mousePoint.x - sliderArea.x) / sliderArea.width;
            if (*targetVolumeVariable < 0.0f) *targetVolumeVariable = 0.0f;
            else if (*targetVolumeVariable > 1.0f) *targetVolumeVariable = 1.0f;

            if (isMusicType || isMasterVolume) UpdateCurrentlyPlayingMusicVolume();
            if (!isMusicType || isMasterVolume) ApplySfxVolume();
        } else { *knobIsDragging = false; }
    }
}

static void Helper_DrawVolumeSlider(const char* labelText, Rectangle sliderArea, Rectangle* knobWidget, bool knobIsBeingDragged, float currentVolumeValue, int labelStartX) {
    char volTextBuffer[64]; sprintf(volTextBuffer, "%s: %d%%", labelText, (int)(currentVolumeValue * 100.0f));
    DrawText(volTextBuffer, labelStartX, (int)(sliderArea.y + sliderArea.height / 2.0f - (float)TEXT_FONT_SIZE_SETTINGS / 2.0f), TEXT_FONT_SIZE_SETTINGS, WHITE);
    DrawRectangleRec(sliderArea, LIGHTGRAY); DrawRectangleLinesEx(sliderArea, 1, DARKGRAY);
    knobWidget->x = sliderArea.x + currentVolumeValue * (sliderArea.width - knobWidget->width);
    DrawRectangleRec(*knobWidget, knobIsBeingDragged ? ORANGE : MAROON);
}

void InitializeSettingsScreen(GameState previousScreen) {
    screenToReturnTo = previousScreen; currentSettingsTab = SETTINGS_TAB_AUDIO;
    int topMarginForTitle = 40; int titleHeight = TITLE_FONT_SIZE_SETTINGS;
    int topMarginForTabs = topMarginForTitle + titleHeight + 20;
    int tabTotalWidth = 0;
    for(int i=0; i < SETTINGS_TAB_COUNT; i++) { tabTotalWidth += MeasureText(settingsTabNames[i], TEXT_FONT_SIZE_SETTINGS) + (TAB_PADDING_SETTINGS * 2); }
    tabTotalWidth += TAB_PADDING_SETTINGS * (SETTINGS_TAB_COUNT - 1);
    int tabStartX = (virtualScreenWidth - tabTotalWidth) / 2;
    for(int i=0; i < SETTINGS_TAB_COUNT; i++) {
        int tabWidth = MeasureText(settingsTabNames[i], TEXT_FONT_SIZE_SETTINGS) + (TAB_PADDING_SETTINGS * 2);
        settingsTabRects[i] = (Rectangle){ (float)tabStartX, (float)topMarginForTabs, (float)tabWidth, (float)TAB_HEIGHT_SETTINGS };
        tabStartX += tabWidth + TAB_PADDING_SETTINGS;
    }

    int contentStartY = topMarginForTabs + TAB_HEIGHT_SETTINGS + 20;
    int audioLabelStartX = virtualScreenWidth / 2 - 280;
    int audioSliderStartX = audioLabelStartX + 300;
    float currentSliderY = (float)contentStartY;

    Helper_SetupSliderGeometry(&masterVolumeSliderArea, &masterVolumeKnob, currentSliderY, audioSliderStartX);
    currentSliderY += (float)V_SPACING_SLIDER_GROUP_SETTINGS;

    Helper_SetupSliderGeometry(&mainMenuMusicVolumeSliderArea, &mainMenuMusicVolumeKnob, currentSliderY, audioSliderStartX);
    currentSliderY += (float)V_SPACING_SLIDER_GROUP_SETTINGS;
    Helper_SetupSliderGeometry(&gameplayMusicVolumeSliderArea, &gameplayMusicVolumeKnob, currentSliderY, audioSliderStartX);
    currentSliderY += (float)V_SPACING_SLIDER_GROUP_SETTINGS;
    Helper_SetupSliderGeometry(&battleMusicVolumeSliderArea, &battleMusicVolumeKnob, currentSliderY, audioSliderStartX);
    currentSliderY += (float)V_SPACING_SLIDER_GROUP_SETTINGS;
    Helper_SetupSliderGeometry(&cutsceneMusicVolumeSliderArea, &cutsceneMusicVolumeKnob, currentSliderY, audioSliderStartX);
    currentSliderY += (float)V_SPACING_SLIDER_GROUP_SETTINGS;
    Helper_SetupSliderGeometry(&ambientNatureVolumeSliderArea, &ambientNatureVolumeKnob, currentSliderY, audioSliderStartX);
    currentSliderY += (float)V_SPACING_SLIDER_GROUP_SETTINGS;
    Helper_SetupSliderGeometry(&ambientCityVolumeSliderArea, &ambientCityVolumeKnob, currentSliderY, audioSliderStartX);
    currentSliderY += (float)V_SPACING_SLIDER_GROUP_SETTINGS;
    Helper_SetupSliderGeometry(&ambientCaveVolumeSliderArea, &ambientCaveVolumeKnob, currentSliderY, audioSliderStartX);
    currentSliderY += (float)V_SPACING_SLIDER_GROUP_SETTINGS;
    Helper_SetupSliderGeometry(&sfxVolumeSliderArea, &sfxVolumeKnob, currentSliderY, audioSliderStartX);

    int keybindListTitleY = contentStartY;
    int keybindListStartY = keybindListTitleY + TEXT_FONT_SIZE_SETTINGS + V_SPACING_SLIDER_GROUP_SETTINGS / 2;
    int keybindListX = virtualScreenWidth / 2 - 200; int keybindListWidth = 400;
    keybindViewAreaRect = (Rectangle){ (float)keybindListX - (float)TAB_PADDING_SETTINGS, (float)keybindListStartY, (float)keybindListWidth + (float)(2 * TAB_PADDING_SETTINGS), (float)virtualScreenHeight - (float)keybindListStartY - 60.0f };
    keybindTotalContentHeight = (float)numKeybindsToDisplay * ((float)TEXT_FONT_SIZE_SETTINGS + 5.0f);
    if (numKeybindsToDisplay > 0) keybindTotalContentHeight -= 5.0f;
    keybindScrollOffset = 0.0f;

    int backButtonWidth = 150; int backButtonHeight = 40;
    float bottomOfContent = sfxVolumeSliderArea.y + SLIDER_HEIGHT_SETTINGS + 10;
    float backButtonY = fmaxf(bottomOfContent, (float)virtualScreenHeight - 70.0f);
    if (backButtonY + (float)backButtonHeight > (float)virtualScreenHeight - 10) backButtonY = (float)virtualScreenHeight - 10 - (float)backButtonHeight;

    backButtonRect = (Rectangle){ (float)((float)virtualScreenWidth / 2 - (float)backButtonWidth / 2), backButtonY, (float)backButtonWidth, (float)backButtonHeight };

    masterVolumeKnobDragging = false;
    mainMenuMusicKnobDragging = gameplayMusicKnobDragging = battleMusicKnobDragging = cutsceneMusicKnobDragging = false;
    ambientNatureKnobDragging = ambientCityKnobDragging = ambientCaveKnobDragging = false;
    sfxKnobDragging = false;
    settingsScreenInitialized = true;
}

void UpdateSettingsScreen(GameState *currentScreen_ptr) {
    if (!currentScreen_ptr) return;
    if (!settingsScreenInitialized) { InitializeSettingsScreen(screenToReturnTo); }
    Vector2 mousePoint = GetMousePosition();
    for (int i = 0; i < SETTINGS_TAB_COUNT; i++) {
        if (CheckCollisionPointRec(mousePoint, settingsTabRects[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (currentSettingsTab != (SettingsTabType)i) { keybindScrollOffset = 0.0f; }
            currentSettingsTab = (SettingsTabType)i; break;
        }
    }

    if (currentSettingsTab == SETTINGS_TAB_AUDIO) {
        Helper_UpdateVolumeSlider(mousePoint, masterVolumeSliderArea, &masterVolumeKnobDragging, &masterVolume, true, true);
        Helper_UpdateVolumeSlider(mousePoint, mainMenuMusicVolumeSliderArea, &mainMenuMusicKnobDragging, &mainMenuMusicVolume, true, false);
        Helper_UpdateVolumeSlider(mousePoint, gameplayMusicVolumeSliderArea, &gameplayMusicKnobDragging, &gameplayMusicVolume, true, false);
        Helper_UpdateVolumeSlider(mousePoint, battleMusicVolumeSliderArea, &battleMusicKnobDragging, &battleMusicVolume, true, false);
        Helper_UpdateVolumeSlider(mousePoint, cutsceneMusicVolumeSliderArea, &cutsceneMusicKnobDragging, &cutsceneMusicVolume, true, false);
        Helper_UpdateVolumeSlider(mousePoint, ambientNatureVolumeSliderArea, &ambientNatureKnobDragging, &ambientNatureVolume, true, false);
        Helper_UpdateVolumeSlider(mousePoint, ambientCityVolumeSliderArea, &ambientCityKnobDragging, &ambientCityVolume, true, false);
        Helper_UpdateVolumeSlider(mousePoint, ambientCaveVolumeSliderArea, &ambientCaveKnobDragging, &ambientCaveVolume, true, false);
        Helper_UpdateVolumeSlider(mousePoint, sfxVolumeSliderArea, &sfxKnobDragging, &sfxVolume, false, false);
    } else if (currentSettingsTab == SETTINGS_TAB_KEYBINDS) {
        if (CheckCollisionPointRec(mousePoint, keybindViewAreaRect)) {
            float wheelMove = GetMouseWheelMove();
            if (wheelMove != 0.0f) {
                keybindScrollOffset += wheelMove * ((float)TEXT_FONT_SIZE_SETTINGS + 5.0f) * keybindScrollSpeedFactor;
                float maxScroll = 0.0f;
                if (keybindTotalContentHeight > keybindViewAreaRect.height) { maxScroll = keybindTotalContentHeight - keybindViewAreaRect.height; }
                if (keybindScrollOffset > 0.0f) keybindScrollOffset = 0.0f;
                if (maxScroll > 0.0f) { if (keybindScrollOffset < -maxScroll) keybindScrollOffset = -maxScroll; }
                else { keybindScrollOffset = 0.0f; }
            }
        }
    }
    backButtonHover = CheckCollisionPointRec(mousePoint, backButtonRect);
    if (backButtonHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { *currentScreen_ptr = screenToReturnTo; settingsScreenInitialized = false; }
    if (IsKeyPressed(KEY_ESCAPE)) { *currentScreen_ptr = screenToReturnTo; settingsScreenInitialized = false; }
}

void DrawSettingsScreen(void) {
    if (!settingsScreenInitialized) { InitializeSettingsScreen(screenToReturnTo); }
    ClearBackground(DARKGRAY);
    int topY = 40;
    DrawText(titleSettings, (int)(((float)virtualScreenWidth - (float)MeasureText(titleSettings, TITLE_FONT_SIZE_SETTINGS)) / 2.0f), topY, TITLE_FONT_SIZE_SETTINGS, WHITE);
    topY += TITLE_FONT_SIZE_SETTINGS + 20;
    for (int i = 0; i < SETTINGS_TAB_COUNT; i++) {
        Color tabFgColor = (currentSettingsTab == (SettingsTabType)i) ? ORANGE : LIGHTGRAY;
        Color tabBgColor = (currentSettingsTab == (SettingsTabType)i) ? Fade(GRAY, 0.7f) : Fade(DARKGRAY, 0.9f);
        DrawRectangleRec(settingsTabRects[i], tabBgColor );
        DrawText(settingsTabNames[i], (int)(settingsTabRects[i].x + (settingsTabRects[i].width - (float)MeasureText(settingsTabNames[i], TEXT_FONT_SIZE_SETTINGS)) / 2.0f), (int)(settingsTabRects[i].y + (settingsTabRects[i].height - (float)TEXT_FONT_SIZE_SETTINGS) / 2.0f), TEXT_FONT_SIZE_SETTINGS, tabFgColor);
        DrawRectangleLinesEx(settingsTabRects[i], 1, (currentSettingsTab == (SettingsTabType)i) ? ORANGE : GRAY);
    }

    int contentStartY = topY + TAB_HEIGHT_SETTINGS + 20;
    int audioLabelStartX = virtualScreenWidth / 2 - 280;

    if (currentSettingsTab == SETTINGS_TAB_AUDIO) {
        // *** CORRIGIDO: A variável 'currentLabelY' não utilizada foi removida. O layout agora é controlado pelos retângulos dos sliders. ***
        Helper_DrawVolumeSlider("Volume Principal", masterVolumeSliderArea, &masterVolumeKnob, masterVolumeKnobDragging, masterVolume, audioLabelStartX);
        Helper_DrawVolumeSlider("Menu Principal", mainMenuMusicVolumeSliderArea, &mainMenuMusicVolumeKnob, mainMenuMusicKnobDragging, mainMenuMusicVolume, audioLabelStartX);
        Helper_DrawVolumeSlider("Jogo Geral", gameplayMusicVolumeSliderArea, &gameplayMusicVolumeKnob, gameplayMusicKnobDragging, gameplayMusicVolume, audioLabelStartX);
        Helper_DrawVolumeSlider("Batalha", battleMusicVolumeSliderArea, &battleMusicVolumeKnob, battleMusicKnobDragging, battleMusicVolume, audioLabelStartX);
        Helper_DrawVolumeSlider("Cutscene", cutsceneMusicVolumeSliderArea, &cutsceneMusicVolumeKnob, cutsceneMusicKnobDragging, cutsceneMusicVolume, audioLabelStartX);
        Helper_DrawVolumeSlider("Amb. Natureza", ambientNatureVolumeSliderArea, &ambientNatureVolumeKnob, ambientNatureKnobDragging, ambientNatureVolume, audioLabelStartX);
        Helper_DrawVolumeSlider("Amb. Cidade", ambientCityVolumeSliderArea, &ambientCityVolumeKnob, ambientCityKnobDragging, ambientCityVolume, audioLabelStartX);
        Helper_DrawVolumeSlider("Amb. Caverna", ambientCaveVolumeSliderArea, &ambientCaveVolumeKnob, ambientCaveKnobDragging, ambientCaveVolume, audioLabelStartX);
        Helper_DrawVolumeSlider("Efeitos Sonoros", sfxVolumeSliderArea, &sfxVolumeKnob, sfxKnobDragging, sfxVolume, audioLabelStartX);

    } else if (currentSettingsTab == SETTINGS_TAB_KEYBINDS) {
        int keybindTitleY = contentStartY;
        DrawText("Atalhos Atuais (Nao Editaveis Ainda):", (int)(((float)virtualScreenWidth - (float)MeasureText("Atalhos Atuais (Nao Editaveis Ainda):", TEXT_FONT_SIZE_SETTINGS)) / 2.0f), keybindTitleY, TEXT_FONT_SIZE_SETTINGS, RAYWHITE);
        DrawRectangleLinesEx(keybindViewAreaRect, 1, GRAY);
        BeginScissorMode((int)keybindViewAreaRect.x, (int)keybindViewAreaRect.y, (int)keybindViewAreaRect.width, (int)keybindViewAreaRect.height);
            int keybindActionX = (int)(keybindViewAreaRect.x + (float)TAB_PADDING_SETTINGS);
            int keybindKeyX = (int)(keybindViewAreaRect.x + keybindViewAreaRect.width / 2.0f + (float)TAB_PADDING_SETTINGS);
            float currentItemDrawY = keybindViewAreaRect.y + keybindScrollOffset;
            for (int i = 0; i < numKeybindsToDisplay; i++) {
                if (currentItemDrawY + (float)TEXT_FONT_SIZE_SETTINGS >= keybindViewAreaRect.y && currentItemDrawY <= keybindViewAreaRect.y + keybindViewAreaRect.height) {
                     DrawText(keybindsToDisplay[i].actionName, keybindActionX, (int)currentItemDrawY, TEXT_FONT_SIZE_SETTINGS, WHITE);
                     DrawText(keybindsToDisplay[i].keyAssigned, keybindKeyX, (int)currentItemDrawY, TEXT_FONT_SIZE_SETTINGS, YELLOW);
                }
                currentItemDrawY += (float)TEXT_FONT_SIZE_SETTINGS + 5.0f;
                 if (currentItemDrawY > keybindViewAreaRect.y + keybindViewAreaRect.height && keybindScrollOffset == 0) break;
            }
        EndScissorMode();
        if (keybindTotalContentHeight > keybindViewAreaRect.height) {
            Rectangle scrollBarBg = { keybindViewAreaRect.x + keybindViewAreaRect.width + 2.0f, keybindViewAreaRect.y, 8.0f, keybindViewAreaRect.height }; DrawRectangleRec(scrollBarBg, LIGHTGRAY);
            float scrollPercentage = 0.0f; if (keybindTotalContentHeight - keybindViewAreaRect.height > 0.001f) { scrollPercentage = -keybindScrollOffset / (keybindTotalContentHeight - keybindViewAreaRect.height); }
            float scrollThumbHeight = fmaxf(20.0f, (keybindViewAreaRect.height / keybindTotalContentHeight) * scrollBarBg.height);
            Rectangle scrollThumb = { scrollBarBg.x, scrollBarBg.y + scrollPercentage * (scrollBarBg.height - scrollThumbHeight), scrollBarBg.width, scrollThumbHeight }; DrawRectangleRec(scrollThumb, DARKGRAY);
        }
    } else if (currentSettingsTab == SETTINGS_TAB_PERSONALIZATION) {
        DrawText("Opcoes de Personalizacao", (int)(((float)virtualScreenWidth - (float)MeasureText("Opcoes de Personalizacao", TEXT_FONT_SIZE_SETTINGS)) / 2.0f), contentStartY, TEXT_FONT_SIZE_SETTINGS, RAYWHITE);
        DrawText("Em breve...", (int)(((float)virtualScreenWidth - (float)MeasureText("Em breve...", TEXT_FONT_SIZE_SETTINGS)) / 2.0f), contentStartY + TEXT_FONT_SIZE_SETTINGS + 20, TEXT_FONT_SIZE_SETTINGS, GRAY);
    }

    Color backBtnBg = backButtonHover ? ORANGE : MAROON;
    DrawRectangleRec(backButtonRect, backBtnBg);
    DrawText(backButtonText, (int)(backButtonRect.x + (backButtonRect.width - (float)MeasureText(backButtonText, TEXT_FONT_SIZE_SETTINGS)) / 2.0f), (int)(backButtonRect.y + (backButtonRect.height - (float)TEXT_FONT_SIZE_SETTINGS) / 2.0f), TEXT_FONT_SIZE_SETTINGS, WHITE);
    DrawRectangleLinesEx(backButtonRect, 2, Fade(BLACK, 0.5f));
    DrawText("ESC para Voltar", 20, virtualScreenHeight - 30, 10, GRAY);
}