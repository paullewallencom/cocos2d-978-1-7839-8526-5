#include "BackgroundManager.h"

bool BackgroundManager::init()
{
	if ( !CCNode::init() )
    {
        return false;
    }

	// create a CCDrawNode to draw the stars
	CCDrawNode* star = CCDrawNode::create();
	addChild(star);

	// draw a white circle with varying opacity & position
	for(int i = 0; i < MAX_STARS; ++i)
	{
		CCPoint position = ccp(CCRANDOM_0_1() * SCREEN_SIZE.width, CCRANDOM_0_1() * SCREEN_SIZE.height);
		float alpha = 0.25f + CCRANDOM_0_1() * 0.75f;
		star->drawDot(position, 1, ccc4f(1, 1, 1, alpha));
	}

	return true;
}