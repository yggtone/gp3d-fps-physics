#ifndef RENDERER_H_
#define RENDERER_H_

#include "Sprite.h"
#include "gameplay.h"
#include "GameModel.h"
#include "Npc.h"
#include <vector>

using namespace gameplay;
using namespace std;



class Renderer
{
public:


    Renderer(int worldWidth, int worldHeight,GameModel *gameModel);
    ~Renderer();



    /**
     * Compositing blitter with a specified material/technique applied from a source buffer into the destination buffer.
     *
     * If destination buffer is NULL then it composites to the default frame buffer.
     *
     * Requried uniforms:
     * sampler2d u_texture - The input texture sampler
     */
    class Compositor
    {
    public:

        static Compositor* create(FrameBuffer* srcBuffer, FrameBuffer* dstBuffer, const char* materialPath, const char* techniqueId);

        ~Compositor();

        FrameBuffer* getSrcFrameBuffer() const;

        FrameBuffer* getDstFrameBuffer() const;

        const char* getTechniqueId() const;

        Material* getMaterial() const;

        void blit(const Rectangle& dst);

    private:

        Compositor();

        Compositor(FrameBuffer* srcBuffer, FrameBuffer* dstBuffer, Material* material, const char* techniqueId);

        FrameBuffer* _srcBuffer;
        FrameBuffer* _dstBuffer;
        Material* _material;
        const char* _techniqueId;



    };
    /**__________________________________________________________
    *-----------------------------------------------
    */




    void load2D();

    void RenderSplash(float alpha);

    void RenderGame();

    void updateCursor(float x, float y)
    {
        cursorX = x;
        cursorY = y;
    };












private:


    GameModel *mGameModel;
    Scene *currentScene;


    float cursorX, cursorY;



    Compositor* compositor;
    FrameBuffer* _frameBuffer;
    static  Model* _quadModel;
    static  Material* _compositorMaterial;


    bool buildRenderQueues(Node* node);
    void drawSceneWithCulling();
    std::vector<Node*> _renderQueues[2];


    int worldWidth;
    int worldHeight;



    //2d assets
    Font *_font;
    Matrix spriteMatrix;
    Sprite * loadSplash;
    Sprite *scanLines;
    Sprite *noise;
    Sprite *cursor;

    Sprite *eye;

    Sprite *city;
    Sprite *street;
    Sprite *window;

    /**
     * Draws the scene each frame.
     */
    bool drawScene(Node* node);


    bool useCompositor;


protected:
    //void _renderQueues();
};



#endif
