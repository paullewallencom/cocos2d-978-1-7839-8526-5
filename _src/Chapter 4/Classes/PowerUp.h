#ifndef POWERUP_H_
#define POWERUP_H_

#include "GameGlobals.h"

class GameWorld;

class PowerUp : public CCDrawNode
{
public:
	PowerUp() : time_left_(0), speed_(CCPointZero), is_active_(false), must_be_removed_(false), game_world_(NULL){}
	~PowerUp(){};

	virtual bool init(GameWorld* instance);
	
	virtual void Update();
	virtual void Tick();
	virtual void Spawn();
	virtual void Activate();
	virtual void Deactivate();
	
	CC_SYNTHESIZE(int, time_left_, TimeLeft);
	CC_SYNTHESIZE(CCPoint, speed_, Speed);
	CC_SYNTHESIZE(bool, is_active_, IsActive);
	CC_SYNTHESIZE(bool, must_be_removed_, MustBeRemoved);

protected:
	GameWorld* game_world_;
};

#endif // POWERUP_H_