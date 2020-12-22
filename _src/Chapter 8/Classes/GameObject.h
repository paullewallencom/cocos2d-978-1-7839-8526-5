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

	virtual bool init(GameWorld* game_world, const char* frame_name);

	virtual void Update();

	// accessors & mutators
	inline b2Body* GetBody() { return body_; }
	virtual void SetBody(b2Body* body);
	
	inline EGameObjectType GetType() { return type_; }
	inline void SetType(EGameObjectType type) { type_ = type; }

protected:
	GameWorld* game_world_;
	b2Body* body_;
	EGameObjectType type_;
};

#endif // GAME_OBJECT_H_
