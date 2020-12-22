#ifndef PENGUIN_H_
#define PENGUIN_H_

#include "GameGlobals.h"

class GameWorld;

class Penguin : public CCSprite
{
public:
	Penguin();
	virtual ~Penguin();

	// returns an autorelease GameObject
	static Penguin* create(GameWorld* game_world, const char* frame_name);
	void CreateBody();

	// accessors & mutators
	inline b2Body* GetBody() { return body_; }

	void UpdatePhysics();
	void UpdateLeap();
	void UpdateNode();
	void Pause();
	void Resume();

	void SetIsAwake(bool is_awake);
	void SetIsDiving(bool is_diving);

	void SetIsFlying(bool is_flying);

	void StartLeap();
	void FinishLeap();
	inline bool GetIsLeaping() { return is_leaping_; }

	void StartFever();
	void FinishFever();

	void OnPerfectSlide();
	void OnBadLanding();

	inline int GetTotalPerfectSlides() { return total_perfect_slides_; }

protected:
	GameWorld* game_world_;
	b2Body* body_;
	CCParticleSystemQuad* trail_;

	bool is_awake_;
	bool is_diving_;
	bool is_flying_;
	bool is_leaping_;
	bool is_feverish_;
	int num_perfect_slides_;
	int total_perfect_slides_;
};

#endif // PENGUIN_H_
