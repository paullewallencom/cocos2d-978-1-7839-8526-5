#ifndef ENEMY_H_
#define ENEMY_H_

#include "GameGlobals.h"

class GameWorld;

class Enemy: public CCSprite
{
public:
	Enemy();
	virtual ~Enemy();

	static Enemy* create(GameWorld* game_world, int type);

	virtual bool init(GameWorld* game_world, int type);
	// copy data within the enemy library inside GameGlobals
	virtual void SetEnemyProperties();

	// create & update the progress bar showing health left
	void CreateHealthBar();
	void UpdateHealthBar();

	// basic enemy behaviour functions
	void StartWalking();
	void FinishWalking();
	void DoDamage();
	void TakeDamage(CCObject* object);
	void Die();
	void TakeSpeedDamage(float speed_damage, float speed_damage_duration);
	void ResetSpeed(float dt);

	// accessors and mutators
	inline void SetAnimationName(const char* animation_name) { animation_name_ = animation_name; }

	inline void SetHealth(int health) { health_ = health; }
	inline int GetHealth() { return health_; }

	inline void SetArmor(int armor) { armor_ = armor; }
	inline int GetArmor() { return armor_; }

	inline void SetMagicResistance(int magic_resistance) { magic_resistance_ = magic_resistance; }
	inline int GetMagicResistance() { return magic_resistance_; }

	inline void SetSpeed(float speed) { speed_ = speed; }
	inline float GetSpeed() { return speed_; }

	inline void SetDamage(int damage) { damage_ = damage; }
	inline int GetDamage() { return damage_; }

	inline void SetReward(int reward) { reward_ = reward; }
	inline int GetReward() { return reward_; }

	inline float GetRadius() { return radius_; }
	inline bool GetHasDied() { return has_died_; }
	inline bool GetIsSlowed() { return is_slowed_; }

	inline void SetWalkPoints(int num_walk_points, vector<CCPoint> walk_points) { num_walk_points_ = num_walk_points; walk_points_ = walk_points; }

protected:
	GameWorld* game_world_;

	// properties that define the enemy
	// these take values straight from the EnemyData struct
	int type_;
	const char* animation_name_;
	int health_;
	int armor_;
	int magic_resistance_;
	float speed_;
	int damage_;
	int reward_;

	// more properties that define the enemy
	float radius_;
	int health_left_;
	bool has_died_;
	bool is_slowed_;
	int num_walk_points_;
	int curr_walk_point_;
	vector<CCPoint> walk_points_;
	
	// the progress bar showing health left
	CCProgressTimer* health_bar_;
};

#endif /* ENEMY_H_ */
