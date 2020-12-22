#ifndef __MAINMENU_H__
#define __MAINMENU_H__

#include "GameGlobals.h"

class BackgroundManager;

class MainMenu : public CCLayer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static CCScene* scene();
    
    // implement the "static node()" method manually
    CREATE_FUNC(MainMenu);

	virtual void didAccelerate(CCAcceleration* pAccelerationValue);
	void OnPlayClicked(CCObject* sender);

private:
	BackgroundManager* background_;
};

#endif // __MAINMENU_H__
