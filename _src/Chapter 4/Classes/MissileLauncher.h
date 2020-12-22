#ifndef MISSILE_LAUNCHER_H_
#define MISSILE_LAUNCHER_H_

#include "GameGlobals.h"
#include "PowerUp.h"

class GameWorld;

class MissileLauncher : public PowerUp
{
public:
	MissileLauncher(){}
	~MissileLauncher(){};

	static MissileLauncher* create(GameWorld* instance);

	virtual bool init(GameWorld* instance);

	virtual void Update();
	virtual void Activate();

private:
	vector<CCPoint> GenerateTargets();
};

#endif // MISSILE_LAUNCHER_H_
