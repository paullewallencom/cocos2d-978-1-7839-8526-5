#include "Brick.h"

Brick::Brick(const char* frame_name)
{
	score_ = 0;

	// different size for different bricks
	if(strstr(frame_name, "1"))
	{
		size_ = CCSizeMake(50, 20);
	}
	else if(strstr(frame_name, "2"))
	{
		size_ = CCSizeMake(55, 20);
	}
}

Brick::~Brick()
{}

Brick* Brick::createWithSpriteFrameName(const char* frame_name)
{
	Brick* brick = new Brick(frame_name);
	if(brick && brick->initWithSpriteFrameName(frame_name))
	{
		brick->autorelease();
		return brick;
	}
	CC_SAFE_DELETE(brick);
	return NULL;
}

int Brick::Crumble()
{
	// play an animation when this brick is hit by player bullet
	CCActionInterval* blast = CCScaleTo::create(0.25f, 0.0f);
	CCRemoveSelf* remove = CCRemoveSelf::create(true);
	runAction(CCSequence::createWithTwoActions(blast, remove));

	SOUND_ENGINE->playEffect("blast_brick.wav");

	// return score_ so it can be credited to the player
	return score_;
}
