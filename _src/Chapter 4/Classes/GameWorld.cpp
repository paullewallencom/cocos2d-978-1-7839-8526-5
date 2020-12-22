#include "GameWorld.h"
#include "BackgroundManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Shield.h"
#include "Bomb.h"
#include "Blast.h"
#include "MissileLauncher.h"
#include "Missile.h"
#include "Popups.h"

GameWorld::GameWorld()
{
	background_ = NULL;
	boundary_rect_ = CCRectZero;
	player_ = NULL;
	enemies_ = NULL;
	powerups_ = NULL;
	blasts_ = NULL;
	missiles_ = NULL;
	score_label_ = NULL;
	seconds_ = 0;
	enemies_killed_total_ = 0;
	enemies_killed_combo_ = 0;
	combo_timer_ = 0;
	score_ = 0;
	is_popup_active_ = false;
}

GameWorld::~GameWorld()
{
	CC_SAFE_RELEASE_NULL(enemies_);
}

CCScene* GameWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    GameWorld *layer = GameWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool GameWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
	// enable accelerometer & touch
    setAccelerometerEnabled(true);
    setTouchEnabled(true);

	CreateGame();

    return true;
}

void GameWorld::CreateGame()
{
	// initialise counters & flags
	seconds_ = 0;
	enemies_killed_total_ = 0;
	enemies_killed_combo_ = 0;
	combo_timer_ = 0;
	score_ = 0;
	is_popup_active_ = false;

	// add the stars
	background_ = BackgroundManager::create();
	addChild(background_, E_LAYER_BACKGROUND);

	CreateBoundary();
	CreatePlayer();
	CreateContainers();
	CreateHUD();

	// initially add some enemies & a powerup
	AddEnemyFormation();
	AddPowerUp();

	// schedule the update and the tick
	scheduleUpdate();
	schedule(schedule_selector(GameWorld::Tick), 1.0f);
}

void GameWorld::CreateBoundary()
{
	float offset = 0.025f;
	// calculate the position & dimension of the game's boundary
	boundary_rect_.origin.x = SCREEN_SIZE.width * offset;
	boundary_rect_.origin.y = SCREEN_SIZE.height * offset;
	boundary_rect_.size.width = SCREEN_SIZE.width - SCREEN_SIZE.width * offset * 2;
	boundary_rect_.size.height = SCREEN_SIZE.height - SCREEN_SIZE.height * offset * 4;

	// generate vertices for the boundary
	CCPoint vertices[4] = {CCPoint(boundary_rect_.origin.x, boundary_rect_.origin.y), 
		CCPoint(boundary_rect_.origin.x, boundary_rect_.origin.y + boundary_rect_.size.height), 
		CCPoint(boundary_rect_.origin.x + boundary_rect_.size.width, boundary_rect_.origin.y + boundary_rect_.size.height), 
		CCPoint(boundary_rect_.origin.x + boundary_rect_.size.width, boundary_rect_.origin.y)};

	// draw the boundary
	CCDrawNode* boundary = CCDrawNode::create();
	boundary->drawPolygon(vertices, 4, ccc4f(0, 0, 0, 0), 1, ccc4f(1, 1, 1, 0.3f));
	addChild(boundary, E_LAYER_FOREGROUND);
}

void GameWorld::CreatePlayer()
{
	// create & add the player at the center of the screen
	player_ = Player::create();
	player_->game_world_ = this;
	player_->setPosition(SCREEN_SIZE.width*0.5f, SCREEN_SIZE.height*0.5f);
	addChild(player_, E_LAYER_PLAYER);
}

void GameWorld::CreateContainers()
{
	// create & retain CCArray container lists
	enemies_ = CCArray::createWithCapacity(MAX_ENEMIES);
	enemies_->retain();
	powerups_ = CCArray::createWithCapacity(MAX_POWERUPS);
	powerups_->retain();
	blasts_ = CCArray::createWithCapacity(MAX_BLASTS);
	blasts_->retain();
	missiles_ = CCArray::createWithCapacity(MAX_MISSILES);
	missiles_->retain();
}

void GameWorld::CreateHUD()
{
	// create & add the score label
	score_label_ = CCLabelBMFont::create("Score: 0", "infont.fnt");
	score_label_->setAnchorPoint(CCPointZero);
	score_label_->setPosition(CCPoint(SCREEN_SIZE.width * 0.1f, boundary_rect_.getMaxY() + boundary_rect_.getMinY()));
	addChild(score_label_, E_LAYER_HUD);
}

void GameWorld::AddEnemyFormation()
{
	// fetch an enemy formation formation
	EEnemyFormation type = GetEnemyFormationType();
	// fetch a list of positions for the given formation
	vector<CCPoint> formation = GameGlobals::GetEnemyFormation(type, boundary_rect_, player_->getPosition());
	int num_enemies_to_create = formation.size();	
	int num_enemies_on_screen = enemies_->count();
	// limit the total number of enemies to MAX_ENEMIES
	if(num_enemies_on_screen + num_enemies_to_create >= MAX_ENEMIES)
	{
		num_enemies_to_create = MAX_ENEMIES - num_enemies_on_screen;
	}
	// create, add & position enemies based on the formation
	for(int i = 0; i < num_enemies_to_create; ++i)
	{
		Enemy* enemy = Enemy::create(this);
		enemy->setPosition(formation[i]);
		enemy->Spawn(i * ENEMY_SPAWN_DELAY);
		addChild(enemy, E_LAYER_ENEMIES);
		enemies_->addObject(enemy);
	}
}

EEnemyFormation GameWorld::GetEnemyFormationType()
{
	// return a formation type from a list of formation types, based on time user has been playing
	// the longer the user has survived, the more difficult the formations will be
	if(seconds_ > E_SKILL6)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill6_formations_size;
		return (EEnemyFormation)(GameGlobals::skill6_formations[random_index]);
	}
	else if(seconds_ > E_SKILL5)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill5_formations_size;
		return (EEnemyFormation)(GameGlobals::skill5_formations[random_index]);
	}
	else if(seconds_ > E_SKILL4)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill4_formations_size;
		return (EEnemyFormation)(GameGlobals::skill4_formations[random_index]);
	}
	else if(seconds_ > E_SKILL3)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill3_formations_size;
		return (EEnemyFormation)(GameGlobals::skill3_formations[random_index]);
	}
	else if(seconds_ > E_SKILL2)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill2_formations_size;
		return (EEnemyFormation)(GameGlobals::skill2_formations[random_index]);
	}
	else if(seconds_ > E_SKILL1)
	{
		int random_index = CCRANDOM_0_1() * GameGlobals::skill1_formations_size;
		return (EEnemyFormation)(GameGlobals::skill1_formations[random_index]);
	}
	else
	{
		return E_FORMATION_RANDOM_EASY;
	}
}

void GameWorld::AddPowerUp()
{
	// limit the number of power-ups on screen
	if(powerups_->count() >= MAX_POWERUPS)
		return;

	PowerUp* powerup = NULL;

	// randomly pick a type of power-up from the power-up frequency array
	int random_index = CCRANDOM_0_1() * GameGlobals::powerup_frequency_size;
	EPowerUpType powerup_type = (EPowerUpType)GameGlobals::powerup_frequency[random_index];

	// create the power-up
	switch(powerup_type)
	{
	case E_POWERUP_BOMB:
		powerup = Bomb::create(this);
		break;
	case E_POWERUP_MISSILE_LAUNCHER:
		powerup = MissileLauncher::create(this);
		break;
	case E_POWERUP_SHIELD:
		powerup = Shield::create(this);
		break;
	default:
		powerup = Bomb::create(this);
	}

	// position it within the boundary & add it
	powerup->setPosition(ccp(boundary_rect_.origin.x + CCRANDOM_0_1() * boundary_rect_.size.width, boundary_rect_.origin.y + CCRANDOM_0_1() * boundary_rect_.size.height));
	powerup->Spawn();
	addChild(powerup, E_LAYER_POWERUPS);
	powerups_->addObject(powerup);
}

void GameWorld::AddBlast(Blast* blast)
{
	// add Blast to screen & respective container
	addChild(blast, E_LAYER_BLASTS);
	blasts_->addObject(blast);
}

void GameWorld::AddMissile(Missile* missile)
{
	// add Missile to screen & respective container
	addChild(missile, E_LAYER_MISSILES);
	missiles_->addObject(missile);
}

void GameWorld::update(float dt)
{
	// don't process if player is dying
	if(player_->is_dying_)
		return;

	// update each enemy
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		Enemy* enemy = (Enemy*)object;
		if(enemy)
		{
			enemy->Update(player_->getPosition(), player_->GetShield() == NULL);
		}
	}

	// update each power-up
	object = NULL;
	CCARRAY_FOREACH(powerups_, object)
	{
		PowerUp* powerup = (PowerUp*)object;
		if(powerup)
		{
			powerup->Update();
		}
	}

	CheckCollisions();
	CheckRemovals();
}

void GameWorld::CheckCollisions()
{
	// save player position & radius
	CCPoint player_position = player_->getPosition();
	float player_radius = player_->getRadius();
	
	// iterate through all enemies
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		Enemy* enemy = (Enemy*)object;
		if(enemy)
		{
			CCPoint enemy_position = enemy->getPosition();

			// check with Player
			if(CIRCLE_INTERSECTS_CIRCLE(player_position, player_radius, enemy_position, ENEMY_RADIUS))
			{
				// if shield is enabled, kill enemy
				if(player_->GetShield())
				{
					enemy->Die();
					EnemyKilled();
				}
				// else kill player...but only if enemy has finished spawning
				else if(!enemy->getIsSpawning())
					player_->Die();
			}

			// check with all blasts
			CCObject* object2 = NULL;
			CCARRAY_FOREACH(blasts_, object2)
			{
				Blast* blast = (Blast*)object2;
				if(blast)
				{
					if(CIRCLE_INTERSECTS_CIRCLE(blast->getPosition(), blast->getRadius(), enemy_position, ENEMY_RADIUS*1.5f))
					{
						enemy->Die();
						EnemyKilled();
					}
				}
			}

			// check with all missiles
			object2 = NULL;
			CCARRAY_FOREACH(missiles_, object2)
			{
				Missile* missile = (Missile*)object2;
				if(missile)
				{
					if(CIRCLE_INTERSECTS_CIRCLE(missile->getPosition(), MISSILE_RADIUS, enemy_position, ENEMY_RADIUS*1.5f))
					{
						missile->Explode();
					}
				}
			}
		}
	}

	// check if player collides with any of the power-ups
	// activate the power-up if collision is found
	object = NULL;
	CCARRAY_FOREACH(powerups_, object)
	{
		PowerUp* powerup = (PowerUp*)object;
		if(powerup && !powerup->getIsActive())
		{
			if(CIRCLE_INTERSECTS_CIRCLE(player_position, player_radius, powerup->getPosition(), POWERUP_ICON_OUTER_RADIUS))
			{
				powerup->Activate();
			}
		}
	}
}

void GameWorld::CheckRemovals()
{
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		Enemy* enemy = (Enemy*)object;
		if(enemy && enemy->getMustBeRemoved())
		{
			enemies_->removeObject(enemy);
		}
	}

	object = NULL;
	CCARRAY_FOREACH(powerups_, object)
	{
		PowerUp* powerup = (PowerUp*)object;
		if(powerup && powerup->getMustBeRemoved())
		{
			powerups_->removeObject(powerup);
		}
	}

	object = NULL;
	CCARRAY_FOREACH(blasts_, object)
	{
		Blast* blast = (Blast*)object;
		if(blast && blast->getMustBeRemoved())
		{
			blasts_->removeObject(blast);
		}
	}

	object = NULL;
	CCARRAY_FOREACH(missiles_, object)
	{
		Missile* missile = (Missile*)object;
		if(missile && missile->getMustBeRemoved())
		{
			missiles_->removeObject(missile);
		}
	}
}

void GameWorld::Tick(float dt)
{
	// don't tick if player is dying
	if(player_->is_dying_)
		return;

	++ seconds_;

	-- combo_timer_;
	// show the combo achieved if time is up
	if(combo_timer_ < 0)
		combo_timer_ = 0;
	else if(combo_timer_ == 0)
		ComboTimeUp();

	// Tick each enemy
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		Enemy* enemy = (Enemy*)object;
		if(enemy)
		{
			enemy->Tick();
		}
	}

	// Tick each power-up
	object = NULL;
	CCARRAY_FOREACH(powerups_, object)
	{
		PowerUp* powerup = (PowerUp*)object;
		if(powerup)
		{
			powerup->Tick();
		}
	}

	// add an enemy formation every 5 seconds
	if(seconds_ % 5 == 0)
		AddEnemyFormation();
	// add a powerup formation every 4 seconds
	if(seconds_ % 4 == 0)
		AddPowerUp();
}

void GameWorld::EnemyKilled()
{
	// increment counters
	++ enemies_killed_total_;
	++ enemies_killed_combo_;
	// reset combo time
	combo_timer_ = COMBO_TIME;

	// add score & update the label
	score_ += 7;
	char buf[16] = {0};
	sprintf(buf, "Score: %d", score_);
	score_label_->setString(buf);
}

void GameWorld::ComboTimeUp()
{
	// combo is considered only more than 5 enemies were killed
	if(enemies_killed_combo_ < 5)
		return;

	// add combo to score and update the label
	score_ += enemies_killed_combo_ * 10;
	char buf[16] = {0};
	sprintf(buf, "Score: %d", score_);
	score_label_->setString(buf);

	// inform the user of the combo by adding a label
	sprintf(buf, "X%d", enemies_killed_combo_);
	CCLabelBMFont* combo_label = CCLabelBMFont::create(buf, "infont.fnt");
	combo_label->setPosition(player_->getPositionX(), player_->getPositionY() + combo_label->getContentSize().height);
	combo_label->setScale(0.6f);
	// animate it to move upwards then remove it
	combo_label->runAction(CCSequence::create(CCMoveBy::create(1.0f, ccp(0, 50)), CCDelayTime::create(0.5f), CCRemoveSelf::create(true), NULL));
	addChild(combo_label, E_LAYER_HUD);
	// reset combo kill counter
	enemies_killed_combo_ = 0;
}

void GameWorld::PauseGame()
{
	// this prevents multiple pause popups
	if(is_popup_active_)
		return;

	is_popup_active_ = true;

	// pause GameWorld update & Tick functions
	pauseSchedulerAndActions();
	
	CCArray* game_world_children = getChildren();
	CCObject* child = NULL;
	// pause update functions & actions on all GameWorld's children
	CCARRAY_FOREACH(game_world_children, child)
	{
		if(child)
		{
			((CCNode*)child)->pauseSchedulerAndActions();
		}
	}

	// create & add the pause popup
	PausePopup* pause_popup = PausePopup::create(this);
	addChild(pause_popup, E_LAYER_POPUP);
}

void GameWorld::ResumeGame()
{
	is_popup_active_ = false;

	// resume GameWorld update & Tick functions
	resumeSchedulerAndActions();
	
	CCArray* game_world_children = getChildren();
	CCObject* child = NULL;
	// resume update functions & actions on all GameWorld's children
	CCARRAY_FOREACH(game_world_children, child)
	{
		if(child)
		{
			((CCNode*)child)->resumeSchedulerAndActions();
		}
	}
}

void GameWorld::GameOver()
{
	is_popup_active_ = true;

	// stop GameWorld update & Tick functions
	unscheduleAllSelectors();

	CCArray* game_world_children = getChildren();
	CCObject* child = NULL;
	// stop update functions & actions on all GameWorld's children
	CCARRAY_FOREACH(game_world_children, child)
	{
		if(child)
		{
			((CCNode*)child)->unscheduleAllSelectors();
			((CCNode*)child)->stopAllActions();
		}
	}

	// display combos if any
	ComboTimeUp();
	// create & add the game over popup
	GameOverPopup* game_over_popup = GameOverPopup::create(this, score_, enemies_killed_total_, seconds_);
	addChild(game_over_popup, E_LAYER_POPUP);
	SOUND_ENGINE->playEffect("game_over.wav");
}

void GameWorld::didAccelerate(CCAcceleration* acceleration_value)
{
	HandleInput(ccp(acceleration_value->x, acceleration_value->y));
}

void GameWorld::ccTouchesBegan(CCSet* set, CCEvent* event)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	// pause the game if touch is outside the game's boundary
	if(!boundary_rect_.containsPoint(touch_point))
	{
		PauseGame();
		return;
	}

	// movement controls when running the game on win32
	CCPoint input = CCPointZero;
	input.x = (touch_point.x - SCREEN_SIZE.width * 0.5f) / (SCREEN_SIZE.width);
	input.y = (touch_point.y - SCREEN_SIZE.height * 0.5f) / (SCREEN_SIZE.height);
	HandleInput(input);
#else
	PauseGame();
#endif	
}

void GameWorld::ccTouchesMoved(CCSet* set, CCEvent* event)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	// movement controls when running the game on win32
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	CCPoint input = CCPointZero;
	input.x = (touch_point.x - SCREEN_SIZE.width * 0.5f) / (SCREEN_SIZE.width);
	input.y = (touch_point.y - SCREEN_SIZE.height * 0.5f) / (SCREEN_SIZE.height);
	HandleInput(input);
#endif
}

void GameWorld::ccTouchesEnded(CCSet* set, CCEvent* event)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	// stop the player when touch has ended
	HandleInput(CCPointZero);
#endif
}

void GameWorld::HandleInput(CCPoint input)
{
	/// don't accept input if popup is active or if player is dead
	if(is_popup_active_ || player_->is_dying_)
		return;

	CCPoint input_abs = CCPoint(fabs(input.x), fabs(input.y));

	// calculate player speed based on how much device has tilted
	// greater speed multipliers for greater tilt values
	player_->speed_.x = input.x * ( (input_abs.x > 0.3f) ? 36 : ( (input_abs.x > 0.2f) ? 28 : 20 ) );
	player_->speed_.y = input.y * ( (input_abs.y > 0.3f) ? 36 : ( (input_abs.y > 0.2f) ? 28 : 20 ) );

	// update the background
	background_->setPosition(ccp(input.x * -30, input.y * -30));
}
