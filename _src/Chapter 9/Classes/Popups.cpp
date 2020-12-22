#include "Popups.h"
#include "GameWorld.h"
#include "MainMenu.h"

Popup::Popup()
{
	// set initial scale & animate the entry of the popup
	setScale(0.0f);
	runAction(CCEaseBackOut::create(CCScaleTo::create(0.25f, 1.0f)));

	menu_ = NULL;
}

Popup::~Popup()
{}

CCMenu* Popup::AddMenu()
{
	// return menu_ if already created & added
	if(menu_)
		return menu_;

	// create & add the menu_
	menu_ = CCMenu::create();
	menu_->setAnchorPoint(CCPointZero);
	menu_->setPosition(CCPointZero);
	addChild(menu_);
	return menu_;
}

CCMenuItem* Popup::AddButton(CCMenuItem* button, CCPoint position)
{
	// if menu_ or button are NULL, return
	if( (menu_ == NULL && AddMenu() == NULL) || button == NULL )
		return NULL;

	// position the button & add to menu_
	button->setPosition(position);
	menu_->addChild(button);
	return button;
}

CCMenuItemLabel* Popup::AddLabelButton(const char* text, CCPoint position, SEL_MenuHandler handler)
{
	// create CCMenuItemLabel with CCLabelTTF
	return (CCMenuItemLabel*)AddButton(CCMenuItemLabel::create(CCLabelTTF::create(text, "Arial", 36.0f), this, handler), position);
}

CCMenuItemLabel* Popup::AddLabelButton(const char* text, const char* font, CCPoint position, SEL_MenuHandler handler)
{
	// create CCMenuItemLabel with CCLabelBMFont
	return (CCMenuItemLabel*)AddButton(CCMenuItemLabel::create(CCLabelBMFont::create(text, font), this, handler), position);
}

CCMenuItemSprite* Popup::AddSpriteButton(const char* frame_name, CCPoint position, SEL_MenuHandler handler)
{
	// create CCMenuItemSprite
	return (CCMenuItemSprite*)AddButton(CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName(frame_name), CCSprite::createWithSpriteFrameName(frame_name), this, handler), position);
}

void Popup::ResumeGame(CCObject* sender)
{
	game_world_->ResumeGame();
	runAction(CCSequence::createWithTwoActions(CCEaseBackIn::create(CCScaleTo::create(0.25f, 0.0f)), CCRemoveSelf::create(true)));
}

void Popup::RestartGame(CCObject* sender)
{	
	removeFromParentAndCleanup(true);
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}

void Popup::NextLevel(CCObject* sender)
{
	++ GameGlobals::level_number_;
	removeFromParentAndCleanup(true);
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}

void Popup::QuitToMainMenu(CCObject* sender)
{
	removeFromParentAndCleanup(true);
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, MainMenu::scene()));
}

PausePopup::PausePopup(GameWorld* game_world)
{
	game_world_ = game_world;
	game_world_->is_popup_active_ = true;

	// add the title/message of the popup
	CCLabelTTF* label = CCLabelTTF::create("Game Paused", "Arial", 52.0f);
	label->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.75f));
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Continue", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.6f), menu_selector(Popup::ResumeGame));
	AddLabelButton("Restart", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.5f), menu_selector(Popup::RestartGame));
	AddLabelButton("Main Menu", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.4f), menu_selector(Popup::QuitToMainMenu));
}

PausePopup::~PausePopup()
{}

PausePopup* PausePopup::create(GameWorld* game_world)
{
	PausePopup* pause_popup = new PausePopup(game_world);
	if(pause_popup && pause_popup->initWithColor(ccc4(0, 0, 0, 128)))
	{
		pause_popup->autorelease();
		return pause_popup;
	}
	CC_SAFE_DELETE(pause_popup);
	return NULL;
}

LevelCompletePopup::LevelCompletePopup(GameWorld* game_world)
{
	game_world_ = game_world;
	game_world_->is_popup_active_ = true;

	// add the title/message of the popup
	CCLabelTTF* label = CCLabelTTF::create("Stage Complete!", "Arial", 52.0f);
	label->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.75f));
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Restart", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.4f), menu_selector(Popup::RestartGame));
	AddLabelButton("Main Menu", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.3f), menu_selector(Popup::QuitToMainMenu));
	AddLabelButton("Next Level", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.5f), menu_selector(Popup::NextLevel));
}

LevelCompletePopup::~LevelCompletePopup()
{}

LevelCompletePopup* LevelCompletePopup::create(GameWorld* game_world)
{
	LevelCompletePopup* level_complete_popup = new LevelCompletePopup(game_world);
	if(level_complete_popup && level_complete_popup->initWithColor(ccc4(0, 0, 0, 128)))
	{
		level_complete_popup->autorelease();
		return level_complete_popup;
	}
	CC_SAFE_DELETE(level_complete_popup);
	return NULL;
}

GameOverPopup::GameOverPopup(GameWorld* game_world)
{
	game_world_ = game_world;
	game_world_->is_popup_active_ = true;

	// add the title/message of the popup
	CCLabelTTF* label = CCLabelTTF::create("Game Over!", "Arial", 52.0f);
	label->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.75f));
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Restart", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.5f), menu_selector(Popup::RestartGame));
	AddLabelButton("Main Menu", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.4f), menu_selector(Popup::QuitToMainMenu));
}

GameOverPopup::~GameOverPopup()
{}

GameOverPopup* GameOverPopup::create(GameWorld* game_world)
{
	GameOverPopup* game_over_popup = new GameOverPopup(game_world);
	if(game_over_popup && game_over_popup->initWithColor(ccc4(0, 0, 0, 128)))
	{
		game_over_popup->autorelease();
		return game_over_popup;
	}
	CC_SAFE_DELETE(game_over_popup);
	return NULL;
}
