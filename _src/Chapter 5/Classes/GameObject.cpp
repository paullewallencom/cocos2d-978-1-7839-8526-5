#include "GameObject.h"

GameObject* GameObject::create(GameWorld* game_world, const char* frame_name)
{
	GameObject* game_object = new GameObject();
	if(game_object)
	{
		if(frame_name)
		{
			if(game_object->initWithSpriteFrameName(frame_name))
			{
				game_object->game_world_ = game_world;
				game_object->autorelease();
				return game_object;
			}
		}
		else
		{
			if(game_object->init())
			{
				game_object->game_world_ = game_world;
				game_object->autorelease();
				return game_object;
			}
		}
	}
	CC_SAFE_DELETE(game_object);
	return NULL;
}

GameObject::~GameObject()
{}