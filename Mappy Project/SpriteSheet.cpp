//Nathan Walter
//CPSC 440
//Program 5: Diving Game

#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "SpriteSheet.h"

Sprite::Sprite()
{
	//bitmaps
	image = NULL;
	bubbles = NULL;

	//audio
	collision = NULL;
}
Sprite::~Sprite()
{
	al_destroy_bitmap(image);
	al_destroy_bitmap(bubbles);
	al_destroy_sample(collision);
}
void Sprite::InitSprites(int width, int height)
{
	x = 700;
	y = 4500;

	//initialize player sprite
	maxFrame = 10;
	curFrame = 0;
	frameCount = 0;
	frameDelay = 6;
	frameWidth = 192;
	frameHeight = 86.7;
	animationColumns = 4;
	animationRows = 3;
	animationDirection = 1;
	lastKey = 1;

	//initialize bubbles sprite
	maxFrameBubbles = 8;
	curFrameBubbles = 1;
	frameCountBubbles = 0;
	frameDelayBubbles = 15;
	frameWidthBubbles = 393.75;
	frameHeightBubbles = 511.5;
	animationColumnsBubbles = 4;
	animationRowsBubbles = 2;

	//bitmaps
	image = al_load_bitmap("diver.png");
	bubbles = al_load_bitmap("bubbles.png");

	//audio
	collision = al_load_sample("collision.wav");
	
	if (!collision)
		exit(9);
}
void Sprite::UpdateSprites(int width, int height, int dir)
{
	int oldx = x;
	int oldy = y;
	int halfWidth = frameWidth / 2;
	int halfHeight = frameHeight / 2;

	if(dir == 1) //right key
	{
		animationDirection = 1;
		x += 3; 
		if (++frameCount > frameDelay)
		{
			frameDelay = 6;
			frameCount = 0;
			if (++curFrame >= maxFrame)
				curFrame =1 ;
		}
		lastKey = 1;
	}
	else if (dir == 0) //left key
	{
		animationDirection = 0;
		x -= 3; 
		if (++frameCount > frameDelay)
		{
			frameDelay = 6;
			frameCount = 0;
			if (++curFrame >= maxFrame)
				curFrame = 1;
		}
		lastKey = 0;
	}
	else if (dir == 2) //up key
	{
		animationDirection = 2;
		y -= 2;
		if (lastKey == 0)
		{
			x -= 1;
		}
		else {
			x += 1;
		}
		if (++frameCount > frameDelay)
		{
			frameDelay = 6;
			frameCount = 0;
			if (++curFrame >= maxFrame)
				curFrame = 1;
		}
	}
	else if (dir == 3) //down key
	{
		animationDirection = 3;
		y += 2;
		if (lastKey == 0)
		{
			x -= 1;
		}
		else {
			x += 1;
		}
		if (++frameCount > frameDelay)
		{
			frameDelay = 6;
			frameCount = 0;
			if (++curFrame >= maxFrame)
				curFrame = 1;
		}
	}
	else if (dir == 4) //hovering
	{
		animationDirection = lastKey;
		if (lastKey == 0)	//when idle, let the diver automatically drift downwards
		{
			x -= 0.1;
			y += 0.3;
		}
		else {
			x += 0.1;
			y += 0.3;
		}
		if (++frameCount > frameDelay)
		{
			frameDelay = 15;
			frameCount = 0;
			if (++curFrame >= maxFrame)
				curFrame = 1;
		}
	}
	//update buubles
	if (++frameCountBubbles > frameDelayBubbles)
	{
		frameCountBubbles = 0;
		if (++curFrameBubbles >= maxFrameBubbles)
			curFrameBubbles = 1;
	}
	//check for collided with foreground tiles
	if (animationDirection == 0)
	{ 
		if (collided(x + 1, y + frameHeight)) { //collision detection to the left
			x = oldx; 
			y = oldy;
			al_play_sample(collision, 0.75, -1.0, 0.5, ALLEGRO_PLAYMODE_ONCE, NULL);
		}
	}
	else if (animationDirection == 1)
	{ 
		if (collided(x + frameWidth - 1, y + frameHeight)) { //collision detection to the right
			x = oldx; 
			y = oldy;
			al_play_sample(collision, 0.75, 1.0, 0.5, ALLEGRO_PLAYMODE_ONCE, NULL);
		}
	}
	else if (animationDirection == 2)
	{ 
		if (collided(x + halfWidth, y)) { //collision detection above
			x = oldx; 
			y = oldy;
			al_play_sample(collision, 0.75, 0.0, 0.5, ALLEGRO_PLAYMODE_ONCE, NULL);
		}
	}
	else if (animationDirection == 3)
	{
		if (collided(x + halfWidth, y + frameHeight)) { //collision detection below
			x = oldx;
			y = oldy;
			al_play_sample(collision, 0.75, 0.0, 0.5, ALLEGRO_PLAYMODE_ONCE, NULL);
		}
	}
	//collision detection for screen boundaries
	if (x <= 0)
	{
		x = 0;
	}
	if (x >= 1728)
	{
		x = 1728;
	}
	if (y <= 100)
	{
		y = 100;
	}
	if (y >= 4550)
	{
		y = 4550;
	}
}
bool Sprite::CollisionEndBlock()
{
	int halfWidth = frameWidth / 2;
	int halfHeight = frameHeight / 2;

	if (endValue(x + halfWidth, y + halfHeight)) //collide when player reaches end of block (left -> right)
		return true;
	else
		return false;
}
void Sprite::DrawSprites(int xoffset, int yoffset, int level)
{
	//sprite grabbing: x,y positions of each sprite frame
	int fx = (curFrame % animationColumns) * frameWidth;
	int fy = (curFrame / animationColumns) * frameHeight;
	float xscale = 0.005;
	float yscale = 0.012;
	float angleR = -0.6;
	float angleL = -5.683;

	//sprite grabbing for the bubbles
	int fxBubbles = (curFrameBubbles % animationColumnsBubbles) * frameWidthBubbles;
	int fyBubbles = (curFrameBubbles / animationColumnsBubbles) * frameHeightBubbles;
	float scaleBubbles;

	//draw larger bubbles for each new level
	switch (level)
	{
	case 1:
		scaleBubbles = 0.1;
		break;
	case 2:
		scaleBubbles = 0.2;
		break;
	case 3:
		scaleBubbles = 0.3;
		break;
	}
	//draw the player sprite and bubbles
	if (animationDirection == 1) //right
	{
		al_draw_bitmap_region(image, fx, fy, frameWidth,frameHeight, x-xoffset, y-yoffset, 0);
		al_draw_scaled_bitmap(bubbles, fxBubbles, fyBubbles, frameWidthBubbles, frameHeightBubbles, x - xoffset + 150, y - yoffset - 50, frameWidthBubbles * scaleBubbles, frameHeightBubbles * scaleBubbles, 0);
	}
	else if (animationDirection == 0) //left
	{
		al_draw_bitmap_region(image, fx, fy, frameWidth,frameHeight, x-xoffset, y-yoffset, ALLEGRO_FLIP_HORIZONTAL);
		al_draw_scaled_bitmap(bubbles, fxBubbles, fyBubbles, frameWidthBubbles, frameHeightBubbles, x - xoffset, y - yoffset - 50, frameWidthBubbles * scaleBubbles, frameHeightBubbles * scaleBubbles, ALLEGRO_FLIP_HORIZONTAL);
	}
	else if (animationDirection == 2) //up
	{
		if (lastKey == 0) //up, facing left
		{
			al_draw_tinted_scaled_rotated_bitmap_region(image, fx, fy, frameWidth, frameHeight, al_map_rgb(255, 255, 255), 0, frameHeight, x - xoffset, y - yoffset, frameWidth * xscale, frameHeight * yscale, angleL, ALLEGRO_FLIP_HORIZONTAL);
			al_draw_scaled_bitmap(bubbles, fxBubbles, fyBubbles, frameWidthBubbles, frameHeightBubbles, x - xoffset + 30, y - yoffset - 120, frameWidthBubbles * scaleBubbles, frameHeightBubbles * scaleBubbles, ALLEGRO_FLIP_HORIZONTAL);
		}
		else //up, facing right
		{
			al_draw_tinted_scaled_rotated_bitmap_region(image, fx, fy, frameWidth, frameHeight, al_map_rgb(255, 255, 255), 0, 0, x - xoffset, y - yoffset, frameWidth * xscale, frameHeight * yscale, angleR, 0);
			al_draw_scaled_bitmap(bubbles, fxBubbles, fyBubbles, frameWidthBubbles, frameHeightBubbles, x - xoffset + 120, y - yoffset - 150, frameWidthBubbles * scaleBubbles, frameHeightBubbles * scaleBubbles, 0);
		}
	}
	else if (animationDirection == 3) //down
	{
		if (lastKey == 0) //down, facing left
		{
			al_draw_tinted_scaled_rotated_bitmap_region(image, fx, fy, frameWidth, frameHeight, al_map_rgb(255, 255, 255), 10, -frameHeight, x - xoffset, y - yoffset, frameWidth * xscale, frameHeight * yscale, -angleL, ALLEGRO_FLIP_HORIZONTAL);
			al_draw_scaled_bitmap(bubbles, fxBubbles, fyBubbles, frameWidthBubbles, frameHeightBubbles, x - xoffset + 60, y - yoffset, frameWidthBubbles * scaleBubbles, frameHeightBubbles * scaleBubbles, ALLEGRO_FLIP_HORIZONTAL);
		}
		else //down, facing right
		{
			al_draw_tinted_scaled_rotated_bitmap_region(image, fx, fy, frameWidth, frameHeight, al_map_rgb(255, 255, 255), 0, 0, x - xoffset, y - yoffset, frameWidth * xscale, frameHeight * yscale, -angleR, 0);
			al_draw_scaled_bitmap(bubbles, fxBubbles, fyBubbles, frameWidthBubbles, frameHeightBubbles, x - xoffset + 100, y - yoffset + 25, frameWidthBubbles * scaleBubbles, frameHeightBubbles * scaleBubbles, 0);
		}
	}
	else if (animationDirection == 4) //hovering
	{
		al_draw_bitmap_region(image,0,0,frameWidth,frameHeight,  x-xoffset, y-yoffset, 0);
	}
}
