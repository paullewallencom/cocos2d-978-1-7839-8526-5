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

// localhost/~Karan/ColourSmash/ColourSmash

var MAX_COLOURS = 4;                        // maximum number of colours we can use
var TILE_SIZE = 32;                         // size in points of each tile (same as tile.png)
var NUM_COLS = 14;                          // maximum number of columns
var NUM_ROWS = 20;                          // maximum number of rows
var GAMEPLAY_OFFSET = cc.p(TILE_SIZE/2, TILE_SIZE);          // offset so that game is not stuck to the bottom-left
var SCORE_PER_TILE = 10;                    // score when a tile is cleared
var BONUS = [50, 40, 30, 20, 10];           // number of tiles used to trigger bonuses eg. Bonus if 50 tiles collected in one shot

// define an object that we can use an enumeration for our colour types
var E_COLOUR_TYPE = {
    E_COLOUR_NONE:0,
    E_COLOUR_RED:1,
    E_COLOUR_GREEN:2,
    E_COLOUR_BLUE:3,
    E_COLOUR_YELLOW:4
};

var GameWorld = cc.Layer.extend({

    // member variable declarations
    // save screenSize for fast access
    screenSize:null,
    // array to represent the colour type for each tile
    tileData:null,
    // array to hold each tile's sprite
    tileSprites:null,
    // batch rendering
    spriteBatchNode:null,
    // arrays to support game logic
    tilesToRemove:null,
    tilesToShift:null,
    // score and time
    score:0,
    scoreLabel:null,
    time:0,
    timeLabel:null,
    // buttons and popups
    pauseButton:null,
    popup:null,
    isGameOver:false,

    init:function () {
        this._super();

        this.screenSize = cc.Director.getInstance().getWinSize();

        this.tilesToRemove = [];
        this.tilesToShift = [];
        this.createBackground();
        this.createTileData();
        this.createTileSprites();
        this.createHUD();
        this.doCountdownAnimation();

        return true;
    },

    createBackground:function(){
        // same as main menu
        var background = cc.LayerColor.create(cc.c4b(25, 0, 51, 255), this.screenSize.width, this.screenSize.height);
        this.addChild(background);

        // generate vertices for the gameplay frame
        var vertices = [];
        vertices[0] = cc.pAdd(GAMEPLAY_OFFSET, cc.p(-1, -1));
        vertices[1] = cc.pAdd(GAMEPLAY_OFFSET, cc.p(-1, (NUM_ROWS * TILE_SIZE)+1));
        vertices[2] = cc.pAdd(GAMEPLAY_OFFSET, cc.p((NUM_COLS * TILE_SIZE)+1, (NUM_ROWS * TILE_SIZE)+1));
        vertices[3] = cc.pAdd(GAMEPLAY_OFFSET, cc.p((NUM_COLS * TILE_SIZE)+1, -1));
        // use new DrawingPrimitive class
        var gamePlayFrame = cc.DrawNode.create();
        // pass vertices, fill colour, border width and border colour to get a nice bordered, coloured rectangle
        gamePlayFrame.drawPoly(vertices, cc.c4f(0.375, 0.375, 0.375, 1), 2, cc.c4f(0.4, 0, 0, 1));
        // must add the DrawNode else it won't be drawn at all
        this.addChild(gamePlayFrame);

        // label to show the title of the game
        var titleLabel = cc.LabelTTF.create("ColourSmash", "Comic Sans MS", 52);
        titleLabel.setPosition(cc.p(this.screenSize.width * 0.5, this.screenSize.height * 0.95));
        this.addChild(titleLabel);

        // menu containing a button to pause the game
        this.pauseButton = cc.MenuItemSprite.create(cc.Sprite.create(s_Pause));
        this.pauseButton.setCallback(this.onPauseClicked, this);
        this.pauseButton.setPosition(cc.p(this.screenSize.width * 0.9, this.screenSize.height * 0.95));
        this.pauseButton.setEnabled(false);
        var pauseMenu = cc.Menu.create(this.pauseButton);
        pauseMenu.setPosition(cc.POINT_ZERO);
        this.addChild(pauseMenu,1);
    },

    createTileData:function(){
        this.tileData = [];
        // generate tile data randomly
        for(var i = 0; i < (NUM_COLS * NUM_ROWS); ++i){
            this.tileData[i] = 1 + Math.floor(Math.random() * MAX_COLOURS);
        }
    },

    createTileSprites:function(){
        // create the batch node passing in path to the texture & initial capacity
        // initial capacity is slightly more than maximum number of sprites
        // this is because new tiles may be added before old tiles are removed
        this.spriteBatchNode = cc.SpriteBatchNode.create(s_Tile, NUM_COLS * NUM_ROWS + NUM_ROWS);
        this.addChild(this.spriteBatchNode);

        this.tileSprites = [];
        for(var i = 0; i < (NUM_COLS * NUM_ROWS); ++i){
            this.createTileSprite(i);
        }
    },

    createTileSprite:function(tileId){
        // create sprite with the image
        this.tileSprites[tileId] = cc.Sprite.create(s_Tile);
        // set colour based on the tile's data
        this.tileSprites[tileId].setColor(this.getColourForTile(this.tileData[tileId]));
        // set colour based on the tile's index
        this.tileSprites[tileId].setPosition(this.getPositionForTile(tileId));
        // save the index of the tile as user data
        this.tileSprites[tileId].setUserData(tileId);
        // add the sprite to the batch node
        this.spriteBatchNode.addChild(this.tileSprites[tileId]);
    },

    createHUD:function(){
        // initialise score and time
        this.score = 0;
        this.time = 60;

        // create labels for score and time
        this.scoreLabel = cc.LabelTTF.create("Score:" + this.score, "Comic Sans MS", 18);
        this.scoreLabel.setPosition(cc.p(this.screenSize.width * 0.33, this.screenSize.height * 0.875));
        this.addChild(this.scoreLabel);
        this.timeLabel = cc.LabelTTF.create("Time:" + this.time, "Comic Sans MS", 18);
        this.timeLabel.setPosition(cc.p(this.screenSize.width * 0.66, this.screenSize.height * 0.875));
        this.addChild(this.timeLabel);
    },

    doCountdownAnimation:function(){
        // create the four labels
        var labels = [];
        for(var i = 0; i < 4; ++i)
        {
            labels[i] = cc.LabelTTF.create("", "Comic Sans MS", 52);
            // position the label at the centre of the screen
            labels[i].setPosition(cc.p(this.screenSize.width/2, this.screenSize.height/2));
            // reduce opacity so that the label is invisible
            labels[i].setOpacity(0);
            // enlarge the label
            labels[i].setScale(3);
            this.addChild(labels[i]);
        }

        // assign strings
        labels[0].setString("3");
        labels[1].setString("2");
        labels[2].setString("1");
        labels[3].setString("Start");

        // fade in and scale down at the same time
        var fadeInScaleDown = cc.Spawn.create(cc.FadeIn.create(0.25), cc.EaseBackOut.create(cc.ScaleTo.create(0.25, 1)));
        // stay on screen for a bit
		var waitOnScreen = cc.DelayTime.create(0.75);
        // remove label and cleanup
        var removeSelf = cc.RemoveSelf.create(true);

        for(var i = 0; i < 4; ++i)
        {
            // since the labels should appear one after the other,
            // we give them increasing delays before they appear
            var delayBeforeAppearing = cc.DelayTime.create(i);
            var countdownAnimation = cc.Sequence.create(delayBeforeAppearing, fadeInScaleDown, waitOnScreen, removeSelf);
            labels[i].runAction(countdownAnimation);
        }

        // after the animation has finished, start the game
        var waitForAnimation = cc.DelayTime.create(4);
        var finishCountdownAnimation = cc.CallFunc.create(this.finishCountdownAnimation, this);
        this.runAction(cc.Sequence.create(waitForAnimation, finishCountdownAnimation));
    },

    finishCountdownAnimation:function(){
        // start executing the game timer
        this.schedule(this.updateTimer, 1);
		// finally allow the user to touch
        this.setTouchEnabled(true);
        this.pauseButton.setEnabled(true);
    },

    findTilesToRemove:function(col, row, tileColour){
        // first do bounds checking
        if(col < 0 || col >= NUM_COLS || row < 0 || row >= NUM_ROWS)
            return;

        // calculate the ID of the tile using col & row
        var tileId = row * NUM_COLS + col;

        // now check if tile is of required colour
        if(this.tileData[tileId] != tileColour)
            return;

        // check if tile is already saved
        if(this.tilesToRemove.indexOf(tileId) >= 0)
            return;

        // save the tile to be removed
        this.tilesToRemove.push(tileId);

        // check up
        this.findTilesToRemove(col, row+1, tileColour);

        // check down
        this.findTilesToRemove(col, row-1, tileColour);

        // check left
        this.findTilesToRemove(col-1, row, tileColour);

        // check right
        this.findTilesToRemove(col+1, row, tileColour);
    },

    findTilesToShift:function(){
        // first sort the tiles to be removed, in descending order
        this.tilesToRemove.sort(function(a, b){return b-a});

        // for each tile, bring down all the tiles belonging to the same column that are above the current tile
        for(var i = 0; i < this.tilesToRemove.length; ++i)
        {
            // calculate column and row for the current tile to be removed
            var col = Math.floor(this.tilesToRemove[i] % NUM_COLS);
            var row = Math.floor(this.tilesToRemove[i] / NUM_COLS);

            // iterate through each row above the current tile
            for(var j = row+1; j < NUM_ROWS; ++j)
            {
                // each tile gets the data of the tile exactly above it
                this.tileData[(j-1) * NUM_COLS + col] = this.tileData[j * NUM_COLS + col];
                // each tile now refers to the sprite of the tile exactly above it
                this.tileSprites[(j-1) * NUM_COLS + col] = this.tileSprites[j * NUM_COLS + col];
                // null checking...this sprite may have already been nullified by removeTilesWithAnimation
                if(this.tileSprites[(j-1) * NUM_COLS + col])
                {
                    // save the new index as user data
                    this.tileSprites[(j-1) * NUM_COLS + col].setUserData((j-1) * NUM_COLS + col);
                    // save this tile's sprite so that it is animated, but only if it hasn't already been saved
                    if(this.tilesToShift.indexOf(this.tileSprites[(j-1) * NUM_COLS + col]) == -1)
                        this.tilesToShift.push(this.tileSprites[(j-1) * NUM_COLS + col]);
                }
            }
            // after shifting the whole column down, the tile at the top of the column will be empty
            // set the data to -1...-1 means empty
            this.tileData[(NUM_ROWS-1) * NUM_COLS + col] = -1;
            // nullify the sprite's reference
            this.tileSprites[(NUM_ROWS-1) * NUM_COLS + col] = null;
        }
    },

    removeTilesWithAnimation:function(){
        for(var i = 0; i < this.tilesToRemove.length; ++i)
        {
            // first clear the tile's data
            this.tileData[this.tilesToRemove[i]] = E_COLOUR_TYPE.E_COLOUR_NONE;
            // the tile should scale down with easing and then remove itself
            this.tileSprites[this.tilesToRemove[i]].runAction(cc.Sequence.create(cc.EaseBackIn.create(cc.ScaleTo.create(0.25, 0.0)), cc.RemoveSelf.create(true)));
            // nullify the tile's sprite
            this.tileSprites[this.tilesToRemove[i]] = null;
        }
        // wait for the scale down animation to finish then bring down the tiles from above
        this.spriteBatchNode.runAction(cc.Sequence.create(cc.DelayTime.create(0.25), cc.CallFunc.create(this.bringDownTiles, this)));
    },

    bringDownTiles:function(){
        for(var i = 0; i < this.tilesToShift.length; ++i)
        {
            // the tiles should move to their new positions with an awesome looking bounce
            this.tilesToShift[i].runAction(cc.EaseBounceOut.create(cc.MoveTo.create(0.25, this.getPositionForTile(this.tilesToShift[i].getUserData()))));
        }
        // wait for the movement to finish then add new tiles
        this.spriteBatchNode.runAction(cc.Sequence.create(cc.DelayTime.create(0.25), cc.CallFunc.create(this.addNewTiles, this)));
    },

    addNewTiles:function(){
        // first search for all tiles having value -1...-1 means empty
        var emptyTileIndices = [], i = -1;
        while( (i = this.tileData.indexOf(-1, i+1)) != -1){
            emptyTileIndices.push(i);
        }

        // now create tile data and sprites
        for(var i = 0; i < emptyTileIndices.length; ++i)
        {
            // generate tile data randomly
            this.tileData[emptyTileIndices[i]] = 1 + Math.floor(Math.random() * MAX_COLOURS);
            // create tile sprite based on tile data
            this.createTileSprite(emptyTileIndices[i]);
        }

        // animate the entry of the sprites
        for(var i = 0; i < emptyTileIndices.length; ++i)
        {
            // set the scale to 0
            this.tileSprites[emptyTileIndices[i]].setScale(0);
            // scale the sprite up with a neat easing effect
            this.tileSprites[emptyTileIndices[i]].runAction(cc.EaseBackOut.create(cc.ScaleTo.create(0.125, 1)));
        }

        // the move has finally finished, do some cleanup
        this.cleanUpAfterMove();
    },

    cleanUpAfterMove:function(){
        // empty the arrays
        this.tilesToRemove = [];
        this.tilesToShift = [];
        // enable touch so the user can continue playing, but only if the game isn't over
        if(this.isGameOver == false)
            this.setTouchEnabled(true);
    },

    updateScore:function(point){
        // count the number of tiles the user just removed
        var numTiles = this.tilesToRemove.length;

        // calculate score for this move
        var scoreToAdd = numTiles * SCORE_PER_TILE;

        // check if a bonus has been achieved
        for(var i = 0; i < BONUS.length; ++i)
        {
            if(numTiles >= BONUS[i])
            {
                // add the bonus to the score for this move
                scoreToAdd += BONUS[i] * 20;
                break;
            }
        }

        // display the score for this move
        this.showScoreText(scoreToAdd, point);
        // add the score for this move to the total score
        this.score += scoreToAdd;
        // update the total score label
        this.scoreLabel.setString("Score:" + this.score);
        // run a simple action so the user knows the score is being added
        // use the ease functions to create a heart beat effect
        this.scoreLabel.runAction(cc.Sequence.create(cc.EaseSineIn.create(cc.ScaleTo.create(0.125, 1.1)), cc.EaseSineOut.create(cc.ScaleTo.create(0.125, 1))));
    },

    // this function can be used to display any message to the user
    // but we will use it to display the score for each move
    showScoreText:function(scoreToAdd, point){
        // create the label with the score & place it at the respective point
        var bonusLabel = cc.LabelTTF.create("+" + scoreToAdd, "Comic Sans MS", 32);
        bonusLabel.setPosition(point);
        // initially scale it down completely
        bonusLabel.setScale(0);
        // give it a yellow colour
        bonusLabel.setColor(cc.YELLOW);
        this.addChild(bonusLabel, 10);

        // animate the bonus label so that it scales up with a nice easing effect
        bonusLabel.runAction( cc.Sequence.create(cc.EaseBackOut.create(cc.ScaleTo.create(0.125, 1)),
            cc.DelayTime.create(1),                                     // it should stay on screen so the user can read it
            cc.EaseBackIn.create(cc.ScaleTo.create(0.125, 0)),          // scale it back down with a nice easing effect
            cc.RemoveSelf.create(true) ));                              // its task is finished, so remove it with cleanup
    },

    updateTimer:function(){
        // this is called every second so reduce the time left by 1
        this.time --;
        // update the time left label
        this.timeLabel.setString("Time:" + this.time);

        // the user's time is up
        if(this.time<= 0)
        {
            // game is now over
            this.isGameOver = true;
            // unschedule the timer
            this.unschedule(this.updateTimer);
            // stop animating the time label
            this.timeLabel.stopAllActions();
            // disable touch
            this.setTouchEnabled(false);
            // disable the pause button
            this.pauseButton.setEnabled(false);
            // display the game over popup
            this.showGameOverPopup();
        }
        else if(this.time == 5)
        {
            // get the user's attention...there are only 5 seconds left
            // make the timer label scale up and down so the user knows the game is about to end
            // use the ease functions to create a heart beat effect
            var timeUp = cc.Sequence.create(cc.EaseSineIn.create(cc.ScaleTo.create(0.125, 1.1)), cc.EaseSineOut.create(cc.ScaleTo.create(0.125, 1)));
            // repeat this action forever
            this.timeLabel.runAction(cc.RepeatForever.create(timeUp));
        }
    },

    onTouchesBegan:function (touches, event) {
        // get touch coordinates
        var touch = cc.p(touches[0].getLocation().x, touches[0].getLocation().y);
        // calculate touch within the grid
        var touchWithinGrid = cc.pSub(touch, GAMEPLAY_OFFSET);
        // calculate the column touched
        var col = Math.floor(touchWithinGrid.x / TILE_SIZE);
        // calculate the row touched
        var row = Math.floor(touchWithinGrid.y / TILE_SIZE);
        // calculate the id of the touched tile
        var touchedTile = row * NUM_COLS + col;

        // simple bounds checking to ignore touches outside of the grid
        if(col < 0 || col >= NUM_COLS || row < 0 || row >= NUM_ROWS)
            return;

        // disable touch so that the subsequent functions have time to execute
        this.setTouchEnabled(false);
        this.findTilesToRemove(col, row, this.tileData[touchedTile]);
        this.updateScore(touch);
        this.removeTilesWithAnimation();
        this.findTilesToShift();
    },

    onPauseClicked:function(){
        // pause the time label if it is animating
        this.timeLabel.pauseSchedulerAndActions();
        // disable touch
        this.setTouchEnabled(false);
        // this will pause all schedulers and actions associated with the GameWorld layer
        this.pauseSchedulerAndActions();
        // disable the pause button
        this.pauseButton.setEnabled(false);
        // display the pause popup
        this.showPausePopup();
    },

    getColourForTile:function(colourData) {
        // return a colour for the specified type
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

    getPositionForTile:function(id){
        // calculate column and row for given tile id
        var col = Math.floor(id % NUM_COLS);
        var row = Math.floor(id / NUM_COLS);
        // return point based on the column and row of the tile
        return cc.pAdd(GAMEPLAY_OFFSET, cc.p(col * TILE_SIZE + TILE_SIZE/2, row * TILE_SIZE + TILE_SIZE/2));
    },

    showPausePopup:function(){
        // create a black semi-transparent layer
        this.popup = cc.LayerColor.create(cc.c4b(0, 0, 0, 196), this.screenSize.width, this.screenSize.height);
        // set opacity so that it is not visible
        this.popup.setOpacity(0);
        // fade it in
        this.popup.runAction(cc.FadeTo.create(0.25, 196));
        this.addChild(this.popup, 10);

        // create the continue button
        var continueButton = cc.MenuItemLabel.create(cc.LabelTTF.create("Continue", "Comic Sans MS", 32), this.onContinueClicked, this);
        continueButton.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.6));

        // create the restart button
        var restartButton = cc.MenuItemLabel.create(cc.LabelTTF.create("Restart", "Comic Sans MS", 32), this.onRestartClicked, this);
        restartButton.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.5));

        // create the menu button
        var menuButton = cc.MenuItemLabel.create(cc.LabelTTF.create("Menu", "Comic Sans MS", 32), this.onMenuClicked, this);
        menuButton.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.4));

        // create the pause menu with the above three button
        var pauseMenu = cc.Menu.create(continueButton, restartButton, menuButton);
        pauseMenu.setPosition(cc.POINT_ZERO);
        this.popup.addChild(pauseMenu);

        // title to inform the user which popup this is
        var pausedLabel = cc.LabelTTF.create("Game Paused", "Comic Sans MS", 52);
        pausedLabel.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.75));
        this.popup.addChild(pausedLabel);
    },

    showGameOverPopup:function(){
        // create a black semi-transparent layer
        this.popup = cc.LayerColor.create(cc.c4b(0, 0, 0, 196), this.screenSize.width, this.screenSize.height);
        // set opacity so that it is not visible
        this.popup.setOpacity(0);
        // fade it in
        this.popup.runAction(cc.FadeTo.create(0.25, 196));
        this.addChild(this.popup, 10);

        // create the restart button
        var restartButton = cc.MenuItemLabel.create(cc.LabelTTF.create("Restart", "Comic Sans MS", 32), this.onRestartClicked, this);
        restartButton.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.5));

        // create the menu button
        var menuButton = cc.MenuItemLabel.create(cc.LabelTTF.create("Menu", "Comic Sans MS", 32), this.onMenuClicked, this);
        menuButton.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.4));

        // create the pause menu with the above three button
        var gameOverMenu = cc.Menu.create(restartButton, menuButton);
        gameOverMenu.setPosition(cc.POINT_ZERO);
        this.popup.addChild(gameOverMenu);

        // title to inform the user which popup this is
        var gameOverLabel = cc.LabelTTF.create("Game Over", "Comic Sans MS", 52);
        gameOverLabel.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.75));
        this.popup.addChild(gameOverLabel);

        // add a label to show the final score
        var scoreLabel = cc.LabelTTF.create("Score:" + this.score, "Comic Sans MS", 48);
        scoreLabel.setPosition(cc.p(this.screenSize.width*0.5, this.screenSize.height*0.6));
        // animate it with a nice heart beat effect to draw the user's attention
        scoreLabel.runAction(cc.Sequence.create(cc.DelayTime.create(0.5),
            cc.EaseSineIn.create(cc.ScaleTo.create(0.25, 1.1)),
            cc.EaseSineOut.create(cc.ScaleTo.create(0.25, 1))));
        this.popup.addChild(scoreLabel);
    },

    onContinueClicked:function(){
        // remove the popup
        this.popup.removeFromParent(true);
        this.popup = null;

        // resume GameWorld's schedulers and actions
        this.resumeSchedulerAndActions();
        // enable touch
        this.setTouchEnabled(true);
        // enable pause button
        this.pauseButton.setEnabled(true);
        // resume the time label's actions
        this.timeLabel.resumeSchedulerAndActions();
    },

    onRestartClicked:function(){
        // replace the scene with a new instance of GameWorldScene...and do this with a transition
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(0.5, new GameWorldScene()));
    },

    onMenuClicked:function(){
        // replace the scene with the MainMenuScene...and do this with a transition
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(0.5, new MainMenuScene()));
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

