#include "../include/Event.h"
#include "../include/Sound.h"
#include "../include/GameProgress.h" // *** ADICIONADO PARA GERENCIAMENTO DE PROGRESSO ***
#include "../include/Dialogue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Removido o array estático s_visitedMaps e suas funções auxiliares.
// Agora usaremos o sistema centralizado de GameProgress.

static GameEvent* s_allEvents = NULL; static int s_eventCount = 0; static int s_eventCapacity = 0;
static void AddEventToList(GameEvent event) {
    if (s_eventCount >= s_eventCapacity) { s_eventCapacity = (s_eventCapacity == 0) ? 16 : s_eventCapacity * 2;
        s_allEvents = (GameEvent*)RL_REALLOC(s_allEvents, (size_t)s_eventCapacity * sizeof(GameEvent));
        if (!s_allEvents) { TraceLog(LOG_FATAL, "Falha ao alocar eventos!"); return; }
    } s_allEvents[s_eventCount++] = event;
}
static char* TrimWhitespace(char* str) {
    char *end; while (isspace((unsigned char)*str)) str++; if (*str == 0) return str;
    end = str + strlen(str) - 1; while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0'; return str;
}
static MusicCategory StringToMusicCategory(const char* categoryName) {
    if (strcmp(categoryName, "MainMenu") == 0) return MUSIC_CATEGORY_MAINMENU; if (strcmp(categoryName, "Game") == 0) return MUSIC_CATEGORY_GAME;
    if (strcmp(categoryName, "Battle") == 0) return MUSIC_CATEGORY_BATTLE; if (strcmp(categoryName, "Cutscene") == 0) return MUSIC_CATEGORY_CUTSCENE;
    if (strcmp(categoryName, "Nature") == 0) return MUSIC_CATEGORY_AMBIENT_NATURE; if (strcmp(categoryName, "City") == 0) return MUSIC_CATEGORY_AMBIENT_CITY;
    if (strcmp(categoryName, "Cave") == 0) return MUSIC_CATEGORY_AMBIENT_CAVE; return MUSIC_CATEGORY_GAME;
}
static void ExecuteEventActions(GameEvent* event) {
    TraceLog(LOG_INFO, "Executando acoes para o evento ID: %d", event->id);
    for (int i = 0; i < event->actionCount; i++) {
        EventAction* action = &event->actions[i];
        switch(action->type) {
            case ACTION_PLAY_MUSIC:
                if (strlen(action->data.musicData.songName) > 0) {
                    MusicCategory cat = StringToMusicCategory(action->data.musicData.category);
                    int trackIndex = Sound_GetMusicIndexByName(cat, action->data.musicData.songName);
                    if (trackIndex != -1) { PlayMusicTrack(cat, trackIndex, action->data.musicData.loop); }
                } else { MusicCategory cat = StringToMusicCategory(action->data.musicData.category); PlayRandomMusicFromCategory(cat, action->data.musicData.loop); }
                break;
            case ACTION_SPAWN_NPC: TraceLog(LOG_INFO, "Acao SPAWN_NPC (nao implementada)."); break;
            case ACTION_GIVE_ITEM: TraceLog(LOG_INFO, "Acao GIVE_ITEM (nao implementada)."); break;
            case ACTION_SHOW_DIALOGUE:
            TraceLog(LOG_INFO, "Evento %d: Acionando dialogo com ID %d.", event->id, action->data.dialogueId);
            Dialogue_StartById(action->data.dialogueId);
            break;
            default: break;
        }
    }
}


void Event_LoadAll(const char* eventsFilePath) {
    if (s_allEvents) { Event_UnloadAll(); }
    // O progresso NÃO é mais resetado aqui. Ele é resetado apenas ao iniciar um "novo jogo".
    // Progress_Reset(); // REMOVIDO DAQUI

    FILE* file = fopen(eventsFilePath, "rt"); if (!file) { TraceLog(LOG_ERROR, "Falha ao abrir eventos: %s", eventsFilePath); return; }
    char line[256]; GameEvent* currentEvent = NULL; int parseState = 0;
    while (fgets(line, sizeof(line), file)) {
        char* trimmedLine = TrimWhitespace(line); if (strlen(trimmedLine) == 0 || trimmedLine[0] == '/') continue;
        if (strncmp(trimmedLine, "event:", 6) == 0) {
            if (currentEvent) { AddEventToList(*currentEvent); RL_FREE(currentEvent); }
            currentEvent = (GameEvent*)RL_CALLOC(1, sizeof(GameEvent)); if (currentEvent) sscanf(trimmedLine + 6, "%d", &currentEvent->id); continue;
        }
        if (strcmp(trimmedLine, "{") == 0 && currentEvent) { if (parseState != 2) parseState = 1; continue; }
        if (strcmp(trimmedLine, "}") == 0 && currentEvent) {
             if (parseState > 1) { if(currentEvent->actionCount < MAX_EVENT_ACTIONS && currentEvent->actions[currentEvent->actionCount].type != ACTION_NONE) currentEvent->actionCount++; parseState = 2; }
             else { AddEventToList(*currentEvent); RL_FREE(currentEvent); currentEvent = NULL; parseState = 0; }
             continue;
        }
        if (strcmp(trimmedLine, "actions:") == 0) { parseState = 2; continue; }
        if (strcmp(trimmedLine, "play_music:") == 0 && parseState == 2) {
             parseState = 3; if(currentEvent && currentEvent->actionCount < MAX_EVENT_ACTIONS) currentEvent->actions[currentEvent->actionCount].type = ACTION_PLAY_MUSIC; continue;
        }
        char* key = strtok(trimmedLine, "="); char* value = strtok(NULL, "=");
        if (key && value) {
            key = TrimWhitespace(key); value = TrimWhitespace(value);
            if(currentEvent) {
                if (parseState == 3) {
                    EventAction* action = &currentEvent->actions[currentEvent->actionCount];
                    if (strcmp(key, "category") == 0) strncpy(action->data.musicData.category, value, MAX_NAME_LENGTH -1);
                    else if (strcmp(key, "songname") == 0) strncpy(action->data.musicData.songName, value, MAX_NAME_LENGTH -1);
                    else if (strcmp(key, "loop") == 0) action->data.musicData.loop = (strcmp(value, "true") == 0);
                } else {
                    if (strcmp(key, "map") == 0) sscanf(value, " (%d,%d)", &currentEvent->mapX, &currentEvent->mapY);
                    else if (strcmp(key, "triggers: player_first_time_in_map") == 0 && strcmp(value, "true") == 0) { currentEvent->trigger = TRIGGER_ON_MAP_ENTRY_FIRST_TIME; }
                }
            }
        }
    }
    if (currentEvent) { AddEventToList(*currentEvent); RL_FREE(currentEvent); }
    fclose(file); TraceLog(LOG_INFO, "%d eventos carregados de '%s'", s_eventCount, eventsFilePath);
}

void Event_UnloadAll(void) { if (s_allEvents) { RL_FREE(s_allEvents); s_allEvents = NULL; } s_eventCount = 0; s_eventCapacity = 0; }

void Event_CheckAndRun(int currentMapX, int currentMapY) {
    for (int i = 0; i < s_eventCount; i++) {
        GameEvent* event = &s_allEvents[i];
        if (event->hasFired && !event->isRepeatable) continue;
        if (event->mapX == currentMapX && event->mapY == currentMapY) {
            bool triggerMet = false;
            switch (event->trigger) {
                case TRIGGER_ON_MAP_ENTRY_FIRST_TIME:
                    // *** USA O NOVO SISTEMA DE PROGRESSO PERSISTENTE ***
                    if (!Progress_HasVisitedMap(currentMapX, currentMapY)) {
                        triggerMet = true;
                        Progress_MapWasVisited(currentMapX, currentMapY); // Marca o mapa como visitado
                    }
                    break;
                default: break;
            }
            if (triggerMet) {
                ExecuteEventActions(event);
                event->hasFired = true;
            }
        }
    }
}