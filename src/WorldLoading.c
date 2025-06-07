#include "../include/WorldLoading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

// Funções auxiliares (AreColorsEqual, AddCollisionRect, etc.) como na Resposta 48
static bool AreColorsEqual(Color c1, Color c2) { return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b); }
static void AddCollisionRect(WorldSection* section, Rectangle rect) { if (!section) return; if (section->collisionRectCount >= section->collisionRectCapacity) { int newCap = (section->collisionRectCapacity == 0) ? 256 : section->collisionRectCapacity * 2; Rectangle* newRects = (Rectangle*)RL_REALLOC(section->collisionRects, (size_t)newCap * sizeof(Rectangle)); if (!newRects) { TraceLog(LOG_ERROR, "Falha ao realocar collisionRects"); return; } section->collisionRects = newRects; section->collisionRectCapacity = newCap; } section->collisionRects[section->collisionRectCount++] = rect; }
static void AddDoor(WorldSection* section, Vector2 pos) { if(!section) return; if(section->doorCount >= section->doorCapacity){ int nCap = section->doorCapacity == 0 ? 8 : section->doorCapacity*2; DoorData* nD = (DoorData*)RL_REALLOC(section->doors, (size_t)nCap*sizeof(DoorData)); if(!nD)return; section->doors=nD; section->doorCapacity=nCap; } section->doors[section->doorCount].position=pos; section->doors[section->doorCount].isOpen=false; section->doorCount++;}
static void AddSpawnToList(Vector2** spawnList, int* spawnCount, int* spawnCapacity, Vector2 pos) { if (!spawnList || !spawnCount || !spawnCapacity) return; if (*spawnCount >= *spawnCapacity) { int newCap = (*spawnCapacity == 0) ? 8 : *spawnCapacity * 2; Vector2* newList = (Vector2*)RL_REALLOC(*spawnList, (size_t)newCap * sizeof(Vector2)); if (!newList) { TraceLog(LOG_ERROR, "Falha ao realocar lista de spawn"); return; } *spawnList = newList; *spawnCapacity = newCap; } (*spawnList)[*spawnCount] = pos; (*spawnCount)++; }
static void ProcessColorPatch(Color* pixels, int imgWidth, int imgHeight, int startX, int startY, Color targetColor, bool* visited, Vector2** spawnList, int* spawnCount, int* spawnCapacity) {
    if (startX < 0 || startX >= imgWidth || startY < 0 || startY >= imgHeight || visited[startY * imgWidth + startX] || !AreColorsEqual(pixels[startY * imgWidth + startX], targetColor)) { return; }
    AddSpawnToList(spawnList, spawnCount, spawnCapacity, (Vector2){(float)startX, (float)startY});
    Vector2* queue = (Vector2*)RL_MALLOC((size_t)imgWidth * (size_t)imgHeight * sizeof(Vector2)); if(!queue) { TraceLog(LOG_ERROR, "Falha ao alocar fila do flood fill"); return; }
    int queueFront = 0, queueRear = 0; queue[queueRear++] = (Vector2){(float)startX, (float)startY}; visited[startY * imgWidth + startX] = true;
    int dx[] = {0, 0, -1, 1}; int dy[] = {-1, 1, 0, 0};
    while (queueFront < queueRear) {
        Vector2 currentPixel = queue[queueFront++];
        for (int i = 0; i < 4; i++) {
            int nextX = (int)currentPixel.x + dx[i]; int nextY = (int)currentPixel.y + dy[i];
            if (nextX >= 0 && nextX < imgWidth && nextY >= 0 && nextY < imgHeight && !visited[nextY * imgWidth + nextX] && AreColorsEqual(pixels[nextY * imgWidth + nextX], targetColor)) {
                visited[nextY * imgWidth + nextX] = true; queue[queueRear++] = (Vector2){(float)nextX, (float)nextY};
            }
        }
    } RL_FREE(queue);
}

WorldSection* LoadWorldSection(int mapX, int mapY) {
    TraceLog(LOG_INFO, "LoadWorldSection: Iniciando carregamento da secao do mapa: %d, %d", mapX, mapY);
    WorldSection* section = (WorldSection*)RL_CALLOC(1, sizeof(WorldSection));
    if (!section) { TraceLog(LOG_ERROR, "LoadWorldSection: Falha ao alocar memoria para WorldSection."); return NULL; }
    section->isLoaded = false;
    char texFilePath[256];
    char placesFilePath[256];

    snprintf(texFilePath, sizeof(texFilePath), "%s%s%d_%d%s", WORLD_TEXTURES_PATH, WORLD_FILE_PREFIX, mapX, mapY, WORLD_FILE_EXTENSION);
    snprintf(placesFilePath, sizeof(placesFilePath), "%s%s%d_%d%s", WORLD_PLACES_PATH, WORLD_FILE_PREFIX, mapX, mapY, WORLD_FILE_EXTENSION);

    // 1. Carrega Textura de Fundo. Suas dimensões são a autoridade.
    if (FileExists(texFilePath)) {
        section->backgroundTexture = LoadTexture(texFilePath);
        if (section->backgroundTexture.id > 0) {
            section->width = section->backgroundTexture.width;
            section->height = section->backgroundTexture.height;
            section->isLoaded = true;
        } else { TraceLog(LOG_WARNING, "LoadWorldSection: Falha ao carregar textura: %s", texFilePath); }
    } else { TraceLog(LOG_WARNING, "LoadWorldSection: Arquivo de textura NAO ENCONTRADO: %s", texFilePath); }

    // 2. Carrega Imagem de Dados (WorldPlaces)
    if (!FileExists(placesFilePath)) {
        TraceLog(LOG_WARNING, "LoadWorldSection: Arquivo de 'places' NAO ENCONTRADO: %s. Nenhum elemento de colisao/spawn sera carregado.", placesFilePath);
        // Se nem a textura carregou, a seção é inútil.
        if (!section->isLoaded) { RL_FREE(section); return NULL; }
        return section;
    }

    Image placesImage = LoadImage(placesFilePath);
    if (placesImage.data == NULL) {
        TraceLog(LOG_WARNING, "LoadWorldSection: Falha ao carregar imagem de 'places': %s", placesFilePath);
        if (!section->isLoaded) { RL_FREE(section); return NULL; }
        return section;
    }

    // *** VERIFICAÇÃO CRÍTICA DE DIMENSÕES ***
    if (section->isLoaded) { // Se a textura de fundo carregou e definiu as dimensões da seção
        if (placesImage.width != section->width || placesImage.height != section->height) {
            TraceLog(LOG_ERROR, "LoadWorldSection: INCOMPATIBILIDADE DE DIMENSOES! Textura de fundo (%s) e %dx%d, mas imagem de places (%s) e %dx%d. Elementos de 'places' NAO SERAO PROCESSADOS.",
                     texFilePath, section->width, section->height, placesFilePath, placesImage.width, placesImage.height);
            UnloadImage(placesImage); // Descarrega a imagem de places pois não será usada
            return section; // Retorna a seção apenas com o fundo visual
        }
    } else { // Textura de fundo não carregou, usa dimensões de places como autoridade
        section->width = placesImage.width;
        section->height = placesImage.height;
        section->isLoaded = true;
    }

    // Processa os pixels da imagem de dados
    Color* pixels = LoadImageColors(placesImage);
    if (!pixels) { UnloadImage(placesImage); return section; }
    bool* visited = (bool*)RL_CALLOC((size_t)section->width * (size_t)section->height, sizeof(bool));
    if (!visited) { UnloadImageColors(pixels); UnloadImage(placesImage); return section; }

    for (int y = 0; y < section->height; y++) {
        for (int x = 0; x < section->width; x++) {
            Color c = pixels[y * section->width + x];
            if (AreColorsEqual(c, COLOR_COLLISION)) { AddCollisionRect(section, (Rectangle){(float)x, (float)y, 1.0f, 1.0f}); }
            // ... (lógica para outros tipos de pixel como portas, spawns, etc.)
        }
    }
    RL_FREE(visited); UnloadImageColors(pixels); UnloadImage(placesImage);
    TraceLog(LOG_INFO, "LoadWorldSection: Secao %d,%d processada com %d retangulos de colisao.", mapX, mapY, section->collisionRectCount);
    return section;
}

void UnloadWorldSection(WorldSection* section) {
    if (!section) return;
    if (section->backgroundTexture.id > 0) { UnloadTexture(section->backgroundTexture); }
    RL_FREE(section->collisionRects); RL_FREE(section->doors); RL_FREE(section->playerSpawns);
    RL_FREE(section->enemySpawns); RL_FREE(section->chestSpawns); RL_FREE(section->forageableSpawns);
    RL_FREE(section);
}
void DrawWorldSectionBackground(const WorldSection* section) {
    if (section && section->isLoaded && section->backgroundTexture.id > 0) {
        DrawTexture(section->backgroundTexture, 0, 0, WHITE);
    }
}
void DrawWorldSectionDebug(const WorldSection* section) {
    if (!section || !section->isLoaded) return;
    for (int i = 0; i < section->collisionRectCount; i++) {
        DrawRectangleRec(section->collisionRects[i], Fade(RED, 0.5f));
    }
}