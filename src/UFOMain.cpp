#include "UFOMain.h"

using namespace gameplay;

// Declare our game instance
UFOMain game;

UFOMain::UFOMain()
    :mGameModel(NULL), mRenderer(NULL)
{
    loadSplashAlpha = 1.f;
    splashAlphaTarget = loadSplashAlpha;
    setImmediateState(INIT);
}



void UFOMain::initialize()
{
    Logger::setEnabled(Logger::LEVEL_INFO, false);
    Logger::setEnabled(Logger::LEVEL_WARN, false);
    setVsync(true);

    setMouseCaptured(true);

    //setCursorVisible(false);
    setCursorVisible(false);
    //setCursorVisible(true);
    //setCursorVisible(false);
    //setMouseCaptured(false);//rerelease for using cursors


    fixedAspectRatio = false;
    pixelWidth = 256.f;
    pixelHeight = 256.f;
    Rectangle worldSize(pixelWidth, pixelHeight);


    static const float screenAspectRatio = (float)getHeight() / (float)getWidth();
    static const float worldAspectRatio = worldSize.height / worldSize.width;
    //center viewport leaving letterbox/pillarbox space if aspect ratio is fixed
    if(fixedAspectRatio)
    {
        if(screenAspectRatio >= worldAspectRatio )//letterbox
        {
            //float letterboxedHeight = (worldSize.width * screenAspectRatio);
            float newWorldHeight = (float)getWidth() * worldAspectRatio;
            float letterBoxHeight = .5f * (getHeight() - newWorldHeight );
            this->setViewport( Rectangle(0, letterBoxHeight, (float)getWidth(), newWorldHeight) );
        }
        else if(screenAspectRatio < worldAspectRatio) //pillarbox
        {
            float newWorldWidth = (float)getHeight() * 1.f/worldAspectRatio;
            float pillarBoxPositionX = .5f * (getWidth() - newWorldWidth );
            this->setViewport( Rectangle(pillarBoxPositionX, 0, newWorldWidth, (float)getHeight() ) );

        }
    }



    //start main model for internal logic
    mGameModel = new GameModel(this->getPhysicsController());
    //mGameModel->mPhysicsController = this->getPhysicsController();
    //start renderer with only loadscreen/base resources
    //pass reference to game model, which will have access to scene for drawing
    mRenderer = new Renderer(pixelWidth, pixelHeight, mGameModel);



    //load main scene, 3D assets, animations, physics, nodes
    //mGameModel->loadScene();
    //load textures for sprites and overlays
    mRenderer->load2D();



    
    setNextState(RUN);



}

void UFOMain::finalize()
{
    //TODO
    SAFE_DELETE(mRenderer);
    SAFE_DELETE(mGameModel);
}

void UFOMain::update(float elapsedTime)
{
    float elapsed = elapsedTime / 1000.f;
    if(elapsed > 0.07f)
        elapsed = 0.07f;
    if(mGameModel)
        mGameModel->update(elapsed);

   //stateUpdates();
}

void UFOMain::stateUpdates()
{
    //update load screen fading, and change states when load screen becomes opaque

    if( NextGameState != GameState )
    {
        if( std::fabs(loadSplashAlpha - splashAlphaTarget) < 0.01f )
        {
            //printf("alpha: %f",loadSplashAlpha);
            loadSplashAlpha = splashAlphaTarget;
            mRenderer->RenderSplash(loadSplashAlpha);

            /*
            if(NextGameState == LOADA)
            {
                mGameModel->LoadAudio();
                mRenderer->loadGame();
                setImmediateState(RUN);
            }
            */

            GameState = NextGameState;
            splashAlphaTarget = 0;
        }

    }
}


void UFOMain::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);


    mRenderer->RenderGame();

}


void UFOMain::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if(!mGameModel)
        return;

    if (evt == Keyboard::KEY_PRESS)
    {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            mGameModel->unloadScene();
            exit();
            break;
        //capital versions needed because of type-handling when holding shift
        case Keyboard::KEY_W:
            mGameModel->upKeyDown = true;
            break;
        case Keyboard::KEY_CAPITAL_W:
            mGameModel->upKeyDown = true;
            break;
        case Keyboard::KEY_A:
            mGameModel->leftKeyDown = true;
            break;
        case Keyboard::KEY_CAPITAL_A:
            mGameModel->leftKeyDown = true;
            break;
        case Keyboard::KEY_S:
            mGameModel->downKeyDown = true;
            break;
        case Keyboard::KEY_CAPITAL_S:
            mGameModel->downKeyDown = true;
            break;
        case Keyboard::KEY_D:
            mGameModel->rightKeyDown = true;
            break;
        case Keyboard::KEY_CAPITAL_D:
            mGameModel->rightKeyDown = true;
            break;
        case Keyboard::KEY_CTRL:
            mGameModel->duckPressed();
            break;
        case Keyboard::KEY_SHIFT:
            mGameModel->runKeyDown = true;
            break;
        case Keyboard::KEY_F:
            mGameModel->flashlightButton();
            break;
        case Keyboard::KEY_CAPITAL_F:
            mGameModel->flashlightButton();
            break;
        case Keyboard::KEY_RETURN:
            if(mGameModel->introTimer > 0.f)
                mGameModel->introTimer = 1.f;
            break;
        }
    }
    else if(evt == Keyboard::KEY_RELEASE)
    {
        switch (key)
        {

        case Keyboard::KEY_W:
            mGameModel->upKeyDown = false;
            break;
        case Keyboard::KEY_CAPITAL_W:
            mGameModel->upKeyDown = false;
            break;
        case Keyboard::KEY_A:
            mGameModel->leftKeyDown = false;
            break;
        case Keyboard::KEY_CAPITAL_A:
            mGameModel->leftKeyDown = false;
            break;
        case Keyboard::KEY_S:
            mGameModel->downKeyDown = false;
            break;
        case Keyboard::KEY_CAPITAL_S:
            mGameModel->downKeyDown = false;
            break;
        case Keyboard::KEY_D:
            mGameModel->rightKeyDown = false;
            break;

        case Keyboard::KEY_CAPITAL_D:
            mGameModel->rightKeyDown = false;
            break;
        case Keyboard::KEY_CTRL:
            mGameModel->duckReleased();
            break;
        case Keyboard::KEY_SHIFT:
            mGameModel->runKeyDown = false;
            break;
        }
    }
}

void UFOMain::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    if(!mGameModel)
        return;

    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        //mGameModel->upKeyDown = true;
        break;
    case Touch::TOUCH_RELEASE:
        //mGameModel->upKeyDown = false;
        break;
    case Touch::TOUCH_MOVE:
        mGameModel->camMove(y * .1f,x * .1f);
        break;
    };
}

bool UFOMain::mouseEvent(Mouse::MouseEvent evt, int x, int y, int wheelDelta)
{
    if(!mGameModel)
        return true;

    if(evt == Mouse::MOUSE_MOVE )
    {
        /*
        if(_scene)
        {
            //_scene->getActiveCamera()->getNode()->setRotation((float)x * .001f,0,0,0);
            //_scene->getActiveCamera()->getNode()->rotateX( .001f);

            //_camRoot->rotateY(-yaw);
            //_camPitch->rotateX(pitch);

            //_camPitch->rotateY((mouseLastX - (float)x ) * .001f);
            //_camPitch->rotateX(( mouseLastY - (float)y) * .001f);

            _camRoot->rotateY(( -(float)x ) * .001f);
            _camPitch->rotateX((  -(float)y) * .001f);
        }
        //if(mRenderer)

        //    mRenderer->updateCursor(x,y);
        //mouseLastX = (float)x;
        //mouseLastY = (float)y;
        */


        mGameModel->camMove(y,x);

    }
    else if(evt == Mouse::MOUSE_PRESS_LEFT_BUTTON )

    {
        //if(mRenderer)

        //    mGameModel->screenClick(x,y);

        mGameModel->primaryAction();


        //mGameModel->upKeyDown = true;


    }
    else if(evt == Mouse::MOUSE_RELEASE_LEFT_BUTTON )
    {

        //mGameModel->upKeyDown = false;
        mGameModel->leftMouseRelease();

    }


    if(evt == Mouse::MOUSE_PRESS_RIGHT_BUTTON )
    {
        //if(mRenderer)

        //    mGameModel->screenClick(x,y);

        mGameModel->rightMouseDown();


        //mGameModel->upKeyDown = true;


    }


    return true;
    //return gameplay::Game::mouseEvent(evt, x, y, wheelDelta);

}


