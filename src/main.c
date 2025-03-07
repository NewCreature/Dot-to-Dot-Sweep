#include "t3f/t3f.h"
#include "t3f/resource.h"
#include "t3f/music.h"
#include "t3f/view.h"
#include "t3f/draw.h"
#include "t3f/input.h"
#include "t3net/leaderboard.h"
#include "avc/avc.h"
#include "instance.h"
#include "intro.h"
#include "game.h"
#include "leaderboard.h"
#include "privacy.h"
#include "input.h"
#include "mouse.h"
#include "text.h"

static bool dot_show_touch_hand = false;
static int dot_screenshot_count = 0;

static T3F_BITMAP * dot_create_scratch_bitmap(int w, int h)
{
	ALLEGRO_STATE old_state;
	T3F_BITMAP * bp;

	al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_NO_PRESERVE_TEXTURE);
	bp = t3f_create_bitmap(w, h, -1, -1, 0);
	al_restore_state(&old_state);

	return bp;
}

/* create view that fits the entire gameplay area, including the HUD */
static void _dot_update_main_view(T3F_VIEW * vp)
{
	float ox, oy, w, h;

	ox = t3f_default_view->left;
	oy = t3f_default_view->top;
	w = t3f_default_view->right - t3f_default_view->left;
	h = (float)DOT_DISPLAY_GAMEPLAY_HEIGHT * (w / (float)DOT_DISPLAY_GAMEPLAY_WIDTH);
	vp->virtual_width = DOT_DISPLAY_GAMEPLAY_WIDTH;
	vp->virtual_height = DOT_DISPLAY_GAMEPLAY_HEIGHT;
	t3f_adjust_view(vp, ox, oy, w, h, DOT_GAME_PLAYFIELD_WIDTH / 2.0, DOT_GAME_PLAYFIELD_HEIGHT / 2.0, 0);
}

static void _dot_update_menu_view(T3F_VIEW * vp, T3F_VIEW * main_view, bool mobile)
{
	float ox, oy, w, h, s;

	if(mobile)
	{
		ox = t3f_default_view->left;
		oy = main_view->offset_y + main_view->height;
		w = t3f_default_view->right - t3f_default_view->left;
		h = t3f_virtual_display_height - oy;
		s = main_view->width / t3f_default_view->virtual_width;
		vp->virtual_width = w / s;
		vp->virtual_height = h / s;
	}
	else
	{
		ox = 0;
		oy = 0;
		w = DOT_DISPLAY_GAMEPLAY_WIDTH;
		h = DOT_DISPLAY_GAMEPLAY_HEIGHT - (DOT_DISPLAY_GAMEPLAY_HEIGHT - DOT_GAME_PLAYFIELD_HEIGHT);
		vp->virtual_width = w;
		vp->virtual_height = h;
	}
	t3f_adjust_view(vp, ox, oy, w, h, DOT_GAME_PLAYFIELD_WIDTH / 2.0, DOT_GAME_PLAYFIELD_HEIGHT / 2.0, 0);
}

static void _dot_update_views(void * data, int w, int h)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int mobile_flags = 0;

	if(w > h)
	{
		app->desktop_mode = true;
	}
	else
	{
		app->desktop_mode = false;
	}
	if((float)w / (float)h < 9.0 / 16.0)
	{
		mobile_flags = T3F_FORCE_ASPECT | T3F_FILL_SCREEN;
	}
	else
	{
		mobile_flags = T3F_FORCE_ASPECT;
	}
	t3f_set_view_virtual_dimensions(t3f_default_view, DOT_DISPLAY_WIDTH, DOT_DISPLAY_HEIGHT);
	t3f_adjust_view(t3f_default_view, 0, 0, w, h, DOT_GAME_PLAYFIELD_WIDTH / 2, DOT_GAME_PLAYFIELD_HEIGHT / 2, app->desktop_mode ? T3F_FORCE_ASPECT : mobile_flags);
	t3f_select_view(t3f_default_view);
	_dot_update_main_view(app->main_view);
	_dot_update_menu_view(app->menu_view, app->main_view, !app->desktop_mode);
	dot_intro_center_menus(data);
}

static void dot_event_handler(ALLEGRO_EVENT * event, void * data)
{
	ALLEGRO_STATE old_state;
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	switch(event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
		{
			t3f_destroy_bitmap(app->bitmap[DOT_BITMAP_SCRATCH]);
			t3f_event_handler(event);
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
		{
			t3f_event_handler(event);
			al_store_state(&old_state, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
			al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_NO_PRESERVE_TEXTURE);
			app->bitmap[DOT_BITMAP_SCRATCH] = dot_create_scratch_bitmap(DOT_BITMAP_SCRATCH_WIDTH, DOT_BITMAP_SCRATCH_HEIGHT);
			al_restore_state(&old_state);
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_ORIENTATION:
		{
			ALLEGRO_MONITOR_INFO info;
			int w, h;

			al_get_monitor_info(0, &info);
			w = info.x2 - info.x1;
			h = info.y2 - info.y1;
//			t3f_set_gfx_mode(DOT_DISPLAY_WIDTH, DOT_DISPLAY_HEIGHT, t3f_flags | T3F_RESET_DISPLAY);
			_dot_update_views(data, w, h);
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		{
			t3f_event_handler(event);
			_dot_update_views(data, event->display.width, event->display.height);
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			t3f_event_handler(event);
			if(app->input_type != DOT_INPUT_MOUSE)
			{
				app->want_mouse = true;
			}
			break;
		}
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			t3f_event_handler(event);
			if(app->touch_cooldown_ticks > 0)
			{
				app->touch_cooldown_ticks--;
			}
			if(app->touch_cooldown_ticks <= 0)
			{
				if(app->input_type != DOT_INPUT_MOUSE)
				{
					app->want_mouse = true;
				}
			}
			break;
		}

		case ALLEGRO_EVENT_TOUCH_BEGIN:
		{
			t3f_event_handler(event);
			app->start_touch = true;
			app->want_touch = true;
			app->input_type = DOT_INPUT_NONE;
			app->touch_cooldown_ticks = DOT_TOUCH_COOLDOWN_TICKS;
			break;
		}
		case ALLEGRO_EVENT_TOUCH_MOVE:
		{
			t3f_event_handler(event);
			app->want_touch = true;
			app->input_type = DOT_INPUT_NONE;
			app->touch_cooldown_ticks = DOT_TOUCH_COOLDOWN_TICKS;
			break;
		}
		case ALLEGRO_EVENT_TOUCH_CANCEL:
		case ALLEGRO_EVENT_TOUCH_END:
		{
			t3f_event_handler(event);
			app->touch_cooldown_ticks = DOT_TOUCH_COOLDOWN_TICKS;
			break;
		}

		/* pass the event through to T3F for handling by default */
		default:
		{
			t3f_event_handler(event);
			dot_handle_joystick_event(&app->controller, event);
			break;
		}
	}
}

void app_mouse_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->touch_id = 0;
	t3f_select_input_view(app->main_view);
	app->old_touch_x = app->touch_x;
	app->old_touch_y = app->touch_y;
	app->touch_x = t3f_get_mouse_x();
	app->touch_y = t3f_get_mouse_y();
}

void app_touch_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	t3f_select_input_view(app->main_view);
	app->touch_id = -1;
	for(i = 1; i < T3F_MAX_TOUCHES; i++)
	{
		if(t3f_touch_active(i))
		{
			app->touch_id = i;
			break;
		}
	}
	app->old_touch_x = app->touch_x;
	app->old_touch_y = app->touch_y;
	if(app->touch_id > 0)
	{
		app->touch_x = t3f_get_touch_x(app->touch_id);
		app->touch_y = t3f_get_touch_y(app->touch_id);
		if(app->start_touch)
		{
			app->old_touch_x = app->touch_x;
			app->old_touch_y = app->touch_y;
			app->start_touch = false;
		}
	}
}

void app_logic(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i, x, y;
	short touch_x, touch_y;
	bool frame;

	if(app->demo_file)
	{
		/* convert touch data to integer for demo operations */
		touch_x = t3f_get_mouse_x();
		touch_y = t3f_get_mouse_y();
//		t3f_touch[0].x = touch_x;
//		t3f_touch[0].y = touch_y;

		/* load touch data from file if we aren't recording */
		if(!app->demo_recording)
		{
			frame = al_fgetc(app->demo_file);
			if(frame)
			{
//				t3f_touch[0].active = al_fgetc(app->demo_file);
//				t3f_touch[0].x = al_fread16le(app->demo_file);
//				t3f_touch[0].y = al_fread16le(app->demo_file);
//				t3f_mouse_x = t3f_touch[0].x;
//				t3f_mouse_y = t3f_touch[0].y;
			}
			else
			{
				app->demo_done = true;
				t3f_exit();
			}
		}

		/* write touch data to file if we are recording */
		else
		{
			al_fputc(app->demo_file, 1);
			al_fputc(app->demo_file, t3f_touch_active(0));
			al_fwrite16le(app->demo_file, touch_x);
			al_fwrite16le(app->demo_file, touch_y);
		}
	}

	/* capture screenshot */
	if(t3f_key_pressed(ALLEGRO_KEY_PRINTSCREEN) || t3f_key_pressed(ALLEGRO_KEY_TILDE) || t3f_key_pressed(104))
	{
		char buf[32];

		sprintf(buf, "screen_%d.png", dot_screenshot_count);
		al_save_bitmap(buf, al_get_backbuffer(t3f_display));
		dot_screenshot_count++;
		t3f_use_key_press(ALLEGRO_KEY_PRINTSCREEN);
		t3f_use_key_press(ALLEGRO_KEY_TILDE);
		t3f_use_key_press(104);
	}

	/* read keyboard and controller state */
	if(!app->entering_name)
	{
		dot_read_input(&app->controller);
	}
	if(app->controller.axis_x != 0.0 || app->controller.axis_y != 0.0 || app->controller.button)
	{
		app->want_controller = true;
	}

	/* handle input type switching */
	if(app->want_controller)
	{
		app->input_type = DOT_INPUT_CONTROLLER;
		app->want_controller = false;
	}
	else if(app->want_mouse)
	{
		t3f_get_mouse_mickeys(&x, &y, &i);
		if(app->state == DOT_STATE_GAME && app->game.state == DOT_GAME_STATE_PLAY)
		{
			t3f_set_mouse_xy(app->game.player.ball.x, app->game.player.ball.y);
			app->mickey_ticks = 3;
		}
		app->input_type = DOT_INPUT_MOUSE;
		app->want_mouse = false;
	}
	else if(app->want_touch)
	{
		app->input_type = DOT_INPUT_TOUCH;
		app->want_touch = false;
	}

	/* read input state */
	switch(app->input_type)
	{
		case DOT_INPUT_MOUSE:
		{
			app_mouse_logic(data);
			break;
		}
		case DOT_INPUT_TOUCH:
		{
			app_touch_logic(data);
			break;
		}
		case DOT_INPUT_CONTROLLER:
		{
			break;
		}
	}

	if(app->mickey_ticks)
	{
		t3f_get_mouse_mickeys(&x, &y, &i);
		if(x != 0 || y != 0)
		{
			app->mickey_ticks--;
		}
	}

	switch(app->state)
	{
		case DOT_STATE_INTRO:
		{
			dot_intro_logic(data);
			break;
		}
		case DOT_STATE_LEADERBOARD:
		{
			dot_leaderboard_logic(data);
			break;
		}
		case DOT_STATE_PRIVACY:
		{
			dot_privacy_logic(data);
			break;
		}
		case DOT_STATE_GAME:
		{
			dot_game_logic(data);
			break;
		}
	}
	dot_mouse_cursor_logic();
	app->point_particles = 0;
	app->splat_particles = 0;
	for(i = 0; i < DOT_MAX_PARTICLES; i++)
	{
		if(app->particle[i].active)
		{
			if(app->particle[i].type == 0)
			{
				app->point_particle[app->point_particles] = &app->particle[i];
				app->point_particles++;
			}
			else
			{
				app->splat_particle[app->splat_particles] = &app->particle[i];
				app->splat_particles++;
			}
		}
		dot_particle_logic(&app->particle[i]);
	}
	if(app->reset_steam_stats)
	{
		t3f_reset_steam_stats();
		for(i = 0; i < app->achievements->entries; i++)
		{
			t3f_update_achievement_progress(app->achievements, i, 0);
			app->achievements->entry[i].store_state = 0;
		}
		app->reset_steam_stats = false;
	}
	t3f_steam_integration_logic();
}

void app_render(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;
	int i;

	switch(app->state)
	{
		case DOT_STATE_INTRO:
		{
			dot_intro_render(data);
			break;
		}
		case DOT_STATE_LEADERBOARD:
		{
			dot_leaderboard_render(data);
			break;
		}
		case DOT_STATE_PRIVACY:
		{
			dot_privacy_render(data);
			break;
		}
		case DOT_STATE_GAME:
		{
			dot_game_render(data);
			break;
		}
	}
	t3f_select_view(app->main_view);
	al_hold_bitmap_drawing(true);
	qsort(app->point_particle, app->point_particles, sizeof(DOT_PARTICLE *), dot_particle_qsort_helper);
	for(i = 0; i < app->point_particles; i++)
	{
		dot_particle_render(app->point_particle[i], app->bitmap[DOT_BITMAP_PARTICLE]);
	}
	if(app->desktop_mode)
	{
		float ox = 0, oy = 0;

		if(dot_show_touch_hand && t3f_get_mouse_y() >= t3f_virtual_display_height - DOT_GAME_PLAYFIELD_HEIGHT)
		{
			ox = 12.0;
			oy = -12.0;
			t3f_draw_rotated_bitmap(app->bitmap[DOT_BITMAP_HAND], al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 92, 24, t3f_get_mouse_x(), t3f_get_mouse_y(), 0, 0, 0);
			al_draw_rotated_bitmap(t3f_touch_active(0) ? app->bitmap[DOT_BITMAP_HAND_DOWN]->bitmap : app->bitmap[DOT_BITMAP_HAND]->bitmap, 92, 24, t3f_get_mouse_x() + ox, t3f_get_mouse_y() + oy, 0, 0);
		}
	}
	dot_render_cursor(app->bitmap[DOT_BITMAP_CURSOR], -2.0, -2.0);
	al_hold_bitmap_drawing(false);
}

static bool dot_load_bitmap(APP_INSTANCE * app, int bitmap, const char * fn, int size, int flags)
{
	char buf[256];
	int i;

	for(i = size; i >= 1; i--)
	{
		sprintf(buf, "data/graphics/%dx/%s", i, fn);
		app->bitmap[bitmap] = t3f_load_bitmap(buf, flags, false);
		if(app->bitmap[bitmap])
		{
			return true;
		}
	}
	printf("Failed to load image %d (%s)!\n", bitmap, fn);
	return false;
}

static bool dot_load_font(APP_INSTANCE * app, int font, const char * fn, int size)
{
	app->font[font] = t3f_load_font(fn, T3F_FONT_TYPE_AUTO, size, 0, false);
	if(!app->font[font])
	{
		printf("Failed to load font %d!\n", font);
		return false;
	}
	return true;
}

static ALLEGRO_COLOR dot_get_ball_color(ALLEGRO_BITMAP * bp)
{
	ALLEGRO_COLOR c;

	al_lock_bitmap(bp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
	c = al_get_pixel(bp, al_get_bitmap_width(bp) / 2, al_get_bitmap_height(bp) / 2);
	al_unlock_bitmap(bp);
	return c;
}

bool app_avc_init_proc(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app->demo_file = al_fopen(app->demo_filename, "rb");
	if(!app->demo_file)
	{
		printf("Failed to open demo file.\n");
		return false;
	}
	app->demo_seed = al_fread32le(app->demo_file);
	t3f_srand(&app->rng_state, app->demo_seed);
	app->demo_done = false;
	app->state = 0;
	return true;
}

bool app_avc_logic_proc(void * data)
{
	APP_INSTANCE * app = (APP_INSTANCE *)data;

	app_logic(data);
	if(app->demo_done)
	{
		al_fclose(app->demo_file);
		app->demo_file = NULL;
		return false;
	}
	return true;
}

static void free_graphics(APP_INSTANCE * app)
{
	int i;

	for(i = 0; i < DOT_MAX_BITMAPS; i++)
	{
		if(app->bitmap[i])
		{
			if(!t3f_destroy_resource(app->bitmap[i]))
			{
				t3f_destroy_bitmap(app->bitmap[i]);
			}
			app->bitmap[i] = NULL;
		}
	}
	for(i = 0; i < DOT_MAX_FONTS; i++)
	{
		if(app->font[i])
		{
			if(!t3f_destroy_resource(app->font[i]))
			{
				t3f_destroy_font(app->font[i]);
			}
			app->font[i] = NULL;
		}
	}
	if(app->atlas)
	{
		t3f_destroy_atlas(app->atlas);
		app->atlas = NULL;
	}
}

static void _dot_show_load_screen(T3F_FONT * fp, ALLEGRO_COLOR color, const char * message)
{
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;

	al_store_state(&old_state, ALLEGRO_STATE_TRANSFORM);
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_set_clipping_rectangle(0, 0, al_get_display_width(t3f_display), al_get_display_height(t3f_display));
	al_draw_filled_rectangle(0.0, 0.0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), color);
	if(fp)
	{
		al_draw_filled_rectangle(0.0, 0.0, al_get_display_width(t3f_display), al_get_display_height(t3f_display), al_map_rgba_f(0.0, 0.0, 0.0, 0.5));
	}
	al_restore_state(&old_state);
	if(fp && message)
	{
		dot_shadow_text(fp, t3f_color_white, al_map_rgba_f(0.0, 0.0, 0.0, 0.75), t3f_virtual_display_width / 2, t3f_virtual_display_height / 2 - t3f_get_font_line_height(fp) / 2, DOT_SHADOW_OX, DOT_SHADOW_OY, T3F_FONT_ALIGN_CENTER, "Loading resources...");
	}
	al_flip_display();
}

static bool load_graphics(APP_INSTANCE * app)
{
	char fn_buf[1024];
	int i;

	/* load fonts first so we can show loading message */
	sprintf(fn_buf, "data/fonts/kongtext_%dx.ini", app->graphics_size_multiplier);
	if(!dot_load_font(app, DOT_FONT_16, fn_buf, 16))
	{
		printf("Failed to load font %d!\n", DOT_FONT_16);
		goto fail;
	}
	_dot_show_load_screen(app->font[DOT_FONT_16], DOT_LEVEL_COLOR_0, "Loading resources...");
	if(!dot_load_font(app, DOT_FONT_14, fn_buf, 14))
	{
		printf("Failed to load font %d!\n", DOT_FONT_8);
		goto fail;
	}
	if(!dot_load_font(app, DOT_FONT_8, fn_buf, 8))
	{
		printf("Failed to load font %d!\n", DOT_FONT_8);
		goto fail;
	}
	if(!dot_load_font(app, DOT_FONT_32, fn_buf, 32))
	{
		printf("Failed to load font %d!\n", DOT_FONT_32);
		goto fail;
	}

	/* load graphics set */
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_RED, "ball_red.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_GREEN, "ball_green.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_BLUE, "ball_blue.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_PURPLE, "ball_purple.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_YELLOW, "ball_yellow.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_ORANGE, "ball_orange.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_BLACK, "ball_black.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BALL_EYES, "ball_eyes.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_COMBO, "combo.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_PARTICLE, "particle.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_BG, "emo_bg.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_FG, "emo_fg.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_NORMAL, "emo_normal.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_BLINK, "emo_blink.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_WOAH, "emo_woah.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_EMO_DEAD, "emo_dead.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_TARGET, "target.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_CURSOR, "cursor.png", app->graphics_size_multiplier, T3F_BITMAP_FLAG_PADDED))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_BG, "bg.png", app->graphics_size_multiplier, 0))
	{
		goto fail;
	}
	if(!dot_load_bitmap(app, DOT_BITMAP_LOGO, "logo.png", app->graphics_size_multiplier, 0))
	{
		goto fail;
	}
	app->bitmap[DOT_BITMAP_SCRATCH] = dot_create_scratch_bitmap(DOT_BITMAP_SCRATCH_WIDTH, DOT_BITMAP_SCRATCH_HEIGHT);
	if(!app->bitmap[DOT_BITMAP_SCRATCH])
	{
		printf("Failed to create effects scratch bitmap!\n");
		goto fail;
	}

	/* build atlas from graphics set */
	app->atlas = t3f_create_atlas(512 * app->graphics_size_multiplier, 512 * app->graphics_size_multiplier);
	if(!app->atlas)
	{
		goto fail;
	}
	for(i = 0; i < DOT_BITMAP_ATLAS_BOUNDARY; i++)
	{
		if(app->bitmap[i] && app->bitmap[i]->bitmap)
		{
			t3f_add_bitmap_to_atlas(app->atlas, &app->bitmap[i]->bitmap, T3F_ATLAS_SPRITE);
		}
	}

	return true;

	fail:
	{
		free_graphics(app);
		return false;
	}
}

bool app_load_data(APP_INSTANCE * app)
{
	const char * val;

	#ifdef ALLEGRO_ANDROID
		t3f_process_events(false);
	#endif

	/* use manually configured graphics size multiplier if setting found */
	val = al_get_config_value(t3f_config, "Game Data", "Graphics Size Multiplier");
	if(val)
	{
		app->graphics_size_multiplier = atoi(val);
	}

	/* otherwise, try to detect the optimal size */
	else
	{
		app->graphics_size_multiplier = al_get_display_width(t3f_display) / t3f_virtual_display_width;
		if(app->graphics_size_multiplier > al_get_display_height(t3f_display) / t3f_virtual_display_height)
		{
			app->graphics_size_multiplier = al_get_display_height(t3f_display) / t3f_virtual_display_height;
		}
	}

	/* keep graphics size multiplier within acceptable range */
	if(app->graphics_size_multiplier < 1)
	{
		app->graphics_size_multiplier = 1;
	}
	else if(app->graphics_size_multiplier > 5)
	{
		app->graphics_size_multiplier = 5;
	}

	/* create main view */
	app->main_view = t3f_create_view(0, 0, 0, 0, 0, 0, 0);
	if(!app->main_view)
	{
		return false;
	}

	/* create menu view */
	app->menu_view = t3f_create_view(0, 0, 0, 0, 0, 0, 0);
	if(!app->menu_view)
	{
		return false;
	}

	/* load graphics */
	for(; app->graphics_size_multiplier >= 1; app->graphics_size_multiplier--)
	{
		if(load_graphics(app))
		{
			break;
		}
	}
	if(app->graphics_size_multiplier < 1)
	{
		return false;
	}

	/* load sounds */
	app->sample[DOT_SAMPLE_START] = al_load_sample("data/sounds/sfx0005.wav");
	if(!app->sample[DOT_SAMPLE_START])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_START);
		return false;
	}
	app->sample[DOT_SAMPLE_GRAB] = al_load_sample("data/sounds/sfx0000.wav");
	if(!app->sample[DOT_SAMPLE_GRAB])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_GRAB);
		return false;
	}
	app->sample[DOT_SAMPLE_LOSE] = al_load_sample("data/sounds/sfx0001.wav");
	if(!app->sample[DOT_SAMPLE_LOSE])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_LOSE);
		return false;
	}
	app->sample[DOT_SAMPLE_GO] = al_load_sample("data/sounds/sfx0004.wav");
	if(!app->sample[DOT_SAMPLE_GO])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_GO);
		return false;
	}
	app->sample[DOT_SAMPLE_SCORE] = al_load_sample("data/sounds/score_up.wav");
	if(!app->sample[DOT_SAMPLE_SCORE])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_SCORE);
		return false;
	}
	app->sample[DOT_SAMPLE_EXTRA_LIFE] = al_load_sample("data/sounds/sfx0008.wav");
	if(!app->sample[DOT_SAMPLE_EXTRA_LIFE])
	{
		printf("Failed to load sound %d!\n", DOT_SAMPLE_EXTRA_LIFE);
		return false;
	}
	return true;
}

/* read user preferences and other data */
void app_read_user_data(APP_INSTANCE * app)
{
	const char * val;

	/* see if we need to run setup */
	app->first_run = true;
	val = al_get_config_value(t3f_user_data, "Game Data", "Setup Done");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			app->first_run = false;
		}
	}

	/* load high scores */
	val = al_get_config_value(t3f_user_data, "Game Data", "High Score");
	if(val)
	{
		app->game.high_score[0] = dot_leaderboard_unobfuscate_score(atoi(val));
	}
	else
	{
		app->game.high_score[0] = 0;
	}
	val = al_get_config_value(t3f_user_data, "Game Data", "High Score Easy");
	if(val)
	{
		app->game.high_score[1] = dot_leaderboard_unobfuscate_score(atoi(val));
	}
	else
	{
		app->game.high_score[1] = 0;
	}

	/* load user key */
	val = al_get_config_value(t3f_user_data, "Game Data", "User Key");
	if(val)
	{
		strcpy(app->user_key, val);
	}

	/* load user name */
	val = al_get_config_value(t3f_user_data, "Game Data", "User Name");
	if(val)
	{
		strcpy(app->user_name, val);
	}
	else
	{
		strcpy(app->user_name, "Anonymous");
	}

	/* load Steam user display name */
	val = t3f_get_steam_user_display_name();
	if(val)
	{
		al_set_config_value(t3f_user_data, "Game Data", "Steam User Display Name", val);
	}
	else
	{
		val = al_get_config_value(t3f_user_data, "Game Data", "Steam User Display Name");
	}
	if(val)
	{
		strcpy(app->user_name, val);
	}

	app->upload_scores = false;
	val = al_get_config_value(t3f_user_data, "Game Data", "Upload Scores");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			app->upload_scores = true;
		}
	}

	app->music_enabled = false;
	val = al_get_config_value(t3f_user_data, "Game Data", "Music Enabled");
	if(val)
	{
		if(!strcasecmp(val, "true"))
		{
			app->music_enabled = true;
		}
	}

	app->game_mode = 1;
	val = al_get_config_value(t3f_user_data, "Game Data", "Game Mode");
	if(val)
	{
		app->game_mode = atoi(val);
		if(app->game_mode != 0 && app->game_mode != 1)
		{
			app->game_mode = 1;
		}
	}

	/* user cheat settings */
	app->game.cheats_enabled = false;
	app->game.speed_multiplier = 1.0;
	val = al_get_config_value(t3f_user_data, "Game Data", "speed_multiplier");
	if(val)
	{
		app->game.speed_multiplier = atof(val);
		app->game.cheats_enabled = true;
	}
	app->game.start_level = 0;
	val = al_get_config_value(t3f_user_data, "Game Data", "start_level");
	if(val)
	{
		app->game.start_level = atoi(val);
		if(app->game.start_level != 0)
		{
			app->game.cheats_enabled = true;
		}
	}
	app->game.start_lives = 0;
	val = al_get_config_value(t3f_user_data, "Game Data", "start_lives");
	if(val)
	{
		app->game.start_lives = atoi(val);
		app->game.cheats_enabled = true;
	}

	/* set up leaderboard URLs */
	val = al_get_config_value(t3f_user_data, "Game Data", "leaderboard_get_user_key_url");
	if(!val)
	{
		val = "https://www.tcubedsoftware.com/scripts/leaderboards/get_user_key.php";
		al_set_config_value(t3f_user_data, "Game Data", "leaderboard_get_user_key_url", val);
	}
	strcpy(app->leaderboard_get_user_key_url, val);
	val = al_get_config_value(t3f_user_data, "Game Data", "leaderboard_set_user_name_url");
	if(!val)
	{
		val = "https://www.tcubedsoftware.com/scripts/leaderboards/set_user_name.php";
		al_set_config_value(t3f_user_data, "Game Data", "leaderboard_set_user_name_url", val);
	}
	strcpy(app->leaderboard_set_user_name_url, val);
	val = al_get_config_value(t3f_user_data, "Game Data", "leaderboard_submit_url");
	if(!val)
	{
		val = "https://www.tcubedsoftware.com/scripts/leaderboards/update.php";
		al_set_config_value(t3f_user_data, "Game Data", "leaderboard_submit_url", val);
	}
	strcpy(app->leaderboard_submit_url, val);
	val = al_get_config_value(t3f_user_data, "Game Data", "leaderboard_retrieve_url");
	if(!val)
	{
		val = "https://www.tcubedsoftware.com/scripts/leaderboards/query.php";
		al_set_config_value(t3f_user_data, "Game Data", "leaderboard_retrieve_url", val);
	}
	strcpy(app->leaderboard_retrieve_url, val);
}

/* read config for local machine */
void app_read_config(APP_INSTANCE * app)
{
	const char * val;

	app->controller.dead_zone = 0.15;
	val = al_get_config_value(t3f_config, "Game Data", "Controller Dead Zone");
	if(val)
	{
		app->controller.dead_zone = atof(val);
	}
	app->mouse_sensitivity = 0.0;
	val = al_get_config_value(t3f_config, "App Config", "Mouse Sensitivity");
	if(val)
	{
		app->mouse_sensitivity = atof(val);
	}
}

bool app_process_arguments(APP_INSTANCE * app, int argc, char * argv[])
{
	int i;

	/* see if we want to record a demo */
	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "--record_demo"))
		{
			if(argc <= i + 1)
			{
				printf("Missing argument.\n");
				return false;
			}
			else
			{
				if(!dot_load_bitmap(app, DOT_BITMAP_HAND, "hand.png", 1, T3F_BITMAP_FLAG_PADDED))
				{
					return false;
				}
				if(!dot_load_bitmap(app, DOT_BITMAP_HAND_DOWN, "hand_down.png", 1, T3F_BITMAP_FLAG_PADDED))
				{
					return false;
				}
				app->demo_file = al_fopen(argv[i + 1], "wb");
				if(!app->demo_file)
				{
					printf("Failed to open demo file.\n");
					return false;
				}
				dot_show_touch_hand = true;
				app->demo_recording = true;
				app->demo_seed = time(0);
				al_fwrite32le(app->demo_file, app->demo_seed);
			}
		}
		if(!strcmp(argv[i], "--play_demo"))
		{
			if(argc <= i + 1)
			{
				printf("Missing argument.\n");
				return false;
			}
			else
			{
				app->demo_filename = argv[i + 1];
				dot_show_touch_hand = true;
			}
			app_avc_init_proc(app);
		}
		if(!strcmp(argv[i], "--capture_demo"))
		{
			if(argc <= i + 1)
			{
				printf("Missing argument.\n");
				return false;
			}
			else
			{
				if(!dot_load_bitmap(app, DOT_BITMAP_HAND, "hand.png", 1, T3F_BITMAP_FLAG_PADDED))
				{
					return false;
				}
				if(!dot_load_bitmap(app, DOT_BITMAP_HAND_DOWN, "hand_down.png", 1, T3F_BITMAP_FLAG_PADDED))
				{
					return false;
				}
				app->demo_filename = argv[i + 1];
				dot_show_touch_hand = true;
			}
			if(!avc_start_capture(t3f_display, "myvideo.mp4", app_avc_init_proc, app_avc_logic_proc, app_render, 60, 0, app))
			{
				printf("Capture failed!\n");
				return false;
			}
		}
		if(!strcmp(argv[i], "--reset-steam-stats"))
		{
			app->reset_steam_stats = true;
		}
	}
	return true;
}

void app_check_mobile_argument(APP_INSTANCE * app, int argc, char * argv[])
{
	int i;

	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "--mobile"))
		{
			app->desktop_mode = false;
			break;
		}
	}
}

static void set_optimal_display_size(APP_INSTANCE * app)
{
	ALLEGRO_MONITOR_INFO info;
	int width, height;
	int current_width = DOT_DISPLAY_WIDTH;
	int current_height = DOT_DISPLAY_HEIGHT;

	al_get_monitor_info(0, &info);
	width = info.x2 - info.x1 - 64;
	height = info.y2 - info.y1 - 64;
	while(current_width <= width && current_height <= height)
	{
		current_width += DOT_DISPLAY_WIDTH / 4;
		current_height += DOT_DISPLAY_HEIGHT / 4;
	}
	current_width -= DOT_DISPLAY_WIDTH / 4;
	current_height -= DOT_DISPLAY_HEIGHT / 4;
	if(current_width != al_get_display_width(t3f_display) || current_height != al_get_display_height(t3f_display))
	{
		t3f_set_gfx_mode(current_width, current_height, t3f_flags);
		al_set_window_position(t3f_display, (info.x2 - info.x1) / 2 - current_width / 2, (info.y2 - info.y1) / 2 - current_height / 2);
	}
}

static void add_particle_list_item(DOT_PARTICLE_LIST * lp, float x, float y)
{
	if(lp->items < DOT_MAX_PARTICLE_LIST_ITEMS)
	{
		lp->item[lp->items].x = x;
		lp->item[lp->items].y = y;
		lp->items++;
	}
}

static bool create_particle_lists(APP_INSTANCE * app)
{
	ALLEGRO_STATE old_state;
	ALLEGRO_TRANSFORM identity;
	ALLEGRO_COLOR c;
	int i, j, k, w, h;
	unsigned char r, g, b, a;
	char buf[16] = {0};
	T3F_FONT * font = NULL;;
	ALLEGRO_BITMAP * scratch = NULL;

	font = t3f_load_font("data/fonts/kongtext_1x.ini", T3F_FONT_TYPE_AUTO, 16, 0, false);
	if(!font)
	{
		goto fail;
	}
	scratch = al_create_bitmap(DOT_BITMAP_SCRATCH_WIDTH, DOT_BITMAP_SCRATCH_HEIGHT);
	if(!scratch)
	{
		goto fail;
	}

	/* create lists for score numbers */
	for(i = 0; i < 10; i++)
	{
		app->number_particle_list[i].items = 0;
		sprintf(buf, "%d", i);
		al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM);
		al_set_target_bitmap(scratch);
		al_identity_transform(&identity);
		al_use_transform(&identity);
		al_set_clipping_rectangle(0, 0, 512, 512);
		al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
		t3f_draw_text(font, t3f_color_white, 0, 0, 0, 0, buf);
		t3f_set_clipping_rectangle(0, 0, 0, 0);
		al_restore_state(&old_state);
		al_lock_bitmap(scratch, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
		w = t3f_get_text_width(font, buf);
		h = t3f_get_font_line_height(font);
		for(j = 0; j < w; j++)
		{
			for(k = 0; k < h; k++)
			{
				c = al_get_pixel(scratch, j, k);
				al_unmap_rgba(c, &r, &g, &b, &a);
				if(a > 192)
				{
					add_particle_list_item(&app->number_particle_list[i], j, k);
				}
			}
		}
		al_unlock_bitmap(scratch);
	}

	/* create list for 1-up */
	app->extra_life_particle_list.items = 0;
	strcpy(buf, DOT_EXTRA_LIFE_TEXT);
	al_store_state(&old_state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_TRANSFORM);
	al_set_target_bitmap(scratch);
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_set_clipping_rectangle(0, 0, 512, 512);
	al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
	t3f_draw_text(font, t3f_color_white, 0, 0, 0, 0, buf);
	t3f_set_clipping_rectangle(0, 0, 0, 0);
	al_restore_state(&old_state);
	al_lock_bitmap(scratch, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
	w = t3f_get_text_width(font, buf);
	h = t3f_get_font_line_height(font);
	for(j = 0; j < w; j++)
	{
		for(k = 0; k < h; k++)
		{
			c = al_get_pixel(scratch, j, k);
			al_unmap_rgba(c, &r, &g, &b, &a);
			if(a > 192)
			{
				add_particle_list_item(&app->extra_life_particle_list, j, k);
			}
		}
	}
	al_unlock_bitmap(scratch);

	/* free resources */
	if(!t3f_destroy_resource(font))
	{
		t3f_destroy_font(font);
	}
	return true;

	fail:
	{
		if(scratch)
		{
			al_destroy_bitmap(scratch);
		}
		if(font)
		{
			if(!t3f_destroy_resource(font))
			{
				t3f_destroy_font(font);
			}
		}
		return false;
	}
}

bool dot_initialize_achievements(APP_INSTANCE * app)
{
	app->achievements = t3f_create_achievements_list(8);
	if(!app->achievements)
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_GETTING_INTO_IT, "GETTING_INTO_IT", "Full Combo", "Complete a level without breaking your combo.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_FULL_COMBO, "FULL_COMBO", "Getting Into It", "Complete a level by clearing the board of all colored dots.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_BOB_AND_WEAVE, "BOB_AND_WEAVE", "Bob and Weave", "Survive for one minute without touching any dots.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_GETTING_GOOD, "GETTING_GOOD", "Getting Good", "Complete a level without losing a life.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_OOPS, "OOPS", "Oops!", "Die within one second of starting a life.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_SEE_IT_THROUGH, "SEE_IT_THROUGH", "See It Through", "Complete the first loop by beating level 10.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_SO_CLOSE, "SO_CLOSE", "So Close!", "Die with only one colored dot remaining.", 1, false))
	{
		goto fail;
	}
	if(!t3f_set_achievement_details(app->achievements, DOT_ACHIEVEMENT_GOOD_GAME, "GOOD_GAME", "Good Game", "Score 100,000 points.", 1, false))
	{
		goto fail;
	}
	t3f_load_achievements_data(app->achievements, t3f_user_data, "Achievements");
	app->achievements->updated = true;
	return true;

	fail:
	{
		if(app->achievements)
		{
			t3f_destroy_achievements_list(app->achievements);
			app->achievements = NULL;
		}
		return false;
	}
}

void dot_deinitialize_achievements(APP_INSTANCE * app)
{
	if(app->achievements)
	{
		t3f_save_achievements_data(app->achievements, t3f_user_data, "Achievements");
		t3f_destroy_achievements_list(app->achievements);
	}
}

static bool first_run(void)
{
	const char * val;

	val = al_get_config_value(t3f_user_data, "Game Data", "Setup Done");
	if(!val || !strcmp(val, "false"))
	{
		return true;
	}
	return false;
}

static bool attempt_restart(void)
{
	const char * val;

	val = al_get_config_value(t3f_user_data, "Game Data", "Restarted with Steam");
	if(!val || !strcmp(val, "false"))
	{
		al_set_config_value(t3f_user_data, "Game Data", "Restarted with Steam", "true");
		t3f_save_user_data();
		return t3f_restart_through_steam(T3F_APP_STEAM_ID);
	}
	return false;
}

static void log_t3net(void)
{
	const char * val;
	char buf[1024];

	val = al_get_config_value(t3f_config, "Debug", "t3net_logging");
	if(val && !strcmp(val, "true"))
	{
		t3net_open_log_file(t3f_get_filename(t3f_config_path, "t3net.log", buf, 1024));
	}
}

/* initialize our app, load graphics, etc. */
bool app_initialize(APP_INSTANCE * app, int argc, char * argv[])
{
	int i;

	/* detect game type */
	app->desktop_mode = false;
	#ifndef ALLEGRO_ANDROID
		app->desktop_mode = true;
	#endif
	app_check_mobile_argument(app, argc, argv);

	/* initialize T3F */
	if(!t3f_initialize(T3F_APP_TITLE, DOT_DISPLAY_WIDTH, DOT_DISPLAY_HEIGHT, 60.0, app_logic, app_render, T3F_DEFAULT | T3F_USE_FIXED_PIPELINE | T3F_USE_FULLSCREEN | T3F_ANY_ORIENTATION, app))
	{
		printf("Error initializing T3F\n");
		return false;
	}
	_dot_show_load_screen(NULL, DOT_LEVEL_COLOR_0, NULL);
	if(al_get_display_width(t3f_display) > al_get_display_height(t3f_display))
	{
		app->desktop_mode = true;
	}
	app_check_mobile_argument(app, argc, argv);
	#ifdef T3F_ENABLE_STEAM_INTEGRATION
		if(first_run())
		{
			if(attempt_restart())
			{
				printf("Attempting to restart game through Steam for first run setup.\n");
				return false;
			}
		}
	#endif
	if(!dot_initialize_achievements(app))
	{
		printf("Error initializing achievements list!\n");
		return false;
	}
//	log_t3net();
	if(!t3f_option_is_set(T3F_OPTION_RENDER_MODE))
	{
		t3f_set_option(T3F_OPTION_RENDER_MODE, T3F_RENDER_MODE_ALWAYS_CLEAR);
	}
	app->steam_running = t3f_init_steam_integration(app->achievements);
	if(!app->steam_running)
	{
		printf("Steam not running!\n");
	}
	else
	{
		app->on_steam_deck = t3f_steam_deck_mode();
	}
	if(app->on_steam_deck)
	{
		dot_enable_soft_cursor(true);
	}
	if(!dot_initialize_input(&app->controller))
	{
		printf("Error initializing input system!\n");
		return false;
	}
	t3f_set_event_handler(dot_event_handler);
	if(!(t3f_flags & T3F_USE_FULLSCREEN))
	{
		set_optimal_display_size(app);
	}
	#ifdef ALLEGRO_ANDROID
		t3net_setup(t3f_run_url, al_path_cstr(t3f_temp_path, '/'));
	#else
		t3net_setup(NULL, al_path_cstr(t3f_temp_path, '/'));
	#endif
	if(!create_particle_lists(app))
	{
		printf("Failed to generate data for particle effects!\n");
		return false;
	}

	if(!app_load_data(app))
	{
		printf("Failed to load data!\n");
		return false;
	}
	_dot_update_views(app, al_get_display_width(t3f_display), al_get_display_height(t3f_display));

	app_read_config(app);
	app_read_user_data(app);

	if(!dot_intro_initialize(app))
	{
		printf("Failed to create menu!\n");
		return false;
	}

	/* create color tables */
	for(i = 0; i <= DOT_BITMAP_BALL_BLACK; i++)
	{
		app->dot_color[i] = dot_get_ball_color(app->bitmap[i]->bitmap);
	}
	app->level_color[0] = DOT_LEVEL_COLOR_0;
	app->level_color[1] = DOT_LEVEL_COLOR_1;
	app->level_color[2] = DOT_LEVEL_COLOR_2;
	app->level_color[3] = DOT_LEVEL_COLOR_3;
	app->level_color[4] = DOT_LEVEL_COLOR_4;
	app->level_color[5] = DOT_LEVEL_COLOR_5;
	app->level_color[6] = DOT_LEVEL_COLOR_6;
	app->level_color[7] = DOT_LEVEL_COLOR_7;
	app->level_color[8] = DOT_LEVEL_COLOR_8;
	app->level_color[9] = DOT_LEVEL_COLOR_9;
	app->controller.current_joy = -1;
	app->controller.current_joy_handle = NULL;

	dot_setup_bg_objects(app);
	dot_setup_credits(&app->credits);

	dot_intro_setup(app);
	if(app->music_enabled)
	{
		t3f_play_music(DOT_MUSIC_TITLE);
	}
	if(!app->desktop_mode)
	{
		app->menu_showing = true;
	}
	if(app->first_run)
	{
		app->current_menu = DOT_MENU_UPLOAD_SCORES;
	}
	app->state = DOT_STATE_INTRO;

	if(!app_process_arguments(app, argc, argv))
	{
		printf("Failed to process command line arguments!\n");
		return false;
	}
	return true;
}

void app_exit(APP_INSTANCE * app)
{
	int i;

	if(app->demo_file)
	{
		al_fputc(app->demo_file, 0);
		al_fclose(app->demo_file);
	}
	t3f_destroy_view(app->main_view);
	free_graphics(app);
	for(i = 0; i < DOT_MAX_SAMPLES; i++)
	{
		if(app->sample[i])
		{
			al_destroy_sample(app->sample[i]);
		}
	}
	for(i = 0; i < DOT_MAX_MENUS; i++)
	{
		if(app->menu[i])
		{
			t3f_destroy_gui(app->menu[i]);
		}
	}
	if(app->leaderboard)
	{
		t3net_destroy_leaderboard(app->leaderboard);
	}
	dot_deinitialize_achievements(app);
	dot_destroy_input(&app->controller);
	t3f_shutdown_steam_integration();
}

int main(int argc, char * argv[])
{
	APP_INSTANCE * app;

	app = malloc(sizeof(APP_INSTANCE));
	if(!app)
	{
		printf("Could not allocation memory for applicartion instance.\n");
		return 1;
	}
	memset(app, 0, sizeof(APP_INSTANCE));
	if(app_initialize(app, argc, argv))
	{
		t3f_run();
	}
	app_exit(app);
	free(app);
	t3f_finish();
	return 0;
}
