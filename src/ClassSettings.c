#include "../include/ClassSettings.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Para RL_MALLOC e RL_FREE
#include "raylib.h" // Para TraceLog e funções de string

static ClassMoveset* s_allMovesets = NULL;
static int s_movesetCount = 0;

// Funções auxiliares para parsing
static Classe StringToClasse(const char* className) {
    if (strcmp(className, "GUERREIRO") == 0) return GUERREIRO;
    if (strcmp(className, "MAGO") == 0) return MAGO;
    if (strcmp(className, "ARQUEIRO") == 0) return ARQUEIRO;
    if (strcmp(className, "BARBARO") == 0) return BARBARO;
    if (strcmp(className, "LADINO") == 0) return LADINO;
    if (strcmp(className, "CLERIGO") == 0) return CLERIGO;
    return CLASSE_COUNT; // Valor inválido
}

static MoveType StringToMoveType(const char* moveType) {
    if (strcmp(moveType, "physical") == 0) return MOVE_TYPE_PHYSICAL;
    if (strcmp(moveType, "magical") == 0) return MOVE_TYPE_MAGICAL;
    if (strcmp(moveType, "status") == 0) return MOVE_TYPE_STATUS;
    return MOVE_TYPE_PHYSICAL;
}

void ClassSettings_LoadAll(const char* filePath) {
    if (s_allMovesets) {
        RL_FREE(s_allMovesets);
        s_allMovesets = NULL;
        s_movesetCount = 0;
    }

    // Alocação inicial para todas as classes possíveis
    s_allMovesets = (ClassMoveset*)RL_CALLOC(CLASSE_COUNT, sizeof(ClassMoveset));
    if (!s_allMovesets) {
        TraceLog(LOG_FATAL, "Falha ao alocar memória para os conjuntos de movimentos.");
        return;
    }

    FILE* file = fopen(filePath, "rt");
    if (!file) {
        TraceLog(LOG_ERROR, "Falha ao abrir o arquivo de configurações de classe: %s", filePath);
        return;
    }

    char lineBuffer[256];
    ClassMoveset* currentMoveset = NULL;

    while (fgets(lineBuffer, sizeof(lineBuffer), file)) {
        if (lineBuffer[0] == '[' && lineBuffer[strlen(lineBuffer) - 2] == ']') {
            char className[50];
            strncpy(className, lineBuffer + 1, strlen(lineBuffer) - 3);
            className[strlen(lineBuffer) - 3] = '\0';
            
            Classe classEnum = StringToClasse(className);
            if (classEnum != CLASSE_COUNT) {
                currentMoveset = &s_allMovesets[s_movesetCount];
                currentMoveset->class_id = classEnum;
                currentMoveset->move_count = 0;
                s_movesetCount++;
            } else {
                currentMoveset = NULL;
            }
        } else if (currentMoveset && strncmp(lineBuffer, "move:", 5) == 0) {
            if (currentMoveset->move_count < MAX_MOVES_PER_CLASS) {
                Move* move = &currentMoveset->moves[currentMoveset->move_count];
                char* token = strtok(lineBuffer + 5, ",");
                if (token) strncpy(move->name, token, MAX_MOVE_NAME_LENGTH - 1);

                token = strtok(NULL, ",");
                if (token) move->type = StringToMoveType(token);

                token = strtok(NULL, ",");
                if (token) move->power_multiplier = (float)atof(token);
                
                token = strtok(NULL, ",\n");
                if (token) move->mana_cost = atoi(token);

                currentMoveset->move_count++;
            }
        }
    }

    fclose(file);
    TraceLog(LOG_INFO, "%d conjuntos de movimentos de classe carregados de '%s'", s_movesetCount, filePath);
}

const ClassMoveset* ClassSettings_GetMovesForClass(Classe player_class) {
    for (int i = 0; i < s_movesetCount; i++) {
        if (s_allMovesets[i].class_id == player_class) {
            return &s_allMovesets[i];
        }
    }
    return NULL;
}

void ClassSettings_UnloadAll(void) {
    if(s_allMovesets){
        RL_FREE(s_allMovesets);
        s_allMovesets = NULL;
        s_movesetCount = 0;
    }
}
