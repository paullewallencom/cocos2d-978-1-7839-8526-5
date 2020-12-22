#ifndef GAME_GLOBALS_H_
#define GAME_GLOBALS_H_

// Global includes go here
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include "Box2D\Box2D.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "Box2D/Box2D.h"
#endif

using namespace cocos2d;
using namespace std;
// Global includes end here

// Helper macros
#define SCREEN_SIZE GameGlobals::screen_size_
#define SOUND_ENGINE CocosDenshion::SimpleAudioEngine::sharedEngine()

#define PTM_RATIO 128
#define SCREEN_TO_WORLD(value) (float)(value)/PTM_RATIO
#define WORLD_TO_SCREEN(value) (float)(value)*PTM_RATIO
//#define ENABLE_DEBUG_DRAW			// uncomment this to enable debug drawing using GLESDebugDraw

#define MAX_HILL_KEY_POINTS 100
#define MAX_HILL_VERTICES 2000
#define MAX_BORDER_VERTICES 5000
#define HILL_SEGMENT_WIDTH 15
#define STRIPE_TEXTURE_SIZE 512
#define PENGUIN_SIZE 15
#define MIN_PENGUIN_VELOCITY_X 3
#define MIN_PENGUIN_VELOCITY_Y -40
#define MAX_PENGUIN_VELOCITY_X 18
#define MAX_PENGUIN_VELOCITY_Y 8
#define MAX_PENGUIN_Y (SCREEN_SIZE.height * 1.5f)
#define PERFECT_TAKE_OFF_VELOCITY 12.0f
#define BAD_LANDING_ANGLE_DIFF 2.4f // in radians

// enum used for proper z-ordering
enum EZorder
{
	E_LAYER_BACKGROUND = 0,
	E_LAYER_FOREGROUND = 2,
	E_LAYER_HUD = 6,
	E_LAYER_POPUP = 8,
};

enum EStripeType
{
	E_STRIPE_HORIZONTAL = 0,
	E_STRIPE_SLOPE_UP,
	E_STRIPE_SLOPE_DOWN,
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

	static CCSize screen_size_;
	static ccColor4F GetRandomColor();
};

#endif // GAME_GLOBALS_H_
