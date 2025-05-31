#include <stddef.h>
#include <unistd.h>
#include "../include/Game.h"

void *pixels();

int main(void){
  int i;
  float music_timer = 0;
  float music_duration = 0;
  float volume = 0.5f;
  unsigned int screen_width = 800;
  unsigned int screen_height = 450;
  Player player[MAX_PLAYERS];
  Color color[MAX_SIZE] = {GREEN, RED};

  for(i = 0; i < MAX_PLAYERS; i++){
    player[i].width = 80;
    player[i].height = 80;

    if(i == 0){
      player[i].posx = player[i].width;
      player[i].posy = player[i].height;
    }

    if(i == 1){
      player[i].posx = screen_width - player[i].width;
      player[i].posy = 0 + player[i].height;
    }
  }

  SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
  SetConfigFlags(FLAG_BORDERLESS_WINDOWED_MODE);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screen_width, screen_height, "Fall Witches");
  InitAudioDevice();

  const char *image = "./assets/images/player1.png";
  player[1].txr = LoadTexture(image);
  player[1].txr.width = 80;
  player[1].txr.height = 80;

  const char *musics[MAX_SIZE] = {
    "./assets/songs/Desmeon_-_My_Sunshine.ogg",
    "./assets/songs/19 William Tell Overture.ogg",
    "./assets/songs/last-summer-by-ikson.mp3",
    "./assets/songs/Floatinurboat - Spirit Of Things.mp3"
  };

  Music playlist[MAX_SIZE];

  for(i = 0; i < MAX_SIZE; i++){
    playlist[i] = LoadMusicStream(musics[i]);
    if((playlist[i].stream.buffer) == NULL){
      TraceLog(LOG_WARNING, "Musica %d não foi carregada\n", i+1);
    }
  }

  int cur_music = 0;
  int isPlaying = 1; // 1 quando a música está tocando e 0 quando a música está parada

  PlayMusicStream(playlist[cur_music]);
  SetMusicVolume(playlist[cur_music], volume);

  SetTargetFPS(30);

  while (!WindowShouldClose()){

    float scaleX = (float)GetScreenWidth() / screen_width;
    float scaleY = (float)GetScreenHeight() / screen_height;

    if(isPlaying){
      UpdateMusicStream(playlist[cur_music]);
      music_duration = GetMusicTimeLength(playlist[cur_music]);
      music_timer += GetFrameTime();
    }

		// pausa a música
    if(IsKeyPressed(KEY_SLASH)){
      if(isPlaying){
        ResumeMusicStream(playlist[cur_music]);
        isPlaying = 1;
      } else {
        PauseMusicStream(playlist[cur_music]);
        isPlaying = 0;
      }
    }

    // próxima música
    if(IsKeyPressed(KEY_PAGE_DOWN)){
      if(isPlaying){
        StopMusicStream(playlist[cur_music]);
        isPlaying = 0;

        cur_music++;
        if(cur_music == MAX_SIZE){
          cur_music = 0;
        }

        PlayMusicStream(playlist[cur_music]);
        isPlaying = 1;
      }
    }

    // música anterior
    if(IsKeyPressed(KEY_PAGE_UP)){
      if(isPlaying){
        StopMusicStream(playlist[cur_music]);
        isPlaying = 0;

        cur_music--;
        if(cur_music < 0){
          cur_music = MAX_SIZE - 1;
        }

        PlayMusicStream(playlist[cur_music]);
        isPlaying = 1;
      }
    }

    if(IsKeyPressed(KEY_ZERO)){
        if(isPlaying){
            volume = 0.0f;
            SetMusicVolume(playlist[cur_music], volume);
        }
    }

    if(IsKeyPressed(KEY_MINUS)){
      if(isPlaying){
        volume -= 0.05f;
        if(volume <= 0.0f){
          volume = 0.0f;
        }
        SetMusicVolume(playlist[cur_music], volume);
      }
    }

    if(IsKeyPressed(KEY_EQUAL)){
      if(isPlaying){
        volume += 0.05f;
        if(volume >= 1.0f){
          volume = 1.0f;
        }
        SetMusicVolume(playlist[cur_music], volume);
      }
    }

    // verifica se a música terminou
    if(music_timer >= music_duration){
			StopMusicStream(playlist[cur_music]);

			cur_music++;
			if(cur_music > MAX_SIZE - 1){
				cur_music = 0;
			}

			PlayMusicStream(playlist[cur_music]);
    }

    move_character(&player[1].posx, &player[1].posy, screen_width, screen_height, KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT);
    move_character(&player[0].posx, &player[0].posy, screen_width, screen_height, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_RIGHT_SHIFT);

    Vector2 mouse = GetMousePosition();

    BeginDrawing();
      ClearBackground(BLACK);

      UpdateTexture(player[1].txr, pixels);

      DrawFPS(5, 20);
      DrawText(TextFormat("X: %d, Y: %d", (int)mouse.x, (int)mouse.y), 5, 40, 18, DARKGRAY);
      DrawRectangle(0, 0, screen_width, 20, DARKGRAY);
      DrawTexture(player[1].txr, player[1].posx, player[1].posy, RAYWHITE);
      DrawRectangle(player[0].posx, player[0].posy, player[0].width, player[0].height, color[0]);
    EndDrawing();
  }

  UnloadTexture(player[1].txr);

  for(i = 0; i < MAX_SIZE; i++){
    UnloadMusicStream(playlist[i]);
  }

  CloseAudioDevice();
  CloseWindow();
  return 0;
}

void *pixels(){

}
