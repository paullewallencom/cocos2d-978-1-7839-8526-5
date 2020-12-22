#include "PowerUp.h"
#include "GameWorld.h"

bool PowerUp::init(GameWorld* instance)
{
	if(!CCDrawNode::init())
		return false;
	
	game_world_ = instance;
	// calculate how much time the power-up should wait on screen before activation
	time_left_ = MAX_POWERUP_WAIT_ON_SCREEN / 2 + CCRANDOM_0_1() * MAX_POWERUP_WAIT_ON_SCREEN / 2;
	// calculate speed
	speed_ = CCPoint(CCRANDOM_MINUS1_1() * 2, CCRANDOM_MINUS1_1() * 2);

	// draw the brown coloured ring
	drawDot(CCPointZero, POWERUP_ICON_OUTER_RADIUS, ccc4f(0.73725f, 0.5451f, 0, 1));
	drawDot(CCPointZero, POWERUP_ICON_OUTER_RADIUS - 3, ccc4f(0, 0, 0, 1));
	setScale(0.0f);

	return true;
}

void PowerUp::Update()
{
	// bounce within the boundary
	if(!RECT_CONTAINS_CIRCLE(game_world_->boundary_rect_, m_obPosition, POWERUP_ICON_OUTER_RADIUS))
	{
		// bounce off the left & right edge
		if( (m_obPosition.x - POWERUP_ICON_OUTER_RADIUS) < game_world_->boundary_rect_.origin.x ||
			(m_obPosition.x + POWERUP_ICON_OUTER_RADIUS) > (game_world_->boundary_rect_.origin.x + game_world_->boundary_rect_.size.width) )
			speed_.x *= -1;
		// bounce off the top & bottom edge
		if( (m_obPosition.y + POWERUP_ICON_OUTER_RADIUS) > (game_world_->boundary_rect_.origin.y + game_world_->boundary_rect_.size.height) ||
			(m_obPosition.y - POWERUP_ICON_OUTER_RADIUS) < game_world_->boundary_rect_.origin.y )
			speed_.y *= -1;
	}

	setPosition(m_obPosition.x + speed_.x, m_obPosition.y + speed_.y);
}

void PowerUp::Tick()
{
	-- time_left_;

	// remove this power-up in the next iteration when it's on-screen time is over
	if(time_left_ < 0)
	{
		must_be_removed_ = true;
		runAction(CCSequence::createWithTwoActions(CCEaseBackIn::create(CCScaleTo::create(0.25f, 0.0f)), CCRemoveSelf::create(true)));
	}
}

void PowerUp::Spawn()
{
	// animate the entry
	runAction(CCEaseElasticOut::create(CCScaleTo::create(1.0f, 1.0f)));
	runAction(CCRepeatForever::create(CCSequence::create(CCDelayTime::create(5 + CCRANDOM_0_1() * 5), CCEaseSineIn::create(CCScaleTo::create(0.1f, 1.2f)), CCEaseSineOut::create(CCScaleTo::create(0.1f, 1.0f)), CCEaseSineIn::create(CCScaleTo::create(0.1f, 1.2f)), CCEaseSineOut::create(CCScaleTo::create(0.1f, 1.0f)), NULL)));
}

void PowerUp::Activate()
{
	// clear the geometry and stop all actions
	// now the child classes can add their own behaviour
	is_active_ = true;
	clear();
	stopAllActions();
}

void PowerUp::Deactivate()
{
	// remove this power-up in the next iteration
	runAction(CCSequence::createWithTwoActions(CCDelayTime::create(0.01f), CCRemoveSelf::create(true)));
	must_be_removed_ = true;
}
