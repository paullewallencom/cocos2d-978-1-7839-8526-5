#ifndef BACKGROUND_MANAGER_H_
#define BACKGROUND_MANAGER_H_

#include "GameGlobals.h"

class GameWorld;

class BackgroundManager : public CCNode
{
public:
	virtual bool init();
	~BackgroundManager(void);

	CREATE_FUNC(BackgroundManager);

	void CreateBG();
	void CreateWall();
	void Update(float speed);
	void UpdateBG(float speed);
	void UpdateWall(float speed);

private:
	CCSpriteBatchNode* batch_node_;
	CCSprite* bg_sprite_;
	int num_bg_pieces_;
	CCArray* bg_pieces_;
	CCArray* wall_sprites_;
	CCSize bg_piece_size_;

public:
	GameWorld* game_world_;
};

#endif // BACKGROUND_MANAGER_H_
