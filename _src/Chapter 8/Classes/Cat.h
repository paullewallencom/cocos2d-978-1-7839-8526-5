#ifndef CAT_H_
#define CAT_H_

#include "GameObject.h"

class Cat : public GameObject
{
public:
	Cat() : max_impulse_(0.0f), is_dying_(false)
	{
		type_ = E_GAME_OBJECT_CAT;
	}
	~Cat();

	static Cat* create(GameWorld* game_world);

	virtual bool init(GameWorld* game_world);
	virtual void Update();
	virtual void Die();
	virtual void Dead();

	inline void SetMaxImpulse(float max_impulse) { max_impulse_ = (max_impulse_ > max_impulse) ? max_impulse_ : max_impulse; }

protected:
	float max_impulse_;
	bool is_dying_;
};

#endif		// CAT_H_