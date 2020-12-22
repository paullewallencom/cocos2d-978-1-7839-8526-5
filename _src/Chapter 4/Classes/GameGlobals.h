#ifndef GAME_GLOBALS_H_
#define GAME_GLOBALS_H_

#include "cocos2d.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

using namespace std;

#define SCREEN_SIZE GameGlobals::screen_size_
#define SOUND_ENGINE CocosDenshion::SimpleAudioEngine::sharedEngine()

#define MAX_STARS 250
#define MAX_ENEMIES 250
#define MAX_POWERUPS 5
#define MAX_BLASTS 25
#define MAX_MISSILES 15

#define PLAYER_RADIUS 20
#define ENEMY_RADIUS 10
#define MISSILE_RADIUS 5
#define POWERUP_ICON_OUTER_RADIUS 20
#define POWERUP_ICON_INNER_RADIUS 10.5

#define ENEMY_SPAWN_DELAY 0.125f
#define MISSILE_SPEED 8
#define SHIELD_DURATION 10
#define MAX_POWERUP_WAIT_ON_SCREEN 30
#define COMBO_TIME 3

#define SHIELD_BLINK_TAG 111

#define CIRCLE_INTERSECTS_CIRCLE(circle1_position, circle1_radius, circle2_position, circle2_radius) (ccpDistance((circle1_position), (circle2_position)) <= ((circle1_radius) + (circle2_radius)))
#define CIRCLE_CONTAINS_POINT(circle_position, circle_radius, point) (ccpDistance((circle_position), (point)) <= (circle_radius))
#define RECT_CONTAINS_CIRCLE(rect, circle_position, circle_radius) \
	((rect).containsPoint(ccp((circle_position).x - (circle_radius), (circle_position).y)) && \
	(rect).containsPoint(ccp((circle_position).x, (circle_position).y + (circle_radius))) && \
	(rect).containsPoint(ccp((circle_position).x + (circle_radius), (circle_position).y)) && \
	(rect).containsPoint(ccp((circle_position).x, (circle_position).y - (circle_radius))))

// enum used for proper z-ordering
enum EZIndex
{
	E_LAYER_BACKGROUND = 0,
	E_LAYER_FOREGROUND = 2,
	E_LAYER_ENEMIES = 4,
	E_LAYER_BLASTS = 6,
	E_LAYER_MISSILES = 8,
	E_LAYER_POWERUPS = 10,
	E_LAYER_PLAYER = 12,
	E_LAYER_HUD = 14,
	E_LAYER_POPUP = 16,
};

enum ESkillTimer
{
	E_SKILL1 = 10,
	E_SKILL2 = 30,
	E_SKILL3 = 45,
	E_SKILL4 = 60,
	E_SKILL5 = 90,
	E_SKILL6 = 120,
};

enum EEnemySpeedTimer
{
	E_SLOW = 5,
	E_MEDIUM = 10,
	E_FAST = 15,
	E_SUPER_FAST = 25,
};

enum EEnemyFormation
{
	E_FORMATION_RANDOM_EASY = 0,
	E_FORMATION_VERTICAL_EASY,
	E_FORMATION_HORIZONTAL_EASY,
	E_FORMATION_POLYGON_EASY,
	E_FORMATION_RANDOM_MEDIUM,
	E_FORMATION_VERTICAL_MEDIUM,
	E_FORMATION_HORIZONTAL_MEDIUM,
	E_FORMATION_POLYGON_MEDIUM,
	E_FORMATION_RANDOM_HARD,
	E_FORMATION_VERTICAL_HARD,
	E_FORMATION_HORIZONTAL_HARD,
	E_FORMATION_POLYGON_HARD,
	E_FORMATION_MAX	//12
};

enum EPowerUpType
{
	E_POWERUP_BOMB = 0,
	E_POWERUP_MISSILE_LAUNCHER,
	E_POWERUP_SHIELD,
	E_POWERUP_MAX	// 3
};

class GameGlobals
{
public:
	GameGlobals(void){};
	~GameGlobals(void){};

	// initialise common global data here...called when application finishes launching
	static void Init();
	// load initial/all game data here
	static void LoadData();

	static void GetRegularPolygonVertices(vector<CCPoint> &vertices, int num_vertices, float circum_radius);
	static void GetRegularPolygonVertices(vector<CCPoint> &vertices, int num_vertices, float circum_radius, float start_angle);
	
	static int GetNumVerticesForFormation(EEnemyFormation type);
	static vector<CCPoint> GetEnemyFormation(EEnemyFormation type, CCRect boundary, CCPoint anchor_point);
	static void GenerateRandomFormation(vector<CCPoint> &vertices, int num_vertices, CCRect boundary);
	static void GenerateVerticalFormation(vector<CCPoint> &vertices, int num_vertices, CCRect boundary);
	static void GenerateHorizontalFormation(vector<CCPoint> &vertices, int num_vertices, CCRect boundary);
	static void GeneratePolygonFormation(EEnemyFormation type, vector<CCPoint> &vertices, int num_vertices, CCRect boundary, CCPoint anchor_point);

	// save screen size for fast access
	static CCSize screen_size_;
	static const int skill1_formations_size = 2;
	static const int skill1_formations[];
	static const int skill2_formations_size = 10;
	static const int skill2_formations[];
	static const int skill3_formations_size = 17;
	static const int skill3_formations[];
	static const int skill4_formations_size = 17;
	static const int skill4_formations[];
	static const int skill5_formations_size = 17;
	static const int skill5_formations[];
	static const int skill6_formations_size = 17;
	static const int skill6_formations[];
	static const int powerup_frequency_size = 10;
	static const int powerup_frequency[];
};

#endif // GAME_GLOBALS_H_
