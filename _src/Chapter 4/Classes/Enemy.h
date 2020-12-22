#ifndef ENEMY_H_
#define ENEMY_H_

#include "GameGlobals.h"

#define NUM_SPIKES 10

class GameWorld;

class Enemy : public CCDrawNode
{
public:
	Enemy() : speed_(CCPointZero), is_dead_(false), is_spawning_(false), must_be_removed_(false), time_alive_(0), speed_multiplier_(0.25f), game_world_(NULL){}
	~Enemy(){};

	static Enemy* create(GameWorld* instance);

	virtual bool init(GameWorld* instance);

	void Update(CCPoint player_position, bool towards_player);
	void Tick();
	void Spawn(float delay);
	void FinishSpawn();
	void Die();

	CC_SYNTHESIZE(CCPoint, speed_, Speed);
	CC_SYNTHESIZE(bool, is_spawning_, IsSpawning);
	CC_SYNTHESIZE(bool, is_dead_, IsDead);
	CC_SYNTHESIZE(bool, must_be_removed_, MustBeRemoved);

protected:
	void GenerateVertices(CCPoint vertices[]);

	int time_alive_;
	float speed_multiplier_;

	GameWorld* game_world_;
};

#endif // ENEMY_H_
