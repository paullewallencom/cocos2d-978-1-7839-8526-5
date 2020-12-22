#ifndef GAME_OBJECT_H_
#define GAME_OBJECT_H_

#include "GameGlobals.h"

class GameWorld;

class GameObject : public CCSprite
{
public:
	GameObject() : game_world_(NULL), body_(NULL), type_(E_GAME_OBJECT_NONE) {}
	virtual ~GameObject();

	// returns an autorelease GameObject
	static GameObject* create(GameWorld* game_world, const char* frame_name);

	// accessors & mutators
	inline b2Body* GetBody() { return body_; }
	
	inline EGameObjectType GetType() { return type_; }
	inline void SetType(EGameObjectType type) { type_ = type; }

	virtual void Update()
	{
		// update position of sprite based on position of body
		if(body_)
		{
			setPosition(ccp(WORLD_TO_SCREEN(body_->GetPosition().x), WORLD_TO_SCREEN(body_->GetPosition().y)));
		}
	}

	virtual void SetBody(b2Body* body)
	{
		// save reference of self into b2Body
		if(body_)
		{
			body_->SetUserData(NULL);
			body_ = NULL;
		}
		if(body)
		{
			body->SetUserData(this);
			body_ = body;
		}
	}

protected:
	GameWorld* game_world_;
	b2Body* body_;
	EGameObjectType type_;
};

#endif // GAME_OBJECT_H_
