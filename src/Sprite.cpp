#include "Sprite.h"



Sprite::Sprite(const char* texturePath, bool filtering, float animationFPS, int animationCols  , int animationRows )
{
    scaleX = 1.0f;
    scaleY = 1.0f;
    rotateAngle = 0.f;

    tint = Vector4::one();


    currentFrame = 0;
    positionX = 0;
    positionY = 0;

    this->animationCols = animationCols;
    this->animationRows = animationRows;
    if(animationCols < 1)
        animationCols = 1;
    if(animationRows < 1)
        animationRows = 1;
    if(animationCols == 1 && animationRows == 1)
        isAnimated = false;
    else
        isAnimated = true;

    animationTimePerFrame = 1.f/animationFPS;

    totalFrames = animationCols * animationRows;

    batch = SpriteBatch::create(texturePath);

    frameWidth = batch->getSampler()->getTexture()->getWidth() / animationCols ;
    frameHeight = batch->getSampler()->getTexture()->getHeight() / animationRows;

    for(int i=0; i < animationRows  ; i++)
    {
        for(int j=0; j < animationCols; j++)
        {
            Rectangle* frameRect = new Rectangle(j * frameWidth, i * frameHeight, frameWidth, frameHeight);
            frameRectangles.push_back(frameRect);
        }
    }

    batch->getSampler()->setWrapMode(Texture::CLAMP, Texture::CLAMP);

    /*
    if(filtering)
        batch->getSampler()->getTexture()->setFilterMode(Texture::LINEAR,Texture::LINEAR);
    else
        batch->getSampler()->getTexture()->setFilterMode(Texture::NEAREST,Texture::NEAREST);
    */
    if(filtering)
        batch->getSampler()->setFilterMode(Texture::LINEAR,Texture::LINEAR);
    else
        batch->getSampler()->setFilterMode(Texture::NEAREST,Texture::NEAREST);
}


Sprite::~Sprite()
{
    SAFE_DELETE(batch);

    //delete rectangle objects
    std::vector<Rectangle*>::iterator iter;
    for (iter = frameRectangles.begin(); iter <  frameRectangles.end(); iter++ )
    {
        delete *iter;
    }
}

void Sprite::Update(float deltaTime)
{
    currentFrameTime += deltaTime;
    if(currentFrameTime > animationTimePerFrame )
    {
        currentFrameTime = 0;
        currentFrame++;
        if(currentFrame >= totalFrames)
            currentFrame = 0;
    }
}

void Sprite::setFrame(int frameNum)
{
    currentFrame = frameNum;
    if(currentFrame < 0)
        currentFrame = 0;
    else if(currentFrame >= totalFrames)
        currentFrame = 0;
}

void Sprite::setPosition(float x, float y)
{
    positionX = x;
    positionY = y;
}


void Sprite::setProjectionMatrix(Matrix pvmatrix)
{
    batch->setProjectionMatrix(pvmatrix);
}


void Sprite::Draw()
{
    //void draw(const Vector3& dst, const Rectangle& src, const Vector2& scale, const Vector4& color,
    //const Vector2& rotationPoint, float rotationAngle);

    //void draw(const Vector3& dst, float width, float height, float u1, float v1, float u2, float v2, const Vector4& color,
    //const Vector2& rotationPoint, float rotationAngle, bool positionIsCenter = false);

    //draw sprite at current position, scale, with current animation frame
    batch->start();
    //batch->draw(Rectangle( positionX, positionY,(float) frameWidth * scaleX, (float) frameHeight * scaleY), *frameRectangles[currentFrame], tint);


    //batch->draw(Vector3( positionX, positionY, 0.f ), *frameRectangles[currentFrame], Vector2((float) frameWidth * scaleX, (float) frameHeight * scaleY),  tint, Vector2(0.5f,0.5f), rotateAngle);

    //fliped verticle, no anim frames
    batch->draw(Vector3( positionX, positionY, 0.f ),  frameWidth * scaleX, (float) frameHeight * scaleY, 1.f, 0.f, 0.f, 1.f,
                tint, Vector2(0.5f,0.5f), rotateAngle);
    //fliped horizontal
    //batch->draw(Vector3( positionX, positionY, 0.f ),  frameWidth * scaleX, (float) frameHeight * scaleY, 1.f, 1.f, 0.f, 0.f,
    //            tint, Vector2(0.5f,0.5f), rotateAngle);

    batch->finish();

}

void Sprite::DrawManualSingle()
{
    // batch->draw(Rectangle( positionX, positionY,(float) frameWidth * scaleX, (float) frameHeight * scaleY), *frameRectangles[currentFrame], tint);
    batch->draw(Vector3( positionX, positionY, 0.f ), *frameRectangles[currentFrame], Vector2((float) frameWidth * scaleX, (float) frameHeight * scaleY),  tint, Vector2(0.5f,0.5f), rotateAngle);

}


