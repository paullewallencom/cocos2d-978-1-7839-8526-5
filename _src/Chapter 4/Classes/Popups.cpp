#include "Popups.h"
#include "GameWorld.h"
#include "MainMenu.h"

Popup::Popup()
{
	CCSprite* popup_frame = CCSprite::createWithSpriteFrameName("iupopup");
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

	// add the title/message of the popup
	CCLabelBMFont* label = CCLabelBMFont::create("Game Paused", "infont.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.7));
	label->setScale(1.4f);
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Continue", "infont.fnt", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.5), menu_selector(Popup::ResumeGame));
	AddLabelButton("Restart", "infont.fnt", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.4), menu_selector(Popup::RestartGame));
	AddLabelButton("Main Menu", "infont.fnt", ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.3), menu_selector(Popup::QuitToMainMenu));
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

GameOverPopup::GameOverPopup(GameWorld* game_world, int score, int kills, int time)
{
	game_world_ = game_world;

	// add the title/message of the popup
	CCLabelBMFont* label = CCLabelBMFont::create("Game Over!", "infont.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.7));
	label->setScale(1.4f);
	addChild(label);

	// add the score
	char buf[32] = {0};
	sprintf(buf, "Score: %d", score);
	label = CCLabelBMFont::create(buf, "infont.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.6));
	addChild(label);

	// add the kills
	sprintf(buf, "Enemies Killed: %d", kills);
	label = CCLabelBMFont::create(buf, "infont.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.55));
	addChild(label);

	// add the time
	int minutes = time/60;
	sprintf(buf, "Time: %02d:%02d", minutes, time - minutes*60);
	label = CCLabelBMFont::create(buf, "infont.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5, SCREEN_SIZE.height*0.5));
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Restart", "infont.fnt", ccp(SCREEN_SIZE.width*0.4, SCREEN_SIZE.height*0.4), menu_selector(Popup::RestartGame));
	AddLabelButton("Main Menu", "infont.fnt", ccp(SCREEN_SIZE.width*0.6, SCREEN_SIZE.height*0.4), menu_selector(Popup::QuitToMainMenu));
}

GameOverPopup::~GameOverPopup()
{}

GameOverPopup* GameOverPopup::create(GameWorld* game_world, int score, int kills, int time)
{
	GameOverPopup* game_over_popup = new GameOverPopup(game_world, score, kills, time);
	if(game_over_popup && game_over_popup->initWithColor(ccc4(0, 0, 0, 128)))
	{
		game_over_popup->autorelease();
		return game_over_popup;
	}
	CC_SAFE_DELETE(game_over_popup);
	return NULL;
}
