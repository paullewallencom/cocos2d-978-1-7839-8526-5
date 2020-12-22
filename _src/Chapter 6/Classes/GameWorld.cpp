#include "GameWorld.h"
#include "Popups.h"
#include "Sky.h"
#include "Terrain.h"
#include "Penguin.h"

#ifdef ENABLE_DEBUG_DRAW
#include "GLES-Render.h"
#endif

GameWorld::GameWorld()
{
	world_ = NULL;
	sky_ = NULL;
	terrain_ = NULL;
	penguin_ = NULL;
	distance_travelled_ = 0;
	distance_label_ = NULL;
	time_left_ = 0;
	time_label_ = NULL;
	message_label_ = NULL;
	has_game_begun_ = false;
	has_game_ended_ = false;
	is_popup_active_ = false;
#ifdef ENABLE_DEBUG_DRAW
	debug_draw_ = NULL;
#endif
}

GameWorld::~GameWorld()
{
	DestroyWorld();
}

CCScene* GameWorld::scene()
{
    CCScene *scene = CCScene::create();
    GameWorld *layer = GameWorld::create();
    scene->addChild(layer);
    return scene;
}

bool GameWorld::init()
{
    if ( !CCLayer::init() )
    {
        return false;
    }

    CreateWorld();
	CreateGame();
    return true;
}

void GameWorld::CreateWorld()
{
	// create world
	b2Vec2 gravity;
	gravity.Set(0, -9.8f);
	world_ = new b2World(gravity);
	// tell world we want to listen for collisions
	world_->SetContactListener(this);

#ifdef ENABLE_DEBUG_DRAW
	debug_draw_ = new GLESDebugDraw(PTM_RATIO);
	world_->SetDebugDraw(debug_draw_);
	uint32 flags = 0;
	flags += b2Draw::e_shapeBit;
	debug_draw_->SetFlags(flags);
#endif
}

void GameWorld::DestroyWorld()
{
#ifdef ENABLE_DEBUG_DRAW
	CC_SAFE_DELETE(debug_draw_);
	debug_draw_ = NULL;
#endif
	CC_SAFE_DELETE(world_);
	world_ = NULL;
}

void GameWorld::CreateGame()
{
	// player starts off with 30 seconds
	time_left_ = 30;

	// create & add the first set of hills at 0
	terrain_ = Terrain::create(world_, 0);
	addChild(terrain_, E_LAYER_FOREGROUND);
	
	// create & add the sky
	sky_ = Sky::create();
	addChild(sky_, E_LAYER_BACKGROUND);

	// create & add the penguin to the hills
	penguin_ = Penguin::create(this, "penguin_1.png");
	terrain_->addChild(penguin_, E_LAYER_FOREGROUND);

	CreateHUD();

	// enable touch and schedule two selectors; the seconds ticker and the update
	setTouchEnabled(true);
	schedule(schedule_selector(GameWorld::Tick), 1.0f);
	scheduleUpdate();
}

void GameWorld::CreateHUD()
{
	// create & add the distance label
	char buf[16] = {0};
	sprintf(buf, "%dm", distance_travelled_);
	distance_label_ = CCLabelBMFont::create(buf, "wal_font.fnt");
	distance_label_->setPosition(ccp(SCREEN_SIZE.width*0.1f, SCREEN_SIZE.height*0.925f));
	distance_label_->setAnchorPoint(ccp(0, 0.5f));
	addChild(distance_label_, E_LAYER_HUD);

	// create & add the time left label
	sprintf(buf, "%ds", time_left_);
	time_label_ = CCLabelBMFont::create(buf, "wal_font.fnt");
	time_label_->setPosition(ccp(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.925f));
	addChild(time_label_, E_LAYER_HUD);

	// create and add a general message label
	message_label_ = CCLabelBMFont::create("", "wal_font.fnt");
	message_label_->setVisible(false);
	addChild(message_label_, E_LAYER_HUD);

	// create & add the pause menu containing pause button
	CCMenuItemSprite* pause_button = CCMenuItemSprite::create(CCSprite::create("pause.png"), CCSprite::create("pause.png"), this, menu_selector(GameWorld::OnPauseClicked));
	pause_button->setPosition(ccp(SCREEN_SIZE.width*0.95f, SCREEN_SIZE.height*0.925f));
	CCMenu* menu = CCMenu::create(pause_button, NULL);
	menu->setAnchorPoint(CCPointZero);
	menu->setPosition(CCPointZero);
	addChild(menu, E_LAYER_HUD);
}

void GameWorld::ccTouchesBegan(CCSet* set, CCEvent* event)
{
	if(is_popup_active_)
		return;

	// while leaping, don't allow touch unless the next hill is close enough
	if(terrain_->GetFootKeyPoint().x > penguin_->getPositionX() &&
		terrain_->GetFootKeyPoint().x - penguin_->getPositionX() > 10)
	{
		return;
	}

	// game begins only after the player touches
	has_game_begun_ = true;
	// tell the penguin to dive
	penguin_->SetIsDiving(true);
}

void GameWorld::ccTouchesEnded(CCSet* set, CCEvent* event)
{
	if(is_popup_active_)
		return;

	// tell the penguin to stop diving
	penguin_->SetIsDiving(false);
}

void GameWorld::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	// get the world manifold for this contact
	b2WorldManifold world_manifold;
	contact->GetWorldManifold(&world_manifold);
	// get velocity of the penguin
	b2Vec2 velocity = penguin_->GetBody()->GetLinearVelocity();

	// get angles for the velocity & normal vectors
	float velocity_angle = atan2f(velocity.y, velocity.x);
	float normal_angle = atan2f(world_manifold.normal.y, world_manifold.normal.x);

	// it is a bad landing if the difference in angles is above a certain threshold
	if (normal_angle - velocity_angle > BAD_LANDING_ANGLE_DIFF)
	{
		penguin_->OnBadLanding();
	}
}

void GameWorld::update(float dt)
{
	// update penguin physics
	penguin_->UpdatePhysics();

	// update the world
	// slow it down...otherwise the game is too fast to enjoy!
	world_->Step(dt * 0.5f, 8, 3);

	// update penguin node
	penguin_->UpdateNode();

	// update the hills and sky
	terrain_->Update(penguin_->getPosition());
	sky_->Update(penguin_->getPosition(), terrain_->GetOffsetX(), terrain_->getScale());

	// if penguin has gone beyond the second last key point, its time to take a leap
	if(penguin_->getPositionX() > terrain_->GetCliffKeyPoint().x && !penguin_->GetIsLeaping())
	{
		penguin_->StartLeap();
	}
	// check if the penguin is leaping first
	if(penguin_->GetIsLeaping())
	{
		// if the next hill's foot is close enough, its time to stop leaping & time to start falling
		if(terrain_->GetFootKeyPoint().x > penguin_->getPositionX() &&
			terrain_->GetFootKeyPoint().x - penguin_->getPositionX() <= SCREEN_SIZE.width * 1.75f)
		{
			penguin_->FinishLeap();
		}
	}

	// update the distance counter & label
	int new_distance_travelled = penguin_->getPositionX();
	if(new_distance_travelled > distance_travelled_)
	{
		char buf[16] = {0};
		sprintf(buf, "%dm", (int)(new_distance_travelled / 2));
		distance_label_->setString(buf);
		distance_travelled_ = new_distance_travelled;
	}
}

void GameWorld::Tick(float dt)
{
	// tick only after the game has started till before it ends
	if(!has_game_begun_ || has_game_ended_)
		return;

	// decrement the time counter
	-- time_left_;

	// update the label
	char buf[16] = {0};
	sprintf(buf, "%ds", time_left_);
	time_label_->setString(buf);

	// no more time...game over
	if(time_left_ <= 0)
	{
		GameOver();
	}

	// when there 5 seconds or less, start animating the label & playing a sound
	if(time_left_ <= 5)
	{
		if(time_label_->numberOfRunningActions() <= 0)
		{
			CCActionInterval* scale_up_down = CCSequence::createWithTwoActions(CCScaleTo::create(0.2f, 1.2f), CCScaleTo::create(0.2f, 1.0f));
			time_label_->runAction(CCRepeatForever::create(scale_up_down));

			CCActionInterval* shake = CCSequence::createWithTwoActions(CCRotateBy::create(0.05f, 20.0f), CCRotateBy::create(0.05f, -20.0f));
			time_label_->setRotation(-10.0f);
			time_label_->runAction(CCRepeatForever::create(shake));
		}
	}
}

void GameWorld::AddTime(int value)
{
	time_left_ += value;
	char buf[16] = {0};
	sprintf(buf, "%ds", time_left_);
	time_label_->setString(buf);
	
	// stop any actions that may been running
	time_label_->stopAllActions();
	// reset scale & rotation
	time_label_->setScale(1.0f);
	time_label_->setRotation(0.0f);

	CCActionInterval* scale_up_down = CCSequence::createWithTwoActions(CCEaseSineIn::create(CCScaleTo::create(0.2f, 1.2f)), CCEaseSineOut::create(CCScaleTo::create(0.2f, 1.0f)));
	time_label_->runAction(scale_up_down);
}

void GameWorld::ShowMessage(const char* message)
{
	// message will be hidden initially so show it
	message_label_->setVisible(true);
	message_label_->setPosition(ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 1.2f));
	message_label_->setOpacity(255);
	message_label_->setString(message);

	// animate entry & exit
	CCActionInterval* sequence = CCSequence::create(CCEaseBackOut::create(CCMoveTo::create(0.2f, ccp(SCREEN_SIZE.width * 0.5f, SCREEN_SIZE.height * 0.8f))),
		CCDelayTime::create(1.0f), 
		CCFadeOut::create(0.1f), 
		CCHide::create(), NULL);
	message_label_->runAction(sequence);
}

void GameWorld::OnPauseClicked(CCObject* sender)
{
	// this prevents multiple pause popups
	if(is_popup_active_)
		return;

	pauseSchedulerAndActions();
	
	// pause game elements here
	penguin_->Pause();
	time_label_->pauseSchedulerAndActions();

	// create & add the pause popup
	PausePopup* pause_popup = PausePopup::create(this);
	addChild(pause_popup, E_LAYER_POPUP);
}

void GameWorld::ResumeGame()
{
	is_popup_active_ = false;

	// resume GameWorld update & Tick functions
	resumeSchedulerAndActions();

	// resume game elements here
	penguin_->Resume();
	time_label_->resumeSchedulerAndActions();
}

void GameWorld::GameOver()
{
	has_game_ended_ = true;

	// stop everything
	time_label_->pauseSchedulerAndActions();
	time_label_->setScale(1.0f);
	time_label_->setRotation(0.0f);
	unscheduleAllSelectors();
	setTouchEnabled(false);

	// create & add the game over popup
	GameOverPopup* game_over_popup = GameOverPopup::create(this, distance_travelled_/2, penguin_->GetTotalPerfectSlides());
	addChild(game_over_popup, E_LAYER_POPUP);
}
