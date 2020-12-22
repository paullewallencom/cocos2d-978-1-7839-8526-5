#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include "GameGlobals.h"

class Environment : public CCNode
{
public:
	Environment();
	~Environment();

	static Environment* create();
	virtual bool init();

	void CreateSky();
	void CreateCloud(float duration, const char* frame_name);
	void CreateHills();
	void CreateBoat();
	void CreatePier();
};

#endif		// ENVIRONMENT_H_