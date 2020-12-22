#include "MainMenu.h"
#include "BackgroundManager.h"
#include "GameWorld.h"

CCScene* MainMenu::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    MainMenu *layer = MainMenu::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

bool MainMenu::init()
{
    if ( !CCLayer::init() )
    {
        return false;
    }

    // create the environment
	BackgroundManager* background_manager = BackgroundManager::create();
	addChild(background_manager, E_LAYER_BACKGROUND);

	// create & add the batch node
	sprite_batch_node_ = CCSpriteBatchNode::create("spacetex.png", 10);
	addChild(sprite_batch_node_, E_LAYER_FOREGROUND);

	// create & add the title of the game
	CCSprite* title = CCSprite::createWithSpriteFrameName("sftitle");
	title->setPosition(ccp(SCREEN_SIZE.width * 0.5, SCREEN_SIZE.height * 1.2));
	title->runAction(CCSequence::createWithTwoActions(CCDelayTime::create(0.5f), CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(0, SCREEN_SIZE.height * -0.5f)))));
	sprite_batch_node_->addChild(title, E_LAYER_FOREGROUND);

	// create & add the play button
	CCMenuItemSprite* play_button = CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName("play"), CCSprite::createWithSpriteFrameName("play"), this, menu_selector(MainMenu::OnPlayClicked));
	play_button->setPosition(ccp(SCREEN_SIZE.width * 0.5, SCREEN_SIZE.height * -0.15));
	play_button->runAction(CCSequence::createWithTwoActions(CCDelayTime::create(1.0f), CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(0, SCREEN_SIZE.height * 0.5f)))));

	// create & add the play button's menu
	CCMenu* play_menu = CCMenu::create(play_button, NULL);
	play_menu->setAnchorPoint(CCPointZero);
	play_menu->setPosition(CCPointZero);
	addChild(play_menu, E_LAYER_FOREGROUND);

	// time for some chaos!
	MainMenu::Animate(0.0f);
	MainMenu::Animate(0.0f);
	schedule(schedule_selector(MainMenu::Animate), 0.5f);

    return true;
}

void MainMenu::Animate(float dt)
{
	CCPoint source = CCPointZero;
	CCPoint distance = CCPointZero;

	// compute random duration of movement
	float duration = 1 + CCRANDOM_0_1() * 3;

	// choose between left-to-right or top-to-bottom
	if(CCRANDOM_MINUS1_1() > 0)
	{
		// left-to-right
		// compute starting position
		source.x = (CCRANDOM_MINUS1_1() > 0) ? (SCREEN_SIZE.width * -0.25f) : (SCREEN_SIZE.width * 1.25f);
		source.y = SCREEN_SIZE.height * 0.25f + CCRANDOM_0_1() * SCREEN_SIZE.height * 0.5f;

		// compute distance to travel
		distance.x = (source.x < 0) ? SCREEN_SIZE.width*1.5f : SCREEN_SIZE.width*-1.5f;
		distance.y = CCRANDOM_0_1() * SCREEN_SIZE.height;
	}
	else
	{
		// top-to-bottom
		// compute starting position
		source.x = SCREEN_SIZE.width * 0.25f + CCRANDOM_0_1() * SCREEN_SIZE.width * 0.5f;
		source.y = (CCRANDOM_MINUS1_1() > 0) ? (SCREEN_SIZE.height * -0.25f) : (SCREEN_SIZE.height * 1.25f);

		// compute distance to travel
		distance.x = CCRANDOM_0_1() * SCREEN_SIZE.width;
		distance.y = (source.y < 0) ? SCREEN_SIZE.height*1.5f : SCREEN_SIZE.height*-1.5f;
	}

	// choose an enemy ship
	char buf[16] = {0};
	int type = 1 + (int)(CCRANDOM_0_1() * 3);
	sprintf(buf, "sfenmy%d", type);

	// create enemy sprite & add it
	CCSprite* enemy_ship = CCSprite::createWithSpriteFrameName(buf);
	sprite_batch_node_->addChild(enemy_ship);

	// position enemy sprite
	enemy_ship->setPosition(source);

	// create & run sequence of move & remove
	CCActionInterval* movement = CCMoveBy::create(duration, distance);
	CCActionInstant* remove = CCRemoveSelf::create(true);
	enemy_ship->runAction(CCSequence::createWithTwoActions(movement, remove));
}

void MainMenu::OnPlayClicked(CCObject* sender)
{
	// handler function for the play button
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}
