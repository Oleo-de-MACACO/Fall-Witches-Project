#include "../include/WorldLoading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

// --- Funções Auxiliares ---
static bool AreColorsEqual(Color c1, Color c2) { return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b); }

static void AddCollisionRect(WorldSection* section, Rectangle rect) {
    if (!section) return;
    if (section->collisionRectCount >= section->collisionRectCapacity) {
        int newCap = (section->collisionRectCapacity == 0) ? 256 : section->collisionRectCapacity * 2;
        Rectangle* newRects = (Rectangle*)RL_REALLOC(section->collisionRects, (size_t)newCap * sizeof(Rectangle));
        if (!newRects) { TraceLog(LOG_ERROR, "Falha ao realocar collisionRects"); return; }
        section->collisionRects = newRects;
        section->collisionRectCapacity = newCap;
    }
    section->collisionRects[section->collisionRectCount++] = rect;
}

static void AddSpawnToList(Vector2** spawnList, int* spawnCount, int* spawnCapacity, Vector2 pos) {
    if (!spawnList || !spawnCount || !spawnCapacity) return;
    if (*spawnCount >= *spawnCapacity) {
        int newCap = (*spawnCapacity == 0) ? 8 : *spawnCapacity * 2;
        Vector2* newList = (Vector2*)RL_REALLOC(*spawnList, (size_t)newCap * sizeof(Vector2));
        if (!newList) { TraceLog(LOG_ERROR, "Falha ao realocar lista de spawn"); return; }
        *spawnList = newList;
        *spawnCapacity = newCap;
    }
    (*spawnList)[*spawnCount] = pos;
    (*spawnCount)++;
}

/**
 * @brief Encontra todos os pixels conectados de uma determinada cor (patch) e os marca como visitados.
 * Isso evita que múltiplos objetos (como spawn points) sejam criados para a mesma área.
 * @param pixels Ponteiro para os dados de cor da imagem.
 * @param imgWidth Largura da imagem.
 * @param imgHeight Altura da imagem.
 * @param startX Coordenada X inicial da busca.
 * @param startY Coordenada Y inicial da busca.
 * @param targetColor A cor do patch a ser consumido.
 * @param visited Ponteiro para o array de pixels visitados.
 */
static void ProcessAndConsumePatch(Color* pixels, int imgWidth, int imgHeight, int startX, int startY, Color targetColor, bool* visited) {
    if (startX < 0 || startX >= imgWidth || startY < 0 || startY >= imgHeight || visited[startY * imgWidth + startX]) {
        return;
    }

    // Usa uma fila para uma busca em largura (flood fill) para encontrar todos os pixels conectados.
    Vector2* queue = (Vector2*)RL_MALLOC((size_t)imgWidth * (size_t)imgHeight * sizeof(Vector2));
    if(!queue) { TraceLog(LOG_ERROR, "Falha ao alocar fila do flood fill"); return; }
    
    int queueFront = 0, queueRear = 0;
    queue[queueRear++] = (Vector2){(float)startX, (float)startY};
    visited[startY * imgWidth + startX] = true;

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    while (queueFront < queueRear) {
        Vector2 currentPixel = queue[queueFront++];
        for (int i = 0; i < 4; i++) {
            int nextX = (int)currentPixel.x + dx[i];
            int nextY = (int)currentPixel.y + dy[i];
            if (nextX >= 0 && nextX < imgWidth && nextY >= 0 && nextY < imgHeight && !visited[nextY * imgWidth + nextX] && AreColorsEqual(pixels[nextY * imgWidth + nextX], targetColor)) {
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
    section->isLoaded = false;
    char texFilePath[256];
    char placesFilePath[256];

    snprintf(texFilePath, sizeof(texFilePath), "%s%s%d_%d%s", WORLD_TEXTURES_PATH, WORLD_FILE_PREFIX, mapX, mapY, WORLD_FILE_EXTENSION);
    snprintf(placesFilePath, sizeof(placesFilePath), "%s%s%d_%d%s", WORLD_PLACES_PATH, WORLD_FILE_PREFIX, mapX, mapY, WORLD_FILE_EXTENSION);

    if (FileExists(texFilePath)) {
        section->backgroundTexture = LoadTexture(texFilePath);
        if (section->backgroundTexture.id > 0) {
            section->width = section->backgroundTexture.width;
            section->height = section->backgroundTexture.height;
            section->isLoaded = true;
        } else { TraceLog(LOG_WARNING, "LoadWorldSection: Falha ao carregar textura: %s", texFilePath); }
    } else { TraceLog(LOG_WARNING, "LoadWorldSection: Arquivo de textura NAO ENCONTRADO: %s", texFilePath); }

    if (!FileExists(placesFilePath)) {
        if (!section->isLoaded) { RL_FREE(section); return NULL; }
        return section;
    }

    Image placesImage = LoadImage(placesFilePath);
    if (placesImage.data == NULL) {
        if (!section->isLoaded) { RL_FREE(section); return NULL; }
        return section;
    }

    if (section->isLoaded) {
        if (placesImage.width != section->width || placesImage.height != section->height) {
            TraceLog(LOG_ERROR, "LoadWorldSection: INCOMPATIBILIDADE DE DIMENSOES! Textura de fundo e imagem de places com tamanhos diferentes.");
            UnloadImage(placesImage);
            return section;
        }
    } else {
        section->width = placesImage.width;
        section->height = placesImage.height;
        section->isLoaded = true;
    }

    Color* pixels = LoadImageColors(placesImage);
    if (!pixels) { UnloadImage(placesImage); return section; }
    
    bool* visited = (bool*)RL_CALLOC((size_t)section->width * (size_t)section->height, sizeof(bool));
    if (!visited) { UnloadImageColors(pixels); UnloadImage(placesImage); return section; }

    for (int y = 0; y < section->height; y++) {
        for (int x = 0; x < section->width; x++) {
            if (visited[y * section->width + x]) continue;
            
            Color c = pixels[y * section->width + x];
            
            if (AreColorsEqual(c, COLOR_COLLISION)) {
                int rectWidth = 1;
                while (x + rectWidth < section->width && !visited[y * section->width + (x + rectWidth)] && AreColorsEqual(pixels[y * section->width + (x + rectWidth)], COLOR_COLLISION)) {
                    rectWidth++;
                }

                int rectHeight = 1;
                bool canExpandDown = true;
                while (canExpandDown && (y + rectHeight < section->height)) {
                    for (int k = 0; k < rectWidth; k++) {
                        if (visited[(y + rectHeight) * section->width + (x + k)] || !AreColorsEqual(pixels[(y + rectHeight) * section->width + (x + k)], COLOR_COLLISION)) {
                            canExpandDown = false;
                            break;
                        }
                    }
                    if (canExpandDown) rectHeight++;
                }

                AddCollisionRect(section, (Rectangle){(float)x, (float)y, (float)rectWidth, (float)rectHeight});
                for (int j = 0; j < rectHeight; j++) {
                    for (int i = 0; i < rectWidth; i++) {
                        visited[(y + j) * section->width + (x + i)] = true;
                    }
                }
            } 
            else if (AreColorsEqual(c, COLOR_PLAYER_SPAWN)) {
                // *** CORREÇÃO: Trata um patch verde como uma única área de spawn ***
                // Adiciona apenas o primeiro pixel encontrado como o ponto de spawn.
                AddSpawnToList(&section->playerSpawns, &section->playerSpawnCount, &section->playerSpawnCapacity, (Vector2){(float)x, (float)y});
                // Em seguida, consome todo o resto do patch verde para que não sejam adicionados mais pontos.
                ProcessAndConsumePatch(pixels, section->width, section->height, x, y, COLOR_PLAYER_SPAWN, visited);
            }
        }
    }

    RL_FREE(visited);
    UnloadImageColors(pixels);
    UnloadImage(placesImage);
    TraceLog(LOG_INFO, "LoadWorldSection: Secao %d,%d processada com %d retangulos de colisao (Otimizado) e %d spawns de jogador (Otimizado).", mapX, mapY, section->collisionRectCount, section->playerSpawnCount);
    return section;
}

void UnloadWorldSection(WorldSection* section) {
    if (!section) return;
    if (section->backgroundTexture.id > 0) { UnloadTexture(section->backgroundTexture); }
    RL_FREE(section->collisionRects);
    RL_FREE(section->doors);
    RL_FREE(section->playerSpawns);
    RL_FREE(section->enemySpawns);
    RL_FREE(section->chestSpawns);
    RL_FREE(section->forageableSpawns);
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
    for (int i = 0; i < section->playerSpawnCount; i++) {
        DrawCircleV(section->playerSpawns[i], 5, Fade(GREEN, 0.7f));
    }
}