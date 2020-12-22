#include "Collectible.h"
#include "GameWorld.h"
#include "Clown.h"

Collectible::~Collectible()
{}

Collectible* Collectible::create(GameWorld* game_world, EGameObjectType type)
{
	Collectible* collectible = new Collectible();

	const char* frame_name = NULL;
	int score = 0;
	// select sprite frame name & score based on type
	switch(type)
	{
	case E_GAME_OBJECT_COLLECTIBLE:
		frame_name = CCRANDOM_MINUS1_1() > 0 ? "cjcollect01.png" : "cjcollect02.png";
		score = 10;
		break;
	case E_GAME_OBJECT_ROCKET:
		frame_name = "cjpowro.png";
		score = 25;
		break;
	case E_GAME_OBJECT_BALLOON:
		frame_name = "cjpoweba.png";
		score = 25;
		break;
	}

	if(collectible && collectible->initWithSpriteFrameName(frame_name))
	{
		collectible->game_world_ = game_world;
		collectible->type_ = type;
		collectible->score_ = score;
		collectible->autorelease();
		return collectible;
	}
	CC_SAFE_DELETE(collectible);
	return NULL;
}

void Collectible::Update()
{
	// call parent class' update
	GameObject::Update();

	// remove collectible if it has exited below the bottom of a few screens
	if(WORLD_TO_SCREEN(game_world_->distance_travelled_) - m_obPosition.y > SCREEN_SIZE.height * 3)
	{
		game_world_->RemoveCollectible(this);
	}
}

void Collectible::SetBody(b2Body* body)
{
	b2PolygonShape collectible_shape;

	// create different box shapes for different collectibles
	switch(type_)
	{
	case E_GAME_OBJECT_COLLECTIBLE:
		collectible_shape.SetAsBox(SCREEN_TO_WORLD(m_obContentSize.width/2), SCREEN_TO_WORLD(m_obContentSize.height/2));
		break;
	case E_GAME_OBJECT_ROCKET:
	case E_GAME_OBJECT_BALLOON:
		collectible_shape.SetAsBox(SCREEN_TO_WORLD(m_obContentSize.width/2), SCREEN_TO_WORLD(m_obContentSize.height/4), b2Vec2(0, SCREEN_TO_WORLD(m_obContentSize.height/4)), 0);
		break;
	}

	// mark the fixture as a sensor
	b2FixtureDef collectible_fixture_def;
	collectible_fixture_def.shape = &collectible_shape;
	collectible_fixture_def.isSensor = true;
	body->CreateFixture(&collectible_fixture_def);

	// call parent class' function
	GameObject::SetBody(body);
}

void Collectible::Init(b2Vec2 position)
{
	// initialise position and scale
	body_->SetTransform(position, 0);
	setScale(1.0f);
}

void Collectible::Reset()
{
	// reset position outside boundary of game
	body_->SetTransform(b2Vec2(SCREEN_TO_WORLD(-1 * WALL_WIDTH), SCREEN_TO_WORLD(-1 * WALL_WIDTH)), 0);
	removeFromParentAndCleanup(true);
	is_collected_ = false;
}

void Collectible::OnCollision()
{
	// can be collected only once
	if(is_collected_)
		return;

	is_collected_ = true;
	// add respective score
	game_world_->AddScore(score_);
	// scale down and exit
	runAction(CCSequence::createWithTwoActions(CCSequence::createWithTwoActions(CCScaleTo::create(0.01f, 1.2f), CCScaleTo::create(0.1f, 0.0f)), CCCallFunc::create(this, callfunc_selector(Collectible::AfterCollision))));

	// inform player of respective state change
	if(type_ == E_GAME_OBJECT_ROCKET)
	{
		game_world_->GetClown()->SetState(E_CLOWN_NONE);
		game_world_->GetClown()->SetState(E_CLOWN_ROCKET);
	}
	else if(type_ == E_GAME_OBJECT_BALLOON)
	{
		game_world_->GetClown()->SetState(E_CLOWN_NONE);
		game_world_->GetClown()->SetState(E_CLOWN_BALLOON);
	}

	ShowParticle();

	SOUND_ENGINE->playEffect("collectible.wav");
}

void Collectible::AfterCollision()
{
	// collision occurred so remove this collectible
	game_world_->RemoveCollectible(this);
}

void Collectible::ShowParticle()
{
	CCParticleSystemQuad* burst = CCParticleSystemQuad::create("explosion.plist");
	burst->setStartColor(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.0f));
	burst->setStartColorVar(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.0f));
	burst->setEndColor(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.0f));
	burst->setEndColorVar(ccc4f(CCRANDOM_0_1(), CCRANDOM_0_1(), CCRANDOM_0_1(), 1.0f));
	burst->setPositionType(kCCPositionTypeGrouped);
	burst->setAutoRemoveOnFinish(true);
	burst->setPosition(m_obPosition);
	game_world_->game_object_layer_->addChild(burst);
}