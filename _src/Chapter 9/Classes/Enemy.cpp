/*
 * Enemy.cpp
 *
 *  Created on: Apr 11, 2015
 *      Author: Karan
 */

#include "Enemy.h"
#include "GameWorld.h"
#include "Tower.h"

Enemy::Enemy()
{
	game_world_ = NULL;
	type_ = 0;
	animation_name_ = NULL;
	health_ = 0;
	armor_ = 0;
	magic_resistance_ = 0;
	speed_ = 0.0f;
	damage_ = 0;
	reward_ = 0;
	radius_ = 0.0f;
	health_left_ = 0;
	has_died_ = false;
	is_slowed_ = false;
	num_walk_points_ = 0;
	curr_walk_point_ = 0;
	walk_points_.clear();
	health_bar_ = NULL;
}

Enemy::~Enemy()
{}

Enemy* Enemy::create(GameWorld* game_world, int type)
{
	Enemy* enemy = new Enemy();
	if(enemy && enemy->init(game_world, type))
	{
		enemy->autorelease();
		return enemy;
	}
	CC_SAFE_DELETE(enemy);
	return NULL;
}

bool Enemy::init(GameWorld* game_world, int type)
{
	if(!CCSprite::init())
	{
		return false;
	}
	// save reference to GameWorld & type of enemy
	game_world_ = game_world;
	type_ = type;
	// set the enemy's properties
	SetEnemyProperties();
	// fetch the first frame of animation so we know the size of this enemy
	CCAnimation* animation = CCAnimationCache::sharedAnimationCache()->animationByName(animation_name_);
	radius_ = ((CCAnimationFrame*)animation->getFrames()->objectAtIndex(0))->getSpriteFrame()->getOriginalSize().width/2;
	// hide the enemy till it starts walking
	setVisible(false);

	CreateHealthBar();

	return true;
}

void Enemy::SetEnemyProperties()
{
	// enemy properties are set from the EnemyData struct
	SetAnimationName(GameGlobals::enemy_data_[type_]->animation_name_);
	SetHealth(GameGlobals::enemy_data_[type_]->health_);
	SetArmor(GameGlobals::enemy_data_[type_]->armor_);
	SetMagicResistance(GameGlobals::enemy_data_[type_]->magic_resistance_);
	SetSpeed(GameGlobals::enemy_data_[type_]->speed_);
	SetDamage(GameGlobals::enemy_data_[type_]->damage_);
	SetReward(GameGlobals::enemy_data_[type_]->reward_);
	health_left_ = health_;
}

void Enemy::CreateHealthBar()
{
	CCPoint position = ccp(radius_, radius_ * 1.75f);
	// sprite behind the progress bar
	CCSprite* red_bar = CCSprite::create("red_bar.png");
	red_bar->setPosition(position);
	addChild(red_bar);
	// create a horizontal progress bar
	health_bar_ = CCProgressTimer::create(CCSprite::create("green_bar.png"));
	health_bar_->setType(kCCProgressTimerTypeBar);
	// progress bar takes values from 0 to 100
	health_bar_->setPercentage( (float)health_left_ / (float)health_ * 100 );
	health_bar_->setMidpoint(ccp(0, 1));
	health_bar_->setBarChangeRate(ccp(1, 0));
	health_bar_->setPosition(position);
	addChild(health_bar_);
}

void Enemy::StartWalking()
{
	// show the enemy when it starts walking
	setVisible(true);
	// position the enemy at the first walking point
	setPosition(walk_points_[curr_walk_point_]);
	// calculate duration in terms of time taken to walk a single tile
	float duration = speed_ * ENEMY_MOVE_DURATION * (ccpDistance(walk_points_[curr_walk_point_ + 1], walk_points_[curr_walk_point_]) / TILE_SIZE);

	// walk to the subsequent walk point
	CCActionInterval* walk = CCMoveTo::create(duration, walk_points_[curr_walk_point_ + 1]);
	CCActionInstant* finish_walking = CCCallFunc::create(this, callfunc_selector(Enemy::FinishWalking));
	CCActionInterval* walk_sequence = CCSequence::createWithTwoActions(walk, finish_walking);
	// create a speed action to control the walking speed
	CCAction* walk_action = CCSpeed::create(walk_sequence, 1.0f);
	walk_action->setTag(ENEMY_MOVE_ACTION_TAG);
	runAction(walk_action);

	if(getActionByTag(ENEMY_ANIMATE_ACTION_TAG) == NULL)
	{
		CCActionInterval* animation = CCAnimate::create(CCAnimationCache::sharedAnimationCache()->animationByName(animation_name_));
		animation->setTag(ENEMY_ANIMATE_ACTION_TAG);
		runAction(animation);
	}
}

void Enemy::FinishWalking()
{
	// can't stop walking if already dead
	if(has_died_)
	{
		return;
	}

	// move to the subsequent walk point
	++ curr_walk_point_;
	if(curr_walk_point_ < num_walk_points_ - 1)
	{
		StartWalking();
	}
	// enemy has reached the pumpkin
	else
	{
		DoDamage();
	}
}

void Enemy::DoDamage()
{
	// inform GameWorld that damage must be done
	game_world_->EnemyAtTheGates(this);

	stopAllActions();
	// hide the enemy
	setVisible(false);
}

void Enemy::TakeDamage(CCObject* object)
{
	// sometimes a dead enemy might get shot
	if(has_died_)
	{
		return;
	}

	Tower* tower = (Tower*)object;
	// calculate total damage taken by this enemy from a given tower
	float physical_damage = tower->GetPhysicalDamage() - armor_;
	float magical_damage = tower->GetMagicalDamage() - magic_resistance_;
	float total_damage = (physical_damage > 0 ? physical_damage : 0) + (magical_damage > 0 ? magical_damage : 0);
	health_left_ -= total_damage;
	
	// slow the enemy if not already being slowed & if the tower has speed damage
	if(is_slowed_ == false && tower->GetSpeedDamage() < 1.0f)
	{
		TakeSpeedDamage(tower->GetSpeedDamage(), tower->GetSpeedDamageDuration());		
	}
	
	// check if enemy should die
	if(health_left_ <= 0)
	{
		Die();
	}

	// update the health bar
	health_bar_->setPercentage( (float)health_left_ / (float)health_ * 100 );
}

void Enemy::Die()
{
	// inform GameWorld that an enemy has died
	has_died_ = true;
	game_world_->EnemyDown(this);

	stopAllActions();
	runAction(CCSequence::createWithTwoActions(CCEaseBackIn::create(CCScaleTo::create(0.2f, 0.0f)), CCHide::create()));
}

void Enemy::TakeSpeedDamage(float speed_damage, float speed_damage_duration)
{
	// reduce the walking speed
	is_slowed_ = true;
	CCSpeed* walk_action = (CCSpeed*)getActionByTag(ENEMY_MOVE_ACTION_TAG);
	if(walk_action != NULL)
	{
		walk_action->setSpeed(speed_damage);
		// walking speed must return back to normal after certain duration
		scheduleOnce(schedule_selector(Enemy::ResetSpeed), speed_damage_duration);
	}	
}

void Enemy::ResetSpeed(float dt)
{
	// walking speed must return back to normal after certain duration
	is_slowed_ = false;
	CCSpeed* walk_action = (CCSpeed*)getActionByTag(ENEMY_MOVE_ACTION_TAG);
	if(walk_action != NULL)
	{
		walk_action->setSpeed(1.0f);
	}
}