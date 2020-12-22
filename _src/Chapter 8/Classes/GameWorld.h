#ifndef GAME_WORLD_H_
#define GAME_WORLD_H_

#include "GameGlobals.h"

class GLESDebugDraw;
class b2dJson;
class Fish;
class Cat;
class GameObject;

class GameWorld : public CCLayer, public b2ContactListener, public b2RayCastCallback
{
public:
	GameWorld();
	virtual ~GameWorld();

    virtual bool init();

    static CCScene* scene();    
    CREATE_FUNC(GameWorld);
    
#ifdef ENABLE_DEBUG_DRAW
	virtual void draw();
#endif
	// update functions
	virtual void update(float dt);

	// creation functions
	void CreateGame();
	void CreateWorld(b2dJson* json, int level);
	void DestroyWorld();
	void CreateGameObjects(b2dJson* json);
	void CreateFish();
	void CreateEnvironment();
	void CreateHUD();

	// life cycle functions
	void SpawnFish(float dt = 0.1f);
	void AddFish(Fish* fish);
	void ReleaseFish();
	void CatHasDied();

	// touch functions
	virtual void ccTouchesBegan(CCSet* set, CCEvent* event);
	virtual void ccTouchesMoved(CCSet* set, CCEvent* event);
	virtual void ccTouchesEnded(CCSet* set, CCEvent* event);
	
	// Box2D callbacks
	void BeginContact(b2Contact* contact);
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
	float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);

	// game event functions
	void OnPauseClicked(CCObject* sender);
	void ResumeGame();
	void LevelComplete();
	void GameOver();

	// accessors & mutators
	inline b2World* GetWorld() { return world_; }
	inline b2Vec2 GetFishSpawnPoint() { return fish_spawn_point_; }
	void SetFishSpawnPoint();
	inline RayCastData GetRayCastData() { return ray_cast_data_; }
	inline void ResetRayCastData() { ray_cast_data_.body_ = NULL; ray_cast_data_.point_ = b2Vec2_zero; }

private:
	b2World* world_;
#ifdef ENABLE_DEBUG_DRAW
    GLESDebugDraw* debug_draw_;
#endif

	// catapult & boundary
	b2Body* boundary_body_;
	b2Body* catapult_body_;
	b2RevoluteJoint* catapult_joint_;
	b2WeldJoint* weld_joint_;
	bool is_catapult_released_;
	bool is_catapult_ready_;
	b2MouseJoint* mouse_joint_;

	// fish
	string fish_list_;
	b2Vec2 fish_spawn_point_;
	int num_throwable_fish_;
	int num_total_fish_;
	int current_fish_;
	vector<Fish*> fish_;

	// cats
	int num_cats_;
	int num_dead_cats_;
	vector<Cat*> cats_;

	// scaffolding
	int num_game_objects_;
	vector<GameObject*> game_objects_;

	// data found from RayCast
	RayCastData ray_cast_data_;

public:
	bool is_popup_active_;
};

#endif // GAME_WORLD_H_