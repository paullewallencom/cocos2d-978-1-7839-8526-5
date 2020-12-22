#ifndef FISH_H_
#define FISH_H_

#include "GameObject.h"

class GameWorld;

class Fish : public GameObject
{
public:
	Fish() : fish_type_(E_FISH_SIMPLE), radius_(0.0f), is_throwable_(false), has_been_fired_(false), has_been_touched_(false), has_hit_(false), last_position_(b2Vec2_zero)
	{
		type_ = E_GAME_OBJECT_FISH;
	}
	~Fish();

	static Fish* create(GameWorld* game_world, float radius = SIMPLE_FISH_RADIUS, const char* frame_name = NULL);

	virtual bool init(GameWorld* game_world, float radius, const char* frame_name);
	virtual void Update();

	virtual void Spawn();
	virtual void Touch();
	virtual void Hit(float check_finish_interval = 1.0f);
	virtual void Finish(float dt);
	virtual void Kill();

	inline EFishType GetFishType() { return fish_type_; }
	inline void SetFishType(EFishType fish_type) { fish_type_ = fish_type; }

	inline float GetRadius() { return radius_; }
	inline void SetRadius(float radius) { radius_ = radius; }

	inline bool GetIsThrowable() { return is_throwable_; }
	inline void SetIsThrowable(bool is_throwable) { is_throwable_ = is_throwable; }

	void SetHasBeenFired(bool has_been_fired);
	inline bool GetHasBeenFired() { return has_been_fired_; }

protected:
	EFishType fish_type_;
	float radius_;
	bool is_throwable_;
	bool has_been_fired_;
	bool has_been_touched_;
	bool has_hit_;
	b2Vec2 last_position_;
};

class ShootingFish : public Fish
{
public:
	ShootingFish();
	~ShootingFish();

	static ShootingFish* create(GameWorld* game_world);

	virtual void Touch();
};

class SplittingFish : public Fish
{
public:
	SplittingFish();
	~SplittingFish();

	static SplittingFish* create(GameWorld* game_world);

	virtual void Touch();
	void AddSplitFish(int num_split);

protected:
	int num_splitting_fish_;
};

class BombingFish : public Fish
{
public:
	BombingFish();
	~BombingFish();

	static BombingFish* create(GameWorld* game_world);

	virtual void Touch();
};

class ExplodingFish : public Fish
{
public:
	ExplodingFish();
	~ExplodingFish();

	static ExplodingFish* create(GameWorld* game_world);

	virtual void Hit(float check_finish_interval = 1.0f);
	void Explode(float dt);

protected:
	int num_rays_;
	float blast_radius_;
	float blast_magnitude_;
};

#endif // FISH_H_