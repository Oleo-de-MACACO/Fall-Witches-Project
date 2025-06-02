#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Singleplayer.h" 
#include <raylib.h>
#include <raymath.h> 
#include <stdbool.h>
#include <stdio.h>   
#include <stdlib.h>  // For abs()
#include <math.h>    // For fmaxf, fminf
#include <string.h>  // For strlen
#include <stddef.h>  // For NULL

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
    if (players_arr == NULL || mainPlaylist_arr == NULL) return;
    for (int i = 0; i < MAX_PLAYERS_SUPPORTED; i++) { 
        players_arr[i].width = 80; players_arr[i].height = 80;
        players_arr[i].txr = (Texture2D){0, 0, 0, 0, 0}; 
    }
    const char *p0Path = "./assets/images/player2.png"; 
    if (MAX_PLAYERS_SUPPORTED > 0) { players_arr[0].txr = LoadTexture(p0Path); if(players_arr[0].txr.id==0) TraceLog(LOG_WARNING,"Falha textura P0: %s", p0Path); }
    const char *p1Path = "./assets/images/player1.png"; 
    if (MAX_PLAYERS_SUPPORTED > 1) { players_arr[1].txr = LoadTexture(p1Path); if(players_arr[1].txr.id==0) TraceLog(LOG_WARNING,"Falha textura P1: %s", p1Path); }
    const char *musicFiles[MAX_MUSIC_PLAYLIST_SIZE]={"./assets/songs/Desmeon_-_My_Sunshine.ogg","./assets/songs/19 William Tell Overture.ogg","./assets/songs/last-summer-by-ikson.mp3","./assets/songs/Floatinurboat - Spirit Of Things.mp3"};
    for(int i=0; i<MAX_MUSIC_PLAYLIST_SIZE; i++){
        if(musicFiles[i] && strlen(musicFiles[i]) > 0){
            mainPlaylist_arr[i] = LoadMusicStream(musicFiles[i]);
            if(mainPlaylist_arr[i].stream.buffer == NULL) TraceLog(LOG_WARNING, "Falha musica %d: %s", i+1, musicFiles[i]);
        } else {
            mainPlaylist_arr[i] = (Music){ 
                .stream = (AudioStream){ .buffer = NULL, .processor = NULL, .sampleRate = 0, .sampleSize = 0, .channels = 0 },
                .frameCount = 0, .looping = false, .ctxType = 0, .ctxData = NULL 
            };
        }
    }
}

void PrepareNewGameSession(Player players_arr[], int *mapX, int *mapY, int numActivePlayers) {
    if (!players_arr || !mapX || !mapY ) return;
    TraceLog(LOG_INFO,"Preparando sessao (%d jogadores)...", numActivePlayers); 
    *mapX=0; *mapY=0;
    int cSectH = gameSectionHeightMultiplier * virtualScreenHeight;
    for(int i=0; i<numActivePlayers && i<MAX_PLAYERS_SUPPORTED; i++){
        players_arr[i].width=80; players_arr[i].height=80;
        if(i==0){players_arr[i].posx = players_arr[i].width+50; players_arr[i].posy = cSectH/2 - players_arr[i].height/2;}
        else if(i==1){players_arr[i].posx = players_arr[i].width+150; players_arr[i].posy = cSectH/2 - players_arr[i].height/2;}
    }
}

static BorderDirection GetPlayerBorderCondition_Internal(Player player_obj){
    int tolerance = 10;
    int cSectW = gameSectionWidthMultiplier * virtualScreenWidth; int cSectH = gameSectionHeightMultiplier * virtualScreenHeight;
    if (player_obj.posx <= tolerance) return BORDER_LEFT;
    if (player_obj.posx >= cSectW - player_obj.width - tolerance) return BORDER_RIGHT;
    if (player_obj.posy <= tolerance) return BORDER_TOP;
    if (player_obj.posy >= cSectH - player_obj.height - tolerance) return BORDER_BOTTOM;
    return BORDER_NONE;
}

void UpdatePlayingScreen(GameState *currentScreen_ptr, Player players_arr[], int numActivePlayers, Music playlist_arr[], 
                         int *currentMusicIndex_ptr, float *volume_ptr, int *isPlaying_ptr, 
                         float *musicPlayingTimer_ptr, float *currentMusicDuration_ptr,
                         int *currentMapX_ptr, int *currentMapY_ptr, Camera2D *gameCamera) {

    if (!currentScreen_ptr || !players_arr || !playlist_arr || !currentMusicIndex_ptr || !volume_ptr || 
        !isPlaying_ptr || !musicPlayingTimer_ptr || !currentMusicDuration_ptr || !currentMapX_ptr || 
        !currentMapY_ptr || !gameCamera) return;

    UpdateCameraCenteredOnPlayers(gameCamera, players_arr, numActivePlayers, (float)virtualScreenWidth, (float)virtualScreenHeight);
    
    if (playlist_arr[*currentMusicIndex_ptr].stream.buffer != NULL) {
        if (*isPlaying_ptr && playlist_arr[*currentMusicIndex_ptr].frameCount > 0 ) UpdateMusicStream(playlist_arr[*currentMusicIndex_ptr]);
        if (IsKeyPressed(KEY_COMMA)||IsKeyPressed(KEY_PERIOD)){if(*isPlaying_ptr){PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]);*isPlaying_ptr=0;}else{ResumeMusicStream(playlist_arr[*currentMusicIndex_ptr]);*isPlaying_ptr=1;}}
        if (IsKeyPressed(KEY_PAGE_DOWN)){if(playlist_arr[*currentMusicIndex_ptr].stream.buffer!=NULL)StopMusicStream(playlist_arr[*currentMusicIndex_ptr]);*currentMusicIndex_ptr=(*currentMusicIndex_ptr+1)%MAX_MUSIC_PLAYLIST_SIZE;if(playlist_arr[*currentMusicIndex_ptr].stream.buffer!=NULL && playlist_arr[*currentMusicIndex_ptr].frameCount>0){PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]);SetMusicVolume(playlist_arr[*currentMusicIndex_ptr],*volume_ptr);*isPlaying_ptr=1;*currentMusicDuration_ptr=GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]);*musicPlayingTimer_ptr=0.0f;}else{*isPlaying_ptr=0;}}
        if (IsKeyPressed(KEY_PAGE_UP)){if(playlist_arr[*currentMusicIndex_ptr].stream.buffer!=NULL)StopMusicStream(playlist_arr[*currentMusicIndex_ptr]);*currentMusicIndex_ptr=(*currentMusicIndex_ptr-1+MAX_MUSIC_PLAYLIST_SIZE)%MAX_MUSIC_PLAYLIST_SIZE;if(playlist_arr[*currentMusicIndex_ptr].stream.buffer!=NULL && playlist_arr[*currentMusicIndex_ptr].frameCount>0){PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]);SetMusicVolume(playlist_arr[*currentMusicIndex_ptr],*volume_ptr);*isPlaying_ptr=1;*currentMusicDuration_ptr=GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]);*musicPlayingTimer_ptr=0.0f;}else{*isPlaying_ptr=0;}}
        if (IsKeyPressed(KEY_KP_0)||IsKeyPressed(KEY_ZERO)){if(*isPlaying_ptr){*volume_ptr=0.0f;SetMusicVolume(playlist_arr[*currentMusicIndex_ptr],*volume_ptr);}}
        if (IsKeyPressed(KEY_KP_SUBTRACT)||IsKeyPressed(KEY_MINUS)){if(*isPlaying_ptr){*volume_ptr-=0.05f;if(*volume_ptr<0.0f)*volume_ptr=0.0f;SetMusicVolume(playlist_arr[*currentMusicIndex_ptr],*volume_ptr);}}
        if (IsKeyPressed(KEY_KP_ADD)||IsKeyPressed(KEY_EQUAL)){if(*isPlaying_ptr){*volume_ptr+=0.05f;if(*volume_ptr>1.0f)*volume_ptr=1.0f;SetMusicVolume(playlist_arr[*currentMusicIndex_ptr],*volume_ptr);}}
        if (*isPlaying_ptr){*musicPlayingTimer_ptr+=GetFrameTime();if(*currentMusicDuration_ptr>0.0f&&*musicPlayingTimer_ptr>=*currentMusicDuration_ptr){StopMusicStream(playlist_arr[*currentMusicIndex_ptr]);*currentMusicIndex_ptr=(*currentMusicIndex_ptr+1)%MAX_MUSIC_PLAYLIST_SIZE;if(playlist_arr[*currentMusicIndex_ptr].stream.buffer!=NULL && playlist_arr[*currentMusicIndex_ptr].frameCount>0){PlayMusicStream(playlist_arr[*currentMusicIndex_ptr]);SetMusicVolume(playlist_arr[*currentMusicIndex_ptr],*volume_ptr);*isPlaying_ptr=1;*currentMusicDuration_ptr=GetMusicTimeLength(playlist_arr[*currentMusicIndex_ptr]);*musicPlayingTimer_ptr=0.0f;}else{*isPlaying_ptr=0;}}}
    }

    if (currentGameMode == GAME_MODE_SINGLE_PLAYER){if(numActivePlayers>0)SinglePlayer_HandleMovement(&players_arr[0]);SinglePlayer_CheckMapTransition(&players_arr[0],currentMapX_ptr,currentMapY_ptr);}
    else{if(numActivePlayers>0)move_character(&players_arr[0],KEY_LEFT,KEY_RIGHT,KEY_UP,KEY_DOWN,KEY_RIGHT_SHIFT);if(numActivePlayers>1)move_character(&players_arr[1],KEY_A,KEY_D,KEY_W,KEY_S,KEY_LEFT_SHIFT);
        if(numActivePlayers>1){
            BorderDirection p0b=GetPlayerBorderCondition_Internal(players_arr[0]); BorderDirection p1b=GetPlayerBorderCondition_Internal(players_arr[1]);
            bool tr=false; BorderDirection tTo=BORDER_NONE; int nMX=*currentMapX_ptr; int nMY=*currentMapY_ptr;
            if(p0b!=BORDER_NONE&&p0b==p1b){tTo=p0b;switch(tTo){case BORDER_LEFT:nMX--;break;case BORDER_RIGHT:nMX++;break;case BORDER_TOP:nMY--;break;case BORDER_BOTTOM:nMY++;break;default:break;}if(nMX>=WORLD_MAP_MIN_X&&nMX<=WORLD_MAP_MAX_X&&nMY>=WORLD_MAP_MIN_Y&&nMY<=WORLD_MAP_MAX_Y)tr=true;}
            if(tr){*currentMapX_ptr=nMX;*currentMapY_ptr=nMY;int cSW=gameSectionWidthMultiplier*virtualScreenWidth;int cSH=gameSectionHeightMultiplier*virtualScreenHeight;
                for(int i=0;i<numActivePlayers;i++){int pX=players_arr[i].posx;int pY=players_arr[i].posy;
                    switch(tTo){case BORDER_RIGHT:players_arr[i].posx=players_arr[i].width/2;players_arr[i].posy=pY;break;case BORDER_LEFT:players_arr[i].posx=cSW-players_arr[i].width-(players_arr[i].width/2);players_arr[i].posy=pY;break;case BORDER_TOP:players_arr[i].posx=pX;players_arr[i].posy=cSH-players_arr[i].height-(players_arr[i].height/2);break;case BORDER_BOTTOM:players_arr[i].posx=pX;players_arr[i].posy=players_arr[i].height/2;break;default:break;}
                    // Corrigido: Misleading indentation com chaves
                    if(players_arr[i].posx<0){players_arr[i].posx=0;} 
                    if(players_arr[i].posx>cSW-players_arr[i].width){players_arr[i].posx=cSW-players_arr[i].width;}
                    if(players_arr[i].posy<0){players_arr[i].posy=0;} 
                    if(players_arr[i].posy>cSH-players_arr[i].height){players_arr[i].posy=cSH-players_arr[i].height;}
                }}}}
    // Transição para outras telas
    if (IsKeyPressed(KEY_P)||IsKeyPressed(KEY_ESCAPE)){
        if (currentScreen_ptr) *currentScreen_ptr=GAMESTATE_PAUSE;
        if(isPlaying_ptr&&*isPlaying_ptr&&playlist_arr&&currentMusicIndex_ptr&&*currentMusicIndex_ptr>=0&&*currentMusicIndex_ptr<MAX_MUSIC_PLAYLIST_SIZE&&playlist_arr[*currentMusicIndex_ptr].stream.buffer!=NULL) {
            PauseMusicStream(playlist_arr[*currentMusicIndex_ptr]);
        }
        TraceLog(LOG_INFO, "[Game] Tecla P ou ESCAPE pressionada. Mudando para GAMESTATE_PAUSE.");
    }
    if (IsKeyPressed(KEY_E)){
        if (currentScreen_ptr) *currentScreen_ptr=GAMESTATE_INVENTORY;
        TraceLog(LOG_INFO, "[Game] Tecla E pressionada. Mudando para GAMESTATE_INVENTORY.");
    }
}

void DrawPlayingScreen(Player players_arr[], int numActivePlayers, float currentVolume, int currentMusicIndex, int isPlaying, int currentMapX, int currentMapY) {
    Color bgColor; int colorIndex = (abs(currentMapX) + abs(currentMapY)) % 4;
    if (colorIndex == 0) bgColor = BLACK; else if (colorIndex == 1) bgColor = DARKBROWN;
    else if (colorIndex == 2) bgColor = DARKPURPLE; else bgColor = (Color){20, 40, 60, 255};
    ClearBackground(bgColor);
    if (players_arr != NULL) {
        for (int i = 0; i < numActivePlayers; i++) {
            if (players_arr[i].txr.id > 0) DrawTextureRec(players_arr[i].txr, (Rectangle){0.0f,0.0f,(float)players_arr[i].txr.width,(float)players_arr[i].txr.height}, (Vector2){(float)players_arr[i].posx,(float)players_arr[i].posy}, WHITE);
            else { Color playerColor=GRAY; switch(players_arr[i].classe){case GUERREIRO:playerColor=(i==0)?RED:MAROON;break;case MAGO:playerColor=(i==0)?BLUE:DARKBLUE;break;case ARQUEIRO:playerColor=(i==0)?GREEN:DARKGREEN;break;case BARBARO:playerColor=(i==0)?ORANGE:BROWN;break;case LADINO:playerColor=(i==0)?GRAY:DARKGRAY;break;case CLERIGO:playerColor=(i==0)?GOLD:YELLOW;break;default:playerColor=(i==0)?PURPLE:DARKPURPLE;break;} DrawRectangle(players_arr[i].posx,players_arr[i].posy,players_arr[i].width,players_arr[i].height,playerColor); }
        }
    }
    char mapCoordsText[64]; sprintf(mapCoordsText, "Mapa (X: %d, Y: %d)", currentMapX, currentMapY);
    DrawText(mapCoordsText, 10, 10, 20, YELLOW);
    DrawText(TextFormat("Volume: %.0f%%", currentVolume * 100.0f), 10, virtualScreenHeight - 60, 10, LIGHTGRAY);
    DrawText(TextFormat("Musica %d (Tocando: %s)", currentMusicIndex + 1, isPlaying ? "Sim" : "Nao"), 10, virtualScreenHeight - 45, 10, LIGHTGRAY);
    DrawText("Controles: P/Esc=Pausa, E=Inventario, Setas/WASD=Mover, Shift=Correr", 10, virtualScreenHeight - 30, 10, RAYWHITE);
    DrawText(",/.=Play/Pause Musica, PgUp/Dn=Mudar Musica, +/-/0=Volume", 10, virtualScreenHeight - 15, 10, RAYWHITE);
    DrawFPS(10, virtualScreenHeight - 75); 
}