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

var MainMenu = cc.Layer.extend({

    screenSize:null,

    init:function () {
        this._super();

        this.screenSize = cc.Director.getInstance().getWinSize();

        // create a coloured layer as background
        var background = cc.LayerColor.create(cc.c4b(25, 0, 51, 255), this.screenSize.width, this.screenSize.height);
        this.addChild(background);

        // create a label to display the name of the game
        var titleLabel = cc.LabelTTF.create("ColourSmash", "Comic Sans MS", 64);
        titleLabel.setPosition(cc.p(this.screenSize.width * 0.5, this.screenSize.height * 0.8));
        this.addChild(titleLabel, 1);

        // create a play button to move to the game world
        var playButton = cc.MenuItemSprite.create(cc.Sprite.create(s_Play));
        playButton.setCallback(this.onPlayClicked, this);
        playButton.setPosition(cc.p(this.screenSize.width * 0.5, this.screenSize.height * 0.5));

        // create a menu that will contain the button above
        var menu = cc.Menu.create(playButton);
        menu.setPosition(0,0);
        this.addChild(menu, 1);

        this.doAnimation();
        this.schedule(this.doAnimation, 2);

        return true;
    },

    doAnimation:function(){
        var numTiles = Math.round(Math.random() * 30);
        for(var i = 0; i < numTiles; ++i)
        {
            var tile = cc.Sprite.create(s_Tile);
            tile.setColor(this.getColourForTile(1 + Math.floor(Math.random() * MAX_COLOURS)));
            tile.setPosition(this.getRandomPositionForTile());
            tile.setScale(0);
            this.addChild(tile);

            var waitBefore = cc.DelayTime.create(Math.random() * 5);
            var scaleUp = cc.EaseBackOut.create(cc.ScaleTo.create(0.125, 1));
            var waitAfter = cc.DelayTime.create(Math.random() * 5);
            var scaleDown = cc.EaseBackIn.create(cc.ScaleTo.create(0.125, 0));
            var removeSelf = cc.RemoveSelf.create(true);
            tile.runAction(cc.Sequence.create(waitBefore, scaleUp, waitAfter, scaleDown, removeSelf));
        }
    },

    getRandomPositionForTile:function() {
        return cc.p( Math.floor(1 + Math.random() * NUM_COLS) * TILE_SIZE, Math.floor(1 + Math.random() * (NUM_ROWS+5)) * TILE_SIZE );
    },

    getColourForTile:function(colourData) {
        switch(colourData)
        {
            case E_COLOUR_TYPE.E_COLOUR_RED:
                return cc.c3b(255, 128, 0);
                break;
            case E_COLOUR_TYPE.E_COLOUR_GREEN:
                return cc.c3b(0, 204, 0);
                break;
            case E_COLOUR_TYPE.E_COLOUR_BLUE:
                return cc.c3b(0, 76, 153);
                break;
            case E_COLOUR_TYPE.E_COLOUR_YELLOW:
                return cc.c3b(102, 0, 102);
                break;
        }
    },

    onPlayClicked:function(){
        // ask the director to change the running scene
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

