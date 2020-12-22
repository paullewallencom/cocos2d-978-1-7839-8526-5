#ifndef BLAST_H_
#define BLAST_H_

#include "GameGlobals.h"

class GameWorld;

class Blast : public CCDrawNode
{
public:
	Blast() : radius_(0.0f), duration_(0.0f), must_be_removed_(false) {};
	virtual ~Blast(){};

	static Blast* createWithRadiusAndDuration(float radius, float duration);

	virtual bool initWithRadiusAndDuration(float radius, float duration);
	void Cooldown();

	CC_SYNTHESIZE(float, radius_, Radius);
	CC_SYNTHESIZE(float, duration_, Duration);
	CC_SYNTHESIZE(bool, must_be_removed_, MustBeRemoved);
};

#endif // BLAST_H_