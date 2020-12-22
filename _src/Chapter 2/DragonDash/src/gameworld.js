/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2008-2010 Ricardo Quesada
 Copyright (c) 2011      Zynga Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

// localhost/DragonDash/DragonDash

var GRAVITY = -0.75;
var HIGHSCORE_KEY = "DragonDashHighScore";

var E_ZORDER = {
    E_LAYER_BG:0,
    E_LAYER_STARS:2,
    E_LAYER_SILHOUETTE:4,
    E_LAYER_CASTLE:6,
    E_LAYER_TOWER:8,
    E_LAYER_PLAYER:10,
    E_LAYER_HUD:12,
    E_LAYER_POPUPS:14
};

var GameWorld = cc.Layer.extend({
    // variables
    screenSize:null,
    spriteBatchNode:null,
    score:0,
    scoreLabel:null,
    mustAddScore:false,
    tutorialSprite:null,
    popup:null,
    castleRoof:0,
    hasGameStarted:false,
    // managers
    towerManager:null,
    dragonManager:null,
    fairytaleManager:null,

    init:function () {
        this._super();
        // enable touch
        this.setTouchEnabled(true);
        // store screen size for fast access
        this.screenSize = cc.Director.getInstance().getWinSize();

        // create and add the batch node
        this.spriteBatchNode = cc.SpriteBatchNode.create(s_SpriteSheetImg, 256);
        this.addChild(this.spriteBatchNode, E_ZORDER.E_LAYER_BG + 1);

        // set the roof of the castle
        this.castleRoof = 100;
        // create & init all managers
        this.towerManager = new TowerManager(this);
        this.towerManager.init();
        this.dragonManager = new DragonManager(this);
        this.dragonManager.init();
        this.fairytaleManager = new FairytaleManager(this);
        this.fairytaleManager.init();
        this.createHUD();

        this.scheduleUpdate();
        return true;
    },

    createHUD:function() {
        this.score = 0;
        // create labels for score
        this.scoreLabel = cc.LabelTTF.create(this.score + "", "Comic Sans MS", 120);
        this.scoreLabel.setPosition(cc.p(this.screenSize.width * 0.5, this.screenSize.height * 0.875));
        this.addChild(this.scoreLabel, E_ZORDER.E_LAYER_HUD);

        // create the tutorial sprite and add it to the batch node
        this.tutorialSprite = cc.Sprite.createWithSpriteFrameName("dhtap");
        this.tutorialSprite.setPosition(cc.p(this.screenSize.width * 0.5, this.screenSize.height * 0.5));
        this.spriteBatchNode.addChild(this.tutorialSprite);
    },

    update:function(deltaTime) {
        // update dragon
        this.dragonManager.update();
        // update towers only after game has started
        if(this.hasGameStarted)
            this.towerManager.update();
        // update environment
        this.fairytaleManager.update();
        this.checkCollisions();
    },

    checkCollisions:function() {
        // first find out which tower is right in front
        var frontTower = this.towerManager.getFrontTower();

        // fetch the bounding boxes of the respective sprites
        var dragonAABB = this.dragonManager.dragonSprite.getBoundingBox();
        var lowerTowerAABB = frontTower.lowerSprite.getBoundingBox();
        var upperTowerAABB = frontTower.upperSprite.getBoundingBox();

        // if the respective rects intersect, we have a collision
        if(cc.rectIntersectsRect(dragonAABB, lowerTowerAABB) || cc.rectIntersectsRect(dragonAABB, upperTowerAABB))
        {
            // dragon must die
            this.dragonManager.dragonDeath();
            // stop the update loop
            this.unscheduleUpdate();
        }
        else if( Math.abs(cc.rectGetMidX(lowerTowerAABB) - cc.rectGetMidX(dragonAABB)) <= MAX_SCROLLING_SPEED/2 )
        {
            // increment score once the dragon has crossed the tower
            this.incrementScore();
        }
    },

    incrementScore:function() {
        ++ this.score;
        this.scoreLabel.setString(this.score + "");
        // run a simple action so the user knows the score is being added
        // use the ease functions to create a heart beat effect
        this.scoreLabel.runAction(cc.Sequence.create(cc.EaseSineIn.create(cc.ScaleTo.create(0.125, 1.2)), cc.EaseSineOut.create(cc.ScaleTo.create(0.125, 1))));
    },

    onGameOver:function() {
        this.showGameOverPopup();
    },

    onTouchesBegan:function (touches, event) {
        this.hasGameStarted = true;
        // remove the tutorial only if it exists
        if(this.tutorialSprite)
        {
            // fade it out and then remove it
            this.tutorialSprite.runAction(cc.Sequence.create(cc.FadeOut.create(0.25), cc.RemoveSelf.create(true)));
            this.tutorialSprite = null;
        }
        // inform DragonManager that the game has started
        this.dragonManager.onGameStart();
        // fly dragon...fly!!!
        this.dragonManager.dragonFlap();
    },

    showGameOverPopup:function(){
        this.popup = cc.LayerColor.create(cc.c4b(0, 0, 0, 196), this.screenSize.width, this.screenSize.height);
        this.popup.setOpacity(0);
        this.popup.runAction(cc.FadeTo.create(0.25, 196));
        this.addChild(this.popup, E_ZORDER.E_LAYER_POPUPS);

        var restartButton = cc.MenuItemSprite.create(cc.Sprite.createWithSpriteFrameName("dhplay"));
        restartButton.setCallback(this.onRestartClicked, this);
        restartButton.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.25));

        var gameOverMenu = cc.Menu.create(restartButton);
        gameOverMenu.setPosition(cc.POINT_ZERO);
        this.popup.addChild(gameOverMenu);

        var gameOverSprite = cc.Sprite.createWithSpriteFrameName("dhgover");
        gameOverSprite.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.75));
        this.popup.addChild(gameOverSprite);

        var scoreLabel = cc.LabelTTF.create("Score:" + this.score, "Comic Sans MS", 60);
        scoreLabel.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.6));
        scoreLabel.runAction(cc.Sequence.create(cc.DelayTime.create(0.5),
            cc.EaseSineIn.create(cc.ScaleTo.create(0.25, 1.1)),
            cc.EaseSineOut.create(cc.ScaleTo.create(0.25, 1))));
        this.popup.addChild(scoreLabel);

        // fetch old high score from browser's local storage
        var oldHighScore = parseInt(sys.localStorage.getItem(HIGHSCORE_KEY));

        var highScoreLabel = cc.LabelTTF.create("Your Best:" + oldHighScore, "Comic Sans MS", 60);
        highScoreLabel.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.5));
        this.popup.addChild(highScoreLabel);

        // check if new high score has been achieved
        if(this.score > oldHighScore)
        {
            // save the new high score
            sys.localStorage.setItem(HIGHSCORE_KEY, this.score+"");

            // animate the button suggesting that a new high score has been achieved
            highScoreLabel.runAction(cc.Sequence.create(cc.DelayTime.create(1),
                cc.EaseSineIn.create(cc.ScaleTo.create(0.25, 1.1)),
                cc.CallFunc.create( function(nodeExecutingAction, data){ nodeExecutingAction.setString("Your Best:" + this.score); }, this ),
                cc.EaseSineOut.create(cc.ScaleTo.create(0.25, 1))));
        }
    },

    onRestartClicked:function(){
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(0.5, new GameWorldScene()));
    }
});

var GameWorldScene = cc.Scene.extend({
    onEnter:function () {
        this._super();
        var layer = new GameWorld();
        layer.init();
        this.addChild(layer);
    }
});

