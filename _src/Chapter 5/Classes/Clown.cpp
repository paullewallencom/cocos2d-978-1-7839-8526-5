#include "Clown.h"
#include "GameWorld.h"

Clown* Clown::create(GameWorld* game_world)
{
	Clown* clown = new Clown();
	if(clown && clown->initWithSpriteFrameName("cjoker01.png"))
	{
		clown->game_world_ = game_world;
		clown->autorelease();
		return clown;
	}
	CC_SAFE_DELETE(clown);
	return NULL;
}

Clown::Clown()
{
	state_ = E_CLOWN_NONE;
	type_ = E_GAME_OBJECT_CLOWN;
	highest_position_ = 0.0f;
	rocket_trail_ = NULL;
}

Clown::~Clown()
{}

void Clown::Update()
{
	// call parent class' update
	GameObject::Update();
	// store the clown's highest position yet
	highest_position_ = m_obPosition.y > highest_position_ ? m_obPosition.y : highest_position_;

	// if clown has moved two screens lower than highest point, its game over
	if(highest_position_ - m_obPosition.y > SCREEN_SIZE.height * 2)
	{
		game_world_->GameOver();
	}
	
	// update rocket jet stream if it exists
	if(rocket_trail_) rocket_trail_->setPosition(m_obPosition.x - m_obContentSize.width/3, m_obPosition.y);

	// do not update state based on velocity for the following states
	switch(state_)
	{
	case E_CLOWN_BOUNCE:
	case E_CLOWN_ROCKET:
	case E_CLOWN_BALLOON:
		return;
	}		

	// udpate state based on vertical component of linear velocity
	b2Vec2 velocity = body_->GetLinearVelocity();
	if(velocity.y >= 5.0f)
		SetState(E_CLOWN_UP);
	else
		SetState(E_CLOWN_DOWN);
}

void Clown::SetBody(b2Body* body)
{
	// create a box shape
	b2PolygonShape clown_shape;
	clown_shape.SetAsBox(SCREEN_TO_WORLD(m_obContentSize.width * 0.5), SCREEN_TO_WORLD(m_obContentSize.height * 0.5f));

	// create a fixture def with the box shape and high restitution
	b2FixtureDef clown_fixture_def;
	clown_fixture_def.shape = &clown_shape;
	clown_fixture_def.restitution = 0.5f;
	body->CreateFixture(&clown_fixture_def);
	// don't let the clown rotate
	body->SetFixedRotation(true);

	// call parent class' function
	GameObject::SetBody(body);
}

void Clown::SetState(EClownState state)
{
	// only accept a change in state
	if(state_ == state)
		return;

	state_ = state;

	// call respective state based action
	switch(state_)
	{
	case E_CLOWN_UP:
		StartGoingUp();
		break;
	case E_CLOWN_DOWN:
		StartComingDown();
		break;
	case E_CLOWN_BOUNCE:
		StartBounce();
		break;
	case E_CLOWN_ROCKET:
		StartRocket();
		break;
	case E_CLOWN_BALLOON:
		StartBalloon();
		break;
	}
}

void Clown::StartGoingUp()
{
	setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjoker02.png"));
}

void Clown::StartComingDown()
{
	setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjoker01.png"));
}

void Clown::StartBounce()
{
	setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjoker05.png"));
	// stay in this state only for a short duration
	scheduleOnce(schedule_selector(Clown::FinishBounce), 0.15f);
}

void Clown::FinishBounce(float dt)
{
	// after bounce, clown will be moving upwards
	SetState(E_CLOWN_UP);
}

void Clown::StartRocket()
{
	setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjroket.png"));
	// unschedule any previously scheduled selectors...possibly by another rocket/balloon
	unschedule(schedule_selector(Clown::FinishRocketBalloon));
	// stay in this state for some time
	scheduleOnce(schedule_selector(Clown::FinishRocketBalloon), ROCKET_DURATION);

	// no gravity while aboard a bottle rocket
	body_->SetGravityScale(0.0f);
	// decently high velocity while aboard a bottle rocket
	body_->SetLinearVelocity(b2Vec2(0.0f, 30.0f));

	// create neat jet stream for the rocket
	rocket_trail_ = CCParticleSystemQuad::create("explosion.plist");
	rocket_trail_->setDuration(-1);
	rocket_trail_->setPositionType(kCCPositionTypeRelative);
	game_world_->game_object_layer_->addChild(rocket_trail_);

	SOUND_ENGINE->playEffect("bottle_rocket.wav");
}

void Clown::StartBalloon()
{
	setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("cjballon.png"));
	// unschedule any previously scheduled selectors...possibly by another rocket/balloon
	unschedule(schedule_selector(Clown::FinishRocketBalloon));
	// stay in this state for some time
	scheduleOnce(schedule_selector(Clown::FinishRocketBalloon), BALLOON_DURATION);

	// no gravity while holding on to a balloon
	body_->SetGravityScale(0.0f);
	// slow velocity while holding on to a balloon
	body_->SetLinearVelocity(b2Vec2(0.0f, 10.0f));
}

void Clown::FinishRocketBalloon(float dt)
{
	// after rocket/balloon, clown will be moving upwards
	SetState(E_CLOWN_UP);
	// resume normal gravity
	body_->SetGravityScale(1.0f);

	// remove any rocket jet stream if it exists
	if(rocket_trail_)
	{
		rocket_trail_->removeFromParentAndCleanup(true);
		rocket_trail_ = NULL;
	}
}

void Clown::Pause()
{
	pauseSchedulerAndActions();
	if(rocket_trail_)
	{
		rocket_trail_->pauseSchedulerAndActions();
	}
}

void Clown::Resume()
{
	resumeSchedulerAndActions();
	if(rocket_trail_)
	{
		rocket_trail_->resumeSchedulerAndActions();
	}
}