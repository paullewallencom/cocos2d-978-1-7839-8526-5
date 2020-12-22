#include "Lightning.h"

Lightning::Lightning()
{
	num_key_points_ = 0;
	key_points_.clear();
	last_key_point_ = 0;
}

Lightning::~Lightning()
{}

Lightning* Lightning::create(CCPoint from, CCPoint to, ccColor4F color, bool is_animated)
{
	Lightning* lightning = new Lightning();
	if(lightning && lightning->init(from, to, color, is_animated))
	{
		lightning->autorelease();
		return lightning;
	}
	CC_SAFE_DELETE(lightning);
	return NULL;
}

bool Lightning::init(CCPoint from, CCPoint to, ccColor4F color, bool is_animated)
{
	if(!CCDrawNode::init())
	{
		return false;
	}

	color_ = color;
	GenerateKeyPoints(from , to);
	if(!is_animated)
	{
		DrawSegments();
	}
	else
	{
		schedule(schedule_selector(Lightning::DrawNextSegment));
	}

	return true;
}

void Lightning::GenerateKeyPoints(CCPoint from, CCPoint to)
{
	// how many key points do we need?
	float distance = ccpDistance(from, to);
	num_key_points_ = (int)(distance / LIGHTNING_KEY_POINT_DIST);

	CCPoint next_point = CCPointZero;
	// calculate the difference between two key points
	CCPoint delta = ccp( (to.x - from.x) / num_key_points_, (to.y - from.y) / num_key_points_ );
	for(int i = 0; i < num_key_points_; ++i)
	{
		// add the delta
		next_point = ccpAdd(from, ccpMult(delta, i));
		// randomise the delta
		next_point.x += LIGHTNING_KEY_POINT_DIST * CCRANDOM_MINUS1_1();
		next_point.y += LIGHTNING_KEY_POINT_DIST * CCRANDOM_MINUS1_1();
		// save the key point
		key_points_.push_back(next_point);
	}	
}

void Lightning::DrawSegments()
{
	// draw all segments at once
	for(int i = 0; i < num_key_points_ - 1; ++i)
	{
		drawSegment(key_points_[i], key_points_[i+1], 6, color_);
	}
}

void Lightning::DrawNextSegment(float dt)
{
	// draw one segment at a time
	if(++ last_key_point_ >= num_key_points_ - 2)
	{
		unschedule(schedule_selector(Lightning::DrawNextSegment));
	}

	drawSegment(key_points_[last_key_point_], key_points_[last_key_point_+1], 6, color_);
}