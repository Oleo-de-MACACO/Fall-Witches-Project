#include "../include/SaveLoad.h"
#include "../include/Classes.h"
#include <stdio.h>
#include "raylib.h" // For TraceLog, DirectoryExists, FileExists

const unsigned int SAVEGAME_FILE_VERSION = 1;

/**
 * @brief Verifica se o diretório de saves existe e loga um aviso se não existir.
 * Não tenta criar o diretório para manter a compatibilidade entre versões da Raylib.
 * @return true se o diretório existe, false caso contrário (operações de save/load devem falhar).
 */
bool EnsureSavesDirectoryExists(void) {
    if (!DirectoryExists(SAVE_DIRECTORY)) {
        TraceLog(LOG_WARNING, "Diretório de saves '%s' NÃO existe. Por favor, crie-o manualmente para que as funções de save/load funcionem.", SAVE_DIRECTORY);
        return false; // Indica que o diretório não está pronto
    }
    return true;
}

/**
 * @brief Gera o nome completo do arquivo para um slot de save.
 */
void GetSaveFileName(char *buffer, int buffer_size, int slot_number) {
    snprintf(buffer, buffer_size, "%s/Save%d.sav", SAVE_DIRECTORY, slot_number + 1);
}

/**
 * @brief Verifica se um arquivo de save para o slot especificado existe.
 */
bool DoesSaveSlotExist(int slot_number) {
    if (!DirectoryExists(SAVE_DIRECTORY)) return false; // Se o diretório base não existe, o slot não pode existir.
    char filename[100];
    GetSaveFileName(filename, sizeof(filename), slot_number);
    return FileExists(filename);
}

/**
 * @brief Salva os dados do jogo no slot especificado.
 */
bool SaveGame(Player players[], int num_players, int slot_number) {
    if (!EnsureSavesDirectoryExists()) { // Verifica (e avisa) se o diretório não existe
        TraceLog(LOG_ERROR, "Diretório de saves '%s' não encontrado. Impossível salvar.", SAVE_DIRECTORY);
        return false;
    }

    char filename[100];
    GetSaveFileName(filename, sizeof(filename), slot_number);

    FILE *saveFile = fopen(filename, "wb");
    if (saveFile == NULL) {
        TraceLog(LOG_ERROR, "Falha ao abrir arquivo de save '%s' para escrita.", filename);
        return false;
    }

    size_t items_written;
    items_written = fwrite(&SAVEGAME_FILE_VERSION, sizeof(unsigned int), 1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever versão do save."); fclose(saveFile); return false; }
    items_written = fwrite(&num_players, sizeof(int), 1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever número de jogadores."); fclose(saveFile); return false; }

    for (int i = 0; i < num_players; i++) {
        items_written = fwrite(&players[i].nivel, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error;
        items_written = fwrite(&players[i].exp, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error;
        items_written = fwrite(&players[i].vida, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error;
        items_written = fwrite(&players[i].mana, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error;
        items_written = fwrite(&players[i].max_vida, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error;
        items_written = fwrite(&players[i].max_mana, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error;

        for (int j = 0; j < MAX_INVENTORY_SLOTS; j++) {
            items_written = fwrite(&players[i].inventory[j], sizeof(InventoryItem), 1, saveFile);
            if (items_written != 1) goto save_error;
        }
        items_written = fwrite(&players[i].inventory_item_count, sizeof(int), 1, saveFile);
        if (items_written != 1) goto save_error;

        for (int j = 0; j < MAX_EQUIP_SLOTS; j++) {
            items_written = fwrite(&players[i].equipped_items[j], sizeof(InventoryItem), 1, saveFile);
            if (items_written != 1) goto save_error;
        }
    }

    fclose(saveFile);
    TraceLog(LOG_INFO, "Jogo salvo com sucesso em '%s'.", filename);
    return true;

save_error:
    TraceLog(LOG_ERROR, "Erro durante a escrita no arquivo de save '%s'.", filename);
    fclose(saveFile);
    return false;
}

/**
 * @brief Carrega os dados do jogo do slot especificado.
 */
bool LoadGame(Player players[], int num_players_in_game, int slot_number) {
    if (!DirectoryExists(SAVE_DIRECTORY)){ // Não tenta carregar se o diretório nem existe
         TraceLog(LOG_WARNING, "Diretório de saves '%s' não encontrado. Impossível carregar.", SAVE_DIRECTORY);
         return false;
    }

    char filename[100];
    GetSaveFileName(filename, sizeof(filename), slot_number);

    if (!FileExists(filename)) {
        TraceLog(LOG_WARNING, "Arquivo de save '%s' não existe para carregar.", filename);
        return false;
    }

    FILE *loadFile = fopen(filename, "rb");
    if (loadFile == NULL) {
        TraceLog(LOG_ERROR, "Falha ao abrir arquivo de save '%s' para leitura (apesar de existir).", filename);
        return false;
    }

    size_t items_read;
    unsigned int file_version = 0;
    items_read = fread(&file_version, sizeof(unsigned int), 1, loadFile);
    if (items_read != 1 || file_version != SAVEGAME_FILE_VERSION) {
        TraceLog(LOG_ERROR, "Versão incompatível ou erro ao ler versão de '%s'. Esperado: %u, Encontrado: %u.", filename, SAVEGAME_FILE_VERSION, file_version);
        fclose(loadFile); return false;
    }
    int num_players_saved = 0;
    items_read = fread(&num_players_saved, sizeof(int), 1, loadFile);
    if (items_read != 1 || num_players_saved != num_players_in_game) {
        TraceLog(LOG_ERROR, "Número de jogadores em '%s' (%d) incompatível com o jogo (%d) ou erro de leitura.", filename, num_players_saved, num_players_in_game);
        fclose(loadFile); return false;
    }

    for (int i = 0; i < num_players_saved; i++) {
        if (i >= num_players_in_game) {
            long offset = sizeof(int) * 6 + MAX_INVENTORY_SLOTS * sizeof(InventoryItem) + sizeof(int) + MAX_EQUIP_SLOTS * sizeof(InventoryItem);
            if (fseek(loadFile, offset, SEEK_CUR) != 0) {
                 TraceLog(LOG_ERROR,"Erro ao pular dados de jogador extra no save file.");
                 fclose(loadFile); return false;
            }
            continue;
        }
        items_read = fread(&players[i].nivel, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error;
        items_read = fread(&players[i].exp, sizeof(int), 1, loadFile);   if (items_read != 1) goto load_error;
        items_read = fread(&players[i].vida, sizeof(int), 1, loadFile);   if (items_read != 1) goto load_error;
        items_read = fread(&players[i].mana, sizeof(int), 1, loadFile);   if (items_read != 1) goto load_error;
        items_read = fread(&players[i].max_vida, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error;
        items_read = fread(&players[i].max_mana, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error;

        for (int j = 0; j < MAX_INVENTORY_SLOTS; j++) {
            items_read = fread(&players[i].inventory[j], sizeof(InventoryItem), 1, loadFile);
            if (items_read != 1) goto load_error;
        }
        items_read = fread(&players[i].inventory_item_count, sizeof(int), 1, loadFile);
        if (items_read != 1) goto load_error;

        for (int j = 0; j < MAX_EQUIP_SLOTS; j++) {
            items_read = fread(&players[i].equipped_items[j], sizeof(InventoryItem), 1, loadFile);
            if (items_read != 1) goto load_error;
        }
         if (players[i].vida > players[i].max_vida) players[i].vida = players[i].max_vida;
         if (players[i].mana > players[i].max_mana) players[i].mana = players[i].max_mana;
    }

    fclose(loadFile);
    TraceLog(LOG_INFO, "Jogo carregado com sucesso de '%s'.", filename);
    return true;

load_error:
    TraceLog(LOG_ERROR, "Erro durante a leitura do arquivo de save '%s'.", filename);
    fclose(loadFile);
    return false;
}