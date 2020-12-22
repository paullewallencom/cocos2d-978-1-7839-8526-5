#ifndef MAIN_MENU_H_
#define MAIN_MENU_H_

#include "GameGlobals.h"

class MainMenu : public CCLayer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static CCScene* scene();
    
    // implement the "static node()" method manually
    CREATE_FUNC(MainMenu);

	void CreateMenu();
	void CreatePenguin();
	void AnimatePenguin(float dt);

	void OnPlayClicked(CCObject* sender);

private:
	CCSprite* penguin_;
};

#endif // MAIN_MENU_H_