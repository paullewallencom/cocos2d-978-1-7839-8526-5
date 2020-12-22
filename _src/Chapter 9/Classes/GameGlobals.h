#ifndef GAME_GLOBALS_H_
#define GAME_GLOBALS_H_

// Global includes go here
#include "cocos2d.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace std;
// Global includes end here

// Helper macros
#define SCREEN_SIZE GameGlobals::screen_size_
#define SOUND_ENGINE CocosDenshion::SimpleAudioEngine::sharedEngine()

#define TILE_SIZE 90
#define MAX_COLS 15
#define MAX_ROWS 10

#define GET_X_FOR_COL(col) ( (col) * TILE_SIZE )
#define GET_Y_FOR_ROW(row, h) ( ( (h) - (row) ) * TILE_SIZE )
#define GET_COL_FOR_X(x) ( floor( (x) / TILE_SIZE ) )
#define GET_ROW_FOR_Y(y, h) ( (h) - ceil( (y) / TILE_SIZE ) )

#define NUM_TOWER_UPGRADES 3
#define TOWER_GID 100
#define ENEMY_MOVE_DURATION 1.0f
#define BULLET_MOVE_DURATION 0.15f
#define ENEMY_MOVE_ACTION_TAG 101
#define ENEMY_ANIMATE_ACTION_TAG 201
#define LIGHTNING_KEY_POINT_DIST 15
#define LIGHTNING_DURATION 0.25f
#define MIN_GESTURE_DISTANCE 10

// enum used for proper z-ordering
enum EZorder
{
	E_LAYER_BACKGROUND = 0,
	E_LAYER_ENEMY = 2,
	E_LAYER_TOWER = 4,
	E_LAYER_HUD = 6,
	E_LAYER_POPUP = 8,
};

enum EGestureType
{
	E_GESTURE_NONE = 0,
	E_GESTURE_TAP,
	E_GESTURE_SWIPE_UP,
	E_GESTURE_SWIPE_DOWN,
	E_GESTURE_SWIPE_LEFT,
	E_GESTURE_SWIPE_RIGHT,
};

struct TowerData
{
	char sprite_name_[256];
	float range_;
	float physical_damage_;
	float magical_damage_;
	float speed_damage_;
	float speed_damage_duration_;
	float fire_rate_;
	int cost_;

	TowerData() : range_(0.0f),
	physical_damage_(0.0f),
	magical_damage_(0.0f),
	speed_damage_(0.0f),
	speed_damage_duration_(0.0f),
	fire_rate_(0.0f),
	cost_(0)
	{
		sprintf(sprite_name_, "%s", "");
	}
};

struct TowerDataSet
{
	char bullet_name_[256];
	bool is_lightning_;
	bool is_rotating_;
	vector<TowerData*> tower_data_;

	TowerDataSet() : is_lightning_(false),
	is_rotating_(false)
	{
		sprintf(bullet_name_, "%s", "");
		tower_data_.clear();
	}

	~TowerDataSet()
	{
		for(int i = 0; i < NUM_TOWER_UPGRADES; ++i)
		{
			delete tower_data_[i];
		}
		tower_data_.clear();
	}
};

struct EnemyData
{
	char animation_name_[256];
	int health_;
	int armor_;
	int magic_resistance_;
	float speed_;
	int damage_;
	int reward_;

	EnemyData() : health_(0),
		armor_(0),
		magic_resistance_(0),
		speed_(0.0f),
		damage_(0),
		reward_(0)
	{
		sprintf(animation_name_, "%s", "");
	}
};

// Helper class containing only static members
class GameGlobals
{
private:
	GameGlobals();
	~GameGlobals();

public:
	// initialise common global data here
	// called when application finishes launching
	static void Init();
	// load initial/all game data here
	static void LoadData();
	static void LoadAnimations();
	static void LoadTowerData();
	static void LoadEnemyData();

	static void ScaleLabel(CCLabelTTF* label);

	// function takes comma separated string & returns vector of values
	static vector<int> GetIntListFromString(string input);

	static CCSize screen_size_;
	static int level_number_;

	// tower & enemy data libraries
	static int num_tower_data_sets_;
	static vector<TowerDataSet*> tower_data_sets_;
	static int num_enemy_data_;
	static vector<EnemyData*> enemy_data_;
};

#endif // GAME_GLOBALS_H_
