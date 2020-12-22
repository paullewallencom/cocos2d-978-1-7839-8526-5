#include "MainMenu.h"
#include "GameWorld.h"
#include "Sky.h"
#include "Terrain.h"

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

	CreateMenu();

	// add the hills
	Terrain* terrain = Terrain::create(NULL, 0);
	terrain->Update(ccp(SCREEN_SIZE.width * 2.5f, SCREEN_SIZE.height * 0.5f));
	addChild(terrain, E_LAYER_FOREGROUND);

	// add the sky
	Sky* sky = Sky::create();
	addChild(sky, E_LAYER_BACKGROUND);

	return true;
}

void MainMenu::CreateMenu()
{
	CCSprite* title = CCSprite::create("title.png");
	title->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.65f));
	addChild(title, E_LAYER_HUD);

	CCActionInterval* scale_up = CCEaseSineOut::create(CCScaleTo::create(1.0f, 1.05f));
	CCActionInterval* scale_down = CCEaseSineIn::create(CCScaleTo::create(1.0f, 1.0f));
	title->runAction(CCRepeatForever::create(CCSequence::createWithTwoActions(scale_up, scale_down)));

	// create & add the play button's menu
	CCMenu* menu = CCMenu::create();
	menu->setAnchorPoint(CCPointZero);
	menu->setPosition(CCPointZero);
	addChild(menu, E_LAYER_HUD);

	// create & add the play button
	CCMenuItemSprite* play_button = CCMenuItemSprite::create(CCSprite::create("play.png"), CCSprite::create("play.png"), this, menu_selector(MainMenu::OnPlayClicked));
	play_button->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.3f));
	menu->addChild(play_button);
}

void MainMenu::OnPlayClicked(CCObject* sender)
{
	// handler function for the play button
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}
