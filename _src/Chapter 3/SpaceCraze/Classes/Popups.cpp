#include "Popups.h"
#include "GameWorld.h"
#include "MainMenu.h"

Popup::Popup()
{
	CCSprite* popup_frame = CCSprite::createWithSpriteFrameName("sfpopup");
	popup_frame->setPosition(ccp(SCREEN_SIZE.width/2, SCREEN_SIZE.height/2));
	addChild(popup_frame);

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
	return (CCMenuItemLabel*)AddButton(CCMenuItemLabel::create(CCLabelTTF::create(text, "Comic Sans MS", 36), this, handler), position);
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
	game_world_->RestartLevel();
	runAction(CCSequence::createWithTwoActions(CCEaseBackIn::create(CCScaleTo::create(0.25f, 0.0f)), CCRemoveSelf::create(true)));
}

void Popup::NextLevel(CCObject* sender)
{
	game_world_->NextLevel();
	runAction(CCSequence::createWithTwoActions(CCEaseBackIn::create(CCScaleTo::create(0.25f, 0.0f)), CCRemoveSelf::create(true)));
}

void Popup::QuitToMainMenu(CCObject* sender)
{
	game_world_->Reset();
	removeFromParentAndCleanup(true);
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, MainMenu::scene()));
}

PausePopup::PausePopup(GameWorld* game_world)
{
	game_world_ = game_world;
	game_world_->is_popup_active_ = true;

	// add the title/message of the popup
	CCLabelTTF* label = CCLabelTTF::create("Game Paused", "Comic Sans MS", 52);
	label->setColor(ccc3(0, 79, 135));
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.75));
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Continue", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.6), menu_selector(Popup::ResumeGame))->setColor(ccc3(0, 79, 135));
	AddLabelButton("Restart", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.5), menu_selector(Popup::RestartGame))->setColor(ccc3(0, 79, 135));
	AddLabelButton("Main Menu", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.4), menu_selector(Popup::QuitToMainMenu))->setColor(ccc3(0, 79, 135));
}

PausePopup::~PausePopup()
{}

PausePopup* PausePopup::create(GameWorld* game_world)
{
	PausePopup* pause_popup = new PausePopup(game_world);
	if(pause_popup && pause_popup->init())
	{
		pause_popup->autorelease();
		return pause_popup;
	}
	CC_SAFE_DELETE(pause_popup);
	return NULL;
}

LevelCompletePopup::LevelCompletePopup(GameWorld* game_world, int score, int lives)
{
	game_world_ = game_world;
	game_world_->is_popup_active_ = true;

	// add the title/message of the popup
	CCLabelTTF* label = CCLabelTTF::create("Stage Complete!", "Comic Sans MS", 52);
	label->setColor(ccc3(0, 79, 135));
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.75));
	addChild(label);

	// add the score
	char buf[16] = {0};
	sprintf(buf, "Score:%d", score);
	label = CCLabelTTF::create(buf, "Comic Sans MS", 44);
	label->setColor(ccc3(0, 79, 135));
	label->setPosition(ccp(SCREEN_SIZE.width*0.35, SCREEN_SIZE.height*0.6));
	addChild(label);

	// add player lives remaining
	sprintf(buf, "Lives:%d/3", lives);
	label = CCLabelTTF::create(buf, "Comic Sans MS", 44);
	label->setColor(ccc3(0, 79, 135));
	label->setPosition(ccp(SCREEN_SIZE.width*0.65, SCREEN_SIZE.height*0.6));
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Restart", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.4), menu_selector(Popup::RestartGame))->setColor(ccc3(0, 79, 135));
	AddLabelButton("Main Menu", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.3), menu_selector(Popup::QuitToMainMenu))->setColor(ccc3(0, 79, 135));
	if(game_world_->current_level_ < MAX_LEVELS)
		AddLabelButton("Next Level", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.5), menu_selector(Popup::NextLevel))->setColor(ccc3(0, 79, 135));
}

LevelCompletePopup::~LevelCompletePopup()
{}

LevelCompletePopup* LevelCompletePopup::create(GameWorld* game_world, int score, int lives)
{
	LevelCompletePopup* level_complete_popup = new LevelCompletePopup(game_world, score, lives);
	if(level_complete_popup && level_complete_popup->init())
	{
		level_complete_popup->autorelease();
		return level_complete_popup;
	}
	CC_SAFE_DELETE(level_complete_popup);
	return NULL;
}

GameOverPopup::GameOverPopup(GameWorld* game_world, int score)
{
	game_world_ = game_world;
	game_world_->is_popup_active_ = true;

	// add the title/message of the popup
	CCLabelTTF* label = CCLabelTTF::create("Game Over!", "Comic Sans MS", 52);
	label->setColor(ccc3(0, 79, 135));
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.75));
	addChild(label);

	// add the score
	char buf[16] = {0};
	sprintf(buf, "Score:%d", score);
	label = CCLabelTTF::create(buf, "Comic Sans MS", 44);
	label->setColor(ccc3(0, 79, 135));
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.6));
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Restart", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.5), menu_selector(Popup::RestartGame))->setColor(ccc3(0, 79, 135));
	AddLabelButton("Main Menu", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.4), menu_selector(Popup::QuitToMainMenu))->setColor(ccc3(0, 79, 135));
}

GameOverPopup::~GameOverPopup()
{}

GameOverPopup* GameOverPopup::create(GameWorld* game_world, int score)
{
	GameOverPopup* game_over_popup = new GameOverPopup(game_world, score);
	if(game_over_popup && game_over_popup->init())
	{
		game_over_popup->autorelease();
		return game_over_popup;
	}
	CC_SAFE_DELETE(game_over_popup);
	return NULL;
}
