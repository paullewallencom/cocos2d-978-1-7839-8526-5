#include "Penguin.h"
#include "GameWorld.h"

Penguin* Penguin::create(GameWorld* game_world, const char* frame_name)
{
	Penguin* penguin = new Penguin();
	if(penguin)
	{
		if(frame_name)
		{
			if(penguin->initWithFile(frame_name))
			{
				penguin->game_world_ = game_world;
				penguin->CreateBody();
				penguin->autorelease();
				return penguin;
			}
		}
		else
		{
			if(penguin->init())
			{
				penguin->game_world_ = game_world;
				penguin->CreateBody();
				penguin->autorelease();
				return penguin;
			}
		}
	}
	CC_SAFE_DELETE(penguin);
	return NULL;
}

Penguin::Penguin()
{
	game_world_ = NULL;
	body_ = NULL;
	trail_ = NULL;
	is_awake_ = false;
	is_diving_ = false;
	is_flying_ = false;
	is_leaping_ = false;
	is_feverish_ = false;
	num_perfect_slides_ = 0;
	total_perfect_slides_ = 0;
}

Penguin::~Penguin()
{}

void Penguin::CreateBody()
{
	// dynamic body for penguin with some linear damping
	b2BodyDef body_def;
	body_def.type = b2_dynamicBody;
	body_def.linearDamping = 0.05f;
	body_def.position.Set(0, SCREEN_TO_WORLD(SCREEN_SIZE.height * 0.4f + PENGUIN_SIZE));
	body_ = game_world_->GetWorld()->CreateBody(&body_def);

	// circle shape for the penguin
	b2CircleShape shape;
	shape.m_radius = SCREEN_TO_WORLD(PENGUIN_SIZE);

	// fixture will have no friction
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;
	fixture_def.friction = 0.0f;
	fixture_def.density = 1.0f;
	body_->CreateFixture(&fixture_def);

	// penguin' body will initially be inactive
	body_->SetActive(false);
	body_->SetUserData(this);
}

void Penguin::UpdatePhysics()
{
	if(is_diving_)
	{
		// if penguin is asleep, awaken him and cancel the dive
		if(!is_awake_)
		{
			SetIsAwake(true);
			is_diving_ = false;
		}
		// else apply a downward force provided penguin isn't leaping
		else if(!is_leaping_)
		{
			body_->ApplyForceToCenter(b2Vec2(0.5f, -2.0f));
		}
	}

	// restrict velocity between minimum & maximum
	b2Vec2 vel = body_->GetLinearVelocity();
	vel.x = vel.x < MIN_PENGUIN_VELOCITY_X ? MIN_PENGUIN_VELOCITY_X : (vel.x > MAX_PENGUIN_VELOCITY_X ? MAX_PENGUIN_VELOCITY_X : vel.x);
	vel.y = vel.y < MIN_PENGUIN_VELOCITY_Y ? MIN_PENGUIN_VELOCITY_Y : (vel.y > MAX_PENGUIN_VELOCITY_Y ? MAX_PENGUIN_VELOCITY_Y : vel.y);
	body_->SetLinearVelocity(vel);

	UpdateLeap();
}

void Penguin::UpdateLeap()
{
	if(!is_leaping_)
		return;

	b2Vec2 vel = body_->GetLinearVelocity();
	b2Vec2 new_vel = vel;
	// increase the velocity
	new_vel.x += (MAX_PENGUIN_VELOCITY_X - new_vel.x) / 15.0f;
	new_vel.y += (MAX_PENGUIN_VELOCITY_Y - new_vel.y) / 15.0f;
	// ensure velocity doesn't exceed maximum
	new_vel.x = (new_vel.x > MAX_PENGUIN_VELOCITY_X) ? MAX_PENGUIN_VELOCITY_X : new_vel.x;
	new_vel.y = (new_vel.y > MAX_PENGUIN_VELOCITY_Y) ? MAX_PENGUIN_VELOCITY_Y : new_vel.y;
	body_->SetLinearVelocity(new_vel);
}

void Penguin::UpdateNode()
{
	// set node position based on body
	CCPoint previous_position = m_obPosition;
	setPositionX(WORLD_TO_SCREEN(body_->GetPosition().x));
	setPositionY(WORLD_TO_SCREEN(body_->GetPosition().y));
	
	// set rotation based on body
	float angle = CC_RADIANS_TO_DEGREES(-1 * ccpToAngle(ccpSub(m_obPosition, previous_position)));
	setRotation(angle);

	// fetch list of contacts
	b2Contact* contact = game_world_->GetWorld()->GetContactList();
	// if contact exists, penguin has landed
	if(contact)
	{
		SetIsFlying(false);
	}
	// else penguin is airborne
	else
	{
		SetIsFlying(true);
	}

	// update the trail if penguin is feverish
	if(is_feverish_)
	{
		trail_->setPosition(m_obPosition);
	}
}

void Penguin::Pause()
{
	// hide the trail when paused
	if(trail_)
		trail_->setVisible(false);
}

void Penguin::Resume()
{
	// show the trail when resuming
	if(trail_)
		trail_->setVisible(true);
}

void Penguin::SetIsAwake(bool is_awake)
{
	// activate the body & apply impulse in the top-right direction
	if(is_awake)
	{
		is_awake_ = true;
		body_->SetActive(true);
		body_->ApplyLinearImpulse(b2Vec2(0.1f, 0.2f), body_->GetWorldCenter());
	}
	// deactivate the body
	else
	{
		is_awake_ = false;
		body_->SetActive(false);
	}
}

void Penguin::SetIsDiving(bool is_diving)
{
	if(is_diving_ == is_diving)
		return;

	is_diving_ = is_diving;

	// set sprite for the respective state
	if(is_diving_)
	{
		initWithFile("penguin_3.png");
	}
	else
	{
		initWithFile("penguin_2.png");
	}
}

void Penguin::SetIsFlying(bool is_flying)
{
	if(is_flying_ == is_flying)
		return;

	is_flying_ = is_flying;

	// if penguin has taken off, check for a perfect slide
	if(is_flying_ && !is_leaping_)
	{
		// if take off velocity is above certain threshold, its a perfect slide
		if(body_->GetLinearVelocity().Length() >= PERFECT_TAKE_OFF_VELOCITY)
		{
			OnPerfectSlide();
		}
	}
}

void Penguin::StartLeap()
{
	if(is_leaping_)
		return;

	// successfully conquered a hill...reward player with extra time
	is_leaping_ = true;
	game_world_->AddTime(5 * (is_feverish_ ? 2 : 1));
	// no gravity so the penguin can be transported weightlessly to the next hill
	body_->SetGravityScale(0.0f);
}

void Penguin::FinishLeap()
{
	if(!is_leaping_)
		return;

	is_leaping_ = false;
	// next hill has arrived, turn gravity back on
	body_->SetGravityScale(1.0f);
	is_diving_ = false;
}

void Penguin::StartFever()
{
	if(is_feverish_)
		return;

	// fever mode means having a nice and colourful trail
	is_feverish_ = true;
	trail_ = CCParticleSystemQuad::create("explosion.plist");
	trail_->setStartColor(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.0f));
	trail_->setEndColor(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.0f));
	trail_->setStartColorVar(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.0f));
	trail_->setEndColorVar(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.0f));
	trail_->setDuration(-1);
	trail_->setPositionType(kCCPositionTypeRelative);
	m_pParent->addChild(trail_);
}

void Penguin::FinishFever()
{
	if(!is_feverish_)
		return;

	is_feverish_ = false;
	// remove the trail
	trail_->removeFromParentAndCleanup(true);
	trail_ = NULL;
}

void Penguin::OnPerfectSlide()
{
	// increment counters
	++ num_perfect_slides_;
	++ total_perfect_slides_;

	// 3 slides in a row activate fever mode
	if(num_perfect_slides_ == 3)
	{
		StartFever();
	}

	// a perfect slide deserves some extra time
	int time = 3 * (is_feverish_ ? 2 : 1);
	game_world_->AddTime(time);

	// inform player of the perfect slide
	char buf[16] = {0};
	sprintf(buf, "Perfect +%ds", time);
	game_world_->ShowMessage(buf);
}

void Penguin::OnBadLanding()
{
	// bad landing means resetting of perfect slides in a row & no more fever
	num_perfect_slides_ = 0;
	FinishFever();
}
