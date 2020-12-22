#ifndef BACKGROUND_MANAGER_H_
#define BACKGROUND_MANAGER_H_

#include "GameGlobals.h"

class BackgroundManager : public CCNode
{
public:
	virtual bool init();
	~BackgroundManager(void){};

	CREATE_FUNC(BackgroundManager);
};

#endif // BACKGROUND_MANAGER_H_