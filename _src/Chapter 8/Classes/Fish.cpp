#include "Fish.h"
#include "GameWorld.h"

Fish::~Fish()
{}

Fish* Fish::create(GameWorld* game_world, float radius, const char* frame_name)
{
	Fish* fish = new Fish();
	if(fish)
	{
		// frame_name will be passed when splitting & bombing fish are touched
		if(frame_name)
		{
			if(fish->init(game_world, radius, frame_name))
			{
				fish->autorelease();
				return fish;
			}
		}
		// default frame_name for simple fish
		else if(fish->init(game_world, radius, "fish_01.png"))
		{
			fish->autorelease();
			return fish;
		}
	}
	CC_SAFE_DELETE(fish);
	return NULL;
}

bool Fish::init(GameWorld* game_world, float radius, const char* frame_name)
{
	if(!GameObject::init(game_world, frame_name))
		return false;

	// every fish must have a radius
	radius_ = radius;
	// fetch a newly created circle fixture body
	b2Body* body = GameGlobals::CreateCircleBody(game_world_->GetWorld(), radius_);
	// initially position it outside the left edge of the screen
	body->SetTransform(b2Vec2(SCREEN_TO_WORLD(-SCREEN_SIZE.width * 0.5f), 0), 0);
	// initially no processing is wasted
	body->SetActive(false);
	// save the body
	SetBody(body);
	return true;
}

void Fish::Update()
{
	// no processing if invisible
	if(!m_bVisible)
		return;

	// Finish & Kill this fish if it has lef the screen
	if(body_->GetPosition().y < 0 || body_->GetPosition().x > SCREEN_TO_WORLD(SCREEN_SIZE.width * 2.0f))
	{
		Kill();
		Finish(0.0f);
	}

	// update the sprite's position
	setPosition(ccp(WORLD_TO_SCREEN(body_->GetPosition().x), WORLD_TO_SCREEN(body_->GetPosition().y)));

	// stop rotating after being fired
	if(has_been_fired_)
	{
		// ease the angle from <whatever> to 0
		setRotation(m_fRotationX / 5);

		// fish can't moonwalk...set the correct direction
		if(body_->GetLinearVelocity().x > 0.5f)
			setFlipX(false);
		else if(body_->GetLinearVelocity().x < -0.5f)
			setFlipX(true);
	}
}

void Fish::Spawn()
{
	// reposition the body near the ledge of the catapult
	body_->SetTransform(game_world_->GetFishSpawnPoint() + b2Vec2(-radius_, radius_), 0);
	// start physics processing
	body_->SetActive(true);
	// start processing the Update
	setVisible(true);
	// animate the spawn
	setScale(0.0f);
	runAction(CCEaseBackOut::create(CCScaleTo::create(0.25f, 1.0f)));
}

void Fish::Touch()
{
	// fish can only be touched if it is active, has already been fired AND
	// if it has not already been touched
	if(!body_->IsActive() || !has_been_fired_ || has_been_touched_)
		return;

	has_been_touched_ = true;
}

void Fish::Hit(float check_finish_interval)
{
	// fish can only hit something if it is has already been fired AND
	// if it has not already hit something
	if(!has_been_fired_ || has_hit_)
		return;

	has_hit_ = true;
	// save last position to find if this fish has stopped moving 
	last_position_ = body_->GetPosition();
	// keep checking if fish has finished
	schedule(schedule_selector(Fish::Finish), check_finish_interval);
}

void Fish::Finish(float dt)
{
	// fish has not finished if it is awake AND
	// if it is moving at a considerable speed
	if(body_->IsAwake() && (body_->GetPosition() - last_position_).Length() > radius_ * 3)
	{
		last_position_ = body_->GetPosition();
		return;
	}

	// fish has more or less stopped...unschedule this function
	unschedule(schedule_selector(Fish::Finish));
	// only throwable fish get to spawn other fish
	if(is_throwable_)
	{
		// ask GameWorld to spawn the next fish
		game_world_->SpawnFish();
	}
}

void Fish::Kill()
{
	// reposition the body out of the screen
	body_->SetTransform(b2Vec2(SCREEN_TO_WORLD(-SCREEN_SIZE.width * 0.5f), 0), 0);
	// deactivate the body so processing is stopped
	body_->SetActive(false);
	// put the body to sleep...this will help this fish Finish
	body_->SetAwake(false);
	// hide the sprite so Update is stopped
	setVisible(false);
}

void Fish::SetHasBeenFired(bool has_been_fired) 
{
	has_been_fired_ = has_been_fired;
}

/******************** ShootingFish ********************/

ShootingFish::ShootingFish()
{
	fish_type_ = E_FISH_SHOOTING;
}

ShootingFish::~ShootingFish()
{}

ShootingFish* ShootingFish::create(GameWorld* game_world)
{
	ShootingFish* shooting_fish = new ShootingFish();
	if(shooting_fish && shooting_fish->init(game_world, SHOOTING_FISH_RADIUS, "fish_02.png"))
	{
		shooting_fish->autorelease();
		return shooting_fish;
	}
	CC_SAFE_DELETE(shooting_fish);
	return NULL;
}

void ShootingFish::Touch()
{
	// fish can only be touched if it is active, has already been fired AND
	// if it has not already been touched
	if(!body_->IsActive() || !has_been_fired_ || has_been_touched_)
		return;

	// call parent class' Touch
	Fish::Touch();
	// Go faster Fish!
	body_->ApplyLinearImpulse(b2Vec2(10.0f, 0), body_->GetWorldCenter());	
}

/******************** SplittingFish ********************/

SplittingFish::SplittingFish()
{
	fish_type_ = E_FISH_SPLITTING;
	// increase this for maximum destruction :D
	num_splitting_fish_ = 3;
}

SplittingFish::~SplittingFish()
{}

SplittingFish* SplittingFish::create(GameWorld* game_world)
{
	SplittingFish* splitting_fish = new SplittingFish();
	if(splitting_fish && splitting_fish->init(game_world, SPLITTING_FISH_RADIUS, "fish_04.png"))
	{
		splitting_fish->autorelease();
		return splitting_fish;
	}
	CC_SAFE_DELETE(splitting_fish);
	return NULL;
}

void SplittingFish::Touch()
{
	// fish can only be touched if it is active, has already been fired AND
	// if it has not already been touched
	if(!body_->IsActive() || !has_been_fired_ || has_been_touched_)
		return;

	// Call parent class' Touch
	Fish::Touch();
	// Since this Fish's Hit won't be called by default, call it here manually
	Fish::Hit(3.0f);

	// spawn "num_splitting_fish_" simple fish
	for(int i = 0; i < num_splitting_fish_; ++i)
	{
		AddSplitFish(i);
	}

	// Kill the splitting fish
	Fish::Kill();
}

void SplittingFish::AddSplitFish(int num_split)
{
	// create a simple fish with a different sprite name & radius
	Fish* split_fish = Fish::create(game_world_, TINY_FISH_RADIUS, "fish_03.png");
	// this fish won't spawn so set the visibility to true
	split_fish->setVisible(true);
	// ask GameWorld to add this fish to its list
	game_world_->AddFish(split_fish);

	// find the direction of the mother fish (SplittingFish*)
	b2Vec2 splitting_fish_velocity = body_->GetLinearVelocity();
	splitting_fish_velocity.Normalize();
	// calculate impulse that the mother fish will apply to baby fish (Fish*)
	b2Vec2 split_fish_impulse = b2Vec2(splitting_fish_velocity.x * 15.0f, splitting_fish_velocity.y * 7.5f + num_split * -2);

	// appropriately position the baby fish
	b2Body* split_fish_body = split_fish->GetBody();
	split_fish_body->SetTransform(b2Vec2(body_->GetPosition().x + radius_ - (radius_ * num_split), body_->GetPosition().y + radius_ - (radius_ * num_split)), 0);
	// activate the fish's body
	split_fish_body->SetActive(true);
	// apply impulse so the baby fish doesn't simply fall to the sea
	split_fish_body->ApplyLinearImpulse(split_fish_impulse, split_fish_body->GetWorldCenter());
}

/******************** BombingFish ********************/

BombingFish::BombingFish()
{
	fish_type_ = E_FISH_BOMBING;
}

BombingFish::~BombingFish()
{}

BombingFish* BombingFish::create(GameWorld* game_world)
{
	BombingFish* bombing_fish = new BombingFish();
	if(bombing_fish && bombing_fish->init(game_world, BOMBING_FISH_RADIUS, "fish_01.png"))
	{
		bombing_fish->autorelease();
		return bombing_fish;
	}
	CC_SAFE_DELETE(bombing_fish);
	return NULL;
}

void BombingFish::Touch()
{
	// fish can only be touched if it is active, has already been fired AND
	// if it has not already been touched
	if(!body_->IsActive() || !has_been_fired_ || has_been_touched_)
		return;

	// call the parent class' Touch
	Fish::Touch();
	// Since this Fish's Hit won't be called by default, call it here manually
	Fish::Hit(2.0f);

	// create a simple fish with a different sprite name & radius
	Fish* bomb_fish = Fish::create(game_world_, TINY_FISH_RADIUS, "fish_03.png");
	// this fish won't spawn so set the visibility to true
	bomb_fish->setVisible(true);
	// ask GameWorld to add this fish to its list
	game_world_->AddFish(bomb_fish);

	b2Body* bomb_fish_body = bomb_fish->GetBody();
	// increase the density of the fixture...which will increase the body's mass
	bomb_fish_body->GetFixtureList()->SetDensity(5.0f);
	// appropriately position the bomb
	bomb_fish_body->SetTransform(b2Vec2(body_->GetPosition().x, body_->GetPosition().y - radius_*0.55f - SIMPLE_FISH_RADIUS*0.55f), 0);
	// activate physics processing on the bomb
	bomb_fish_body->SetActive(true);
	// apply impulse so the bomb doesn't simply fall to the sea
	bomb_fish_body->ApplyLinearImpulse(b2Vec2(0, -15), bomb_fish_body->GetWorldCenter());

	// apply impulse to the BombingFish so it flies off
	body_->ApplyLinearImpulse(b2Vec2(8, 10), body_->GetWorldCenter());
}

/******************** ExplodingFish ********************/

ExplodingFish::ExplodingFish()
{
	fish_type_ = E_FISH_EXPLODING;
	num_rays_ = 32;
	blast_radius_ = 4.5f;
	blast_magnitude_ = 7.0f;
}

ExplodingFish::~ExplodingFish()
{}

ExplodingFish* ExplodingFish::create(GameWorld* game_world)
{
	ExplodingFish* exploding_fish = new ExplodingFish();
	if(exploding_fish && exploding_fish->init(game_world, EXPLODING_FISH_RADIUS, "fish_05.png"))
	{
		exploding_fish->autorelease();
		return exploding_fish;
	}
	CC_SAFE_DELETE(exploding_fish);
	return NULL;
}

void ExplodingFish::Hit(float check_finish_interval)
{
	// fish can only hit something if it is has already been fired AND
	// if it has not already hit something
	if(!has_been_fired_ || has_hit_)
		return;

	// call parent class' Hit
	Fish::Hit(1.0f);
	// schedule the explosion
	scheduleOnce(schedule_selector(ExplodingFish::Explode), 2.0f);
}

void ExplodingFish::Explode(float dt)
{
	// find bodies to blow apart
	for(int i = 0; i < num_rays_; ++i)
	{
		// calculate angle based on number of rays
		float angle = CC_DEGREES_TO_RADIANS((i / (float)num_rays_) * 360);
		// calculate direction vector based on angle
		b2Vec2 ray_direction(sinf(angle), cosf(angle));
		// calculate end point of the ray based on blast radius & direction vector
		b2Vec2 ray_end = body_->GetWorldCenter() + blast_radius_ * ray_direction;

		// perform RayCast on b2World from GameWorld...start point is the centre of this fish's body
		game_world_->GetWorld()->RayCast(game_world_, body_->GetWorldCenter(), ray_end);
		// query data found by the RayCast
		RayCastData ray_cast_data = game_world_->GetRayCastData();
		// did the ray hit a body?
		if(ray_cast_data.body_)
		{
			// apply a linear impulse at point of contact in the direction of this ray
			ray_cast_data.body_->ApplyLinearImpulse(b2Vec2(ray_direction.x * blast_magnitude_, 0.5f * blast_magnitude_), ray_cast_data.point_);
			// reset the data found by the RayCast
			game_world_->ResetRayCastData();
		}
	}
}