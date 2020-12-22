#ifndef LIGHTNING_H_
#define LIGHTNING_H_

#include "GameGlobals.h"

class Lightning : public CCDrawNode
{
public:
	Lightning();
	~Lightning();

	static Lightning* create(CCPoint from, CCPoint to, ccColor4F color, bool is_animated);

	virtual bool init(CCPoint from, CCPoint to, ccColor4F color, bool is_animated);

protected:
	void GenerateKeyPoints(CCPoint from, CCPoint to);
	void DrawSegments();
	void DrawNextSegment(float dt);

	int num_key_points_;
	vector<CCPoint> key_points_;
	int last_key_point_;
	ccColor4F color_;
};

#endif // LIGHTNING_H_