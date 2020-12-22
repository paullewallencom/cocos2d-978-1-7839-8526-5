#ifndef GESTURE_LAYER_H_
#define GESTURE_LAYER_H_

#include "GameGlobals.h"

class GestureLayer : public CCLayer
{
public:
	GestureLayer();
	~GestureLayer();

	static GestureLayer* create(CCObject* target, SEL_CallFuncO handler);

	virtual bool init(CCObject* target, SEL_CallFuncO handler);

	// touch listeners
	virtual void ccTouchesBegan(CCSet* set, CCEvent* event);
	virtual void ccTouchesMoved(CCSet* set, CCEvent* event);
	virtual void ccTouchesEnded(CCSet* set, CCEvent* event);

	// accessors and mutators
	inline CCPoint GetTouchStart() { return touch_start_; }
	inline CCPoint GetTouchEnd() { return touch_end_; }
	inline EGestureType GetGestureType() { return gesture_type_; }

protected:
	void HandleTouch();

	// target to pass the gesture event to
	CCObject* target_;
	// function to call when gesture event occurs
	SEL_CallFuncO handler_;
	// member variables
	bool is_touch_active_;
	CCPoint touch_start_;
	CCPoint touch_end_;
	EGestureType gesture_type_;
};

#endif // GESTURE_LAYER_H_