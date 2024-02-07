//Nathan Walter
//CPSC 440
//Program 5: Diving Game

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "SpriteSheet.h"
#include "mappy_A5.h"
#include <iostream>
using namespace std;

int collided(int x, int y);  //Tile Collision
bool endValue( int x, int y );	//End Block with the User Value = 8
void getGameInfo(const int WIDTH, const int HEIGHT, int oxygen, const int FPS, int level, int depth, int ms);
void showOxygenGauge(Sprite& player, int oxygen, const int O2_FULL, const int FPS, const int WIDTH, const int HEIGHT);
/*This is main. It is responsible for creating an instance of the sprite and for loading in the underwater map from mappy. It creates the game loop and 
* determines the key bindings. It also controls the x and y offsets such that the screen will scrolll horizontally or vertically depending on the player's 
* position. It calls on the SpriteSheet.h functions to draw the sprite to the screen and update it's position each frame.*/
int main(void)
{
	const int WIDTH = 900;
	const int HEIGHT = 480;
	bool keys[] = {false, false, false, false, false};
	enum KEYS{UP, DOWN, LEFT, RIGHT};

	//variables
	bool done = false;
	bool render = false;
	bool introScreen = true;
	const int FPS = 60;
	char name[80];
	const int O2_FULL = 4200;	//full tank of oxygen
	int ms = 0;		//time in milliseconds

	//object variables
	Sprite player;

	//player's hud elements
	int depth = 220;	//depth in feet
	int oxygen = 280 * FPS;	//set the starting oxygen level
	int level = 1;	//each new level, lose air faster and draw larger bubbles

	//allegro variable
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer;

	//audio
	ALLEGRO_SAMPLE* ambiance = NULL;	//background underwater ambiance
	ALLEGRO_SAMPLE* breathing = NULL;	//breathing
	ALLEGRO_SAMPLE* lowOxygen = NULL;	//low oxygen warning

	//program init
	if(!al_init())										//initialize Allegro
		return -1;

	display = al_create_display(WIDTH, HEIGHT);			//create our display object

	if(!display)										//test display object
		return -1;

	if (!al_install_audio())
		return -1;

	if (!al_init_acodec_addon())
		return -1;

	//addon init
	al_install_keyboard();
	al_init_image_addon();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();

	if (!al_reserve_samples(4))
		return -1;

	ambiance = al_load_sample("ambiance.wav");
	breathing = al_load_sample("breathing.wav");
	lowOxygen = al_load_sample("low_oxygen.wav");

	if (!ambiance || !breathing || !lowOxygen)
		exit(9);

	player.InitSprites(WIDTH,HEIGHT);
	int xOff = 0;
	int yOff = 0;

	if(MapLoad("map.fmp", 1))	//load into the map
		return -5;

	//load the fonts
	ALLEGRO_FONT* font20 = al_load_ttf_font("Mysterio.ttf", 20, 0);
	ALLEGRO_FONT* font24 = al_load_ttf_font("Mysterio.ttf", 24, 0);
	ALLEGRO_FONT* font30 = al_load_ttf_font("Mysterio.ttf", 30, 0);

	event_queue = al_create_event_queue();
	timer = al_create_timer(1.0 / FPS);

	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	//load the background and hud images
	ALLEGRO_BITMAP* ocean = NULL;
	ocean = al_load_bitmap("ocean.png");
	ALLEGRO_BITMAP* coral = NULL;
	coral = al_load_bitmap("coral.png");

	//display a brief intro screen before starting the game, lasts 15 seconds
	while (introScreen)
	{
		al_clear_to_color(al_map_rgb(0, 204, 255));
		al_draw_text(font30, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) - 200, ALLEGRO_ALIGN_CENTER, "< Welcome to the diving game! >");
		al_draw_text(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) - 130, ALLEGRO_ALIGN_CENTER, "You have 4200 psi of oxygen to reach the surface before you drown.");
		al_draw_text(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) - 95, ALLEGRO_ALIGN_CENTER, "Use the arrow keys to navigave the underwater caverns.");
		al_draw_text(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) - 60, ALLEGRO_ALIGN_CENTER, "There are 3 sections. Each new section increases the rate of oxygen loss.");
		al_draw_text(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) - 25, ALLEGRO_ALIGN_CENTER, "Good luck!");
		
		al_draw_text(font20, al_map_rgb(0, 0, 0), 8, HEIGHT - 25, ALLEGRO_ALIGN_LEFT, "The game will begin shortly...");
		al_flip_display();
		al_rest(15);	//show this page for 15 seconds, then start the game
		introScreen = false;
	}
	al_start_timer(timer);
	al_flip_display();
	
	//play background audio
	al_play_sample(ambiance, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
	al_play_sample(breathing, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);

	while (!done)	//game loop
	{
		if (oxygen <= 0 || depth <= 0)	//if the player drowns or reaches the surface
		{
			//stop sound effects
			player.~Sprite();
			al_destroy_sample(ambiance);
			al_destroy_sample(breathing);
			al_destroy_sample(lowOxygen);
			
			getGameInfo(WIDTH, HEIGHT, oxygen, FPS, level, depth, ms);

			al_flip_display();
			done = true;
		}
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);
		if (ev.type == ALLEGRO_EVENT_TIMER)
		{
			render = true;
			MapUpdateAnims();	//update animated tiles
			if (keys[UP])
				player.UpdateSprites(WIDTH, HEIGHT, 2);

			else if (keys[DOWN])
				player.UpdateSprites(WIDTH, HEIGHT, 3);

			else if (keys[LEFT])
				player.UpdateSprites(WIDTH, HEIGHT, 0);

			else if (keys[RIGHT])
				player.UpdateSprites(WIDTH, HEIGHT, 1);

			else
				player.UpdateSprites(WIDTH, HEIGHT, 4);

			render = true;
		}
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			done = true;
		}
		else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch (ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				done = true;
				break;
			case ALLEGRO_KEY_UP:
				keys[UP] = true;
				break;
			case ALLEGRO_KEY_DOWN:
				keys[DOWN] = true;
				break;
			case ALLEGRO_KEY_LEFT:
				keys[LEFT] = true;
				break;
			case ALLEGRO_KEY_RIGHT:
				keys[RIGHT] = true;
				break;
			}
		}
		else if (ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch (ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				done = true;
				break;
			case ALLEGRO_KEY_UP:
				keys[UP] = false;
				break;
			case ALLEGRO_KEY_DOWN:
				keys[DOWN] = false;
				break;
			case ALLEGRO_KEY_LEFT:
				keys[LEFT] = false;
				break;
			case ALLEGRO_KEY_RIGHT:
				keys[RIGHT] = false;
				break;
			}
		}
		if (render && al_is_event_queue_empty(event_queue))
		{
			render = false;

			//update the map scroll position
			xOff = player.getX() + player.getWidth() - WIDTH / 2 - 96;
			yOff = player.getY() + player.getHeight() - HEIGHT / 2 - 43;

			//avoid moving beyond the map edge
			if (xOff < 0) xOff = 0;
			if (xOff > (mapwidth * mapblockwidth - WIDTH))
				xOff = mapwidth * mapblockwidth - WIDTH;
			if (yOff < 0)
				yOff = 0;
			if (yOff > (mapheight * mapblockheight - HEIGHT) - 40)
				yOff = mapheight * mapblockheight - HEIGHT - 40;
			
			//draw the background bitmaps
			al_draw_bitmap(ocean, 0 - xOff, 0 - yOff - 600, 0);		//the background will move in relation to the diver, making it apper as though they are really swimming
			al_draw_bitmap(coral, 0 - xOff, 4200 - yOff, 0);

			//draw the background tiles
			MapDrawBG(xOff, yOff, 0, 0, WIDTH, HEIGHT);

			//draw foreground tiles
			MapDrawFG(xOff, yOff, 0, 0, WIDTH, HEIGHT, 0);

			player.DrawSprites(xOff, yOff, level);

			//draw the player's hud elements
			al_draw_textf(font24, al_map_rgb(255, 255, 255), WIDTH / 2, 8, ALLEGRO_ALIGN_CENTER, "< %d feet >", depth);	//depth in feet

			if ((oxygen / FPS) * 15 < (O2_FULL * 0.2))	//if player has 2% of oxygen left, print in red
			{
				al_play_sample(lowOxygen, 1.0, 0.0, 0.5, ALLEGRO_PLAYMODE_LOOP, NULL);	//low oxygen warning
				al_draw_textf(font24, al_map_rgb(255, 0, 0), WIDTH - 8, 8, ALLEGRO_ALIGN_RIGHT, "Oxygen: %d psi", (oxygen / FPS) * 15);
			}
			else
			{
				al_draw_textf(font24, al_map_rgb(255, 255, 255), WIDTH - 8, 8, ALLEGRO_ALIGN_RIGHT, "Oxygen: %d psi", (oxygen / FPS) * 15);	//oxygen level in psi
			}
			showOxygenGauge(player, oxygen, O2_FULL, FPS, WIDTH, HEIGHT);	//oxygen meter

			al_draw_textf(font24, al_map_rgb(255, 255, 255), 8, 8, ALLEGRO_ALIGN_LEFT, "Section %d", level);	//level

			getGameInfo(WIDTH, HEIGHT, oxygen, FPS, level, depth, ms);	//print game information (drowned, or reached the surface)

			al_flip_display();
			al_clear_to_color(al_map_rgb(0, 0, 0));
		}
		//update level based on player's depth
		if (player.getY() >= 3150)
		{
			level = 1;
			oxygen--;
		}
		else if (player.getY() >= 1500 && player.getY() < 3150)
		{
			level = 2;
			oxygen -= 2;	//lose air 2x as fast
		}
		else if (player.getY() < 1500)
		{
			level = 3;
			oxygen -= 3;	//lose air 3x as fast
		}
		depth = ((player.getY() / 10) - 10) / 2;	//update player's depth
		ms++;	//update milliseconds elapsed
	}
	al_rest(12.0);	//rest for 12 seconds
	//deallocate memory
	MapFreeMem();
	al_destroy_event_queue(event_queue);
	al_destroy_font(font20);
	al_destroy_font(font24);
	al_destroy_font(font30);
	al_destroy_display(display);	//destroy our display object

	return 0;
}
/*Given x,y values (corresponding to each corner of the block), checks if the player has collided with a side of the block.
@param int, x
@param int, y
@return int, 0 if no collision*/
int collided(int x, int y)
{
	BLKSTR *blockdata;
	blockdata = MapGetBlock(x/mapblockwidth, y/mapblockheight);
	return blockdata->tl;
}
/*Checks if a block is an end block by checking its user data value. If the block has a matching user data value as whatever is in the if statement, return true.
@param int, x
@param int, y
@return bool, true if block is an end block, false otherwise*/
bool endValue( int x, int y )
{
	BLKSTR* data;
	data = MapGetBlock( x/mapblockwidth, y/mapblockheight );

	if( data->user1 == 8 )
	{
		return true;
	}else
		return false;
}
/*Prints the appropriate message to the screen, if the player drowns or reaches the surface
@param const int, WIDTH
@param const int, HEIGHT
@param int, oxygen
@param const int, FPS
@param int, level
@param int, depth
@param int, ms
@return void*/
void getGameInfo(const int WIDTH, const int HEIGHT, int oxygen, const int FPS, int level, int depth, int ms)
{
	//load the fonts
	ALLEGRO_FONT* font30 = al_load_ttf_font("Mysterio.ttf", 30, 0);
	ALLEGRO_FONT* font24 = al_load_ttf_font("Mysterio.ttf", 24, 0);
	ALLEGRO_FONT* font20 = al_load_ttf_font("Mysterio.ttf", 20, 0);

	int sec = ms / FPS;

	if (oxygen <= 0)	//if the player drowns
	{
		al_clear_to_color(al_map_rgb(255, 77, 77));
		al_draw_textf(font30, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) - 35, ALLEGRO_ALIGN_CENTER, "< You drowned at %d feet! >", depth);

		if (level == 1)
		{
			al_draw_text(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) + 35, ALLEGRO_ALIGN_CENTER, "Didn't get past section 1");
			al_draw_textf(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) + 70, ALLEGRO_ALIGN_CENTER, "Survived %d seconds", sec);
		}
		else
		{
			al_draw_textf(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) + 35, ALLEGRO_ALIGN_CENTER, "Reached section %d", level);
			al_draw_textf(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) + 70, ALLEGRO_ALIGN_CENTER, "Survived %d seconds", sec);
		}
		al_draw_text(font20, al_map_rgb(0, 0, 0), 8, HEIGHT - 25, ALLEGRO_ALIGN_LEFT, "Closing shortly...");
	}
	if (depth <= 0)		//if the player reaches the surface
	{
		al_clear_to_color(al_map_rgb(0, 204, 255));
		al_draw_text(font30, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) - 35, ALLEGRO_ALIGN_CENTER, "< You survived! >");
		al_draw_textf(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) + 35, ALLEGRO_ALIGN_CENTER, "%d psi left in your tank", (oxygen / FPS) * 15);
		al_draw_textf(font24, al_map_rgb(0, 0, 0), WIDTH / 2, (HEIGHT / 2) + 70, ALLEGRO_ALIGN_CENTER, "Reached the surface in %d seconds", sec);
		al_draw_text(font20, al_map_rgb(0, 0, 0), 8, HEIGHT - 25, ALLEGRO_ALIGN_LEFT, "Closing shortly...");
	}
	al_destroy_font(font30);
	al_destroy_font(font24);
	al_destroy_font(font20);
}
/*Responsible for displaying the oxygen meter based on player's amount of oxygen left in the tank.
@param Sprite&, player
@param int, oxygen
@param const int, FPS
@param const int, WIDTH
@param const int, HEIGHT
@return void*/
void showOxygenGauge(Sprite& player, int oxygen, const int O2_FULL, const int FPS, const int WIDTH, const int HEIGHT)
{
	int oxygenLeft = (oxygen / FPS) * 15;

	//display diffent levels of oxygen on the oxygen meter based on player's oxygen amount
	if (oxygenLeft >= (O2_FULL * 0.95))		//above 95%
	{
		al_draw_filled_rectangle(WIDTH - 168, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.95) && oxygenLeft >= (O2_FULL * 0.9))	//90 - 95%
	{
		al_draw_filled_rectangle(WIDTH - 160, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.9) && oxygenLeft >= (O2_FULL * 0.85))	//85 - 90%
	{
		al_draw_filled_rectangle(WIDTH - 152, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.85) && oxygenLeft >= (O2_FULL * 0.8))	//80 - 85%
	{
		al_draw_filled_rectangle(WIDTH - 144, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.8) && oxygenLeft >= (O2_FULL * 0.75))	//75 - 80%
	{
		al_draw_filled_rectangle(WIDTH - 136, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.75) && oxygenLeft >= (O2_FULL * 0.7))	//70 - 75%
	{
		al_draw_filled_rectangle(WIDTH - 128, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.7) && oxygenLeft >= (O2_FULL * 0.65))	//65 - 70%
	{
		al_draw_filled_rectangle(WIDTH - 120, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.65) && oxygenLeft >= (O2_FULL * 0.6))	//60 - 65%
	{
		al_draw_filled_rectangle(WIDTH - 112, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.6) && oxygenLeft >= (O2_FULL * 0.55))	//55 - 60%
	{
		al_draw_filled_rectangle(WIDTH - 104, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.55) && oxygenLeft >= (O2_FULL * 0.5))	//50 - 55%
	{
		al_draw_filled_rectangle(WIDTH - 96, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.5) && oxygenLeft >= (O2_FULL * 0.45))	//45 - 50%
	{
		al_draw_filled_rectangle(WIDTH - 88, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.45) && oxygenLeft >= (O2_FULL * 0.4))	//40 - 45%
	{
		al_draw_filled_rectangle(WIDTH - 80, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.4) && oxygenLeft >= (O2_FULL * 0.35))	//35 - 40%
	{
		al_draw_filled_rectangle(WIDTH - 72, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.35) && oxygenLeft >= (O2_FULL * 0.3))	//30 - 35%
	{
		al_draw_filled_rectangle(WIDTH - 64, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.3) && oxygenLeft >= (O2_FULL * 0.25))	//25 - 30%
	{
		al_draw_filled_rectangle(WIDTH - 56, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.25) && oxygenLeft >= (O2_FULL * 0.2))	//20 - 25%
	{
		al_draw_filled_rectangle(WIDTH - 48, 40, WIDTH - 8, 60, al_map_rgb(0, 204, 255));
	}
	else if (oxygenLeft < (O2_FULL * 0.2) && oxygenLeft >= (O2_FULL * 0.15))	//15 - 20%
	{
		al_draw_filled_rectangle(WIDTH - 40, 40, WIDTH - 8, 60, al_map_rgb(255, 0, 0));
	}
	else if (oxygenLeft < (O2_FULL * 0.15) && oxygenLeft >= (O2_FULL * 0.1))	//10 - 15%
	{
		al_draw_filled_rectangle(WIDTH - 32, 40, WIDTH - 8, 60, al_map_rgb(255, 0, 0));
	}
	else if (oxygenLeft < (O2_FULL * 0.1) && oxygenLeft >= (O2_FULL * 0.05))	//5 - 10%
	{
		al_draw_filled_rectangle(WIDTH - 24, 40, WIDTH - 8, 60, al_map_rgb(255, 0, 0));
	}
	else if (oxygenLeft < (O2_FULL * 0.05))		//below 5%
	{
		al_draw_filled_rectangle(WIDTH - 16, 40, WIDTH - 8, 60, al_map_rgb(255, 0, 0));
	}
	al_draw_rectangle(WIDTH - 168, 40, WIDTH - 8, 60, al_map_rgb(0, 0, 0), 3);	//black border
}
