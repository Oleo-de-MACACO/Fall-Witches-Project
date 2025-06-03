#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Singleplayer.h" 
#include "../include/WalkCycle.h"   
#include <raylib.h>
#include <raymath.h> 
#include <stdbool.h>
#include <stdio.h>   
#include <stdlib.h>  
#include <math.h>    
#include <string.h>  
#include <stddef.h>  

extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;
extern GameModeType currentGameMode; 
extern int currentActivePlayers;    

const int gameSectionWidthMultiplier = 3; 
const int gameSectionHeightMultiplier = 3;

static BorderDirection GetPlayerBorderCondition_Internal(Player player_obj);

void UpdateCameraCenteredOnPlayers(Camera2D *camera, Player players[], int numActivePlayers, float vWidth, float vHeight) {
    if (!camera || !players || numActivePlayers <= 0) return;
    const float MIN_CAMERA_ZOOM = 1.0f / fmaxf((float)gameSectionWidthMultiplier, (float)gameSectionHeightMultiplier);
    const float MAX_CAMERA_ZOOM = 1.0f; 
    const float BORDER_PADDING_PERCENT = 0.20f;
    if (numActivePlayers == 1) {
        camera->target.x = (float)players[0].posx + ((float)players[0].width / 2.0f);
        camera->target.y = (float)players[0].posy + ((float)players[0].height / 2.0f);
        camera->zoom = MAX_CAMERA_ZOOM; 
    } else if (numActivePlayers >= 2) {
        camera->target.x = ((float)players[0].posx + ((float)players[0].width / 2.0f) + (float)players[1].posx + ((float)players[1].width / 2.0f)) / 2.0f;
        camera->target.y = ((float)players[0].posy + ((float)players[0].height / 2.0f) + (float)players[1].posy + ((float)players[1].height / 2.0f)) / 2.0f;
        float minX = fminf((float)players[0].posx, (float)players[1].posx);
        float maxX = fmaxf((float)players[0].posx + (float)players[0].width, (float)players[1].posx + (float)players[1].width);
        float minY = fminf((float)players[0].posy, (float)players[1].posy);
        float maxY = fmaxf((float)players[0].posy + (float)players[0].height, (float)players[1].posy + (float)players[1].height);
        float boundingBoxWidth = maxX - minX; float boundingBoxHeight = maxY - minY;
        float paddedWidth = boundingBoxWidth * (1.0f + BORDER_PADDING_PERCENT * 2.0f);
        float paddedHeight = boundingBoxHeight * (1.0f + BORDER_PADDING_PERCENT * 2.0f);
        float minPracticalPaddingW = (float)players[0].width * 0.5f;
        float minPracticalPaddingH = (float)players[0].height * 0.5f;
        if (paddedWidth < minPracticalPaddingW) paddedWidth = minPracticalPaddingW;
        if (paddedHeight < minPracticalPaddingH) paddedHeight = minPracticalPaddingH;
        if (paddedWidth < (vWidth / MAX_CAMERA_ZOOM)) paddedWidth = (vWidth / MAX_CAMERA_ZOOM);
        if (paddedHeight < (vHeight / MAX_CAMERA_ZOOM)) paddedHeight = (vHeight / MAX_CAMERA_ZOOM);
        if (paddedWidth < 1.0f) paddedWidth = 1.0f; 
        if (paddedHeight < 1.0f) paddedHeight = 1.0f;
        float zoomRequiredX = vWidth / paddedWidth; float zoomRequiredY = vHeight / paddedHeight;
        camera->zoom = fminf(zoomRequiredX, zoomRequiredY);
    }
    if (camera->zoom < MIN_CAMERA_ZOOM) camera->zoom = MIN_CAMERA_ZOOM;
    if (camera->zoom > MAX_CAMERA_ZOOM) camera->zoom = MAX_CAMERA_ZOOM;
    float currentSectionTotalWidth = (float)gameSectionWidthMultiplier * vWidth;
    float currentSectionTotalHeight = (float)gameSectionHeightMultiplier * vHeight;
    float visibleWorldHalfWidth = (vWidth / camera->zoom) / 2.0f;
    float visibleWorldHalfHeight = (vHeight / camera->zoom) / 2.0f;
    camera->target.x = fmaxf(visibleWorldHalfWidth, camera->target.x);
    camera->target.x = fminf(currentSectionTotalWidth - visibleWorldHalfWidth, camera->target.x);
    camera->target.y = fmaxf(visibleWorldHalfHeight, camera->target.y);
    camera->target.y = fminf(currentSectionTotalHeight - visibleWorldHalfHeight, camera->target.y);
    camera->offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
}

void move_character(Player *player_obj, int keyLeft, int keyRight, int keyUp, int keyDown, int keyShift){
    if(!player_obj) return;
    int speed = 3; int sprintSpeed = 5;
    int currentSpeed = IsKeyDown(keyShift) ? sprintSpeed : speed;
    if (IsKeyDown(keyLeft))  { player_obj->posx -= currentSpeed; }
    if (IsKeyDown(keyRight)) { player_obj->posx += currentSpeed; }
    if (IsKeyDown(keyUp))    { player_obj->posy -= currentSpeed; }
    if (IsKeyDown(keyDown))  { player_obj->posy += currentSpeed; }
    int cSectW = gameSectionWidthMultiplier * virtualScreenWidth;
    int cSectH = gameSectionHeightMultiplier * virtualScreenHeight;
    if (player_obj->posx < 0) player_obj->posx = 0;
    if (player_obj->posx > cSectW - player_obj->width) player_obj->posx = cSectW - player_obj->width;
    if (player_obj->posy < 0) player_obj->posy = 0;
    if (player_obj->posy > cSectH - player_obj->height) player_obj->posy = cSectH - player_obj->height;
}

void InitGameResources(Player players_arr[], Music mainPlaylist_arr[]) {
    if (players_arr == NULL) {
        TraceLog(LOG_ERROR, "[InitGameResources] players_arr e NULL.");
        return;
    }
    for (int i = 0; i < MAX_PLAYERS_SUPPORTED; i++) { 
        players_arr[i].width = 24; 
        players_arr[i].height = 36;
        // Animações são carregadas após init_player/LoadGame quando spriteType é conhecido
    }
    // Chamadas LoadCharacterAnimations removidas daqui. Devem ser chamadas APÓS player->spriteType ser definido.
    // Por exemplo, em CharacterCreation.c ou SaveLoad.c

    if (mainPlaylist_arr != NULL) { // Processa músicas apenas se o array for fornecido
        const char *musicFiles[MAX_MUSIC_PLAYLIST_SIZE] = { 
            "./assets/songs/Desmeon_-_My_Sunshine.ogg", "./assets/songs/19 William Tell Overture.ogg",
            "./assets/songs/last-summer-by-ikson.mp3", "./assets/songs/Floatinurboat - Spirit Of Things.mp3"
        };
        for(int i=0; i<MAX_MUSIC_PLAYLIST_SIZE; i++){
            if(musicFiles[i] && strlen(musicFiles[i]) > 0){ 
                mainPlaylist_arr[i] = LoadMusicStream(musicFiles[i]);
                if(mainPlaylist_arr[i].stream.buffer == NULL) {
                    TraceLog(LOG_WARNING, "Falha musica %d: %s", i+1, musicFiles[i]);
                }
            } else {
                mainPlaylist_arr[i] = (Music){ 
                    .stream = (AudioStream){ .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
                    .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL 
                };
            }
        }
    }
}

void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY, int numActivePlayers) {
    if (!players_arr || !mapX || !mapY ) return;
    TraceLog(LOG_INFO,"Preparando sessao (%d jogadores)...", numActivePlayers); 
    *mapX=0; *mapY=0;
    int cSectH = gameSectionHeightMultiplier * virtualScreenHeight;
    for(int i=0; i<numActivePlayers && i<MAX_PLAYERS_SUPPORTED; i++){
        players_arr[i].width=24; 
        players_arr[i].height=36;
        if(i==0){players_arr[i].posx = players_arr[i].width+50; players_arr[i].posy = cSectH/2 - players_arr[i].height/2;}
        else if(i==1){players_arr[i].posx = players_arr[i].width+150; players_arr[i].posy = cSectH/2 - players_arr[i].height/2;}
    }
}

static BorderDirection GetPlayerBorderCondition_Internal(Player player_obj){
    int tolerance = 10;
    int cSectW = gameSectionWidthMultiplier * virtualScreenWidth; 
    int cSectH = gameSectionHeightMultiplier * virtualScreenHeight;
    if (player_obj.posx <= tolerance) return BORDER_LEFT;
    if (player_obj.posx >= cSectW - player_obj.width - tolerance) return BORDER_RIGHT;
    if (player_obj.posy <= tolerance) return BORDER_TOP;
    if (player_obj.posy >= cSectH - player_obj.height - tolerance) return BORDER_BOTTOM;
    return BORDER_NONE;
}

void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], int numActivePlayers, 
                         Music playlist_arr[], int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, 
                         float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr,
                         int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera) {

    if (!currentScreen_ptr || !players_arr || !gameCamera || 
        !currentMapX_ptr || !currentMapY_ptr) return; // Checagens essenciais

    // Parâmetros de música podem ser NULL se Sound.c gerencia tudo
    // Adicionar (void) para silenciar warnings se eles não forem usados aqui E Sound.c é o mestre
    // (void)playlist_arr; (void)currentMusicIndex_ptr; (void)volume_ptr; (void)isPlaying_ptr;
    // (void)musicPlayingTimer_ptr; (void)currentMusicDuration_ptr;

    UpdateCameraCenteredOnPlayers(gameCamera, players_arr, numActivePlayers, (float)virtualScreenWidth, (float)virtualScreenHeight);
    
    // Exemplo: Se ainda quiser usar a playlist global aqui, precisa checar os ponteiros
    if (playlist_arr && currentMusicIndex_ptr && isPlaying_ptr && volume_ptr && musicPlayingTimer_ptr && currentMusicDuration_ptr &&
        *currentMusicIndex_ptr >= 0 && *currentMusicIndex_ptr < MAX_MUSIC_PLAYLIST_SIZE && 
        playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
        if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].frameCount > 0 ) UpdateMusicStream(playlist_arr[*currentMusicIndex_ptr]);
        // ... (toda a lógica de controle de música como antes, com checagens de ponteiros) ...
    }
    
    int oldPosX[MAX_PLAYERS_SUPPORTED] = {0}; 
    int oldPosY[MAX_PLAYERS_SUPPORTED] = {0};
    for(int i=0; i < numActivePlayers && i < MAX_PLAYERS_SUPPORTED; i++) {
        oldPosX[i] = players_arr[i].posx;
        oldPosY[i] = players_arr[i].posy;
    }

    if (currentGameMode == GAME_MODE_SINGLE_PLAYER){
        if(numActivePlayers>0) {
            SinglePlayer_HandleMovement(&players_arr[0]);
            SinglePlayer_CheckMapTransition(&players_arr[0],currentMapX_ptr,currentMapY_ptr);
        }
    } else { // GAME_MODE_TWO_PLAYER
        if(numActivePlayers>0)move_character(&players_arr[0],KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT);
        if(numActivePlayers>1)move_character(&players_arr[1],KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT);
        if(numActivePlayers>1){ /* ... (lógica de transição de mapa para 2P como antes) ... */ }
    }
    
    for(int i=0; i<numActivePlayers && i < MAX_PLAYERS_SUPPORTED; i++){
        bool isMovingNow = (players_arr[i].posx != oldPosX[i] || players_arr[i].posy != oldPosY[i]);
        float mX = (float)(players_arr[i].posx - oldPosX[i]); 
        float mY = (float)(players_arr[i].posy - oldPosY[i]);
        UpdateWalkCycle(&players_arr[i], isMovingNow, mX, mY);
    }
    
    if (IsKeyPressed(KEY_P)||IsKeyPressed(KEY_ESCAPE)){
        *currentScreen_ptr=GAMESTATE_PAUSE;
        if(isPlaying_ptr && *isPlaying_ptr && playlist_arr && currentMusicIndex_ptr &&
           *currentMusicIndex_ptr>=0 && *currentMusicIndex_ptr<MAX_MUSIC_PLAYLIST_SIZE && 
           playlist_arr[*currentMusicIndex_ptr].stream.buffer!=NULL) {
               PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]);
        }
    }
    if (IsKeyPressed(KEY_E)){*currentScreen_ptr=GAMESTATE_INVENTORY;}
}

void DrawPlayingScreen(Player players_arr[], int numActivePlayers, float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY) {
    Color bgColor; int colorIndex = (abs(currentMapX) + abs(currentMapY)) % 4;
    if (colorIndex == 0) bgColor = BLACK; else if (colorIndex == 1) bgColor = DARKBROWN;
    else if (colorIndex == 2) bgColor = DARKPURPLE; else bgColor = (Color){20, 40, 60, 255};
    ClearBackground(bgColor);

    if (players_arr != NULL) {
        for (int i = 0; i < numActivePlayers && i < MAX_PLAYERS_SUPPORTED; i++) {
            Texture2D currentSprite = GetCurrentCharacterSprite(&players_arr[i]);
            if (currentSprite.id > 0) {
                DrawTexturePro(currentSprite, 
                               (Rectangle){0.0f,0.0f,(float)currentSprite.width,(float)currentSprite.height}, 
                               (Rectangle){(float)players_arr[i].posx,(float)players_arr[i].posy,(float)players_arr[i].width,(float)players_arr[i].height}, 
                               (Vector2){0.0f,0.0f}, 0.0f, WHITE);
            } else { 
                DrawRectangle(players_arr[i].posx,players_arr[i].posy,players_arr[i].width,players_arr[i].height,GRAY); 
            }
        }
    }
    char mapCoordsText[64]; sprintf(mapCoordsText, "Mapa (X: %d, Y: %d)", currentMapX, currentMapY);
    DrawText(mapCoordsText, 10, 10, 20, YELLOW);
    DrawText(TextFormat("Volume: %.0f%%", currentVolume * 100.0f), 10, virtualScreenHeight - 60, 10, LIGHTGRAY);
    // currentMusicIndex e isPlaying podem vir do Sound.c ou de um sistema de playlist legado
    DrawText(TextFormat("Musica %d (Tocando: %s)", currentMusicIndex + 1, isPlaying ? "Sim" : "Nao"), 10, virtualScreenHeight - 45, 10, LIGHTGRAY);
    DrawText("Controles: P/Esc=Pausa, E=Inventario, Setas/WASD=Mover, Shift=Correr", 10, virtualScreenHeight - 30, 10, RAYWHITE);
    DrawText(",/.=Play/Pause Musica, PgUp/Dn=Mudar Musica, +/-/0=Volume", 10, virtualScreenHeight - 15, 10, RAYWHITE);
    DrawFPS(10, virtualScreenHeight - 75); 
}