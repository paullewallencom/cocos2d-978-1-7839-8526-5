#ifndef GAME_WORLD_H_
#define GAME_WORLD_H_

#include "GameGlobals.h"

#ifdef ENABLE_DEBUG_DRAW
class GLESDebugDraw;
#endif

class Sky;
class Terrain;
class Penguin;

class GameWorld : public CCLayer, public b2ContactListener
{
public:
	GameWorld();
	virtual ~GameWorld();

    static CCScene* scene();
    CREATE_FUNC(GameWorld);

    virtual bool init();

#ifdef ENABLE_DEBUG_DRAW
//	virtual void draw();
#endif
	// update functions
	virtual void update(float dt);
	void Tick(float dt);

	// creation functions
	void CreateWorld();
	void DestroyWorld();
	void CreateGame();
	void CreateHUD();

	void AddTime(int value);
	void ShowMessage(const char* message);

	// touch control & contact listener events
	virtual void ccTouchesBegan(CCSet* set, CCEvent* event);
	virtual void ccTouchesEnded(CCSet* set, CCEvent* event);
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);

	void OnPauseClicked(CCObject* sender);
	void ResumeGame();
	void GameOver();

	inline b2World* GetWorld() { return world_; }

private:
	b2World* world_;
#ifdef ENABLE_DEBUG_DRAW
    GLESDebugDraw* debug_draw_;
#endif

	Sky* sky_;
	Terrain* terrain_;
	Penguin* penguin_;
	
	int distance_travelled_;
	CCLabelBMFont* distance_label_;
	int time_left_;
	CCLabelBMFont* time_label_;
	CCLabelBMFont* message_label_;

	bool has_game_begun_;
	bool has_game_ended_;

public:
	bool is_popup_active_;
};

#endif // GAME_WORLD_H_
