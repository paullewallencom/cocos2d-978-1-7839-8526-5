#include "Blast.h"
#include "GameWorld.h"

Blast* Blast::createWithRadiusAndDuration(float radius, float duration)
{
	Blast* blast = new Blast();
	if(blast && blast->initWithRadiusAndDuration(radius, duration))
	{
		blast->autorelease();
		return blast;
	}
	CC_SAFE_DELETE(blast);
	return NULL;
}

bool Blast::initWithRadiusAndDuration(float radius, float duration)
{
	if(!CCDrawNode::init())
	{
		return false;
	}

	radius_ = radius;
	duration_ = duration;

	// initially scale down completely
	setScale(0.0f);
	drawDot(CCPointZero, radius_, ccc4f(1, 0.34118f, 0, 1));
	drawDot(CCPointZero, radius_ * 0.8f, ccc4f(1, 0.68235f, 0, 0.25f));
	drawDot(CCPointZero, radius_ * 0.75f, ccc4f(1, 0.68235f, 0, 0.5f));
	drawDot(CCPointZero, radius_ * 0.7f, ccc4f(1, 0.68235f, 0, 0.5f));
	drawDot(CCPointZero, radius_ * 0.6f, ccc4f(1, 0.83529f, 0.40392f, 0.25f));
	drawDot(CCPointZero, radius_ * 0.55f, ccc4f(1, 0.83529f, 0.40392f, 0.5f));
	drawDot(CCPointZero, radius_ * 0.5f, ccc4f(1, 0.83529f, 0.40392f, 0.5));
	drawDot(CCPointZero, radius_ * 0.4f, ccc4f(1, 1, 1, 0.25f));
	drawDot(CCPointZero, radius_ * 0.35f, ccc4f(1, 1, 1, 0.75f));
	drawDot(CCPointZero, radius_ * 0.3f, ccc4f(1, 1, 1, 1));
	// scale-up, then wait for 'duration_' amount of seconds before cooling down
	runAction(CCSequence::create(CCEaseSineOut::create(CCScaleTo::create(0.25f, 1.0f)), CCDelayTime::create(duration_), CCCallFunc::create(this, callfunc_selector(Blast::Cooldown)), NULL));

	return true;
}

void Blast::Cooldown()
{
	// remove this blast in the next iteration
	must_be_removed_ = true;
	// animate exit then remove with cleanup
	runAction(CCSequence::createWithTwoActions(CCEaseSineOut::create(CCScaleTo::create(0.5f, 0.0f)), CCRemoveSelf::create(true)));
}
