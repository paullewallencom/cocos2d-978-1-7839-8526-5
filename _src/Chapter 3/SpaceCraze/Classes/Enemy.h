#ifndef ENEMY_H_
#define ENEMY_H_

#include "GameGlobals.h"
#include "CustomSprite.h"

class GameWorld;

class Enemy : public CustomSprite
{
public:
	Enemy(GameWorld* game_world, const char* frame_name);
	virtual ~Enemy();

	// returns an autorelease Enemy
	static Enemy* createWithSpriteFrameName(GameWorld* game_world, const char* frame_name);

	// function called when this enemy is hit by player bullet
	int Die();

	CC_SYNTHESIZE(int, score_, Score);
	CC_SYNTHESIZE(const char*, bullet_name_, BulletName);
	CC_SYNTHESIZE(float, bullet_duration_, BulletDuration);
	
	ccColor4F particle_color_;

private:
	GameWorld* game_world_;
};

#endif // ENEMY_H_
