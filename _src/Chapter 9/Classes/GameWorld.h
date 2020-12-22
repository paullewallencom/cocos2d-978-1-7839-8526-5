#ifndef GAME_WORLD_H_
#define GAME_WORLD_H_

#include "GameGlobals.h"

class Tower;
class Enemy;
class TowerMenu;
class GestureLayer;

struct Wave
{
	int num_enemies_;
	int num_enemies_spawned_;
	int num_enemies_walking_;
	vector<Enemy*> enemies_;
	float spawn_delay_;

	Wave()
	{
		num_enemies_ = 0;
		num_enemies_spawned_ = 0;
		num_enemies_walking_ = 0;
		enemies_.clear();
		spawn_delay_ = 0.0f;
	}
};

class GameWorld : public CCLayer
{
public:
	GameWorld();
	virtual ~GameWorld();

    static CCScene* scene();    
    CREATE_FUNC(GameWorld);

    virtual bool init();

	void CreateGame();
	void CreateTiledMap();
	void CreateGrid();
	void CreateWalkPoints();
	void CreateWaves();
	void CreateHUD();
	void CreatePumpkin();

	void PlaceTower(int type, CCPoint position);
	void SellTower(int index);

	void StartNextWave(float dt);
	void SpawnEnemy(float dt);
	void EnemyAtTheGates(Enemy* enemy);
	void EnemyDown(Enemy* enemy);

	void ReduceLives(int amount);
	void UpdateCash(int amount);

	virtual void update(float dt);
	void UpdateHUD();
	void CheckWaveCompletion();

	void OnGestureReceived(CCObject* sender);
	void HandleTap(CCPoint position);
	void HandleSwipeUp();
	void HandleSwipeDown();
	void HandleSwipeLeft();
	void HandleSwipeRight();

	void OnTowerButtonClicked(CCObject* sender);
	void OnUpgradeTowerClicked(CCObject* sender);
	void OnSellTowerClicked(CCObject* sender);
	void OnToggleSpeedClicked(CCObject* sender);

	void OnPauseClicked(CCObject* sender);
	void ResumeGame();
	void GameOver(bool is_level_complete);

	inline Wave* GetCurrentWave() { return curr_wave_; }
	inline int GetCash() { return cash_; }

private:
	float time_scale_;
	CCTMXTiledMap* tiled_map_;
	CCTMXLayer* tmx_layer_;

	int num_enemy_walk_points_;
	vector<CCPoint> enemy_walk_points_;
	
	int num_towers_;
	vector<Tower*> towers_;

	int num_waves_;
	int curr_wave_index_;
	bool is_wave_starting_;
	Wave* curr_wave_;
	vector<Wave*> waves_;
	CCLabelTTF* waves_label_;

	int cash_;
	CCLabelTTF* cash_label_;

	int lives_left_;
	CCLabelTTF* lives_label_;
	CCPoint pumpkin_position_;
	CCSprite* pumpkin_;

	TowerMenu* tower_menu_;
	CCDrawNode* grid_node_;
	CCMenu* hud_menu_;

	GestureLayer* gesture_layer_;

public:
	bool is_popup_active_;
};

#endif // GAME_WORLD_H_