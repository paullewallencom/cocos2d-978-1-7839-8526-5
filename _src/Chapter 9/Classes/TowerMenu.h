#ifndef TOWER_MENU_H_
#define TOWER_MENU_H_

#include "GameGlobals.h"

class GameWorld;

class TowerMenu : public CCNode
{
public:
	TowerMenu();
	~TowerMenu();
	
	static TowerMenu* create(GameWorld* game_world);

	virtual bool init(GameWorld* game_world);
	void CreatePlacementMenu();
	void CreateMaintenanceMenu();

	void ShowPlacementMenu(CCPoint position);
	void HidePlacementMenu();
	void EnablePlacementButtons();

	void ShowMaintenanceMenu(CCPoint position, int tower_index, int tower_type, int tower_level);
	void HideMaintenanceMenu();
	void EnableMaintenanceButtons(int tower_type, int tower_level);

	CCNode* placement_node_;
	CCNode* maintenance_node_;
protected:
	CCMenuItemSprite* CreateButton(const char* sprite_name, CCPoint position, int tag, SEL_MenuHandler handler);
	CCLabelTTF* CreateLabel(int content, const char* font, float size, CCPoint position);

	GameWorld* game_world_;

	vector<CCMenuItemSprite*> placement_buttons_;
	CCMenuItemSprite* upgrade_button_;
	CCLabelTTF* upgrade_label_;
	CCMenuItemSprite* sell_button_;
	CCLabelTTF* sell_label_;
};

#endif // TOWER_MENU_H_