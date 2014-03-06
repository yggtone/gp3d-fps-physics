
#ifndef TEMPLATEGAME_H_
#define TEMPLATEGAME_H_

#include "gameplay.h"
#include "Renderer.h"
#include "GameModel.h"

using namespace gameplay;

/**
 * Main game class.
 */
class UFOMain: public Game
{
public:





    /**
     * Constructor.
     */
    UFOMain();

    /**
     * @see Game::keyEvent
     */
    void keyEvent(Keyboard::KeyEvent evt, int key);

    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

    bool mouseEvent(Mouse::MouseEvent evt, int x, int y, int wheelDelta);

protected:

    /**
     * @see Game::initialize
     */
    void initialize();

    /**
     * @see Game::finalize
     */
    void finalize();

    /**
     * @see Game::update
     */
    void update(float elapsedTime);

    /**
     * @see Game::render
     */
    void render(float elapsedTime);




private:




    GameModel *mGameModel;
    Renderer *mRenderer;


    void play(const char* id, bool repeat, float speed = 1.0f);

    //pixel,world(area within letterbox) dimensions, independent from screen/render resolution
    float pixelWidth, pixelHeight;
    //whether viewport uses fixed (letterbox capable) or stretched display
    bool fixedAspectRatio;
    /*
    Compositor* compositor;
    FrameBuffer* _frameBuffer;

    static  Model* _quadModel;
    static  Material* _compositorMaterial;

    Animation* _animation;
    AnimationClip* _currentClip;
    AnimationClip* _clipa;
    AnimationClip* _clipb;

    SpriteBatch* batch;

    PhysicsCharacter* _charController;

    Node* _camPitch;
    Node* _camRoot;
    */
    float mouseLastX, mouseLastY;








    enum GAMESTATETYPE
    {
        INIT,
        LOAD,
        RUN,
        CLEANUP

    };

    float loadSplashAlpha;
    float splashAlphaTarget;

    void setNextState(GAMESTATETYPE newState)
    {
        NextGameState = newState;
        splashAlphaTarget = 1.f;
    };

    void setImmediateState(GAMESTATETYPE newState)
    {
        NextGameState = newState;
        GameState = newState;
    };

    void stateUpdates();

    GAMESTATETYPE GameState, NextGameState;



};

#endif
