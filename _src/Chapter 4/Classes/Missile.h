#ifndef MISSILE_H_
#define MISSILE_H_

#include "GameGlobals.h"

class GameWorld;

class Missile : public CCDrawNode
{
public:
	Missile() : target_(CCPointZero), speed_(CCPointZero), must_be_removed_(false), has_exploded_(false), game_world_(NULL), previous_position_(CCPointZero) {};
	virtual ~Missile(){};

	static Missile* createWithTarget(GameWorld* instance, CCPoint target, CCPoint speed);

	virtual bool initWithTarget(GameWorld* instance, CCPoint target, CCPoint speed);
	virtual void update(float dt);
	void Explode(float dt = 0);

	CC_SYNTHESIZE(CCPoint, target_, Target);
	CC_SYNTHESIZE(CCPoint, speed_, Speed);
	CC_SYNTHESIZE(bool, must_be_removed_, MustBeRemoved);
	CC_SYNTHESIZE(bool, has_exploded_, HasExploded);

protected:
	GameWorld* game_world_;
	CCPoint previous_position_;
};

#endif // MISSILE_H_
