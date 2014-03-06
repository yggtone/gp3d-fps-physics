#include "Renderer.h"

#include "GameModel.h"

#include "Sprite.h"
#include "gameplay.h"
#include <vector>

//384 224 converted in shader
#define FRAMEBUFFER_WIDTH 256
#define FRAMEBUFFER_HEIGHT 256


using namespace gameplay;
using namespace std;

//





//TODO initialization list
Renderer::Renderer ( int worldWidth, int worldHeight,GameModel *gameModel )
    :currentScene ( NULL ), _font ( NULL ), loadSplash ( NULL ), scanLines ( NULL ), noise ( NULL ), eye(NULL), useCompositor(false)
{

    this->mGameModel = gameModel;

    this->worldHeight = worldHeight;
    this->worldWidth = worldWidth;


    Rectangle worldSize ( worldWidth, worldHeight );
    //create projection and view matrix for sprite batches, so coordinates will be in world space rather than screen resolution
    Matrix projectionMatrix;
    float halfWidth = worldSize.width / 2.f, halfHeight = worldSize.height / 2.f;

    Matrix::createOrthographicOffCenter ( -halfWidth, halfWidth, -halfHeight, halfHeight, 0, 1, &projectionMatrix );

    // view matrix, looking down with x,y axis starting at the top left corner of the screen
    Matrix viewMatrix;
    Matrix::createLookAt ( halfWidth, halfHeight , 0, halfWidth, halfHeight , 1, 0, -1, 0, &viewMatrix );

    Matrix::multiply ( projectionMatrix, viewMatrix, &spriteMatrix );

    //small offset to match exactly with the pixellize compositor effect
    //spriteMatrix.translate(0,.5f,0);



    Matrix fontMatrix = spriteMatrix;
    fontMatrix.translate(0,.5f,0);
    _font = Font::create ( "res/pf_tempesta_five.gpb" );
    _font->getSpriteBatch()->getSampler()->setFilterMode(Texture::NEAREST,Texture::NEAREST);
    _font->getSpriteBatch()->setProjectionMatrix(spriteMatrix);


    loadSplash = new Sprite ( "res/splash.png",false,10.f,1,1 );
    loadSplash->setProjectionMatrix ( spriteMatrix );
    loadSplash->setScale ( 2.f );
    //loadSplash->setScale ( 700.3f );

    useCompositor = true;
    if(useCompositor)
    {

        // Create one frame buffer for the full screen compositerss.
        _frameBuffer = FrameBuffer::create ( "PostProcessTest", FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT );
        //_frameBuffer->getRenderTarget()->getTexture()->setFilterMode(Texture::LINEAR, Texture::LINEAR);//test
        //_frameBuffer->getRenderTarget()->getTexture()->setFilterMode(Texture::NEAREST, Texture::NEAREST);
        //_frameBuffer->getRenderTarget()->getTexture()->setWrapMode(Texture::CLAMP,Texture::CLAMP);
        //_frameBuffer->getRenderTarget()->

        //Texture::Sampler* sampler = Texture::Sampler::create ( srcBuffer->getRenderTarget()->getTexture() );
        //need to access texture options through Sampler friend class, but won't work here, do it in Compositor::create() below
        /*
        Texture::Sampler* fbSampler =  Texture::Sampler::create(_frameBuffer->getRenderTarget()->getTexture());
        fbSampler->setFilterMode(Texture::NEAREST, Texture::NEAREST);
        fbSampler->setWrapMode(Texture::CLAMP,Texture::CLAMP);
        SAFE_RELEASE ( fbSampler );
        */


        DepthStencilTarget* dst = DepthStencilTarget::create ( "PostProcessTest", DepthStencilTarget::DEPTH_STENCIL, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT );
        _frameBuffer->setDepthStencilTarget ( dst );
        SAFE_RELEASE ( dst );





        compositor = Compositor::create ( _frameBuffer, NULL, "res/common/postprocess/postprocess.material", "DitherB" ); // Dither DitherB Pixelate, Passthrough Gaussian Blur SBlur

        //for gaussian blur
        //compositor->getMaterial()->getParameter("u_length")->setValue(1.0f / ((float)FRAMEBUFFER_WIDTH / 2.0f));
        //compositor->getMaterial()->getParameter("u_length")->setValue(1.0f / ((float)FRAMEBUFFER_WIDTH / 2.0f));

        //sblur
        //compositor->getMaterial()->getParameter("u_length")->setValue(1.0f / ((float)FRAMEBUFFER_WIDTH / 2.f));
        //compositor->getMaterial()->getParameter("u_width")->setValue(0.56f / ((float)FRAMEBUFFER_WIDTH / 2.f));//sblur only 0.56f


    }
}


Renderer::~Renderer()
{
    //TODO add all resources to clean up
    SAFE_RELEASE ( _font );
    //SAFE_DELETE(batch);
    //SAFE_DELETE(phrasesSprite);
    SAFE_DELETE ( scanLines );
    SAFE_DELETE ( loadSplash );
    SAFE_DELETE ( noise );
    SAFE_DELETE(eye);
    SAFE_DELETE(cursor);
}



void Renderer::load2D()
{
    //


    scanLines = new Sprite ( "res/png/scanlines.png",false,1.f,1,1 );
    scanLines->setTint ( 1.f,1.f,1.f,0.08f );
    scanLines->setProjectionMatrix ( spriteMatrix );
    //scanLines->setPosition ( 0,0.5f ); //offset to match pixellize compositor, now done in matrix

    noise = new Sprite ( "res/png/noise.png",false,1.f,1,1 );
    noise->setTint ( 1.f,1.f,1.f,0.45f );
    noise->setProjectionMatrix ( spriteMatrix );
    //noise->setPosition ( 0,0.5f );

    cursor = new Sprite ( "res/png/cursor.png", false, 1.f,1,1);
    cursor->setTint ( 1.f,1.f,1.f,0.5f );
    cursor->setProjectionMatrix ( spriteMatrix );
    cursor->setPosition(128.f,128.f);

    city = new Sprite ( "res/png/city.png",false,1.f,1,1 );
    //city->setTint ( 1.f,1.f,1.f,0.08f );
    city->setProjectionMatrix ( spriteMatrix );
    city->setPosition ( 0,0.0f );

    street = new Sprite ( "res/png/street.png",false,1.f,1,1 );
    //street->setTint ( 1.f,1.f,1.f,0.08f );
    street->setProjectionMatrix ( spriteMatrix );
    street->setPosition ( 0,0.0f );

    window = new Sprite ( "res/png/window.png",false,1.f,1,1 );
    //window->setTint ( 1.f,1.f,1.f,0.08f );
    window->setProjectionMatrix ( spriteMatrix );
    window->setPosition ( 0,0.0f );
    window->setScale(1.2f);


    //eye = new Sprite ( "res/png/pnevb.png",false,1.f,1,1 );
    //eye->setPosition(Game::getInstance()->getWidth()*.5f - 256.f, Game::getInstance()->getHeight() * .5f - 256.f);
}


void Renderer::RenderSplash ( float alpha )
{
    //loadSplash->setTint ( 1.f,1.f,1.f,alpha );
    //temp make solid for retro style
    if(alpha > 0.01f)
        loadSplash->setTint ( 1.f,1.f,1.f,1.0f );
    else
        loadSplash->setTint ( 1.f,1.f,1.f,0.0f );
    loadSplash->Draw();
}

// Render queue indexes (in order of drawing).
enum RenderQueue
{
    QUEUE_OPAQUE = 0,
    QUEUE_TRANSPARENT,
    QUEUE_COUNT
};

bool Renderer::buildRenderQueues(Node* node)
{
    Model* model = node->getModel();
    if (model)
    {
        // Perform view-frustum culling for this node
        if ( !node->getBoundingSphere().intersects(currentScene->getActiveCamera()->getFrustum()))
            return true;


        // Determine which render queue to insert the node into
        std::vector<Node*>* queue;
        if (node->hasTag("transparent"))
            queue = &_renderQueues[QUEUE_TRANSPARENT];
        else
            queue = &_renderQueues[QUEUE_OPAQUE];


        queue->push_back(node);
    }
    return true;
}


bool Renderer::drawScene ( Node* node )
{
    // If the node visited contains a model, draw it
    Model* model = node->getModel();
    if ( model )
    {
        model->draw();
    }
    return true;
}

void Renderer::drawSceneWithCulling()
{
    // Iterate through each render queue and draw the nodes in them
    for (unsigned int i = 0; i < QUEUE_COUNT; ++i)
    {
        std::vector<Node*>& queue = _renderQueues[i];

        for (size_t j = 0, ncount = queue.size(); j < ncount; ++j)
        {
            queue[j]->getModel()->draw();
        }
    }
}




void Renderer::RenderGame()
{

    //currentScene = mGameModel->getScene();
    currentScene = GameModel::getInstance()->getScene();

    if ( currentScene )
    {
        // Visit all the nodes in the scene for drawing
        //GameModel::getInstance()->getScene()->visit ( this, &Renderer::drawScene );

        // Visit all the nodes in the scene to build our render queues
        for (unsigned int i = 0; i < QUEUE_COUNT; ++i)
            _renderQueues[i].clear();
        currentScene->visit(this, &Renderer::buildRenderQueues);


        Rectangle defaultViewport = Game::getInstance()->getViewport();

        FrameBuffer* previousFrameBuffer = NULL;
        if(useCompositor)
        {
            // Draw into the framebuffer
            Game::getInstance()->setViewport ( Rectangle ( FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT ) );
            previousFrameBuffer = _frameBuffer->bind();
        }
        //Game::getInstance()->clear ( Game::CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0 );
        Game::getInstance()->clear ( Game::CLEAR_COLOR_DEPTH, Vector4(GameModel::getInstance()->getFogColor().x,GameModel::getInstance()->getFogColor().y, GameModel::getInstance()->getFogColor().z , 1.f) , 1.0f, 0 );
        //draw scene
        //GameModel::getInstance()->getScene()->visit ( this, &Renderer::drawScene );


        drawSceneWithCulling();


        if( GameModel::getInstance()->particleNodeA)
        {
            ParticleEmitter* emitter = GameModel::getInstance()->particleNodeA->getParticleEmitter();
            if (emitter)
            {
                emitter->draw();
            }
        }

        cursor->Draw();

        /*
        //draw text
        if(currentScene->findNode("playerA") )
        {
            _font->start();
            //char text[1024];  sprintf(text, "%f\n%f", currentScene->findNode("playerA")->getTranslationX(), currentScene->findNode("playerA")->getTranslationY());
            //_font->drawText(text, 0, 64.5f, Vector4(.2,.1,.6,1.0f), _font->getSize() );
            _font->drawText(GameModel::getInstance()->getMessage().c_str(), 4, 4, Vector4(.6,.4,.9,1.0f), _font->getSize() );
            _font->finish();
        }*/

        if(mGameModel->npcA->currentState == Npc::GRAB)
        {
            noise->setPosition ( MATH_RANDOM_0_1() * 128.f - 128.f, MATH_RANDOM_0_1()  * 128.f - 128.f );
            noise->Draw();
        }

        if(mGameModel->introTimer > 0.f)
        {
            city->setPosition(mGameModel->introScrollB, 0.f);
            city->Draw();
            city->setPosition(mGameModel->introScrollB-256.f, 0.f);
            city->Draw();
            street->setPosition(mGameModel->introScrollA, 0.f);
            street->Draw();
            street->setPosition(mGameModel->introScrollA-256.f, 0.f);
            street->Draw();
            street->setPosition(mGameModel->introScrollA-512.f, 0.f);
            street->Draw();
            window->setPosition(0,mGameModel->introHeight - 10.f);
            window->Draw();
            //noise->setPosition(mGameModel->introScrollA, 0.f);
            //noise->setPosition ( MATH_RANDOM_0_1() * 128.f - 128.f, MATH_RANDOM_0_1()  * 128.f - 128.f );
            //noise->Draw();

            if(mGameModel->introTimer < 30.f)
            {
                _font->start();
                char textb[1024];  sprintf(textb, "..");
                _font->drawText(textb, 4, 70.f, Vector4::one(), _font->getSize() );
                _font->finish();
            }
            else if(mGameModel->introTimer > 40.f)
            {
                _font->start();
                char textb[1024];  sprintf(textb, "..");
                _font->drawText(textb, 100, 100, Vector4::one(), _font->getSize() );
                _font->finish();
            }



        }

        if(mGameModel->hitCounter >= 3)
        {
            int packages = mGameModel->packagesCount;
            std::string srank;
            if(mGameModel->gameWin)
            {
                if(packages > 24)
                {
                    srank = "A+";
                }
                else if(packages > 20)
                {
                    srank = "A";
                }
                else if(packages > 15)
                {
                    srank = "B";
                }
                else if(packages > 10)
                {
                    srank = "C";
                }
                else if(packages > 5)
                {
                    srank = "C";
                }
                else
                    srank = "D";
            }
            else
            {
                if(packages > 24)
                {
                    srank = "B+";
                }
                else if(packages > 20)
                {
                    srank = "B";
                }
                else if(packages > 15)
                {
                    srank = "C";
                }
                else if(packages > 10)
                {
                    srank = "D";
                }
                else if(packages > 5)
                {
                    srank = "E";
                }
                else
                    srank = "F";
            }

            city->setPosition(mGameModel->introScrollB, 0.f);
            city->Draw();
            city->setPosition(mGameModel->introScrollB-256.f, 0.f);
            city->Draw();
            street->setPosition(mGameModel->introScrollA, 0.f);
            street->Draw();
            street->setPosition(mGameModel->introScrollA-256.f, 0.f);
            street->Draw();
            street->setPosition(mGameModel->introScrollA-512.f, 0.f);
            street->Draw();
            window->setPosition(0,mGameModel->introHeight - 10.f);
            window->Draw();
            noise->setPosition ( MATH_RANDOM_0_1() * 128.f - 128.f, MATH_RANDOM_0_1()  * 128.f - 128.f );
            if(!mGameModel->gameWin)
                noise->Draw();


            _font->start();
            char textb[1024];
            if(mGameModel->gameWin)
                sprintf(textb, " ESCAPE\n P: %i \n TIME: %f" , packages, mGameModel->gameTimer);
            else
               sprintf(textb, " LOSE\n P: %i \n TIME: %f" , packages, mGameModel->gameTimer);
           string finalMessage = string(textb) + "\n RANK: " + srank;
           _font->drawText(finalMessage.c_str(), 4, 40, Vector4::one(), _font->getSize() );
            _font->finish();
        }


        //draw anything else to be filtered by compositor before this line, overlay/text/etc
        //------------------
        if(useCompositor)
        {

            // Bind the current compositor
            Game::getInstance()->setViewport ( defaultViewport );
            //Compositor* compositor = _compositors[_compositorIndex];

            FrameBuffer* compositorDstFrameBuffer = compositor->getDstFrameBuffer();
            FrameBuffer* prevToCompositeFrameBuffer = NULL;
            if ( compositorDstFrameBuffer )
            {
                prevToCompositeFrameBuffer = compositorDstFrameBuffer->bind();
            }
            else
            {
                prevToCompositeFrameBuffer = previousFrameBuffer->bind();
            }

            Game::getInstance()->clear ( Game::CLEAR_COLOR, Vector4 ( 0, 0, 0, 1 ), 1.0f, 0 );
            compositor->blit ( defaultViewport );
            //drawFrameRate(_font, Vector4(0, 0.5f, 1, 1), 5, 1, getFrameRate());
            //drawTechniqueId(compositor->getTechniqueId());

            previousFrameBuffer->bind();
        }



        //physics debug wireframes
        //Game::getInstance()->getPhysicsController()->drawDebug(currentScene->getActiveCamera()->getViewProjectionMatrix());


        //eye->Draw();


    }







    ////*2D and overlay ----------________________---
    //TODO get state from game model for overlay configurations


    //debug
    if(mGameModel->getScene())
    {
        /*
        _font->start();
        char text[1024];  sprintf(text, "%f,%f", mGameModel->pitchB, mGameModel->pitchC);
        _font->drawText(text, 0, 64.5f, Vector4(0,0,0,1), _font->getSize() );
        //drawText(const char* text, const Rectangle& area, const Vector4& color, unsigned int size = 0
        //_font->drawText(text, Rectangle(0,65.f,100.f,100.f), Vector4::one(), _font->getSize() );
        _font->finish();
        */
        /*
        //draw text
        if(currentScene->findNode("playerA") )
        {
        _font->start();
        char text[1024];  sprintf(text, "%f\n%f", currentScene->findNode("playerA")->getTranslationX(), currentScene->findNode("playerA")->getTranslationZ());
        _font->drawText(text, 0, 64.5f, Vector4(.2,.1,.6,1.0f), _font->getSize() );
        _font->finish();
        }
        */

    }







    //noise->setPosition ( MATH_RANDOM_0_1() * 128.f - 128.f, MATH_RANDOM_0_1()  * 128.f - 128.f );
    //noise->Draw();

    //scanLines->Draw();

    //level fading splash
    if ( mGameModel->getFadeAlpha() > 0.f )
    {
        //printf("fadealpha %f:\n", mGameModel->getFadeAlpha());
        loadSplash->setTint ( 1.f,1.f,1.f,mGameModel->getFadeAlpha() );
        loadSplash->Draw();
    }




}



































//compositor section____________________________------------__________________________
Model* Renderer::_quadModel = NULL;
Material* Renderer::_compositorMaterial = NULL;


Renderer::Compositor* Renderer::Compositor::create ( FrameBuffer* srcBuffer, FrameBuffer* dstBuffer, const char* materialPath, const char* techniqueId )
{
    GP_ASSERT ( srcBuffer );

    Material* material = Material::create ( materialPath );
    Texture::Sampler* sampler = Texture::Sampler::create ( srcBuffer->getRenderTarget()->getTexture() );

    //have to set options here because of new library changes
    sampler->setFilterMode(Texture::NEAREST, Texture::NEAREST);
    sampler->setWrapMode(Texture::CLAMP,Texture::CLAMP);


    material->getParameter ( "u_texture" )->setValue ( sampler );

    //dither stuff
    //Texture::Sampler* paletteSampler =  Texture::Sampler::create("res/png/palette.png");
    //paletteSampler->setFilterMode(Texture::NEAREST, Texture::NEAREST);
    //paletteSampler->setWrapMode(Texture::CLAMP,Texture::CLAMP);
    //material->getParameter ( "u_palette" )->setValue ( paletteSampler );

    SAFE_RELEASE ( sampler );
    if ( _quadModel == NULL )
    {
        Mesh* mesh = Mesh::createQuadFullscreen();
        _quadModel = Model::create ( mesh );
        SAFE_RELEASE ( mesh );
    }

    return new Compositor ( srcBuffer, dstBuffer, material, techniqueId );
}

Renderer::Compositor::Compositor ( FrameBuffer* srcBuffer, FrameBuffer* dstBuffer, Material* material, const char* techniqueId )
    : _srcBuffer ( srcBuffer ), _dstBuffer ( dstBuffer ), _material ( material ),  _techniqueId ( techniqueId )
{
}

Renderer::Compositor::~Compositor()
{
    SAFE_RELEASE ( _material );
}

FrameBuffer* Renderer::Compositor::getSrcFrameBuffer() const
{
    return _srcBuffer;
}

FrameBuffer* Renderer::Compositor::getDstFrameBuffer() const
{
    return _dstBuffer;
}

const char* Renderer::Compositor::getTechniqueId() const
{
    return _techniqueId;
}

Material* Renderer::Compositor::getMaterial() const
{
    return _material;
}

void Renderer::Compositor::blit ( const Rectangle& dst )
{
    if ( _compositorMaterial != _material )
    {
        _compositorMaterial = _material;
        _quadModel->setMaterial ( _compositorMaterial );
    }
    _compositorMaterial->setTechnique ( _techniqueId );

    _quadModel->draw();
}

//Renderer::Renderer()
//   : _font(NULL), _scene(NULL), _modelNode(NULL), _frameBuffer(NULL), _compositorIndex(0)
//{
//}
//____________________________------------__________________________




