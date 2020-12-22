#include "MainMenu.h"
#include "GameWorld.h"

#include "GestureLayer.h"

CCScene* MainMenu::scene()
{
    CCScene *scene = CCScene::create();
    MainMenu *layer = MainMenu::create();
    scene->addChild(layer);
    return scene;
}

bool MainMenu::init()
{
    if ( !CCLayer::init() )
    {
        return false;
    }

	CCSprite* bg = CCSprite::create("TWtitle.png");
	bg->setAnchorPoint(CCPointZero);
	addChild(bg);
	
	// create & add the play button's menu
	CCMenu* menu = CCMenu::create();
	menu->setAnchorPoint(CCPointZero);
	menu->setPosition(CCPointZero);
	addChild(menu);

	// create & add the play button
	CCMenuItemSprite* play_button = CCMenuItemSprite::create(CCSprite::create("TWplaybtn.png"), CCSprite::create("TWplaybtn.png"), this, menu_selector(MainMenu::OnPlayClicked));
	play_button->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.2f));
	menu->addChild(play_button);

	return true;
}

void MainMenu::OnPlayClicked(CCObject* sender)
{
	// handler function for the play button
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}