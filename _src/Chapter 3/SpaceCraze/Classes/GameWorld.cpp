#include "GameWorld.h"
#include "BackgroundManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Brick.h"
#include "Popups.h"

GameWorld::GameWorld()
{
	sprite_batch_node_ = NULL;
	player_ = NULL;
	player_bullets_ = NULL;
	player_fire_rate_ = 0.0f;
	enemies_ = NULL;
	enemy_bullets_ = NULL;
	enemy_movement_duration_ = 0.0f;
	enemy_fire_rate_ = 0.0f;
	left_side_enemy_position_ = 0.0f;
	right_side_enemy_position_ = 0.0f;
	max_enemy_size_ = CCSizeZero;
	bricks_ = NULL;
	score_label_ = NULL;
	life_sprites_ = NULL;
	score_ = 0;
	current_level_ = 0;
	lives_to_carry_ = 3;
	score_to_carry_ = 0;
	has_game_started_ = false;
	has_game_stopped_ = false;
	is_popup_active_ = false;
}

GameWorld::~GameWorld()
{}

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

bool GameWorld::init()
{
    if ( !CCLayer::init() )
    {
        return false;
    }
    
	setTouchEnabled(true);

	// game always starts with first level
	current_level_ = 1;
	// initially lives will be 3 & score will be 0
	lives_to_carry_ = 3;
	score_to_carry_ = 0;
	CreateLevel();

    return true;
}

void GameWorld::update(float dt)
{
	// no collision checking if game has not started OR has stopped OR is paused
	if(!has_game_started_ || has_game_stopped_ || is_game_paused_)
		return;

	CheckCollisions();
}

void GameWorld::ccTouchesBegan(CCSet* set, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);
	HandleTouch(touch_point);
}

void GameWorld::ccTouchesMoved(CCSet* set, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);
	HandleTouch(touch_point);
}

void GameWorld::CreateLevel()
{
	// create the environment
	BackgroundManager* background_manager = BackgroundManager::create();
	addChild(background_manager, E_LAYER_BACKGROUND);

	// create & add the batch node
	sprite_batch_node_ = CCSpriteBatchNode::create("spacetex.png", 128);
	addChild(sprite_batch_node_);

	// initialize score & state machine flags
	score_ = score_to_carry_;
	has_game_started_ = false;
	has_game_stopped_ = false;
	is_game_paused_ = false;
	// initialize enemy position variables
	left_side_enemy_position_ = SCREEN_SIZE.width/2;
	right_side_enemy_position_ = SCREEN_SIZE.width/2;

	// generate level filename
	char level_file[16] = {0};
	sprintf(level_file, "Level%02d.xml", current_level_);

	// fetch level file data
	unsigned long size;
	char* data = (char*)CCFileUtils::sharedFileUtils()->getFileData(level_file, "rb", &size);

	// parse the level file
	tinyxml2::XMLDocument xml_document;
	tinyxml2::XMLError xml_result = xml_document.Parse(data, size);

	CC_SAFE_DELETE(data);

	// print the error if parsing was unsuccessful
	if(xml_result != tinyxml2::XML_SUCCESS)
	{
		CCLOGERROR("Error:%d while reading %s", xml_result, level_file);
		return;
	}

	// save player data
	tinyxml2::XMLNode* level_node = xml_document.FirstChild();
	player_fire_rate_ = level_node->ToElement()->FloatAttribute("player_fire_rate");

	// create set of enemies
	tinyxml2::XMLNode* enemy_set_node = level_node->FirstChild(); 
	CreateEnemies(enemy_set_node);

	// create set of bricks
	tinyxml2::XMLNode* brick_set_node = enemy_set_node->NextSibling(); 
	CreateBricks(brick_set_node);

	CreatePlayer();

	CreateHUD();

	// everything created, start updating
	scheduleUpdate();
}

void GameWorld::CreateEnemies(tinyxml2::XMLNode* enemy_set)
{
	// save enemy movement & firing information
	enemy_movement_duration_ = enemy_set->ToElement()->FloatAttribute("move_duration");
	enemy_fire_rate_ = enemy_set->ToElement()->FloatAttribute("fire_rate");
	
	// create array to hold enemies
	enemies_ = CCArray::create();
	enemies_->retain();

	// create array to hold enemy bullers
	enemy_bullets_ = CCArray::createWithCapacity(MAX_BULLETS);
	enemy_bullets_->retain();

	// iterate through <EnemySet> and create Enemy objects
	tinyxml2::XMLElement* enemy_element = NULL;
	for(tinyxml2::XMLNode* enemy_node = enemy_set->FirstChild(); enemy_node != NULL; enemy_node = enemy_node->NextSibling())
	{
		enemy_element = enemy_node->ToElement();
		// Enemy sprite frame name taken from "name" attribute of <Enemy>
		Enemy* enemy = Enemy::createWithSpriteFrameName(this, enemy_element->Attribute("name"));
		// Enemy score taken from "score" attribute of <Enemy>
		enemy->setScore(enemy_element->IntAttribute("score"));
		// add Enemy to batch node & array
		sprite_batch_node_->addChild(enemy, E_LAYER_ENEMIES_BRICKS);
		enemies_->addObject(enemy);

		// Enemy position taken from "position" attribute of <Enemy>
		CCPoint position = GameGlobals::GetPointFromString(string(enemy_element->Attribute("position")));
		enemy->setPosition(position);

		// save enemies at the left & right extremes
		left_side_enemy_position_ = (position.x < left_side_enemy_position_) ? position.x : left_side_enemy_position_;
		right_side_enemy_position_ = (position.x > right_side_enemy_position_) ? position.x : right_side_enemy_position_;

		// save size of largest enemy
		CCSize size = enemy->getContentSize();
		max_enemy_size_.width = (size.width > max_enemy_size_.width) ? size.width:max_enemy_size_.width;
		max_enemy_size_.height = (size.height > max_enemy_size_.height) ? size.height:max_enemy_size_.height;
	}
}

void GameWorld::CreateBricks(tinyxml2::XMLNode* brick_set)
{
	// create array to hold bricks
	bricks_ = CCArray::create();
	bricks_->retain();

	// iterate through <BrickSet> and create Brick objects
	tinyxml2::XMLElement* brick_element = NULL;
	for(tinyxml2::XMLNode* brick_node = brick_set->FirstChild(); brick_node != NULL; brick_node = brick_node->NextSibling())
	{
		brick_element = brick_node->ToElement();
		// Brick sprite frame name taken from "name" attribute of <Brick>
		Brick* brick = Brick::createWithSpriteFrameName(brick_element->Attribute("name"));
		// Brick score taken from "score" attribute of <Brick>
		brick->setScore(brick_element->IntAttribute("score"));
		// Brick position taken from "position" attribute of <Brick>
		brick->setPosition(GameGlobals::GetPointFromString(string(brick_element->Attribute("position"))));
		// add Brick to batch node & array
		sprite_batch_node_->addChild(brick, E_LAYER_ENEMIES_BRICKS);
		bricks_->addObject(brick);
	}
}

void GameWorld::CreatePlayer()
{
	// create & add Player to batch node
	player_ = Player::createWithSpriteFrameName(this, "sfgun");
	sprite_batch_node_->addChild(player_, E_LAYER_PLAYER);

	// create array to hold Player bullets
	player_bullets_ = CCArray::createWithCapacity(MAX_BULLETS);
	player_bullets_->retain();

	// initialize Player properties
	player_->setLives(lives_to_carry_);
	player_->setIsRespawning(false);
	// tell Player to move into the screen
	player_->Enter();
}

void GameWorld::CreateHUD()
{
	// create & add "score" text
	CCSprite* score_sprite = CCSprite::createWithSpriteFrameName("sfscore");
	score_sprite->setPosition(ccp(SCREEN_SIZE.width*0.15f, SCREEN_SIZE.height*0.925f));
	sprite_batch_node_->addChild(score_sprite, E_LAYER_HUD);

	// create & add "lives" text
	CCSprite* lives_sprite = CCSprite::createWithSpriteFrameName("sflives");
	lives_sprite->setPosition(ccp(SCREEN_SIZE.width*0.7f, SCREEN_SIZE.height*0.925f));
	sprite_batch_node_->addChild(lives_sprite, E_LAYER_HUD);

	// create & add score label
	char buf[8] = {0};
	sprintf(buf, "%04d", score_);
	score_label_ = CCLabelBMFont::create(buf, "sftext.fnt");
	score_label_->setPosition(ccp(SCREEN_SIZE.width*0.3f, SCREEN_SIZE.height*0.925f));
	addChild(score_label_, E_LAYER_HUD);

	// save size of life sprite
	CCSize icon_size = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("sflifei")->getOriginalSize();
	// create array to hold life sprites
	life_sprites_ = CCArray::createWithCapacity(player_->getLives());
	life_sprites_->retain();

	// position life sprites some distance away from "life" text
	float offset_x = lives_sprite->getPositionX() + lives_sprite->getContentSize().width*1.5f + icon_size.width;
	for(int i = 0; i < player_->getLives(); ++i)
	{
		// position each life sprite further away from "life" text
		offset_x -= icon_size.width * 1.5f;
		CCSprite* icon_sprite = CCSprite::createWithSpriteFrameName("sflifei");
		icon_sprite->setPosition(ccp( offset_x, SCREEN_SIZE.height*0.925f));
		// add life sprite to batch node & array
		sprite_batch_node_->addChild(icon_sprite, E_LAYER_HUD);
		life_sprites_->addObject(icon_sprite);
	}

	// create & add the pause menu containing pause button
	CCMenuItemSprite* pause_button = CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName("sfpause"), CCSprite::createWithSpriteFrameName("sfpause"), this, menu_selector(GameWorld::OnPauseClicked));
	pause_button->setPosition(ccp(SCREEN_SIZE.width*0.95f, SCREEN_SIZE.height*0.925f));
	CCMenu* menu = CCMenu::create(pause_button, NULL);
	menu->setAnchorPoint(CCPointZero);
	menu->setPosition(CCPointZero);
	addChild(menu, E_LAYER_HUD);
}

void GameWorld::StartGame()
{
	// call this function only once when the game starts
	if(has_game_started_)
		return;

	has_game_started_ = true;
	// start firing player & enemy bullets
	schedule(schedule_selector(GameWorld::FirePlayerBullet), player_fire_rate_);
	schedule(schedule_selector(GameWorld::FireEnemyBullet), enemy_fire_rate_);
	// start moving enemies
	StartMovingEnemies();
}

void GameWorld::StopGame()
{
	has_game_stopped_ = true;
	// stop firing player & enemy bullets
	unschedule(schedule_selector(GameWorld::FirePlayerBullet));
	unschedule(schedule_selector(GameWorld::FireEnemyBullet));

	// stop Enemy movement
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		CCSprite* enemy = (CCSprite*)object;
		if(enemy)
		{
			enemy->stopAllActions();
		}
	}
}

void GameWorld::ResumeGame()
{
	is_game_paused_ = false;
	is_popup_active_ = false;

	// resume player & enemy bullet firing
	resumeSchedulerAndActions();

	// resume Enemy movement
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		CCSprite* enemy = (CCSprite*)object;
		if(enemy)
		{
			enemy->resumeSchedulerAndActions();
		}
	}

	// resume Player bullet movement
	object = NULL;
	CCARRAY_FOREACH(player_bullets_, object)
	{
		CCSprite* player_bullet = (CCSprite*)object;
		if(player_bullet)
		{
			player_bullet->resumeSchedulerAndActions();
		}
	}

	// resume Enemy bullet movement
	object = NULL;
	CCARRAY_FOREACH(enemy_bullets_, object)
	{
		CCSprite* enemy_bullet = (CCSprite*)object;
		if(enemy_bullet)
		{
			enemy_bullet->resumeSchedulerAndActions();
		}
	}
}

void GameWorld::StartMovingEnemies()
{
	// compute maximum distance movable on both sides
	float max_distance_left = left_side_enemy_position_;
	float max_distance_right = SCREEN_SIZE.width - right_side_enemy_position_;
	// compute how much distance to cover per step
	float distance_per_move = max_enemy_size_.width*0.5;
	
	// calculate how many steps on both sides
	int max_moves_left = floor(max_distance_left / distance_per_move);
	int max_moves_right = floor(max_distance_right / distance_per_move);
	int moves_between_left_right = floor( (right_side_enemy_position_ - left_side_enemy_position_) / distance_per_move );

	CCActionInterval* move_left = CCSequence::createWithTwoActions(CCDelayTime::create(enemy_movement_duration_), CCEaseSineOut::create(CCMoveBy::create(0.25f, ccp(distance_per_move*-1, 0))));
	CCActionInterval* move_right = CCSequence::createWithTwoActions(CCDelayTime::create(enemy_movement_duration_), CCEaseSineOut::create(CCMoveBy::create(0.25f, ccp(distance_per_move, 0))));
	CCActionInterval* move_start_to_left = CCRepeat::create(move_left, max_moves_left);
	CCActionInterval* move_left_to_start = CCRepeat::create(move_right, max_moves_left);
	CCActionInterval* move_start_to_right = CCRepeat::create(move_right, max_moves_right);
	CCActionInterval* move_right_to_start = CCRepeat::create(move_left, max_moves_right);
	CCActionInterval* movement_sequence = CCSequence::create(move_start_to_left, move_left_to_start, move_start_to_right, move_right_to_start, NULL);

	// Move each Enemy
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		CCSprite* enemy = (CCSprite*)object;
		if(enemy)
		{
			enemy->runAction(CCRepeatForever::create( (CCActionInterval*) movement_sequence->copy() ));
		}
	}
}

void GameWorld::CheckCollisions()
{
	CCObject* object = NULL;
	CCSprite* bullet = NULL;
	bool found_collision = false;

	// collisions between player bullets and bricks & enemies
	CCARRAY_FOREACH(player_bullets_, object)
	{
		bullet = (CCSprite*)object;
		if(bullet)
		{
			CCRect bullet_aabb = bullet->boundingBox();
			
			object = NULL;
			CCARRAY_FOREACH(bricks_, object)
			{
				CCSprite* brick = (CCSprite*)object;
				// rectangular collision detection between player bullet & brick
				if(brick && bullet_aabb.intersectsRect(brick->boundingBox()))
				{
					// on collision, remove brick & player bullet
					RemoveBrick(brick);
					RemovePlayerBullet(bullet);
					found_collision = true;
					break;
				}
			}

			// found collision so stop checking
			if(found_collision)
				break;

			object = NULL;
			CCARRAY_FOREACH(enemies_, object)
			{
				CCSprite* enemy = (CCSprite*)object;
				// rectangular collision detection between player bullet & enemy
				if(enemy && bullet_aabb.intersectsRect(enemy->boundingBox()))
				{
					// on collision, remove enemy & player bullet
					RemoveEnemy(enemy);
					RemovePlayerBullet(bullet);
					found_collision = true;
					break;
				}
			}

			// found collision so stop checking
			if(found_collision)
				break;
		}
	}

	// no collision checking with player when player is respawning
	if(player_->getIsRespawning())
		return;

	// reset variables
	object = NULL;
	bullet = NULL;

	// collisions between enemy bullets and player
	CCRect player_aabb = player_->boundingBox();
	CCARRAY_FOREACH(enemy_bullets_, object)
	{
		bullet = (CCSprite*)object;
		if(bullet)
		{
			// rectangular collision detection between player & enemy bullet
			if(player_aabb.intersectsRect(bullet->boundingBox()))
			{
				// tell player to die, reduce lives & remove enemy
				player_->Die();
				ReduceLives();
				RemoveEnemyBullet(bullet);
				break;
			}
		}
	}
}

void GameWorld::HandleTouch(CCPoint touch)
{
	// don't take touch when a popup is active & when player is respawning
	if(is_popup_active_ || player_->getIsRespawning())
		return;

	player_->setPositionX(touch.x);
}

void GameWorld::OnPauseClicked(CCObject* sender)
{
	// this prevents multiple pause popups
	if(is_game_paused_ || is_popup_active_)
		return;

	// pause player & enemy bullet firing
	is_game_paused_ = true;
	pauseSchedulerAndActions();

	// pause Enemy movement
	CCObject* object = NULL;
	CCARRAY_FOREACH(enemies_, object)
	{
		CCSprite* enemy = (CCSprite*)object;
		if(enemy)
		{
			enemy->pauseSchedulerAndActions();
		}
	}

	// pause Player bullet movement
	object = NULL;
	CCARRAY_FOREACH(player_bullets_, object)
	{
		CCSprite* player_bullet = (CCSprite*)object;
		if(player_bullet)
		{
			player_bullet->pauseSchedulerAndActions();
		}
	}

	// pause Enemy bullet movement
	object = NULL;
	CCARRAY_FOREACH(enemy_bullets_, object)
	{
		CCSprite* enemy_bullet = (CCSprite*)object;
		if(enemy_bullet)
		{
			enemy_bullet->pauseSchedulerAndActions();
		}
	}

	// create & add the pause popup
	PausePopup* pause_popup = PausePopup::create(this);
	addChild(pause_popup, E_LAYER_POPUP);
}

void GameWorld::GameOver()
{
	// stop game & reset level variables
	StopGame();
	current_level_ = 1;
	lives_to_carry_ = 3;
	score_to_carry_ = 0;
	// create & add the game over popup
	GameOverPopup* game_over_popup = GameOverPopup::create(this, score_);
	addChild(game_over_popup, E_LAYER_POPUP);
	SOUND_ENGINE->playEffect("game_over.wav");
}

void GameWorld::LevelComplete()
{
	// tell player to leave screen
	player_->Leave();
	// stop game & update level variables
	StopGame();
	lives_to_carry_ = player_->getLives();
	score_to_carry_ = score_;
	// create & add the level complete popup
	LevelCompletePopup* level_complete_popup = LevelCompletePopup::create(this, score_, player_->getLives());
	addChild(level_complete_popup, E_LAYER_POPUP);
	SOUND_ENGINE->playEffect("level_complete.wav");
}

void GameWorld::Reset()
{
	// remove everything here
	player_->removeFromParentAndCleanup(true);
	player_ = NULL;

	player_bullets_->removeAllObjects();
	CC_SAFE_RELEASE_NULL(player_bullets_);

	enemies_->removeAllObjects();
	CC_SAFE_RELEASE_NULL(enemies_);

	enemy_bullets_->removeAllObjects();
	CC_SAFE_RELEASE_NULL(enemy_bullets_);

	bricks_->removeAllObjects();
	CC_SAFE_RELEASE_NULL(bricks_);

	score_label_->removeFromParentAndCleanup(true);
	score_label_ = NULL;

	life_sprites_->removeAllObjects();
	CC_SAFE_RELEASE_NULL(life_sprites_);

	sprite_batch_node_->removeFromParentAndCleanup(true);
	sprite_batch_node_ = NULL;

	is_popup_active_ = false;
	// remove all schedulers & stop all actions
	cleanup();
}

void GameWorld::RestartLevel()
{
	// called from class Popup
	// reset level variables
	current_level_ = 1;
	lives_to_carry_ = 3;
	score_to_carry_ = 0;
	// remove everything
	Reset();
	// create a new level
	CreateLevel();
}

void GameWorld::NextLevel()
{
	// called from class Popup
	// increment level number
	current_level_ = (current_level_ < MAX_LEVELS) ? ++ current_level_ : current_level_;
	// remove everything
	Reset();
	// create a new level
	CreateLevel();
}

void GameWorld::RemoveEnemy(CCSprite* enemy)
{
	// remove Enemy from array
	enemies_->removeObject(enemy);
	// tell Enemy to die & credit score
	AddScore(((Enemy*)enemy)->Die());

	// if all enemies are dead, level is complete
	if(enemies_->count() <= 0)
		LevelComplete();
}

void GameWorld::RemoveBrick(CCSprite* brick)
{
	// remove Brick from array
	bricks_->removeObject(brick);
	// tell Brick to crumble & credit score
	AddScore(((Brick*)brick)->Crumble());
}

void GameWorld::AddScore(int score_to_add)
{
	// add score & update HUD
	score_ += score_to_add;

	char buf[8] = {0};
	sprintf(buf, "%04d", score_);
	score_label_->setString(buf);
	score_label_->runAction(CCSequence::createWithTwoActions(CCEaseSineIn::create(CCScaleTo::create(0.125f, 1.2f)), CCEaseSineOut::create(CCScaleTo::create(0.125f, 1.0f))));
}

void GameWorld::ReduceLives()
{
	if(life_sprites_->count() <= 0)
		return;

	// remove the last life sprite from array
	CCSprite* life_sprite = (CCSprite*)life_sprites_->lastObject();
	life_sprites_->removeObject(life_sprite);

	// animate exit of life sprite
	CCActionInterval* scale_down = CCEaseBackIn::create(CCScaleTo::create(0.25f, 0.0f));
	CCActionInstant* remove_self = CCRemoveSelf::create(true);
	life_sprite->runAction(CCSequence::createWithTwoActions(scale_down, remove_self));
}

void GameWorld::FirePlayerBullet(float dt)
{
	// position the bullet slightly above Player
	CCPoint bullet_position = ccpAdd(player_->getPosition(), ccp(0, player_->getContentSize().height * 0.3));

	// create & add the bullet sprite
	CCSprite* bullet = CCSprite::createWithSpriteFrameName("sfbullet");
	sprite_batch_node_->addChild(bullet, E_LAYER_BULLETS);
	player_bullets_->addObject(bullet);

	// initialize position & scale
	bullet->setPosition(bullet_position);
	bullet->setScale(0.5f);

	// animate the bullet's entry
	CCScaleTo* scale_up = CCScaleTo::create(0.25f, 1.0f);
	bullet->runAction(scale_up);

	// move the bullet up
	CCMoveTo* move_up = CCMoveTo::create(BULLET_MOVE_DURATION, ccp(bullet_position.x, SCREEN_SIZE.height));
	CCCallFuncN* remove = CCCallFuncN::create(this, callfuncN_selector(GameWorld::RemovePlayerBullet));
	bullet->runAction(CCSequence::createWithTwoActions(move_up, remove));

	SOUND_ENGINE->playEffect("shoot_player.wav");
}

void GameWorld::FireEnemyBullet(float dt)
{
	if(enemies_->count() <= 0)
		return;

	// randomly pick an Enemy & position the bullet slightly below the Enemy
	Enemy* enemy = (Enemy*)enemies_->randomObject();
	CCPoint bullet_position = ccpSub(enemy->getPosition(), ccp(0, enemy->getContentSize().height * 0.3));
	
	// create & add the bullet sprite for this Enemy
	CCSprite* bullet = CCSprite::createWithSpriteFrameName(enemy->getBulletName());
	sprite_batch_node_->addChild(bullet, E_LAYER_BULLETS);
	enemy_bullets_->addObject(bullet);

	// initialize position & scale
	bullet->setPosition(bullet_position);
	bullet->setScale(0.5f);

	// animate the bullet's entry
	CCScaleTo* scale_up = CCScaleTo::create(0.25f, 1.0f);
	bullet->runAction(scale_up);

	// move the bullet up
	CCMoveTo* move_down = CCMoveTo::create(enemy->getBulletDuration() * (bullet_position.y / SCREEN_SIZE.height), ccp(bullet_position.x, 0));
	CCCallFuncN* remove = CCCallFuncN::create(this, callfuncN_selector(GameWorld::RemoveEnemyBullet));
	bullet->runAction(CCSequence::createWithTwoActions(move_down, remove));

	SOUND_ENGINE->playEffect("shoot_enemy.wav");
}

void GameWorld::RemovePlayerBullet(CCNode* bullet)
{
	// remove bullet from list & GameWorld
	player_bullets_->removeObject(bullet);
	bullet->removeFromParentAndCleanup(true);
}

void GameWorld::RemoveEnemyBullet(CCNode* bullet)
{
	// remove bullet from list & GameWorld
	enemy_bullets_->removeObject(bullet);
	bullet->removeFromParentAndCleanup(true);
}
