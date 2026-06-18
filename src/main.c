#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "config.h"

int main(void) {
    srand((unsigned)time(NULL));

    if (!al_init())                    { fprintf(stderr, "al_init failed\n"); return 1; }
    if (!al_init_primitives_addon())   { fprintf(stderr, "primitives failed\n"); return 1; }
    if (!al_install_keyboard())        { fprintf(stderr, "keyboard failed\n"); return 1; }
    if (!al_init_font_addon())         { fprintf(stderr, "font failed\n"); return 1; }
    if (!al_init_ttf_addon())          { fprintf(stderr, "ttf failed\n"); return 1; }
    if (!al_install_audio())           { fprintf(stderr, "audio failed\n"); return 1; }
    if (!al_init_acodec_addon())       { fprintf(stderr, "acodec failed\n"); return 1; }
    if (!al_reserve_samples(4))        { fprintf(stderr, "samples failed\n"); return 1; }

    GAME *game = game_create();
    if (!game) return 1;

    if (game_init(game) == ERROR) {
        game_destroy(game);
        return 1;
    }

    ALLEGRO_MONITOR_INFO info;
    al_get_monitor_info(0, &info);
    int mx = info.x2 - info.x1;
    int my = info.y2 - info.y1;
    al_set_window_position(game_get_display(game),
        (mx - DISPLAY_WIDTH) / 2, (my - DISPLAY_HEIGHT) / 2);

    al_start_timer(game_get_timer(game));

    ALLEGRO_KEYBOARD_STATE key;
    while (!game_is_done(game)) {
        al_wait_for_event(game_get_queue(game), game_get_event(game));
        al_get_keyboard_state(&key);

        if (game_get_event(game)->type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;

        if (game_get_event(game)->type == ALLEGRO_EVENT_TIMER) {
            if (game_update(game, &key) == ERROR) break;
            if (game_render(game) == ERROR) break;
        }
    }

    game_destroy(game);
    return 0;
}
