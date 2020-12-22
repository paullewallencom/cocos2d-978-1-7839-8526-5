#ifndef PLAYER_H_
#define PLAYER_H_

#include "GameGlobals.h"
#include "CustomSprite.h"

class GameWorld;

class Player : public CustomSprite
{
public:
	Player(GameWorld* game_world);
	~Player();

	// returns an autorelease Player
	static Player* createWithSpriteFrameName(GameWorld* game_world, const char* frame_name);
	
	// called at the start of the level
	void Enter();
	// callback after Enter()
	void EnterFinished();
	// called at the end of the level
	void Leave();
	// called when player is hit by enemy bullet
	void Die();
	// called after Die if lives are left
	void Respawn();
	// callback after Respawn()
	void OnRespawnComplete();
	// callback after Die if lives are over
	void OnAllLivesFinished();

	CC_SYNTHESIZE(int, lives_, Lives);
	CC_SYNTHESIZE(bool, is_respawning_, IsRespawning);

private:
	GameWorld* game_world_;
};

#endif // PLAYER_H_
