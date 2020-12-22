#include "BackgroundManager.h"

bool BackgroundManager::init()
{
	if ( !CCNode::init() )
    {
        return false;
    }

	// create an array to hold the star sprites
	stars_ = CCArray::createWithCapacity(MAX_STARS);
	stars_->retain();
	star_size_ = CCSizeZero;

	// add the bright blue background
	CCSprite* background_sprite = CCSprite::create("sfbg.png");
	background_sprite->setPosition(ccp(SCREEN_SIZE.width/2, SCREEN_SIZE.height/2));
	addChild(background_sprite);

	CreateStars();

	// the stars need to move so schedule an update
	scheduleUpdate();

	return true;
}

BackgroundManager::~BackgroundManager(void)
{
	CC_SAFE_RELEASE_NULL(stars_);
}

void BackgroundManager::CreateStars()
{
	// create a batch node
	CCSpriteBatchNode* sprite_batch_node = CCSpriteBatchNode::create("spacetex.png", MAX_STARS);
	addChild(sprite_batch_node);

	// create stars, position them randomly and add them to the batch node & the array
	for(int i = 0; i < MAX_STARS; ++i)
	{
		CCSprite* star = CCSprite::createWithSpriteFrameName("star");
		star->setPosition(ccp(CCRANDOM_0_1() * SCREEN_SIZE.width, CCRANDOM_0_1() * SCREEN_SIZE.height));
		sprite_batch_node->addChild(star);
		stars_->addObject(star);
	}

	// save the size of the stars
	star_size_ = ((CCSprite*)stars_->randomObject())->getContentSize();
}

void BackgroundManager::update(float dt)
{
	CCObject* object = NULL;
	CCSprite* star = NULL;
	float speed = 10.0f;
	float position = 0.0f;

	// update each star's position
	// reposition star at top if it has crossed the bottom of the screen
	CCARRAY_FOREACH(stars_, object)
	{
		star = (CCSprite*)object;
		position = star->getPositionY() - speed;

		if(position < star_size_.height * -0.5)
			position = SCREEN_SIZE.height + star_size_.height * 0.5;

		star->setPositionY(position);
		speed += 0.5f;
	}
}
