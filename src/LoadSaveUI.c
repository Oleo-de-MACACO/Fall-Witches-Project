#include "../include/LoadSaveUI.h"
#include "../include/Menu.h"           
#include "../include/SaveLoad.h"        
#include "../include/CharacterCreation.h" 
#include "../include/PauseMenu.h"       
#include "../include/Game.h"            
#include <stdio.h>                     
#include <stddef.h> 

extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
// extern GameModeType currentGameMode; // Não precisa extern aqui, usa Menu_GetSaveLoadContextMode()
extern int currentActivePlayers;    

#define SLOT_PADDING_SAVE_LOAD 10 
#define SLOT_HEIGHT_SAVE_LOAD 70  

static MenuButton saveLoadSlotButtons[MAX_SAVE_SLOTS]; 
static bool saveLoadSlotsInitialized = false;       
static float saveLoadScrollOffset = 0.0f;           
static float saveLoadTotalContentHeight = 0.0f;     
static Rectangle saveLoadSlotsViewArea = {0.0f, 0.0f, 0.0f, 0.0f}; 
static MenuButton s_confirmationButtons[2]; 
static bool s_confirmationButtonsInitialized = false;

static void InitializeSaveLoadSlotsMenuButtons() {
    float panelPadding = 20.0f;
    float topOffsetForTitle = 70.0f; 
    float bottomOffsetForEsc = 30.0f;
    saveLoadSlotsViewArea = (Rectangle){ panelPadding, topOffsetForTitle, (float)virtualScreenWidth - 2.0f * panelPadding, (float)virtualScreenHeight - topOffsetForTitle - bottomOffsetForEsc };
    float slotWidth = saveLoadSlotsViewArea.width - 2.0f * (float)SLOT_PADDING_SAVE_LOAD;
    saveLoadTotalContentHeight = 0.0f; 
    for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
        saveLoadSlotButtons[i] = (MenuButton){ {(float)SLOT_PADDING_SAVE_LOAD, (float)i * ((float)SLOT_HEIGHT_SAVE_LOAD + (float)SLOT_PADDING_SAVE_LOAD), slotWidth, (float)SLOT_HEIGHT_SAVE_LOAD}, "", DARKGRAY, GRAY, DARKGRAY, WHITE, true, false, BUTTON_ACTION_NONE };
        if (i < MAX_SAVE_SLOTS -1) saveLoadTotalContentHeight += (float)SLOT_HEIGHT_SAVE_LOAD + (float)SLOT_PADDING_SAVE_LOAD;
        else saveLoadTotalContentHeight += (float)SLOT_HEIGHT_SAVE_LOAD; 
    }
    saveLoadSlotsInitialized = true;
}

static void InitializeSaveLoadConfirmationButtons() {
    float btnWidth = 120.0f; float btnHeight = 40.0f;
    bool isNewGameFlow = Menu_IsNewGameFlow();
    float boxWidth = isNewGameFlow ? 550.0f : 450.0f;
    float boxHeight = isNewGameFlow ? 180.0f : 150.0f;
    float boxX = ((float)virtualScreenWidth - boxWidth) / 2.0f;
    float boxY = ((float)virtualScreenHeight - boxHeight) / 2.0f;
    float spacing = 20.0f; float totalBtnWidth = 2.0f * btnWidth + spacing;
    float buttonY = boxY + boxHeight - btnHeight - 20.0f;

    s_confirmationButtons[0] = (MenuButton){ {boxX + (boxWidth - totalBtnWidth)/2.0f, buttonY, btnWidth, btnHeight}, "Sim", MAROON, ORANGE, GRAY, WHITE, true, false, BUTTON_ACTION_NONE };
    s_confirmationButtons[1] = (MenuButton){ {boxX + (boxWidth - totalBtnWidth)/2.0f + btnWidth + spacing, buttonY, btnWidth, btnHeight}, "Nao", MAROON, ORANGE, GRAY, WHITE, true, false, BUTTON_ACTION_NONE };
    s_confirmationButtonsInitialized = true;
 }

void UpdateSaveLoadMenuScreen(GameState *currentScreen_ptr, Player players[], Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsCurrentlyPlaying_ptr, int *currentMapX_ptr, int *currentMapY_ptr, Vector2 virtualMousePos) {
    if (!saveLoadSlotsInitialized) InitializeSaveLoadSlotsMenuButtons();
    float wheelMove = GetMouseWheelMove();
    int currentSubState = Menu_GetSaveLoadSubState();
    GameModeType modeForSaveLoad = Menu_GetSaveLoadContextMode(); 

    if (currentSubState == 0) { 
        if (wheelMove != 0) { /* ... scroll ... */ }
        if (IsKeyPressed(KEY_ESCAPE)) { *currentScreen_ptr = Menu_GetPreviousScreenBeforeSaveLoad(); Menu_SetIsNewGameFlow(false); saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f; return; }
        for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
            Rectangle onScreenSlotRect = {saveLoadSlotsViewArea.x + saveLoadSlotButtons[i].rect.x, saveLoadSlotsViewArea.y + saveLoadSlotButtons[i].rect.y + saveLoadScrollOffset, saveLoadSlotButtons[i].rect.width, saveLoadSlotButtons[i].rect.height};
            saveLoadSlotButtons[i].is_hovered = false;
            if (CheckCollisionRecs(onScreenSlotRect, saveLoadSlotsViewArea)) {
                 if (CheckCollisionPointRec(virtualMousePos, onScreenSlotRect)) {
                    saveLoadSlotButtons[i].is_hovered = true;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        Menu_SetSelectedSlotForAction(i); 
                        bool isInSaveMode = Menu_IsInSaveMode(); bool isNewGameFlow = Menu_IsNewGameFlow();
                        int playersToSaveOrLoad = (modeForSaveLoad == GAME_MODE_SINGLE_PLAYER) ? 1 : MAX_PLAYERS_SUPPORTED;
                        if (isInSaveMode) {
                            if (DoesSaveSlotExist(i, modeForSaveLoad)) { Menu_SetSaveLoadSubState(1); s_confirmationButtonsInitialized = false; } 
                            else { 
                                if (isNewGameFlow) { PrepareNewGameSession(players, currentMapX_ptr, currentMapY_ptr, playersToSaveOrLoad); InitializeCharacterCreation(); *currentScreen_ptr = GAMESTATE_CHARACTER_CREATION; } 
                                else { SaveGame(players, playersToSaveOrLoad, i, *currentMapX_ptr, *currentMapY_ptr, modeForSaveLoad); *currentScreen_ptr = Menu_GetPreviousScreenBeforeSaveLoad(); }
                            }
                        } else { 
                            if (DoesSaveSlotExist(i, modeForSaveLoad)) { int numLoaded; if (LoadGame(players, MAX_PLAYERS_SUPPORTED, i, currentMapX_ptr, currentMapY_ptr, modeForSaveLoad, &numLoaded)) { currentActivePlayers = numLoaded; *currentScreen_ptr = GAMESTATE_PLAYING; if (playlist[currentMusicIndex].stream.buffer != NULL) { if(IsMusicStreamPlaying(playlist[currentMusicIndex])) StopMusicStream(playlist[currentMusicIndex]); PlayMusicStream(playlist[currentMusicIndex]); SetMusicVolume(playlist[currentMusicIndex], currentVolume); if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 1; } else { if(musicIsCurrentlyPlaying_ptr) *musicIsCurrentlyPlaying_ptr = 0; } } }
                        }
                        if (*currentScreen_ptr != GAMESTATE_SAVE_LOAD_MENU || Menu_GetSaveLoadSubState() == 1) { saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f; }
                        return; 
                    }
                }
            }
        }
    } else if (currentSubState == 1) { 
        if (!s_confirmationButtonsInitialized) InitializeSaveLoadConfirmationButtons();
        if (IsKeyPressed(KEY_ESCAPE)) { Menu_SetSaveLoadSubState(0); Menu_SetSelectedSlotForAction(-1); saveLoadSlotsInitialized = false; return; }
        for (int j = 0; j < 2; j++) { 
            s_confirmationButtons[j].is_hovered = false;
            if (CheckCollisionPointRec(virtualMousePos, s_confirmationButtons[j].rect)) {
                s_confirmationButtons[j].is_hovered = true;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    int selectedSlot = Menu_GetSelectedSlotForAction(); bool isNewGameFlow = Menu_IsNewGameFlow();
                    int playersToSave = (modeForSaveLoad == GAME_MODE_SINGLE_PLAYER) ? 1 : MAX_PLAYERS_SUPPORTED;
                    if (j == 0) { 
                        if (isNewGameFlow) { PrepareNewGameSession(players, currentMapX_ptr, currentMapY_ptr, playersToSave); InitializeCharacterCreation(); *currentScreen_ptr = GAMESTATE_CHARACTER_CREATION; } 
                        else { SaveGame(players, playersToSave, selectedSlot, *currentMapX_ptr, *currentMapY_ptr, modeForSaveLoad); *currentScreen_ptr = Menu_GetPreviousScreenBeforeSaveLoad(); }
                    } else { Menu_SetSaveLoadSubState(0); }
                    if (*currentScreen_ptr != GAMESTATE_SAVE_LOAD_MENU || Menu_GetSaveLoadSubState() == 0){ saveLoadSlotsInitialized = false; s_confirmationButtonsInitialized = false; saveLoadScrollOffset = 0.0f; }
                    Menu_SetSelectedSlotForAction(-1); 
                    if (*currentScreen_ptr == Menu_GetPreviousScreenBeforeSaveLoad() && Menu_GetPreviousScreenBeforeSaveLoad() != GAMESTATE_PLAYER_MODE_MENU && *currentScreen_ptr != GAMESTATE_CHARACTER_CREATION) Menu_SetIsNewGameFlow(false);
                    return; 
                }
            }
        }
    }
}

void DrawSaveLoadMenuScreen(Player players[], Music playlist[], int currentMusicIndex, int musicIsPlaying, float musicVolume, int mapX, int mapY) {
    // ... (código igual, com casts (int) para DrawText e usando Menu_GetSaveLoadContextMode()) ...
    (void)playlist; 
    GameModeType modeForSaveLoad = Menu_GetSaveLoadContextMode(); 
    GameState prevScreen = Menu_GetPreviousScreenBeforeSaveLoad();
    if (prevScreen == GAMESTATE_PAUSE) DrawPauseScreen(players, musicVolume, currentMusicIndex, musicIsPlaying, mapX, mapY);
    else if (prevScreen == GAMESTATE_PLAYER_MODE_MENU) { ClearBackground(DARKGRAY); DrawRectangle(0,0,virtualScreenWidth,virtualScreenHeight,Fade(BLACK,0.85f)); }
    else { ClearBackground(DARKGRAY); DrawRectangle(0,0,virtualScreenWidth,virtualScreenHeight,Fade(BLACK,0.85f)); }
    if (!saveLoadSlotsInitialized) InitializeSaveLoadSlotsMenuButtons();
    if (Menu_GetSaveLoadSubState() == 1 && !s_confirmationButtonsInitialized) InitializeSaveLoadConfirmationButtons();
    char title[128];
    if (Menu_IsNewGameFlow()) sprintf(title, "SELECIONAR SLOT PARA NOVO JOGO (%s)", modeForSaveLoad == GAME_MODE_SINGLE_PLAYER ? "1P" : "2P");
    else sprintf(title, "SLOTS DE %s (%s)", Menu_IsInSaveMode() ? "SALVAR" : "CARREGAR", modeForSaveLoad == GAME_MODE_SINGLE_PLAYER ? "1P" : "2P");
    DrawText(title, (int)(((float)virtualScreenWidth - (float)MeasureText(title, 30)) / 2.0f), 30, 30, WHITE);
    if (Menu_GetSaveLoadSubState() == 0) { 
        BeginScissorMode((int)saveLoadSlotsViewArea.x, (int)saveLoadSlotsViewArea.y, (int)saveLoadSlotsViewArea.width, (int)saveLoadSlotsViewArea.height);
            for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                float onScreenSlotY = saveLoadSlotsViewArea.y + saveLoadSlotButtons[i].rect.y + saveLoadScrollOffset;
                Rectangle onScreenSlotRect = { saveLoadSlotsViewArea.x + saveLoadSlotButtons[i].rect.x, onScreenSlotY, saveLoadSlotButtons[i].rect.width, saveLoadSlotButtons[i].rect.height };
                if ((onScreenSlotRect.y + onScreenSlotRect.height > saveLoadSlotsViewArea.y) && (onScreenSlotRect.y < saveLoadSlotsViewArea.y + saveLoadSlotsViewArea.height)) {
                    Color slotColor = saveLoadSlotButtons[i].is_hovered ? LIGHTGRAY : DARKGRAY;
                    if (!Menu_IsInSaveMode() && !DoesSaveSlotExist(i, modeForSaveLoad)) slotColor = Fade(DARKGRAY, 0.5f);
                    DrawRectangleRec(onScreenSlotRect, slotColor); DrawRectangleLinesEx(onScreenSlotRect, 2, GRAY);
                    char slotText[64]; if (DoesSaveSlotExist(i, modeForSaveLoad)) sprintf(slotText, "Slot %d - Ocupado", i + 1); else sprintf(slotText, "Slot %d - Vazio", i + 1);
                    DrawText(slotText, (int)(onScreenSlotRect.x + 10.0f), (int)(onScreenSlotRect.y + (onScreenSlotRect.height - 20.0f)/2.0f), 20, (DoesSaveSlotExist(i, modeForSaveLoad) ? WHITE : LIGHTGRAY) );
                }
            }
        EndScissorMode();
        float maxScroll = 0.0f; if (saveLoadTotalContentHeight > saveLoadSlotsViewArea.height) maxScroll = saveLoadTotalContentHeight - saveLoadSlotsViewArea.height;
        if (maxScroll > 0.0f) { 
            Rectangle scrollBarArea = { saveLoadSlotsViewArea.x + saveLoadSlotsViewArea.width + 2.0f, saveLoadSlotsViewArea.y, 8.0f, saveLoadSlotsViewArea.height };
            DrawRectangleRec(scrollBarArea, LIGHTGRAY); 
            float thumbHeight = (saveLoadSlotsViewArea.height / saveLoadTotalContentHeight) * scrollBarArea.height; if (thumbHeight < 20.0f) thumbHeight = 20.0f; 
            float thumbY = scrollBarArea.y; if (maxScroll > 0.001f) thumbY += (-saveLoadScrollOffset / maxScroll) * (scrollBarArea.height - thumbHeight);
            DrawRectangle((int)scrollBarArea.x, (int)thumbY, (int)scrollBarArea.width, (int)thumbHeight, DARKGRAY);
        } DrawText("ESC para Voltar | Roda do Mouse para Scroll", 10, virtualScreenHeight - 20, 10, WHITE);
    } else if (Menu_GetSaveLoadSubState() == 1) { 
        bool isNewGameFlow = Menu_IsNewGameFlow(); float boxWidth = isNewGameFlow ? 550.0f : 450.0f; float boxHeight = isNewGameFlow ? 180.0f : 150.0f;
        Rectangle confBoxRect = {((float)virtualScreenWidth - boxWidth)/2.0f, ((float)virtualScreenHeight - boxHeight)/2.0f, boxWidth, boxHeight};
        DrawRectangleRec(confBoxRect, Fade(BLACK, 0.95f)); DrawRectangleLinesEx(confBoxRect, 2, WHITE); 
        float textY = confBoxRect.y + 20.0f; float textPadding = 15.0f; int selectedSlot = Menu_GetSelectedSlotForAction();
        if (isNewGameFlow) {
            char l1[128], l2[128], l3[128]; sprintf(l1, "Slot %d contem dados salvos.", selectedSlot+1); sprintf(l2, "Iniciar novo jogo aqui?"); sprintf(l3, "(O jogo anterior sera perdido se nao salvo em outro slot)");
            DrawText(l1, (int)(confBoxRect.x+(boxWidth-(float)MeasureText(l1,20))/2.0f), (int)textY, 20, WHITE); textY+=25.0f;
            DrawText(l2, (int)(confBoxRect.x+(boxWidth-(float)MeasureText(l2,20))/2.0f), (int)textY, 20, WHITE); textY+=25.0f+5.0f; 
            DrawText(l3, (int)(confBoxRect.x+(boxWidth-(float)MeasureText(l3,10))/2.0f), (int)textY, 10, LIGHTGRAY);
        } else { char ct[128]; sprintf(ct, "Voce quer sobrescrever o Save %d?", selectedSlot+1); DrawText(ct, (int)(confBoxRect.x+(boxWidth-(float)MeasureText(ct,20))/2.0f), (int)(confBoxRect.y+(boxHeight-s_confirmationButtons[0].rect.height-20.0f-20.0f-10.0f)/2.0f),20,WHITE); }
        for (int i=0; i<2; i++) { Color bc=s_confirmationButtons[i].is_hovered?ORANGE:MAROON; DrawRectangleRec(s_confirmationButtons[i].rect,bc); DrawText(s_confirmationButtons[i].text, (int)(s_confirmationButtons[i].rect.x+(s_confirmationButtons[i].rect.width-(float)MeasureText(s_confirmationButtons[i].text,20))/2.0f), (int)(s_confirmationButtons[i].rect.y+(s_confirmationButtons[i].rect.height-20.0f)/2.0f),20,WHITE); }
        DrawText("ESC para Cancelar", (int)(confBoxRect.x+textPadding), (int)(confBoxRect.y+confBoxRect.height-20.0f),10,GRAY);
    }
}