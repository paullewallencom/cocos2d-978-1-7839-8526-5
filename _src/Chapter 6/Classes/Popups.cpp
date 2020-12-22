#include "Popups.h"
#include "GameWorld.h"
#include "MainMenu.h"

Popup::Popup()
{
	CCSprite* popup_frame = CCSprite::create("popup.png");
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
	game_world_->DestroyWorld();
	removeFromParentAndCleanup(true);
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, GameWorld::scene()));
}

void Popup::QuitToMainMenu(CCObject* sender)
{
	game_world_->DestroyWorld();
	removeFromParentAndCleanup(true);
	CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, MainMenu::scene()));
}

PausePopup::PausePopup(GameWorld* game_world)
{
	game_world_ = game_world;
	game_world_->is_popup_active_ = true;

	// add the title/message of the popup
	CCLabelBMFont* label = CCLabelBMFont::create("Game Paused", "wal_font.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.75f));
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Continue", "wal_font.fnt", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.55f), menu_selector(Popup::ResumeGame))->setScale(0.8f);
	AddLabelButton("Restart", "wal_font.fnt", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.45f), menu_selector(Popup::RestartGame))->setScale(0.8f);
	AddLabelButton("Main Menu", "wal_font.fnt", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.35f), menu_selector(Popup::QuitToMainMenu))->setScale(0.8f);
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

GameOverPopup::GameOverPopup(GameWorld* game_world, int score, int num_perfect_slides)
{
	game_world_ = game_world;
	game_world_->is_popup_active_ = true;

	// add the title/message of the popup
	CCLabelBMFont* label = CCLabelBMFont::create("Game Over!", "wal_font.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.8f));
	addChild(label);

	// add the distance
	char buf[128] = {0};
	sprintf(buf, "Distance: %dm", score);
	label = CCLabelBMFont::create(buf, "wal_font.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.65f));
	label->setScale(0.9f);
	addChild(label);

	// add perfect slides
	sprintf(buf, "Perfect slides: %d", num_perfect_slides);
	label = CCLabelBMFont::create(buf, "wal_font.fnt");
	label->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.55f));
	label->setScale(0.9f);
	addChild(label);

	// create menu & buttons
	AddMenu();
	AddLabelButton("Restart", "wal_font.fnt", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.4f), menu_selector(Popup::RestartGame))->setScale(0.8f);
	AddLabelButton("Main Menu", "wal_font.fnt", ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.3f), menu_selector(Popup::QuitToMainMenu))->setScale(0.8f);
}

GameOverPopup::~GameOverPopup()
{}

GameOverPopup* GameOverPopup::create(GameWorld* game_world, int score, int num_perfect_slides)
{
	GameOverPopup* game_over_popup = new GameOverPopup(game_world, score, num_perfect_slides);
	if(game_over_popup && game_over_popup->initWithColor(ccc4(0, 0, 0, 128)))
	{
		game_over_popup->autorelease();
		return game_over_popup;
	}
	CC_SAFE_DELETE(game_over_popup);
	return NULL;
}
