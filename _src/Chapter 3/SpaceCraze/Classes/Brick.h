#ifndef BRICK_H_
#define BRICK_H_

#include "GameGlobals.h"
#include "CustomSprite.h"

class Brick : public CustomSprite
{
public:
	Brick(const char* frame_name);
	virtual ~Brick();

	// returns an autorelease Brick
	static Brick* createWithSpriteFrameName(const char* frame_name);

	// function called when this brick is hit by player bullet
	int Crumble();

	CC_SYNTHESIZE(int, score_, Score);
};

#endif // BRICK_H_
