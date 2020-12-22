#ifndef TOWER_H_
#define TOWER_H_

#include "GameGlobals.h"

class GameWorld;
class Enemy;

class Tower: public CCSprite
{
public:
	Tower();
	virtual ~Tower();

	static Tower* create(GameWorld* game_world, int type, CCPoint position);

	virtual bool init(GameWorld* game_world, int type, CCPoint position);
	// copy the data within the TowerDataSet library inside GameGlobals
	void SetTowerProperties();

	// update functions
	virtual void Update();
	void UpdateRotation();

	// functions that take care of upgradation & resale
	void Upgrade();
	void Sell();

	// basic tower behaviour
	void CheckForEnemies();
	void SetTarget(Enemy* enemy);
	void Shoot(float dt);
	void ShootBullet();
	void ShootLightning();
	
	// show the range for this tower
	void CreateRangeNode();
	void ShowRange();

	// accessors & mutators
	void SetSpriteName(const char* sprite_name);
	void SetIsRotating(bool is_rotating);

	inline void SetBulletName(const char* bullet_name) { bullet_name_ = bullet_name; }

	inline void SetIsLightning(bool is_lightning) { is_lightning_ = is_lightning; }
	inline bool GetIsLightning() { return is_lightning_; }

	inline void SetRange(float range) { range_ = range * TILE_SIZE; }
	inline float GetRange() { return range_; }

	inline void SetPhysicalDamage(float physical_damage) { physical_damage_ = physical_damage; }
	inline float GetPhysicalDamage() { return physical_damage_; }

	inline void SetMagicalDamage(float magical_damage) { magical_damage_ = magical_damage; }
	inline float GetMagicalDamage() { return magical_damage_; }

	inline void SetSpeedDamage(float speed_damage) { speed_damage_ = speed_damage; }
	inline float GetSpeedDamage() { return speed_damage_; }

	inline void SetSpeedDamageDuration(float speed_damage_duration) { speed_damage_duration_ = speed_damage_duration; }
	inline float GetSpeedDamageDuration() { return speed_damage_duration_; }

	inline void SetFireRate(float fire_rate) { fire_rate_ = fire_rate; }
	inline float GetFireRate() { return fire_rate_; }

	inline void SetCost(int cost) { cost_ = cost; }
	inline int GetCost() { return cost_; }

	inline int GetType() { return type_; }
	inline int GetLevel() { return current_level_; }

protected:
	GameWorld* game_world_;

	// properties that define the tower
	// these take values straight from the TowerDataSet & TowerData structs
	int type_;
	const char* bullet_name_;
	bool is_lightning_;
	bool is_rotating_;
	float range_;
	float physical_damage_;
	float magical_damage_;
	float speed_damage_;
	float speed_damage_duration_;
	float fire_rate_;
	int cost_;

	// the level of upgrade the tower is currently at
	int current_level_;
	// the tower's current target
	Enemy* target_;

	// a sprite to represent the base for a rotating tower
	CCSprite* base_sprite_;
	// a node to draw the circular range for this tower
	CCDrawNode* range_node_;
};

#endif /* TOWER_H_ */
