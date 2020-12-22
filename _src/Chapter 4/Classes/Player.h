#ifndef PLAYER_H_
#define PLAYER_H_

#include "GameGlobals.h"

class GameWorld;
class Shield;

class Player : public CCDrawNode
{
public:
	Player() : radius_(PLAYER_RADIUS), shield_(NULL), game_world_(NULL), speed_(CCPointZero), is_dying_(false) {}
	~Player(){};

	static Player* create();

	virtual bool init();
	virtual void update(float dt);
	void UpdatePosition();
	void UpdateRotation(CCPoint previous_position);

	void Die();
	void Dead();

	CC_SYNTHESIZE(float, radius_, Radius);

	inline Shield* GetShield() { return shield_; };
	void SetShield(Shield* shield);
protected:
	Shield* shield_;

public:
	GameWorld* game_world_;
	CCPoint speed_;
	bool is_dying_;
};

#endif // PLAYER_H_
