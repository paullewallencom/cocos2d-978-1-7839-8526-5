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

// on "init" you need to initialize your instance
bool MainMenu::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }

	// enable accelerometer
	setAccelerometerEnabled(true);

	// create and add the stars
	background_ = BackgroundManager::create();
	addChild(background_, E_LAYER_BACKGROUND);

	// create & add the title of the game
	CCSprite* title = CCSprite::createWithSpriteFrameName("iutitle");
	title->setPosition(ccp(SCREEN_SIZE.width * 0.5, SCREEN_SIZE.height * 1.2));
	title->runAction(CCSequence::createWithTwoActions(CCDelayTime::create(0.5f), CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(0, SCREEN_SIZE.height * -0.5)))));
	addChild(title, E_LAYER_FOREGROUND);

	// create & add the play button's menu
	CCMenu* menu = CCMenu::create();
	menu->setAnchorPoint(CCPointZero);
	menu->setPosition(CCPointZero);
	addChild(menu, E_LAYER_FOREGROUND);

	// create & add the play button
	CCMenuItemSprite* play_button = CCMenuItemSprite::create(CCSprite::create("play_button.png"), CCSprite::create("play_button.png"), this, menu_selector(MainMenu::OnPlayClicked));
	play_button->setColor(ccc3(77, 210, 245));
	play_button->setPosition(ccp(SCREEN_SIZE.width * 0.5, SCREEN_SIZE.height * -0.15));
	play_button->runAction(CCSequence::createWithTwoActions(CCDelayTime::create(1.0f), CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(0, SCREEN_SIZE.height * 0.5)))));
	menu->addChild(play_button);
    
    return true;
}

void MainMenu::didAccelerate(CCAcceleration* acceleration_value)
{
	// move the stars according to the accelerometer
	background_->setPosition(ccp(acceleration_value->x * -30, acceleration_value->y * -30));
}

void MainMenu::OnPlayClicked(CCObject* sender)
{
	// handler function for the play button
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}
