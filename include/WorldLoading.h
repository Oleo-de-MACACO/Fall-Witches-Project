#ifndef WORLD_LOADING_H
#define WORLD_LOADING_H

#include "raylib.h"
#include <stdbool.h> // Para bool

// --- Constantes para Caminhos e Arquivos ---
#define WORLD_TEXTURES_PATH "assets/WorldTextures/"
#define WORLD_PLACES_PATH   "assets/WorldPlaces/"
#define WORLD_FILE_PREFIX   "section_"
#define WORLD_FILE_EXTENSION ".png"

// --- Definições de Cores para o Mapa de Dados (WorldPlaces) ---
#define COLOR_COLLISION   (Color){255, 0,   0,   255}
#define COLOR_DOOR        (Color){0,   0,   255, 255}
#define COLOR_PLAYER_SPAWN (Color){0,   255, 0,   255}
#define COLOR_ENEMY_SPAWN (Color){255, 0,   255, 255}
#define COLOR_CHEST_SPAWN (Color){255, 255, 0,   255}
#define COLOR_FORAGE_SPAWN (Color){0,   255, 255, 255}

typedef enum { SPAWN_TYPE_PLAYER, SPAWN_TYPE_ENEMY, SPAWN_TYPE_CHEST, SPAWN_TYPE_FORAGEABLE } SpawnType;
typedef struct { Vector2 position; SpawnType type; } SpawnPoint;
typedef struct { Vector2 position; bool isOpen; } DoorData;
typedef struct WorldSection {
    Texture2D backgroundTexture;
    int width; int height; bool isLoaded;
    Rectangle* collisionRects; int collisionRectCount; int collisionRectCapacity;
    DoorData* doors; int doorCount; int doorCapacity;
    Vector2* playerSpawns; int playerSpawnCount; int playerSpawnCapacity;
    Vector2* enemySpawns; int enemySpawnCount; int enemySpawnCapacity;
    Vector2* chestSpawns; int chestSpawnCount; int chestSpawnCapacity;
    Vector2* forageableSpawns; int forageableSpawnCount; int forageableSpawnCapacity;
} WorldSection;

// Protótipos de Funções
WorldSection* LoadWorldSection(int mapX, int mapY);
void UnloadWorldSection(WorldSection* section);
void DrawWorldSectionBackground(const WorldSection* section);
void DrawWorldSectionDebug(const WorldSection* section);

#endif // WORLD_LOADING_H