#include "../include/EnemyLoader.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"

static EnemyData* s_allEnemyData = NULL;
static int s_enemyDataCount = 0;

// Converte uma string de nome de classe para o enum Classe
static Classe StringToClasse(const char* className) {
    if (strcmp(className, "GUERREIRO") == 0) return GUERREIRO;
    if (strcmp(className, "MAGO") == 0) return MAGO;
    if (strcmp(className, "ARQUEIRO") == 0) return ARQUEIRO;
    if (strcmp(className, "BARBARO") == 0) return BARBARO;
    if (strcmp(className, "LADINO") == 0) return LADINO;
    if (strcmp(className, "CLERIGO") == 0) return CLERIGO;
    return CLASSE_COUNT; // Valor padrão/inválido
}

// Remove espaços em branco do início e do fim de uma string
static char* TrimWhitespace(char* str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

void EnemyLoader_LoadAll(const char* filePath) {
    if (s_allEnemyData) {
        EnemyLoader_UnloadAll();
    }

    FILE* file = fopen(filePath, "rt");
    if (!file) {
        TraceLog(LOG_ERROR, "Falha ao abrir o arquivo de inimigos: %s", filePath);
        return;
    }

    s_allEnemyData = (EnemyData*)RL_CALLOC(MAX_ENEMY_TYPES, sizeof(EnemyData));
    if (!s_allEnemyData) {
        TraceLog(LOG_FATAL, "Falha ao alocar memória para dados de inimigos.");
        fclose(file);
        return;
    }

    char lineBuffer[256];
    EnemyData* currentEnemy = NULL;

    while (fgets(lineBuffer, sizeof(lineBuffer), file)) {
        char* line = TrimWhitespace(lineBuffer);

        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            if (s_enemyDataCount < MAX_ENEMY_TYPES) {
                currentEnemy = &s_allEnemyData[s_enemyDataCount];
                s_enemyDataCount++;
                // Extrai o nome
                strncpy(currentEnemy->name, line + 1, strlen(line) - 2);
                currentEnemy->name[strlen(line) - 2] = '\0';
            } else {
                currentEnemy = NULL;
            }
        } else if (currentEnemy && strchr(line, '=')) {
            char* key = strtok(line, "=");
            char* value = strtok(NULL, "\n");

            if (key && value) {
                key = TrimWhitespace(key);
                value = TrimWhitespace(value);
                if (strcmp(key, "specialty") == 0) currentEnemy->specialty = StringToClasse(value);
                else if (strcmp(key, "hp") == 0) currentEnemy->hp = atoi(value);
                else if (strcmp(key, "mp") == 0) currentEnemy->mp = atoi(value);
                else if (strcmp(key, "atk") == 0) currentEnemy->attack = atoi(value);
                else if (strcmp(key, "def") == 0) currentEnemy->defense = atoi(value);
                else if (strcmp(key, "matk") == 0) currentEnemy->magic_attack = atoi(value);
                else if (strcmp(key, "mdef") == 0) currentEnemy->magic_defense = atoi(value);
            }
        }
    }

    fclose(file);
    TraceLog(LOG_INFO, "%d tipos de inimigos carregados de '%s'", s_enemyDataCount, filePath);
}

const EnemyData* EnemyLoader_GetByName(const char* enemyName) {
    for (int i = 0; i < s_enemyDataCount; i++) {
        if (strcmp(s_allEnemyData[i].name, enemyName) == 0) {
            return &s_allEnemyData[i];
        }
    }
    return NULL;
}

void EnemyLoader_UnloadAll(void) {
    if (s_allEnemyData) {
        RL_FREE(s_allEnemyData);
        s_allEnemyData = NULL;
        s_enemyDataCount = 0;
    }
}
