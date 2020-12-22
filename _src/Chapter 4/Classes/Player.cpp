#include "Player.h"
#include "GameWorld.h"

Player* Player::create()
{
	Player* player = new Player();
	if(player && player->init())
	{
		player->autorelease();
		return player;
	}
	CC_SAFE_DELETE(player);
	return NULL;
}

bool Player::init()
{
	if(!CCDrawNode::init())
		return false;

	// generate vertices for the player
	CCPoint vertices[] = {CCPoint(PLAYER_RADIUS * 1.75f, 0), CCPoint(PLAYER_RADIUS * -0.875f, PLAYER_RADIUS), CCPoint(PLAYER_RADIUS * -1.75, 0), CCPoint(PLAYER_RADIUS * -0.875f, PLAYER_RADIUS * -1)};
	// draw a green coloured player
	drawPolygon(vertices, 4, ccc4f(0, 0, 0, 0), 1.5f, ccc4f(0, 1, 0, 1));

	scheduleUpdate();
	return true;
}

void Player::update(float dt)
{
	CCDrawNode::update(dt);
	CCPoint previous_position = m_obPosition;
	UpdatePosition();
	UpdateRotation(previous_position);
}

void Player::UpdatePosition()
{
	// don't move if speed is too low
	if(ccpLength(speed_) > 0.75f)
	{
		// add speed but limit movement within the boundary
		CCPoint next_position = ccpAdd(m_obPosition, speed_);
		if(RECT_CONTAINS_CIRCLE(game_world_->boundary_rect_, next_position, PLAYER_RADIUS))
		{
			setPosition(next_position);
		}
		else
		{
			if(RECT_CONTAINS_CIRCLE(game_world_->boundary_rect_, CCPoint(next_position.x - speed_.x, next_position.y), PLAYER_RADIUS))
			{
				setPosition(ccp(next_position.x - speed_.x, next_position.y));
			}
			else if(RECT_CONTAINS_CIRCLE(game_world_->boundary_rect_, CCPoint(next_position.x, next_position.y - speed_.y), PLAYER_RADIUS))
			{
				setPosition(ccp(next_position.x, next_position.y - speed_.y));
			}
		}
	}
}

void Player::UpdateRotation(CCPoint previous_position)
{
	// don't rotate if speed is too low
	if(ccpLength(speed_) > 1.0f)
	{
		float previous_rotation = getRotation();
		// calculate target angle based on previous & current position
		float target_rotation = CC_RADIANS_TO_DEGREES(ccpToAngle(ccpSub(m_obPosition, previous_position)) * -1);
		
		// add some easing to the rotation
		float rotation_step = 0.0f;
		if(target_rotation > 90.0f && previous_rotation < -90.0f)
			rotation_step = (360.0f - fabs(target_rotation - previous_rotation)) / -4;
		else if(target_rotation < -90.0f && previous_rotation > 90.0f)
			rotation_step = (360.0f - fabs(target_rotation - previous_rotation)) / 4;
		else
			rotation_step = (target_rotation - previous_rotation) / 4;

		// limit rotation between -180 & 180 degrees
		float final_rotation = previous_rotation + rotation_step;
		if( final_rotation > 180.0f )
			final_rotation -= 360.0f;
		else if( final_rotation < -180.0f )
			final_rotation += 360.0f;

		setRotation(final_rotation);
	}
}

void Player::Die()
{
	// don't die if already dying
	if(is_dying_)
		return;

	is_dying_ = true;
	// stop moving
	speed_.x = 0;
	speed_.y = 0;

	float shake_duration = 0.5f;
	int num_shakes = 8;
	// create & animate the death and end the game afterwards
	CCActionInterval* shake = CCSpawn::createWithTwoActions(CCScaleTo::create(shake_duration, 1.2f), CCRepeat::create(CCSequence::createWithTwoActions(CCRotateBy::create(shake_duration/(num_shakes*2), -20.0), CCRotateBy::create(shake_duration/(num_shakes*2), 20.0)), num_shakes));
	CCActionInterval* shrink = CCEaseSineIn::create(CCScaleTo::create(0.1f, 0.0f));
	CCActionInterval* death = CCSequence::create(shake, shrink, NULL);

	runAction(CCSequence::createWithTwoActions(death, CCCallFunc::create(this, callfunc_selector(Player::Dead))));
	SOUND_ENGINE->playEffect("blast_player.wav");
}

void Player::Dead()
{
	game_world_->GameOver();
}

void Player::SetShield(Shield* shield)
{
	shield_ = shield;
	// with shield enabled, the collision radius must be increased to the size of the shield
	radius_ = (shield_) ? PLAYER_RADIUS * 3 : PLAYER_RADIUS;
}
