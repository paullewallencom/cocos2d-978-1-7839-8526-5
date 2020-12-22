#include "Tower.h"
#include "GameWorld.h"
#include "Enemy.h"
#include "Lightning.h"

Tower::Tower()
{
	game_world_ = NULL;
	type_ = 0;
	bullet_name_ = NULL;
	is_lightning_ = false;
	is_rotating_ = false;
	range_ = 0.0f;
	physical_damage_ = 0.0f;
	magical_damage_ = 0.0f;
	speed_damage_ = 0.0f;
	speed_damage_duration_ = 0.0f;
	fire_rate_ = 0.0f;
	cost_ = 0;
	current_level_ = 0;
	target_ = NULL;
	range_node_ = NULL;
	base_sprite_ = NULL;
}

Tower::~Tower()
{}

Tower* Tower::create(GameWorld* game_world, int type, CCPoint position)
{
	Tower* tower = new Tower();
	if(tower && tower->init(game_world, type, position))
	{
		tower->autorelease();
		return tower;
	}
	CC_SAFE_DELETE(tower);
	return NULL;
}

bool Tower::init(GameWorld* game_world, int type, CCPoint position)
{
	if(!CCSprite::init())
	{
		return false;
	}
	// save reference to GameWorld & type of tower
	game_world_ = game_world;
	type_ = type;
	// initialise scale and position
	setScale(0.0f);
	setPosition(position);
	// set the tower's properties
	SetTowerProperties();

	// animate the tower's spawning
	if(base_sprite_)
	{
		base_sprite_->runAction(CCEaseBackOut::create(CCScaleTo::create(0.2f, 1.0f)));
	}
	runAction(CCEaseBackOut::create(CCScaleTo::create(0.2f, 1.0f)));
	return true;
}

void Tower::SetTowerProperties()
{
	// tower properties are set from the TowerDataSet & TowerData structs
	SetBulletName(GameGlobals::tower_data_sets_[type_]->bullet_name_);
	SetIsLightning(GameGlobals::tower_data_sets_[type_]->is_lightning_);
	SetIsRotating(GameGlobals::tower_data_sets_[type_]->is_rotating_);
	SetSpriteName(GameGlobals::tower_data_sets_[type_]->tower_data_[current_level_]->sprite_name_);
	SetRange(GameGlobals::tower_data_sets_[type_]->tower_data_[current_level_]->range_);
	SetPhysicalDamage(GameGlobals::tower_data_sets_[type_]->tower_data_[current_level_]->physical_damage_);
	SetMagicalDamage(GameGlobals::tower_data_sets_[type_]->tower_data_[current_level_]->magical_damage_);
	SetSpeedDamage(GameGlobals::tower_data_sets_[type_]->tower_data_[current_level_]->speed_damage_);
	SetSpeedDamageDuration(GameGlobals::tower_data_sets_[type_]->tower_data_[current_level_]->speed_damage_duration_);
	SetFireRate(GameGlobals::tower_data_sets_[type_]->tower_data_[current_level_]->fire_rate_);
	SetCost(GameGlobals::tower_data_sets_[type_]->tower_data_[current_level_]->cost_);
}

void Tower::Update()
{
	CheckForEnemies();
	UpdateRotation();
}

void Tower::UpdateRotation()
{
	// rotation to be updated only for rotating towers and if there is an enemy to target
	if(is_rotating_ == false || target_ == NULL)
	{
		return;
	}

	// update rotation so tower is always facing enemy
	float angle = 180 + CC_RADIANS_TO_DEGREES(-1 * ccpToAngle(ccpSub(m_obPosition, target_->getPosition())));
	setRotation(angle);
}

void Tower::Upgrade()
{
	// are there any upgrades left?
	if(current_level_ >= NUM_TOWER_UPGRADES - 1)
	{
		return;
	}

	// increment upgrade level and reset tower properties
	++ current_level_;
	SetTowerProperties();
	// debit cash
	game_world_->UpdateCash(-cost_);

	// reset the range
	range_node_->removeFromParentAndCleanup(true);
	range_node_ = NULL;
	ShowRange();
}

void Tower::Sell()
{
	// animate the tower's removal
	if(base_sprite_)
	{
		base_sprite_->runAction(CCSequence::createWithTwoActions(CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.0f)), CCRemoveSelf::create(true)));
	}
	runAction(CCSequence::createWithTwoActions(CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.0f)), CCRemoveSelf::create(true)));
}

void Tower::CheckForEnemies()
{
	// only check the current wave for enemies
	Wave* curr_wave = game_world_->GetCurrentWave();
	if(curr_wave == NULL)
	{
		return;
	}

	// search for a target only when there isn't one already
	if(target_ == NULL)
	{
		// loop through each enemy in the current wave
		for(int i = 0; i < curr_wave->num_enemies_; ++i)
		{
			Enemy* curr_enemy = curr_wave->enemies_[i];
			// save this enemy as a target it if it still alive and if it is within range
			if(curr_enemy->GetHasDied() == false && ccpDistance(m_obPosition, curr_enemy->getPosition()) <= (range_ + curr_enemy->GetRadius()))
			{
				SetTarget(curr_enemy);
				break;
			}
		}
	}
	
	// check if a target should still be considered
	if(target_ != NULL)
	{
		// a target is still valid if it is alive and if it is within range
		if(target_->GetHasDied() == true || ccpDistance(m_obPosition, target_->getPosition()) > (range_ + target_->GetRadius()))
		{
			SetTarget(NULL);
		}
	}
}

void Tower::SetTarget(Enemy* enemy)
{
	target_ = enemy;

	if(target_ != NULL)
	{
		// shoot as soon as you get a target
		Shoot(0.0f);
		schedule(schedule_selector(Tower::Shoot), fire_rate_);
	}
	else
	{
		// stop shooting when you lose a target
		unschedule(schedule_selector(Tower::Shoot));
	}
}

void Tower::Shoot(float dt)
{
	// don't do anything for a NULL target
	if(target_ == NULL)
	{
		return;
	}
	
	// shoot lightning or a bullet?
	if(is_lightning_)
	{
		ShootLightning();
	}
	else
	{
		ShootBullet();
	}
}

void Tower::ShootBullet()
{
	float bullet_move_duration = ccpDistance(m_obPosition, target_->getPosition()) / TILE_SIZE * BULLET_MOVE_DURATION;

	// damage the enemy
	CCActionInterval* damage_enemy = CCSequence::createWithTwoActions(CCDelayTime::create(bullet_move_duration), CCCallFuncO::create(target_, callfuncO_selector(Enemy::TakeDamage), this));
	target_->runAction(damage_enemy);

	// create the bullet
	CCSprite* bullet = CCSprite::create(bullet_name_);
	bullet->setScale(0.0f);
	bullet->setPosition(m_obPosition);
	game_world_->addChild(bullet, E_LAYER_TOWER - 1);

	// animate the bullet
	CCActionInterval* scale_up = CCScaleTo::create(0.05f, 1.0f);
	bullet->runAction(scale_up);
	
	// move the bullet then remove it
	CCActionInterval* move = CCSequence::create(CCMoveTo::create(bullet_move_duration, target_->getPosition()),
		CCRemoveSelf::create(true), NULL);
	bullet->runAction(move);
}

void Tower::ShootLightning()
{
	// damage the enemy
	CCActionInterval* damage_enemy = CCSequence::createWithTwoActions(CCDelayTime::create(LIGHTNING_DURATION * 0.5f), CCCallFuncO::create(target_, callfuncO_selector(Enemy::TakeDamage), this));
	target_->runAction(damage_enemy);

	// create the lightning without animation
	Lightning* lightning = Lightning::create(m_obPosition, target_->getPosition(), ccc4f(0.1098f, 0.87059f, 0.92157f, 1.0f), false);
	game_world_->addChild(lightning, E_LAYER_TOWER - 1);

	// animate the lightning
	CCActionInterval* shake = CCSequence::create(CCMoveTo::create(0.01f, ccp(3, 0)), CCMoveTo::create(0.01f, ccp(-3, 0)), CCMoveTo::create(0.01f, ccp(0, 3)), CCMoveTo::create(0.01f, ccp(0, -3)), NULL);
	lightning->runAction(CCRepeat::create(shake, 5));

	// remove the lightning
	CCActionInterval* wait_remove = CCSequence::createWithTwoActions(CCDelayTime::create(LIGHTNING_DURATION), CCRemoveSelf::create(true));
	lightning->runAction(wait_remove);
}

void Tower::ShowRange()
{
	if(range_node_ == NULL)
	{
		CreateRangeNode();
	}

	// show the range node then hide it a few seconds later
	range_node_->setScale(0.0f);
	range_node_->runAction(CCSequence::create(CCShow::create(), CCEaseBackOut::create(CCScaleTo::create(0.2f, 1.0f)), CCDelayTime::create(3.0f), CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.0f)), CCHide::create(), NULL));
}

void Tower::CreateRangeNode()
{
	int num_vertices = 30;
	CCPoint vertices[30];
	float theta = 0.0f;
	// generate vertices for the circle
	for(int i = 0; i < num_vertices; ++i)
	{
		vertices[i] = ccp(range_ * cosf(theta), range_ * sinf(theta));
		theta += 2 * M_PI / num_vertices;
	}

	// draw a semi transparent green circle with a border
	range_node_ = CCDrawNode::create();
	range_node_->drawPolygon(vertices, num_vertices, ccc4f(0.0f, 1.0f, 0.0f, 0.15f), 2, ccc4f(0.0f, 1.0f, 0.0f, 0.25f));
	range_node_->setVisible(false);
	range_node_->setPosition(m_obContentSize.width/2, m_obContentSize.height/2);
	addChild(range_node_);
}

void Tower::SetIsRotating(bool is_rotating)
{
	is_rotating_ = is_rotating;
	// a base must be added separately for a tower that rotates
	if(is_rotating_ && base_sprite_ == NULL)
	{
		base_sprite_ = CCSprite::create("TD_tbase.png");
		base_sprite_->setPosition(m_obPosition);
		base_sprite_->setScale(0.0f);
		game_world_->addChild(base_sprite_, E_LAYER_TOWER-1);
	}
}

void Tower::SetSpriteName(const char* sprite_name)
{
	// init this tower's sprite
	initWithFile(sprite_name);
}
