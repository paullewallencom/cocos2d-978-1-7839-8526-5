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

	// create & add the BG
	CCSprite* bg = CCSprite::createWithSpriteFrameName("cjname.png");//create("cjname.png");
	bg->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.5f));
	addChild(bg);

	// create & add the title of the game
	CCSprite* title = CCSprite::createWithSpriteFrameName("cjname01.png");//create("cjname01.png");
	title->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.35f));
	addChild(title, E_LAYER_FOREGROUND);

	CCActionInterval* move_up = CCEaseSineOut::create(CCScaleTo::create(1.0f, 1.05f));
	CCActionInterval* move_down = CCEaseSineIn::create(CCScaleTo::create(1.0f, 1.0f));
	title->runAction(CCRepeatForever::create(CCSequence::createWithTwoActions(move_up, move_down)));

	// create & add the play button
	CCMenuItemSprite* play_button = CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName("cjbplay.png"), CCSprite::createWithSpriteFrameName("cjbplay.png"), this, menu_selector(MainMenu::OnPlayClicked));
	play_button->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.05f));

	// create & add the play button's menu
	CCMenu* play_menu = CCMenu::create(play_button, NULL);
	play_menu->setAnchorPoint(CCPointZero);
	play_menu->setPosition(CCPointZero);
	addChild(play_menu, E_LAYER_FOREGROUND);

    return true;
}

void MainMenu::OnPlayClicked(CCObject* sender)
{
	// handler function for the play button
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}
