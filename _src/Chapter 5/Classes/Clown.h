#ifndef CLOWN_H_
#define CLOWN_H_

#include "GameGlobals.h"
#include "GameObject.h"

class Clown : public GameObject
{
public:
	Clown();
	~Clown();

	// returns an autorelease Clown
	static Clown* create(GameWorld* game_world);

	virtual void Update();
	virtual void SetBody(b2Body* body);

	// state machine modifiers
	inline EClownState GetState() { return state_; }
	void SetState(EClownState state);

	// state change actions
	void StartGoingUp();
	void StartComingDown();
	void StartBounce();
	void FinishBounce(float dt);
	void StartRocket();
	void StartBalloon();
	void FinishRocketBalloon(float dt);

	// life-cycle functions
	void Pause();
	void Resume();
	
	CC_SYNTHESIZE(float, highest_position_, HighestPosition);
private:
	EClownState state_;
	CCParticleSystemQuad* rocket_trail_;
};

#endif //CLOWN_H_