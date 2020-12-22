#ifndef SKY_H_
#define SKY_H_

#include "GameGlobals.h"

class Sky : public CCNode
{
public:
	static Sky* create();
	virtual bool init();
	~Sky();

	CCSprite* GenerateSkySprite();
	void Update(CCPoint penguin_position, float terrain_offset_x, float terrain_scale);

private:
	CCSprite* sprite_;
	CCSprite* clouds_;
	float offset_x_;
};

#endif // SKY_H_
