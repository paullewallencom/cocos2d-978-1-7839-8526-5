#include "GestureLayer.h"

GestureLayer::GestureLayer()
{
	target_ = NULL;
	handler_ = NULL;
	is_touch_active_ = false;
	touch_start_ = CCPointZero;
	touch_end_ = CCPointZero;
	gesture_type_ = E_GESTURE_NONE;
}

GestureLayer::~GestureLayer()
{}

GestureLayer* GestureLayer::create(CCObject* target, SEL_CallFuncO handler)
{
	GestureLayer* gesture_layer = new GestureLayer();
	if(gesture_layer && gesture_layer->init(target, handler))
	{
		gesture_layer->autorelease();
		return gesture_layer;
	}
	CC_SAFE_DELETE(gesture_layer);
	return NULL;
}

bool GestureLayer::init(CCObject* target, SEL_CallFuncO handler)
{
	if(!CCLayer::init())
	{
		return false;
	}

	if(target == NULL || handler == NULL)
	{
		CCLOGERROR("GestureLayer received invalid arguments");
		return false;
	}

	target_ = target;
	handler_ = handler;
	setTouchEnabled(true);
	return true;
}

void GestureLayer::ccTouchesBegan(CCSet* set, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);
	
	// first reset variables
	gesture_type_ = E_GESTURE_NONE;
	touch_end_ = CCPointZero;
	// start observing touch
	is_touch_active_ = true;
	// save first touch point
	touch_start_ = touch_point;
}

void GestureLayer::ccTouchesMoved(CCSet* set, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);
	
	// save subsequent touch
	touch_end_ = touch_point;
	HandleTouch();
}

void GestureLayer::ccTouchesEnded(CCSet* set, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)(*set->begin());
	CCPoint touch_point = touch->getLocationInView();
	touch_point = CCDirector::sharedDirector()->convertToGL(touch_point);

	// save subsequent touch
	touch_end_ = touch_point;
	HandleTouch();

	// stop observing touch
	is_touch_active_ = false;
}

void GestureLayer::HandleTouch()
{
	// don't do anything if not observing touch
	if(is_touch_active_ == false)
	{
		return;
	}

	// check for a single tap
	if(ccpFuzzyEqual(touch_start_, touch_end_, 1))
	{
		gesture_type_ = E_GESTURE_TAP;
		(target_->*handler_)(this);
		is_touch_active_ = false;
		return;
	}

	// calculate distance between first and last touch
	CCPoint touch_difference = ccpSub(touch_end_, touch_start_);
	// horizontal swipe
	if(fabs(touch_difference.x) > MIN_GESTURE_DISTANCE)
	{
		gesture_type_ = (touch_difference.x > 0) ? E_GESTURE_SWIPE_RIGHT : E_GESTURE_SWIPE_LEFT;
		(target_->*handler_)(this);
		is_touch_active_ = false;
		return;
	}
	// vertical swipe
	if(fabs(touch_difference.y) > MIN_GESTURE_DISTANCE)
	{
		gesture_type_ = (touch_difference.y > 0) ? E_GESTURE_SWIPE_UP : E_GESTURE_SWIPE_DOWN;
		(target_->*handler_)(this);
		is_touch_active_ = false;
		return;
	}
}