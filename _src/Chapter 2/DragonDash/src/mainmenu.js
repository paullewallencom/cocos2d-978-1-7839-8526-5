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

var MainMenu = cc.Layer.extend({
    // variables
    screenSize:null,
    spriteBatchNode:null,
    fairytaleManager:null,

    init:function () {
        this._super();

        // set default value for high score
        // this will be executed only the first time the game is launched
        // local storage stores data persistently
        if(sys.localStorage.getItem(HIGHSCORE_KEY) == null)
            sys.localStorage.setItem(HIGHSCORE_KEY, "0");

        // preload audio and sprite sheet
        cc.AudioEngine.getInstance().preloadEffect(s_Flap_mp3);
        cc.AudioEngine.getInstance().preloadEffect(s_Crash_mp3);
        cc.SpriteFrameCache.getInstance().addSpriteFrames(s_SpriteSheetPlist);

        // store screen size for fast access
        this.screenSize = cc.Director.getInstance().getWinSize();

        // create and add the batch node
        this.spriteBatchNode = cc.SpriteBatchNode.create(s_SpriteSheetImg, 256);
        this.addChild(this.spriteBatchNode, E_ZORDER.E_LAYER_BG + 1);

        // create and init the environment
        this.fairytaleManager = new FairytaleManager(this);
        this.fairytaleManager.init();

        this.addDragonAnimation();
        this.addDragon();

        // add the game's title
        var titleSprite = cc.Sprite.createWithSpriteFrameName("dhtitle");
        titleSprite.setPosition(cc.p(this.screenSize.width * 0.5, this.screenSize.height * 0.75));
        this.spriteBatchNode.addChild(titleSprite, 1);

        // add the play button
        var playButton = cc.MenuItemSprite.create(cc.Sprite.createWithSpriteFrameName("dhplay"));
        playButton.setCallback(this.onPlayClicked, this);
        playButton.setPosition(cc.p(this.screenSize.width * 0.5, this.screenSize.height * 0.25));

        // add the menu
        var menu = cc.Menu.create(playButton);
        menu.setPosition(0,0);
        this.addChild(menu, 1);

        // start ticking
        this.scheduleUpdate();
        return true;
    },

    addDragon:function(){
        // create sprite and add to the sprite batch node
        var dragonSprite = cc.Sprite.createWithSpriteFrameName("dhch_1");
        dragonSprite.setPosition(cc.p(this.screenSize.width * 0.2, this.screenSize.height * 0.5));
        this.spriteBatchNode.addChild(dragonSprite, E_ZORDER.E_LAYER_PLAYER);

        // fetch flying animation from cache & repeat it on the dragon's  sprite
        var animation = cc.AnimationCache.getInstance().getAnimation("dragonFlying");
        dragonSprite.runAction(cc.RepeatForever.create(cc.Animate.create(animation)));

        // create a hover movement and repeat it on the dragon's sprite
        var flySequence = cc.Sequence.create(cc.EaseSineOut.create(cc.MoveBy.create(animation.getDuration()/2, cc.p(0, 10))), cc.EaseSineOut.create(cc.MoveBy.create(animation.getDuration()/2, cc.p(0, -10))));
        dragonSprite.runAction(cc.RepeatForever.create(flySequence));
    },

    addDragonAnimation:function(){
        // push the frames that will make up the dragon's flying animation
        var spriteFrames = [];
        spriteFrames.push(cc.SpriteFrameCache.getInstance().getSpriteFrame("dhch_1"));
        spriteFrames.push(cc.SpriteFrameCache.getInstance().getSpriteFrame("dhch_2"));
        spriteFrames.push(cc.SpriteFrameCache.getInstance().getSpriteFrame("dhch_3"));
        spriteFrames.push(cc.SpriteFrameCache.getInstance().getSpriteFrame("dhch_2"));
        spriteFrames.push(cc.SpriteFrameCache.getInstance().getSpriteFrame("dhch_1"));

        // create the animation with the array of sprite frames and delay per frame
        var dragonAnimation = cc.Animation.create(spriteFrames, 0.1);
        // add the created animation to the cache with a name so it can be reused
        cc.AnimationCache.getInstance().addAnimation(dragonAnimation, "dragonFlying");
    },

    update:function(deltaTime){
        this.fairytaleManager.update();
    },

    onPlayClicked:function(){
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(0.5, new GameWorldScene()));
    }
});

var MainMenuScene = cc.Scene.extend({
    onEnter:function () {
        this._super();
        var layer = new MainMenu();
        layer.init();
        this.addChild(layer);
    }
});

