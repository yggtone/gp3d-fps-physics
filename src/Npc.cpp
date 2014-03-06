#include "Npc.h"

using namespace gameplay;
using namespace std;


Npc::Npc(Vector3 initpos, Material *npcmat, NPCTYPE ntype )
    : changeDirectionTimer(0.f), pathFinder(NULL), pathIndex(0), pathUpdateTimer(0.f), patrolForward(false),
    playerSeenCount(0), hideCount(0), monsterLookTimer(0.f), moveSpeed(1.f), playerHitCounter(0),
    searchSoundNode(NULL), yellSoundNode(NULL), grabSound(NULL)
{

    moveSpeed = 0.1f;



    currentState = IDLE;

    pathFinder = new AStar();

    _npcType = ntype;





    if(_npcType == TALLCAPE)
    {

        Bundle* bundleNPCA = Bundle::create( "res/npca.gpb" );
        Scene* npcAScene = bundleNPCA->loadScene();
        SAFE_RELEASE(bundleNPCA);
        npcNode = npcAScene->findNode("npca");

        npcNode->setScale(0.03f);
        //npcNode->setScale(0.4f);
        npcNode->setTranslation(initpos);
        //npcNode->translateY(2.f);
        //npcNode->rotateX(MATH_DEG_TO_RAD(-90.f));//fucek up  export fix

        npcNode->getModel()->setMaterial(npcmat);
        Animation *animnpc = npcNode->getAnimation("animations");//Bone_002 animations

        walkAnim = animnpc->createClip("walk",0, 20000);
        walkAnim->setRepeatCount(10000.f);
        walkAnim->setSpeed(11.3f);

        idleAnim = animnpc->createClip("idle",0,20000);
        idleAnim->setRepeatCount(10000.f);
        idleAnim->setSpeed(11.0f);
        idleAnim->play();

        PhysicsRigidBody::Parameters params ( 70.f,  0.3f, 0.f, 0.f, 0.f, false, Vector3::one(), Vector3(1.0,1.0,1.0), Vector3(0.0,1.0,0.0)  );
        controllerNode = Node::create();
        controllerNode->setTranslation(initpos);
        controllerNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,
                                             PhysicsCollisionShape::capsule(0.35f,1.9f), &params );
        npcController = static_cast<PhysicsRigidBody*> ( controllerNode->getCollisionObject() );
        npcController->setGravity(Vector3(0, -11.f,0));
        npcController->setDamping(0.95f,0.01f);

        //neckBone = npcNode->findNode("neck");
        //neckBone->rotate(Vector3::unitY(), 1.2f);

    }
    else if(_npcType == GHOSTA)
    {
        Bundle* bundleNPCA = Bundle::create( "res/npcb.gpb" );
        Scene* npcAScene = bundleNPCA->loadScene();
        SAFE_RELEASE(bundleNPCA);
        npcNode = npcAScene->findNode("npcb");

        npcNode->setScale(0.04f);
        //npcNode->setScale(0.4f);
        npcNode->setTranslation(initpos);
        //npcNode->translateY(1.f);

        npcNode->getModel()->setMaterial(npcmat);
        Animation *animnpc = npcNode->getAnimation("animations");//Bone_002 animations

        walkAnim = animnpc->createClip("walk",0, 8000);
        walkAnim->setRepeatCount(10000.f);
        walkAnim->setSpeed(5.3f);
        walkAnim->play();

        newHeading = Vector3(1,0,1);


    }



    searchSoundNode = Node::create();
    //TODO release properly?
    AudioSource* audioSource = AudioSource::create ( "res/audio/search.ogg" );
    assert ( audioSource );
    audioSource->setLooped ( true );
    //audioSource->setVelocity(10.f,10.f,10.f);
    searchSoundNode->setAudioSource ( audioSource );
    //searchSoundNode->getAudioSource()->play();
    searchSoundNode->getAudioSource()->setGain ( 2.5f );
    //searchSoundNode->getAudioSource()->
    audioSource->release();
    //searchSoundNode->setTranslation(4.5f,1.f,2.f);
    //_scene->getNode()->get
    npcNode->addChild( searchSoundNode );

    yellSoundNode = Node::create();
    AudioSource* yellSource = AudioSource::create ( "res/audio/see.ogg" );
    assert ( yellSource );
    yellSource->setGain(1.2f);
    yellSource->setPitch(0.7f);
    yellSource->setLooped ( true );
    yellSoundNode->setAudioSource ( yellSource );
    yellSource->release();
    npcNode->addChild( yellSoundNode );


    grabSound = AudioSource::create ( "res/audio/grab.ogg" );
    assert ( grabSound );
    grabSound->setLooped ( false );
    grabSound->setGain(0.5f);
}

Npc::~Npc()
{

    SAFE_DELETE(pathFinder);
}

void Npc::stopSounds()
{
    yellSoundNode->getAudioSource()->stop();
    grabSound->stop();
    searchSoundNode->getAudioSource()->stop();


}

void Npc::setPathNodes(std::vector<AStar::PNode> pathNodes)
{
    pathFinder->setPathNodes(pathNodes);

}


void Npc::collisionEvent ( PhysicsCollisionObject::CollisionListener::EventType type,

                           const PhysicsCollisionObject::CollisionPair& collisionPair,

                           const Vector3& contactPointA,

                           const Vector3& contactPointB )

{
}


void Npc::animationEvent ( AnimationClip* clip, AnimationClip::Listener::EventType type )

{
}

float angleB(const Vector3& v1, const Vector3& v2)
{
    float dx = v1.y * v2.z - v1.z * v2.y;
    float dy = v1.z * v2.x - v1.x * v2.z;
    float dz = v1.x * v2.y - v1.y * v2.x;

    //return atan2f(sqrt(dx * dx + dy * dy + dz * dz) + MATH_FLOAT_SMALL, Vector3::dot(v1, v2));

    float cc = Vector3::dot(v1, v2) / sqrt(dx * dx + dy * dy + dz * dz);

    if(cc < -1.f)
        cc = -1.f;
    else if(cc > 1.f)
        cc = 1.f;

    printf("an %f\n", cc);

    return std::acos(cc );
}


void lookAt(Node* node, const Vector3& eye, const Vector3& target, const Vector3& up)
{
    // Create lookAt matrix
    Matrix matrix;
    Matrix::createLookAt(eye, target, up, &matrix);
    matrix.invert();

    // Pull SRT components out of matrix
    Vector3 scale;
    Quaternion rotation;
    Vector3 translation;
    matrix.decompose(&scale, &rotation, &translation);

    // Set SRT on node
    node->setScale(scale);
    //node->setTranslation(translation);
    //node->setRotation(rotation);
}


void headLookAt(Node* node, const Vector3& eye, const Vector3& target, const Vector3& up)
{
    // Create lookAt matrix
    Matrix matrix;
    Matrix::createLookAt(eye, target, up, &matrix);
    matrix.invert();

    // Pull SRT components out of matrix
    Vector3 scale;
    Quaternion rotation;
    Vector3 translation;
    matrix.decompose(&scale, &rotation, &translation);

    // Set SRT on node
    //node->setScale(scale);
    //node->setTranslation(translation);
    node->setRotation(rotation);
}


void Npc::update(Vector3 targetPos, float elapsedTime, bool flashlightOn)
{
    if(searchSoundNode->getAudioSource()->getState() != AudioSource::PLAYING)
    {
        searchSoundNode->getAudioSource()->play();
    }


    npcNode->setTranslation(controllerNode->getTranslation() + Vector3(0,-0.075f,0) );
    //return;
    //currentState = CHASE;
    //moveSpeed = 5.f;

        //monster states
        /*
        IDLE/PARTROL-patrol area between two points
         -if player is seen from raytest once, send to SEARCH mode
         -switch to SEARCH anyway after a certain number of viewcounts, so there is
          less chance of player missing the monster entirely
        CHASE-pursue, get faster over time
         -chase player as long as player is not hidden from view for a time,
          (several consecutive lookcheck intervals fail), then switch to SEARCH
         -after a grab, resume chasing but start out slow for a fair chance
          to escape.
        GRAB-If you are close enough to the player during a chase.
          -struggle for a few seconds and release back to chase mode
        SEARCH-look for player, choose two random offsets from player
            and patrol there.
          -go to patrol if many consecutive look-checks fail (more than
           the amount for a failed chase)
          -go to chase mode if two views



        */

    monsterLookTimer += elapsedTime;
    if(monsterLookTimer > 0.6f)
    {
        monsterLookTimer = 0.f;
        //Game::getInstance()->getPhysicsController()->getGravity();
        PhysicsController::HitResult hitResult;
        Vector3 rayStart = controllerNode->getTranslation() + Vector3(0,0.75f,0);//monster head
        Vector3 rayDirect = targetPos - rayStart;
        float sightDistance = 10.f;
        if(!flashlightOn)
            sightDistance = 6.f;
        if(Game::getInstance()->getPhysicsController()->rayTest( Ray(rayStart,rayDirect), sightDistance, &hitResult ))
        {
            //printf("000\n");
            if(hitResult.object)
            {

                _message = hitResult.object->getNode()->getId();
                if( std::string(hitResult.object->getNode()->getId()) == "playerA" )
                {
                    playerSeenCount++;
                    char text[1024];  sprintf(text, "%i\n", playerSeenCount);
                    _message = "playerhit " + std::string(text);

                    if(moveSpeed < 4.0f)
                        moveSpeed += elapsedTime;

                    //reset hideCount since only consecutive hides should count
                    hideCount = 0;
                }
                else
                    hideCount++;


            }
        }
    }




    char textb[1024];  sprintf(textb, "%i\n", currentState);
    _message = "state: " + std::string(textb);


    if(currentState == IDLE)
    {
        patrolA = Vector3(-33.f, 0.f, -33.f);
        patrolB = Vector3(-5.f, 0.f, -5.f);

        if(playerSeenCount > 0)
        {
            playerSeenCount = 0;
            hideCount = 0;
            currentState = SEARCH;
        }
        //eventually start searching
        if(hideCount > 20)
        {
            playerSeenCount = 0;
            hideCount = 0;
            currentState = SEARCH;

        }


        if(patrolForward)
            targetPos = patrolA;
        else
            targetPos = patrolB;
        Vector3 currentTarget = targetPos;
        currentTarget.y = 0;
        Vector3 currentPos = controllerNode->getTranslation();
        currentPos.y = 0;
        float ppdist = std::abs((currentTarget - currentPos).lengthSquared() ) ;
        if( ppdist  < 5.f )
            patrolForward = !patrolForward;





    }
    else if(currentState == SEARCH)
    {



        //choose new targets relative to player
        //float patrolX = targetPos.x - ((int)targetPos.x %10);
        // patrolZ = targetPos.z - ((int)targetPos.z %10);
        float patrolX = ((int)(targetPos.x - 5.f) /10) * 10.f;
        float patrolZ = ((int)(targetPos.z - 5.f) /10) * 10.f;
        patrolA = Vector3(patrolX - 2.f,0,patrolZ - 2.f);
        patrolB = Vector3(patrolX + 2.f,0,patrolZ + 1.f);

        // char textc[1024];  sprintf(textc, "%f %f\n", patrolX, patrolZ);
        //_message = " " + std::string(textc);


        if(patrolForward)
            targetPos = patrolA;
        else
            targetPos = patrolB;
        Vector3 currentTarget = targetPos;
        currentTarget.y = 0;
        Vector3 currentPos = controllerNode->getTranslation();
        currentPos.y = 0;
        float ppdist = std::abs((currentTarget - currentPos).lengthSquared() ) ;
        if( ppdist  < 2.f )
        {
            patrolForward = !patrolForward;
            moveSpeed = 0.4f;//slow down when next to player searching a bit
        }
        else if(ppdist > 44.f)
        {
            moveSpeed = 1.f;
        }

        if(playerSeenCount > 1)
        {
            playerSeenCount = 0;
            hideCount = 0;
            moveSpeed = 1.f;
            currentState = CHASE;
            yellSoundNode->getAudioSource()->play();
        }

        if(hideCount > 30)//give up search
        {
            playerSeenCount = 0;
            hideCount = 0;
            currentState = IDLE;
            moveSpeed = 1.f;
        }


    }
    else if(currentState == CHASE)
    {

        //additional ramp up to normal speed
        if(moveSpeed < 1.25f)
            moveSpeed += elapsedTime * .5f;

        if(hideCount > 10)//lose sight of player, start search mode
        {
            playerSeenCount = 0;
            hideCount = 0;
            currentState = SEARCH;
            yellSoundNode->getAudioSource()->stop();
        }

        if(playerSeenCount > 6)
        {
            //check distance to player for grabbing
            Vector3 currentTarget = targetPos;
            currentTarget.y = 0;
            Vector3 currentPos = controllerNode->getTranslation();
            currentPos.y = 0;
            float ppdist = std::abs((currentTarget - currentPos).lengthSquared() ) ;
            if( ppdist  < 2.f )
            {
                grabSound->play();
                currentState = GRAB;
                playerSeenCount = 0;
                hideCount = 0;
            }
        }

    }
    else if(currentState == GRAB)
    {

        //let go after a time, hideCount included for rare cases
        //when player slides behind a corner after grab
        if( (playerSeenCount + hideCount) > 7 )//7 * .3 = 2.1
        {
            moveSpeed = 0.45f;
            playerSeenCount = 0;
            hideCount = 0;
            currentState = CHASE;
            playerHitCounter++;

        }




    }


    pathUpdateTimer += elapsedTime;
    if(pathUpdateTimer > 0.3f)
    {
        pathUpdateTimer = 0.f;
        currentPath = pathFinder->FindPath(controllerNode->getTranslation(), targetPos);
        pathIndex = 0;

        //printf("pathsize: %i\n", currentPath.size());
    }




    if(_npcType == TALLCAPE)
    {




        Vector3 movingLine = Vector3::zero();
        //Vector3 movingLine = playerPos - npcNode->getTranslation();
        if(pathIndex >= 0 && pathIndex < currentPath.size())
        {
            //printf("pathindex: %i\n", pathIndex);
            //printf("curpath: %f %f %f %f\n\n", currentPath[pathIndex].x, currentPath[pathIndex].z, npcNode->getTranslation().x, npcNode->getTranslation().z);

            Vector3 currentTarget = currentPath[pathIndex];
            currentTarget.y = 0;
            Vector3 currentPos = controllerNode->getTranslation();
            currentPos.y = 0;

            float pdist = std::abs((currentTarget - currentPos).lengthSquared() ) ;
            //printf("pdist: %f \n", pdist);
            if( pdist  < 1.f )//waypoint reached
            //if( currentPath[pathIndex].distanceSquared( npcNode->getTranslation()) < 1.f   )
            {
                if(pathIndex < (currentPath.size() - 1) )
                    pathIndex++;

                //printf("plus");
                //if(pathIndex >= currentPath.size())
                //    pathIndex = 0;
            }
        }
        if(pathIndex >= 0 && pathIndex < currentPath.size())
        {
            movingLine = currentPath[pathIndex] - controllerNode->getTranslation();
        }


        movingLine.y = 0;

        //moveSpeed += elapsedTime * .1f;



        // Get the current forward vector for the mesh node (negate it since the character was modelled facing +z)
        Vector3 currentHeading(-npcNode->getForwardVectorWorld());
        //Vector3 currentHeading(-npcNode->getUpVectorWorld());//change because of messed up exporting

        // Construct a new forward vector for the mesh node
        //Vector3 newHeading(cameraForward * _currentDirection.y + cameraRight * _currentDirection.x);
        Vector3 newHeading = movingLine;
        // Compute the rotation amount based on the difference between the current and new vectors
        float angle = atan2f(newHeading.x, newHeading.z) - atan2f(currentHeading.x, currentHeading.z);
        if (angle > MATH_PI)
            angle -= MATH_PIX2;
        else if (angle < -MATH_PI)
            angle += MATH_PIX2;
        angle *= (float)elapsedTime * MATH_PIX2;
        npcNode->rotate(Vector3::unitY(), angle);
        //npcNode->rotate(Vector3::unitZ(), angle);


        if( true) // ( movingLine).lengthSquared() > 0.f || npcNode->getTranslationZ() > 6.f)//if far away or past the doorway
        {
            movingLine.normalize();
            movingLine.scale(moveSpeed);
            //npcController->setVelocity(movingLine);
            if(currentState != GRAB)
                npcController->applyImpulse(movingLine * 9.f);//10.f 0.9f


            //npcController->applyForce(movingLine * 200.f);


            /*
            if(idleAnim->isPlaying() && !walkAnim->isPlaying() )
            {
                idleAnim->crossFade(walkAnim, 500);

            }
            else if(!idleAnim->isPlaying() && !walkAnim->isPlaying() )
            {
                walkAnim->play();
            }
            walkTimeout = .5f;

            */


        }
        else
        {
            //npcController->setVelocity(Vector3::zero());
            /*
            if(walkTimeout > 0.f)
            {
                walkTimeout -= elapsedTime;
                if(walkTimeout <= 0.f)
                {
                    if(walkAnim->isPlaying() && !idleAnim->isPlaying() )
                    {
                        walkAnim->crossFade(idleAnim, 500);

                    }
                }
            }
            */
        }

    }
    else if(_npcType == GHOSTA)
    {
        //npcNode->setTranslation(controllerNode->getTranslation() + Vector3(0,-0.5f,0) );

        //Vector3 movingLine = playerPos - npcNode->getTranslation();
        Vector3 movingLine = newHeading;
        movingLine.y = 0;


        changeDirectionTimer += elapsedTime;
        if(changeDirectionTimer > 4.f || npcNode->getTranslationZ() > 6.f)//if far away or past the doorway
        {
            changeDirectionTimer = 0.f;
            newHeading = Vector3( MATH_RANDOM_0_1() - .5f, 0.f, MATH_RANDOM_0_1() - .5f);


        }



        movingLine.normalize();
        movingLine.scale(11.6f * elapsedTime);
        //npcController->setVelocity(movingLine);
        //npcController->applyImpulse(movingLine * 10.f);
        npcNode->translate(movingLine);




        // Get the current forward vector for the mesh node (negate it since the character was modelled facing +z)
        Vector3 currentHeading(-npcNode->getForwardVectorWorld());
        // Construct a new forward vector for the mesh node
        //Vector3 newHeading(cameraForward * _currentDirection.y + cameraRight * _currentDirection.x);
        Vector3 newHeading = movingLine;
        // Compute the rotation amount based on the difference between the current and new vectors
        float angle = atan2f(newHeading.x, newHeading.z) - atan2f(currentHeading.x, currentHeading.z);
        if (angle > MATH_PI)
            angle -= MATH_PIX2;
        else if (angle < -MATH_PI)
            angle += MATH_PIX2;
        angle *= (float)elapsedTime * MATH_PIX2;
        npcNode->rotate(Vector3::unitY(), angle);

    }

}

