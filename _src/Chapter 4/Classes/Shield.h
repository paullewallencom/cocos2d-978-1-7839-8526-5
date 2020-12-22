#ifndef SHIELD_H_
#define SHIELD_H_

#include "GameGlobals.h"
#include "PowerUp.h"

class GameWorld;

class Shield : public PowerUp
{
public:
	Shield() : shield_time_left_(0){}
	~Shield(){};

	static Shield* create(GameWorld* instance);

	virtual bool init(GameWorld* instance);

	virtual void Update();
	virtual void Tick();
	virtual void Activate();
	virtual void Deactivate();
	virtual void Reset();

private:
	int shield_time_left_;
};

#endif // SHIELD_H_
