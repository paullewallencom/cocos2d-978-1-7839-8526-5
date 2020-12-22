#include "MainMenu.h"
#include "GameWorld.h"

CCScene* MainMenu::scene()
{
    CCScene *scene = CCScene::create();
    MainMenu *layer = MainMenu::create();
    scene->addChild(layer);
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

	// create & add the background
	CCSprite* background = CCSprite::create("titel.png");
	background->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.5f));
	addChild(background);

	// create & add the play button's menu
	CCMenu* menu = CCMenu::create();
	menu->setAnchorPoint(CCPointZero);
	menu->setPosition(CCPointZero);
	addChild(menu);

	// create & add the play button
	CCMenuItemSprite* play_button = CCMenuItemSprite::create(CCSprite::create("play_button.png"), CCSprite::create("play_button.png"), this, menu_selector(MainMenu::OnPlayClicked));
	play_button->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.1f));
	menu->addChild(play_button);

	return true;
}

void MainMenu::OnPlayClicked(CCObject* sender)
{
	// handler function for the play button
	GameGlobals::level_number_ = 1;
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}