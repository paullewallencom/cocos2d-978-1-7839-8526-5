#include "CustomSprite.h"

CustomSprite::~CustomSprite()
{}

CustomSprite* CustomSprite::createWithSpriteFrameName(const char* frame_name)
{
	CustomSprite* sprite = new CustomSprite();
	if(sprite && sprite->initWithSpriteFrameName(frame_name))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return NULL;
}

CCRect CustomSprite::boundingBox()
{
	// return bounding box based on our own size_ variable
	return CCRectMake(m_obPosition.x - size_.width/2, m_obPosition.y - size_.height/2, size_.width, size_.height);
}
