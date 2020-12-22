#ifndef CUSTOM_SPRITE_H_
#define CUSTOM_SPRITE_H_

#include "GameGlobals.h"

class CustomSprite : public CCSprite
{
public:
	CustomSprite() : size_(CCSizeZero) {}
	virtual ~CustomSprite();

	// returns and autorelease CustomSprite
	static CustomSprite* createWithSpriteFrameName(const char* frame_name);

	// override CCSprite's boundingBox method
	virtual CCRect boundingBox();

	// add a customized CCSize used for the boundingBox
	CC_SYNTHESIZE(CCSize, size_, Size);
};

#endif // CUSTOM_SPRITE_H_
