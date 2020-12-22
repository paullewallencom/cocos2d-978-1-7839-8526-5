#ifndef BOMB_H_
#define BOMB_H_

#include "GameGlobals.h"
#include "PowerUp.h"

class GameWorld;

class Bomb : public PowerUp
{
public:
	Bomb(){}
	~Bomb(){};

	static Bomb* create(GameWorld* instance);

	virtual bool init(GameWorld* instance);

	virtual void Update();
	virtual void Activate();
};

#endif // BOMB_H_