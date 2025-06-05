#include "../include/WorldLoading.h"
#include <stdio.h>
#include <stdlib.h> // Para RL_CALLOC, RL_REALLOC, RL_FREE (se não vierem de raylib.h diretamente)
#include <string.h>
#include "raylib.h" // Para tipos Raylib, FileExists, LoadTexture, LoadImage, etc.

const Color APP_CYAN = {0, 255, 255, 255};

static bool AreColorsEqual(Color c1, Color c2) {
    return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}

static void AddCollisionRect(WorldSection* section, Rectangle rect) {
    if (!section) return;
    if (section->collisionRectCount >= section->collisionRectCapacity) {
        int newCapacity = (section->collisionRectCapacity == 0) ? 16 : section->collisionRectCapacity * 2;
        // Cast explícito para (size_t) para a capacidade ao calcular o novo tamanho
        Rectangle* newRects = (Rectangle*)RL_REALLOC(section->collisionRects, (size_t)newCapacity * sizeof(Rectangle));
        if (!newRects) {
            TraceLog(LOG_ERROR, "Falha ao realocar memoria para collisionRects (capacidade: %d)", newCapacity);
            // Se RL_REALLOC falhar e retornar NULL, section->collisionRects original ainda é válido mas pequeno.
            // Ou pode-se optar por liberar section->collisionRects e tratar como erro fatal.
            return;
        }
        section->collisionRects = newRects;
        section->collisionRectCapacity = newCapacity;
    }
    section->collisionRects[section->collisionRectCount++] = rect;
}

static void AddDoor(WorldSection* section, Vector2 pos) {
    if (!section) return;
    if (section->doorCount >= section->doorCapacity) {
        int newCapacity = (section->doorCapacity == 0) ? 8 : section->doorCapacity * 2;
        // Cast explícito para (size_t)
        DoorData* newDoors = (DoorData*)RL_REALLOC(section->doors, (size_t)newCapacity * sizeof(DoorData));
        if (!newDoors) {
            TraceLog(LOG_ERROR, "Falha ao realocar memoria para doors (capacidade: %d)", newCapacity);
            return;
        }
        section->doors = newDoors;
        section->doorCapacity = newCapacity;
    }
    section->doors[section->doorCount].position = pos;
    section->doors[section->doorCount].isOpen = false;
    section->doorCount++;
}

static void AddSpawnToList(Vector2** spawnList, int* spawnCount, int* spawnCapacity, Vector2 pos) {
    if (!spawnList || !spawnCount || !spawnCapacity) return;
    if (*spawnCount >= *spawnCapacity) {
        int newCapacity = (*spawnCapacity == 0) ? 8 : *spawnCapacity * 2;
        // Cast explícito para (size_t)
        Vector2* newList = (Vector2*)RL_REALLOC(*spawnList, (size_t)newCapacity * sizeof(Vector2));
        if (!newList) {
            TraceLog(LOG_ERROR, "Falha ao realocar memoria para lista de spawn (capacidade: %d)", newCapacity);
            return;
        }
        *spawnList = newList;
        *spawnCapacity = newCapacity;
    }
    (*spawnList)[*spawnCount] = pos;
    (*spawnCount)++;
}

static void ProcessColorPatch(Color* pixels, int imgWidth, int imgHeight, int startX, int startY, Color targetColor, bool* visited,
                               Vector2** spawnList, int* spawnCount, int* spawnCapacity) {
    if (startX < 0 || startX >= imgWidth || startY < 0 || startY >= imgHeight ||
        visited[startY * imgWidth + startX] || !AreColorsEqual(pixels[startY * imgWidth + startX], targetColor)) {
        return;
    }
    AddSpawnToList(spawnList, spawnCount, spawnCapacity, (Vector2){(float)startX, (float)startY});

    // Cast explícito para (size_t) para o tamanho da fila
    Vector2* queue = (Vector2*)RL_MALLOC((size_t)imgWidth * (size_t)imgHeight * sizeof(Vector2));
    if(!queue) { TraceLog(LOG_ERROR, "Falha ao alocar memoria para fila do flood fill"); return; }

    int queueFront = 0, queueRear = 0;
    queue[queueRear++] = (Vector2){(float)startX, (float)startY};
    visited[startY * imgWidth + startX] = true;
    int dx[] = {0, 0, -1, 1}; int dy[] = {-1, 1, 0, 0};
    while (queueFront < queueRear) {
        Vector2 currentPixel = queue[queueFront++];
        for (int i = 0; i < 4; i++) {
            int nextX = (int)currentPixel.x + dx[i]; int nextY = (int)currentPixel.y + dy[i];
            if (nextX >= 0 && nextX < imgWidth && nextY >= 0 && nextY < imgHeight &&
                !visited[nextY * imgWidth + nextX] &&
                AreColorsEqual(pixels[nextY * imgWidth + nextX], targetColor)) {
                visited[nextY * imgWidth + nextX] = true;
                queue[queueRear++] = (Vector2){(float)nextX, (float)nextY};
            }
        }
    }
    RL_FREE(queue);
}

WorldSection* LoadWorldSection(int mapX, int mapY) {
    TraceLog(LOG_INFO, "LoadWorldSection: Iniciando carregamento da secao do mapa: %d, %d", mapX, mapY);
    WorldSection* section = (WorldSection*)RL_CALLOC(1, sizeof(WorldSection));
    if (!section) { TraceLog(LOG_ERROR, "LoadWorldSection: Falha ao alocar memoria para WorldSection."); return NULL; }
    section->isLoaded = false; char filePath[256]; char texFilePath[256]; char placesFilePath[256];
    snprintf(texFilePath, sizeof(texFilePath), "%s%s%d_%d%s", WORLD_TEXTURES_PATH, WORLD_FILE_PREFIX, mapX, mapY, WORLD_FILE_EXTENSION);
    snprintf(placesFilePath, sizeof(placesFilePath), "%s%s%d_%d%s", WORLD_PLACES_PATH, WORLD_FILE_PREFIX, mapX, mapY, WORLD_FILE_EXTENSION);

    if (FileExists(texFilePath)) {
        section->backgroundTexture = LoadTexture(texFilePath);
        if (section->backgroundTexture.id > 0) {
            section->width = section->backgroundTexture.width; section->height = section->backgroundTexture.height;
            TraceLog(LOG_INFO, "LoadWorldSection: Textura de fundo '%s' CARREGADA. Dimensoes: %dx%d", texFilePath, section->width, section->height);
            section->isLoaded = true;
        } else { TraceLog(LOG_WARNING, "LoadWorldSection: Falha ao carregar textura de fundo: %s.", texFilePath); section->width = 0; section->height = 0; }
    } else { TraceLog(LOG_WARNING, "LoadWorldSection: Arquivo de textura de fundo NAO ENCONTRADO: %s", texFilePath); section->width = 0; section->height = 0; }

    if (!FileExists(placesFilePath)) {
        TraceLog(LOG_WARNING, "LoadWorldSection: Arquivo de 'places' NAO ENCONTRADO: %s.", placesFilePath);
        if (!section->isLoaded) { RL_FREE(section); return NULL; } return section;
    }
    Image placesImage = LoadImage(placesFilePath);
    if (placesImage.data == NULL) { TraceLog(LOG_WARNING, "Falha ao carregar imagem de 'places': %s", placesFilePath); if (!section->isLoaded) { RL_FREE(section); return NULL; } return section; }

    if (section->isLoaded) { // Se textura de fundo carregou
        if (placesImage.width != section->width || placesImage.height != section->height) {
            TraceLog(LOG_ERROR, "LoadWorldSection: INCOMPATIBILIDADE DE DIMENSOES! Textura (%dx%d) vs Places (%dx%d) para secao (%d,%d). Elementos de 'places' NAO PROCESSADOS.",
                     section->width, section->height, placesImage.width, placesImage.height, mapX, mapY);
            UnloadImage(placesImage); return section;
        }
    } else { // Fundo não carregou, usa dimensões de places se válidas
        if (placesImage.width > 0 && placesImage.height > 0) {
            section->width = placesImage.width; section->height = placesImage.height; section->isLoaded = true;
            TraceLog(LOG_INFO, "LoadWorldSection: Usando dimensoes de places (%dx%d) para secao (%d,%d).", section->width, section->height, mapX, mapY);
        } else {
            TraceLog(LOG_ERROR, "LoadWorldSection: Imagem de places (%s) com dimensoes invalidas e fundo falhou. Falha ao carregar secao.", placesFilePath);
            UnloadImage(placesImage); RL_FREE(section); return NULL;
        }
    }

    Color* pixels = LoadImageColors(placesImage);
    if (!pixels) { TraceLog(LOG_ERROR, "Falha ao carregar cores de places."); UnloadImage(placesImage); if(!section->isLoaded) RL_FREE(section); return section->isLoaded ? section : NULL; }
    
    // Cast explícito para (size_t) para o tamanho do array 'visited'
    bool* visited = (bool*)RL_CALLOC((size_t)section->width * (size_t)section->height, sizeof(bool));
    if (!visited) { TraceLog(LOG_ERROR, "Falha ao alocar 'visited'."); UnloadImageColors(pixels); UnloadImage(placesImage); if(!section->isLoaded) RL_FREE(section); return section->isLoaded ? section : NULL; }

    for (int y = 0; y < section->height; y++) {
        for (int x = 0; x < section->width; x++) {
            Color currentColor = pixels[y * section->width + x]; Vector2 currentPos = {(float)x, (float)y};
            if (AreColorsEqual(currentColor, COLOR_COLLISION)) { AddCollisionRect(section, (Rectangle){currentPos.x, currentPos.y, 1.0f, 1.0f});}
            else if (AreColorsEqual(currentColor, COLOR_DOOR)) { if (!visited[y * section->width + x]) { AddDoor(section, currentPos); visited[y * section->width + x] = true; }}
            else if (AreColorsEqual(currentColor, COLOR_PLAYER_SPAWN)) { if (!visited[y * section->width + x]) { ProcessColorPatch(pixels, section->width, section->height, x, y, COLOR_PLAYER_SPAWN, visited, &section->playerSpawns, &section->playerSpawnCount, &section->playerSpawnCapacity);}}
            else if (AreColorsEqual(currentColor, COLOR_ENEMY_SPAWN)) { if (!visited[y * section->width + x]) { ProcessColorPatch(pixels, section->width, section->height, x, y, COLOR_ENEMY_SPAWN, visited, &section->enemySpawns, &section->enemySpawnCount, &section->enemySpawnCapacity);}}
            else if (AreColorsEqual(currentColor, COLOR_CHEST_SPAWN)) { if (!visited[y * section->width + x]) { ProcessColorPatch(pixels, section->width, section->height, x, y, COLOR_CHEST_SPAWN, visited, &section->chestSpawns, &section->chestSpawnCount, &section->chestSpawnCapacity);}}
            else if (AreColorsEqual(currentColor, COLOR_FORAGE_SPAWN)) { if (!visited[y * section->width + x]) { ProcessColorPatch(pixels, section->width, section->height, x, y, COLOR_FORAGE_SPAWN, visited, &section->forageableSpawns, &section->forageableSpawnCount, &section->forageableSpawnCapacity);}}
        }
    }
    RL_FREE(visited); UnloadImageColors(pixels); UnloadImage(placesImage);
    TraceLog(LOG_INFO, "LoadWorldSection: Secao %d,%d processada. Elementos: Col:%d, Por:%d, PSp:%d, ESp:%d, CSp:%d, FSp:%d", mapX, mapY, section->collisionRectCount, section->doorCount, section->playerSpawnCount, section->enemySpawnCount, section->chestSpawnCount, section->forageableSpawnCount);
    return section;
}

void UnloadWorldSection(WorldSection* section) {
    if (!section) return;
    TraceLog(LOG_INFO, "UnloadWorldSection: Descarregando secao (W:%d, H:%d)", section->width, section->height);
    if (section->backgroundTexture.id > 0) UnloadTexture(section->backgroundTexture);
    RL_FREE(section->collisionRects); section->collisionRects = NULL;
    RL_FREE(section->doors); section->doors = NULL;
    RL_FREE(section->playerSpawns); section->playerSpawns = NULL;
    RL_FREE(section->enemySpawns); section->enemySpawns = NULL;
    RL_FREE(section->chestSpawns); section->chestSpawns = NULL;
    RL_FREE(section->forageableSpawns); section->forageableSpawns = NULL;
    RL_FREE(section);
}

void DrawWorldSectionBackground(const WorldSection* section) { if (section && section->isLoaded && section->backgroundTexture.id > 0) DrawTexture(section->backgroundTexture, 0, 0, WHITE); }
void DrawWorldSectionDebug(const WorldSection* section) { 
    if (!section || !section->isLoaded) return;
    for (int i = 0; i < section->collisionRectCount; i++) DrawRectangleLinesEx(section->collisionRects[i], 1.0f, Fade(RED, 0.7f));
    for (int i = 0; i < section->doorCount; i++) { DrawCircleV(section->doors[i].position, 3, Fade(BLUE, 0.7f)); DrawText("D", (int)section->doors[i].position.x - 4, (int)section->doors[i].position.y - 6, 10, BLUE); }
    for (int i = 0; i < section->playerSpawnCount; i++) { DrawCircleV(section->playerSpawns[i], 4, Fade(GREEN, 0.7f)); DrawText("P", (int)section->playerSpawns[i].x - 4, (int)section->playerSpawns[i].y - 7, 12, GREEN); }
    for (int i = 0; i < section->enemySpawnCount; i++) { DrawCircleV(section->enemySpawns[i], 4, Fade(MAGENTA, 0.7f)); DrawText("E", (int)section->enemySpawns[i].x - 4, (int)section->enemySpawns[i].y - 7, 12, MAGENTA); }
    for (int i = 0; i < section->chestSpawnCount; i++) { DrawRectangleRoundedLinesEx((Rectangle){section->chestSpawns[i].x -3, section->chestSpawns[i].y-3, 6,6}, 0.3f, 4, 1.5f, Fade(YELLOW,0.7f)); DrawText("C", (int)section->chestSpawns[i].x - 4, (int)section->chestSpawns[i].y - 7, 12, YELLOW); }
    for (int i = 0; i < section->forageableSpawnCount; i++) { DrawCircleV(section->forageableSpawns[i], 3, Fade(APP_CYAN, 0.7f)); DrawText("F", (int)section->forageableSpawns[i].x - 3, (int)section->forageableSpawns[i].y - 6, 10, APP_CYAN); }
}