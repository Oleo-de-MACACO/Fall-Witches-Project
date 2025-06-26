#include "../include/ClassAttacks.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include <ctype.h>

static ClassAttackSet* s_allAttackSets = NULL;
static int s_attackSetCount = 0;

// --- CORREÇÃO: Função local para remover espaços em branco ---
// A função TextTrim não existe na Raylib. Esta função faz o mesmo trabalho.
static char* TrimWhitespace(char* str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static Classe StringToClasse(const char* className) {
    if (strcmp(className, "GUERREIRO") == 0) return GUERREIRO;
    if (strcmp(className, "MAGO") == 0) return MAGO;
    if (strcmp(className, "ARQUEIRO") == 0) return ARQUEIRO;
    if (strcmp(className, "BARBARO") == 0) return BARBARO;
    if (strcmp(className, "LADINO") == 0) return LADINO;
    if (strcmp(className, "CLERIGO") == 0) return CLERIGO;
    return CLASSE_COUNT;
}

static AttackType StringToAttackType(const char* attackType) {
    if (strcmp(attackType, "physical") == 0) return ATTACK_TYPE_PHYSICAL;
    if (strcmp(attackType, "magical") == 0) return ATTACK_TYPE_MAGICAL;
    if (strcmp(attackType, "status") == 0) return ATTACK_TYPE_STATUS;
    return ATTACK_TYPE_PHYSICAL;
}

void ClassAttacks_LoadAll(const char* filePath) {
    if (s_allAttackSets) {
        ClassAttacks_UnloadAll();
    }

    s_allAttackSets = (ClassAttackSet*)RL_CALLOC(CLASSE_COUNT, sizeof(ClassAttackSet));
    if (!s_allAttackSets) {
        TraceLog(LOG_FATAL, "Falha ao alocar memória para os ataques das classes.");
        return;
    }

    FILE* file = fopen(filePath, "rt");
    if (!file) {
        TraceLog(LOG_ERROR, "Falha ao abrir o arquivo de ataques de classe: %s", filePath);
        return;
    }

    char lineBuffer[256];
    ClassAttackSet* currentAttackSet = NULL;

    while (fgets(lineBuffer, sizeof(lineBuffer), file)) {
        if (lineBuffer[0] == '\n' || lineBuffer[0] == '/') continue;

        if (lineBuffer[0] == '[') {
            char className[50];
            sscanf(lineBuffer, "[%[^]]]", className);
            
            Classe classEnum = StringToClasse(className);
            if (classEnum != CLASSE_COUNT) {
                currentAttackSet = &s_allAttackSets[s_attackSetCount];
                currentAttackSet->class_id = classEnum;
                currentAttackSet->attack_count = 0;
                s_attackSetCount++;
            } else {
                currentAttackSet = NULL;
            }
        } else if (currentAttackSet && strncmp(lineBuffer, "ataque:", 7) == 0) {
            if (currentAttackSet->attack_count < MAX_ATTACKS_PER_CLASS) {
                Attack* attack = &currentAttackSet->attacks[currentAttackSet->attack_count];
                char typeStr[20];
                char* linePtr = lineBuffer + 7;
                
                char* token = strtok(linePtr, ",");
                if (token) strncpy(attack->name, TrimWhitespace(token), MAX_ATTACK_NAME_LENGTH - 1);
                
                token = strtok(NULL, ",");
                if (token) {
                    strncpy(typeStr, TrimWhitespace(token), 19);
                    attack->type = StringToAttackType(typeStr);
                }

                token = strtok(NULL, ",");
                if (token) attack->power_multiplier = (float)atof(TrimWhitespace(token));
                
                token = strtok(NULL, ",\n");
                if (token) attack->mana_cost = atoi(TrimWhitespace(token));

                currentAttackSet->attack_count++;
            }
        }
    }

    fclose(file);
    TraceLog(LOG_INFO, "%d conjuntos de ataques carregados de '%s'", s_attackSetCount, filePath);
}

void ClassAttacks_UnloadAll(void) {
    if (s_allAttackSets) {
        RL_FREE(s_allAttackSets);
        s_allAttackSets = NULL;
        s_attackSetCount = 0;
    }
}

const ClassAttackSet* ClassAttacks_GetAttackSetForClass(Classe player_class) {
    for (int i = 0; i < s_attackSetCount; i++) {
        if (s_allAttackSets[i].class_id == player_class) {
            return &s_allAttackSets[i];
        }
    }
    return NULL;
}