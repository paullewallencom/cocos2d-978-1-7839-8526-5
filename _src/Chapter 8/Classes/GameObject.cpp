#include "GameObject.h"

GameObject::~GameObject()
{}

GameObject* GameObject::create(GameWorld* game_world, const char* frame_name)
{
	GameObject* game_object = new GameObject();
	if(game_object && game_object->init(game_world, frame_name))
	{
		game_object->autorelease();
		return game_object;
	}
	CC_SAFE_DELETE(game_object);
	return NULL;
}

bool GameObject::init(GameWorld* game_world, const char* frame_name)
{
	if(frame_name)
	{
		if(!CCSprite::initWithFile(frame_name))
			return false;
	}
	else
	{
		if(!CCSprite::init())
			return false;
	}

	game_world_ = game_world;
	return true;
}

void GameObject::Update()
{
	if(!m_bVisible)
		return;

	// update position of sprite based on position of body
	if(body_)
	{
		setPosition(ccp(WORLD_TO_SCREEN(body_->GetPosition().x), WORLD_TO_SCREEN(body_->GetPosition().y)));
		setRotation(CC_RADIANS_TO_DEGREES(body_->GetAngle() * -1));
	}
}

void GameObject::SetBody(b2Body* body)
{
	// save reference of self into b2Body
	body_ = body;
	if(body_)
	{
		body_->SetUserData(this);
	}
}