#ifndef COLLECTIBLE_H_
#define COLLECTIBLE_H_

#include "GameGlobals.h"
#include "GameObject.h"

class Collectible : public GameObject
{
public:
	Collectible() : score_(0), is_collected_(false) {}
	~Collectible();

	// returns an autorelease Collectible
	static Collectible* create(GameWorld* game_world, EGameObjectType type);
	
	virtual void Update();
	virtual void SetBody(b2Body* body);
	
	// life-cycle functions
	virtual void Init(b2Vec2 position);
	virtual void Reset();
	
	// collision response functions
	virtual void OnCollision();
	virtual void AfterCollision();

	void ShowParticle();

	CC_SYNTHESIZE(int, score_, Score);
	CC_SYNTHESIZE(bool, is_collected_, IsCollected);
}; 

#endif // COLLECTIBLE_H_