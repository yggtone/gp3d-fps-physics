#include "GameModel.h"
#include "math.h"
#include "gameplay.h"

using namespace gameplay;
using namespace std;

static GameModel* __gameModelInstance = NULL;

GameModel::GameModel(PhysicsController * physicsControl) : //TODO all pointers init!!
    upKeyDown(false), leftKeyDown(false), rightKeyDown(false), downKeyDown(false),
    particleNodeA(NULL), fadeAlpha(0.f), waveB(0.f), _scene(NULL), _animation(NULL),
    _charControllerR(NULL), _camBackup(NULL), _camPitch(NULL), _camRoot(NULL), _lightNode(NULL),
    _globe(NULL), guyNode(NULL), playerWarpFrame(false), npcA(NULL), matColorParam(NULL), colorWave(0.f),
    droneA(NULL), walkSpeed(0.f), walkSpeedTarget(0.f), walkWave(0.f), rollTarget(0.f), bobY(0.f), bobYTarget(0.f),
    camRotX(0.f), camRotY(0.f), camRotZ(0.f), camRotXTarget(0.f), camRotYTarget(0.f), pEmitterA(NULL),
    mPhysicsController(NULL), pickedBody(NULL), pickingLength(2.f), _message(" "), doorBody(NULL), knockingSoundNode(NULL),
    knockStarted(false),  gameTimer(0.f), checkTimer(0.f), mazeCols(0), mazeRows(0), mazeFloors(0),
    headHeight(0.6f), headHeightTarget(0.6f), flashlightOn(true), grabRoll(0.f), grabRollTarget(0.f), grabWave(0.f),
    packagesCount(0), introTimer(0.f), introScrollA(0.f), introScrollB(0.f), hitCounter(0), gameWin(false),
    stepSound(NULL), doorSound(NULL), driveSound(NULL), introHeight(0.f), packageSound(NULL), flashlightSound(NULL)
{
    mPhysicsController = physicsControl;
    LevelState = INIT;
    NextLevelState = HOUSE;
    pickingLength = 1.5f;

    introTimer = 50.f;//50


  
    GP_ASSERT ( __gameModelInstance == NULL );
    __gameModelInstance = this;


    duckReleased();

}

GameModel::~GameModel()
{
    unloadScene();

    //TODO need releasing based on scenes loaded, diff audio, etc.

    //SAFE_RELEASE(hitA);
    //TODO cleanup all


    //SAFE_RELEASE(crowdA);

    //SAFE_RELEASE(npcAwalk);
    //SAFE_RELEASE(npcAidle);
    //SAFE_RELEASE(npcAcontroller);
    //SAFE_RELEASE(npcANodeA);

    SAFE_DELETE(npcA);

    //TODO release in separate function for multiple scenes
    SAFE_RELEASE ( _scene );

    SAFE_RELEASE ( _camPitch );
    SAFE_RELEASE ( _camRoot );
}

GameModel* GameModel::getInstance()
{
    GP_ASSERT ( __gameModelInstance );
    return __gameModelInstance;
}

void GameModel::duckPressed()
{
    headHeightTarget = -0.2f;
}

void GameModel::duckReleased()
{
    headHeightTarget = 0.6f;
}


void GameModel::primaryAction()
{

    //flashlightOn = !flashlightOn;

    //Vector3 targetHolding = _camPitch->getTranslationWorld() + _camPitch->getForwardVectorWorld() * 2.f;

    //if(mPhysicsController->rayTest( Ray(guyNode->getTranslation(), Vector3(0,-1.f,0) ),1.5f ))

    if( LevelState == HOUSE && introTimer <= 0.f && hitCounter < 3)
    {


        PhysicsController::HitResult hitResult;
        if(mPhysicsController->rayTest( Ray(_camPitch->getTranslationWorld(), _camPitch->getForwardVectorWorld()), pickingLength * 2.f, &hitResult ))
        {
            //printf("000\n");

            if(hitResult.object)
            {
                _message = hitResult.object->getNode()->getId();



                if(hitResult.object->isDynamic() )
                {
                    //printf("bbb\n");
                    pickedBody = hitResult.object->asRigidBody();
                    pickedBody->setDamping(.98f,.99f);
                }

                //'pick up and remove' test
                if(hitResult.object->getNode()->getTag("type") )
                {
                    if(std::string(hitResult.object->getNode()->getTag("type")) == "package" )
                    {
                        packageSound->play();
                        packagesCount++;
                        pickedBody = NULL;
                        //hitResult.object->asRigidBody()->getNode()->release();//crash!
                        //hitResult.object->asRigidBody()->getNode()->setScale(0.01f);
                        hitResult.object->setEnabled(false);
                        _scene->removeNode(hitResult.object->asRigidBody()->getNode());
                        //hitResult.object->asRigidBody()->getNode()->release();
                    }
                    else if(std::string(hitResult.object->getNode()->getTag("type")) == "exit" )
                    {
                        gameWin = true;
                        hitCounter = 3;
                        doorSound->play();
                        npcA->stopSounds();
                        driveSound->play();
                    }

                }


            }

        }
    }

    /*
    return;

    if(LevelState == LEVELB)
        NextLevelState = LEVELA;

    if(LevelState == LEVELA)
    {
        NextLevelState = LEVELB;
    }
    */
}

void GameModel::leftMouseRelease()
{
    if(pickedBody)
    {
        pickedBody->setDamping(.2f,.5f);
        pickedBody = NULL;

    }
}


void GameModel::rightMouseDown()
{

    if(pickedBody)
    {
        pickedBody->setDamping(.2f,.5f);
        pickedBody->applyImpulse(_camPitch->getForwardVectorWorld() * 5.f);
        pickedBody = NULL;
    }
}



void GameModel::camMove ( float pitch, float yaw )
{
    //return;


    if ( ! ( _camRoot && _camPitch ) )
        return;

    camRotXTarget += ( - ( float ) pitch ) * .001f;
    camRotYTarget += ( - ( float ) yaw   ) * .001f;


    if(camRotXTarget > 1.5f)
        camRotXTarget = 1.5f;
    else if(camRotXTarget < -1.35f)
        camRotXTarget = -1.35f;




}



void GameModel::createMaze(int cols, int rows, int floors)
{
	mazeCols = cols;
	mazeRows = rows;
	mazeFloors = floors;
        //Randomized Primm  Start with a grid full of walls.
        //-Pick a cell, mark it as part of the maze. Add the walls of the cell to the wall list.
        //-While there are walls in the list:
        //    1.Pick a random wall from the list. If the cell on the opposite side isn't in the maze yet:
        //        -Make the wall a passage and mark the cell on the opposite side as part of the maze.
        //        -Add the neighboring walls of the cell to the wall list.
        //    2.If the cell on the opposite side already was in the maze, remove the wall from the list.
        bool cubePaths = false;
	if(floors > 1)
		cubePaths = true;
	mazeNodes.clear();
	mazeNodes.resize(rows * cols * floors);
	mazeWalls.clear();


	int beginWall = 0;
	int endWall = 3;
	if(cubePaths)
		endWall = 5;


	if(mazeNodes.empty() )
		return;

	//####
	//####

	//pick first cell/node
	unsigned int firstMazeIndex = 0;
	mazeNodes[firstMazeIndex].partOfMaze = true;

	MazeWall nWall,sWall,eWall,wWall;
	nWall.wallDirection = NORTH;
	nWall.firstCellIndex = firstMazeIndex;
	nWall.secondCellIndex = firstMazeIndex - cols;
	if(nWall.secondCellIndex < 0 )
		nWall.secondCellIndex = -1;//make invalid, check for index

	sWall.wallDirection = SOUTH;
	sWall.firstCellIndex = firstMazeIndex;
	sWall.secondCellIndex = firstMazeIndex + cols;
	if(sWall.secondCellIndex >= mazeNodes.size() )
		sWall.secondCellIndex = -1;

	eWall.wallDirection = EAST;
	eWall.firstCellIndex = firstMazeIndex;
	eWall.secondCellIndex = firstMazeIndex + 1;
	if(firstMazeIndex%cols == (cols-1))//right edge of maze
		eWall.secondCellIndex = -1;

	wWall.wallDirection = WEST;
	wWall.firstCellIndex = firstMazeIndex;
	wWall.secondCellIndex = firstMazeIndex - 1;
	if(firstMazeIndex%cols == 0)//left edge of maze
		wWall.secondCellIndex = -1;

	mazeWalls.push_back(nWall);
	mazeWalls.push_back(sWall);
	mazeWalls.push_back(eWall);
	mazeWalls.push_back(wWall);

	unsigned int seed = static_cast<unsigned int>(std::time(NULL));
        std::srand(seed);
	//std::rand() % (end - begin + 1) + begin

	while(!mazeWalls.empty() )
	{
		//pick random wall
		unsigned int randWallIndex = std::rand() % (mazeWalls.size() );
		//    1.If the cell on the opposite side isn't in the maze yet:
		//        -Make the wall a passage and mark the cell on the opposite side as part of the maze.
		//        -Add the neighboring walls of the cell to the wall list.
		//    2.If the cell on the opposite side already was in the maze, remove the wall from the list.

		//first check if the passage goes off of the maze grid
		if(mazeWalls[randWallIndex].firstCellIndex < 0 ||
			mazeWalls[randWallIndex].firstCellIndex >= mazeNodes.size() ||
			mazeWalls[randWallIndex].secondCellIndex < 0 ||
			mazeWalls[randWallIndex].secondCellIndex >= mazeNodes.size()  )
		{
			mazeWalls.erase(mazeWalls.begin() + randWallIndex);
		}
		else//valid maze nodes on both sides, potential path
		{
			if(mazeNodes[mazeWalls[randWallIndex].secondCellIndex].partOfMaze)//already in maze
			{
				mazeWalls.erase(mazeWalls.begin() + randWallIndex);
			}
			else
			{
			//    -Make the wall a passage and mark the cell on the opposite side as part of the maze.
			//    -Add the neighboring walls of the cell to the wall list.
				mazeNodes[mazeWalls[randWallIndex].secondCellIndex].partOfMaze = true;
				if(mazeWalls[randWallIndex].wallDirection == NORTH)
				{
					mazeNodes[mazeWalls[randWallIndex].firstCellIndex].north = true;
					mazeNodes[mazeWalls[randWallIndex].secondCellIndex].south = true;
				}
				else if(mazeWalls[randWallIndex].wallDirection == SOUTH)
				{
					mazeNodes[mazeWalls[randWallIndex].firstCellIndex].south = true;
					mazeNodes[mazeWalls[randWallIndex].secondCellIndex].north = true;
				}
				else if(mazeWalls[randWallIndex].wallDirection == EAST)
				{
					mazeNodes[mazeWalls[randWallIndex].firstCellIndex].east = true;
					mazeNodes[mazeWalls[randWallIndex].secondCellIndex].west = true;
				}
				else if(mazeWalls[randWallIndex].wallDirection == WEST)
				{
					mazeNodes[mazeWalls[randWallIndex].firstCellIndex].west = true;
					mazeNodes[mazeWalls[randWallIndex].secondCellIndex].east = true;
				}

				unsigned int newMazeIndex = mazeWalls[randWallIndex].secondCellIndex;

				//add to wall list
				MazeWall nWall,sWall,eWall,wWall;
				nWall.wallDirection = NORTH;
				nWall.firstCellIndex = newMazeIndex;
				nWall.secondCellIndex = newMazeIndex - cols;
				if(nWall.secondCellIndex < 0 )
					nWall.secondCellIndex = -1;//make invalid, check for index

				sWall.wallDirection = SOUTH;
				sWall.firstCellIndex = newMazeIndex;
				sWall.secondCellIndex = newMazeIndex + cols;
				if(sWall.secondCellIndex >= mazeNodes.size() )
					sWall.secondCellIndex = -1;

				eWall.wallDirection = EAST;
				eWall.firstCellIndex = newMazeIndex;
				eWall.secondCellIndex = newMazeIndex + 1;
				if(newMazeIndex%cols == (cols-1))//right edge of maze
					eWall.secondCellIndex = -1;

				wWall.wallDirection = WEST;
				wWall.firstCellIndex = newMazeIndex;
				wWall.secondCellIndex = newMazeIndex - 1;
				if(newMazeIndex%cols == 0)//left edge of maze
					wWall.secondCellIndex = -1;

				mazeWalls.push_back(nWall);
				mazeWalls.push_back(sWall);
				mazeWalls.push_back(eWall);
				mazeWalls.push_back(wWall);


			}

		}

	}





	for(unsigned int i=0; i < mazeNodes.size() ; i++)
	{
		//printf("node:%i,%i: %i %i %i %i\n", i%cols, i/cols, mazeNodes[i].north,
		//	mazeNodes[i].south, mazeNodes[i].east, mazeNodes[i].west );

	}



}


void GameModel::loadPit()
{
     fogColor = Vector3(.1f,.1f,.3f);
    ambientLight = Vector3(.0f,.0f,.07f);


    Bundle* bundleRoom = Bundle::create ( "res/apartment.gpb" );
    _scene =  bundleRoom->loadScene();
    SAFE_RELEASE ( bundleRoom );



    Scene* cscene;
    Bundle* bundleRoomC = Bundle::create ( "res/box.gpb" );
    cscene =  bundleRoomC->loadScene();
    SAFE_RELEASE ( bundleRoomC );


    Node *boxNode = cscene->findNode ( "box" );
    //_scene->addNode ( boxNode );
    boxNode->getModel()->setMaterial ( "res/wall.material" );


    //point  light test
    _lightNode = Node::create ( "lighta" );
    Light* light = Light::createSpot(Vector3(0.8f,0.8f,0.8f), 101.0f, MATH_DEG_TO_RAD(30.0f), MATH_DEG_TO_RAD(60.0f));
    //Light* light = Light::createPoint ( Vector3 ( 1.0f,1.0f,0.99f ), 7.0f );

    _lightNode->setLight ( light );
    //_lightNode->setTranslation(0.f,2.f,0);
    //_lightNode->setTranslation ( _scene->findNode ( "lightA" )->getTranslation() );
    //_lightNode->setTranslation ( _scene->findNode ( "playerA" )->getTranslation() );
    //_lightNode->setRotation(Vector3(1.f,0,0), MATH_DEG_TO_RAD(-180.0f));
    //_scene->addNode ( _lightNode );


    Node* roomANode = _scene->findNode ( "apartment" );
    Model* roomAModel = roomANode->getModel();
    roomANode->setScale(1.0f);
    //roomAModel->setMaterial("res/box.material");
    //Material* boxMaterial = roomAModel->setMaterial("res/light.material");
    //boxMaterial->getParameter("u_ambientColor")->setValue(_scene->getAmbientColor());
    //boxMaterial->getParameter("u_lightColor")->setValue(light->getColor());
    //boxMaterial->getParameter("u_lightDirection")->setValue(lightNode->getForwardVectorView());

    //static level physics tree
    PhysicsRigidBody::Parameters paramsr;
    paramsr.mass = 0.0f;
    roomANode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( roomANode->getModel()->getMesh() ), &paramsr );


    Node* cityNode = _scene->findNode("city");
    Model* cityModel = cityNode->getModel();
    Material* cityMat = cityModel->setMaterial("res/lamp.material");
    //cityMat->getTechnique("main")->getParameter("u_fogColor")->setValue(fogColor );


    //solid objects

    Node* couchNode = _scene->findNode("couch");
    Model* couchModel = couchNode->getModel();
    Material* couchMat = couchModel->setMaterial("res/couch.material");
    //setupMaterialSpot(_lightNode,couchMat);
    couchNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( couchNode->getModel()->getMesh() ) );

    Node* shelfNode = _scene->findNode("shelf");
    Model* shelfModel = shelfNode->getModel();
    Material* shelfMat = shelfModel->setMaterial("res/couch.material");
    //setupMaterialSpot(_lightNode,shelfMat);
    shelfNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( shelfNode->getModel()->getMesh() ) );

    Node* bedNode = _scene->findNode("bed");
    Model* bedModel = bedNode->getModel();
    Material* bedMat = bedModel->setMaterial("res/couch.material");
    //setupMaterialSpot(_lightNode,bedMat);
    bedNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( bedNode->getModel()->getMesh() ) );


    Node* tableNode = _scene->findNode("table");
    Model* tableModel = tableNode->getModel();
    tableModel->setMaterial("res/wall.material");
    tableNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( tableNode->getModel()->getMesh() ) );






    Node* doorNode = _scene->findNode("door");
    Model* doorModel = doorNode->getModel();
    //doorNode->setScale(0.6f);

    //doorNode->rotateX(MATH_DEG_TO_RAD(90.f) );
    doorModel->setMaterial("res/brown.material");
    paramsr.mass = 5.0f;
    doorNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(), &paramsr );
    doorNode->setScale(1.04f);
    doorNode->translate(0.05f,0.0f,0.0f);

    //PhysicsHingeConstraint* doorHinge = mPhysicsController->createHingeConstraint(doorNode->getCollisionObject()->asRigidBody(),Quaternion(Vector3(0,0,1), MATH_DEG_TO_RAD(-90.f)) * Quaternion(Vector3(0,1,0), MATH_DEG_TO_RAD(-90.f)), Vector3(0.3f,0,0) );
    //PhysicsGenericConstraint* doorHinge = mPhysicsController->createGenericConstraint(doorNode->getCollisionObject()->asRigidBody());
    //doorHinge->setAngularLowerLimit(Vector3(0.f,0,-99.f));
    //doorHinge->setAngularUpperLimit(Vector3(0.f,0,99.f));
    //doorHinge->setTranslationOffsetA(doorNode->getTranslation() + Vector3(0,-0.99f,0));

    PhysicsSocketConstraint* doorHinge = mPhysicsController->createSocketConstraint(doorNode->getCollisionObject()->asRigidBody(), Vector3(.5f,0,-.5f) );
    mPhysicsController->createSocketConstraint(doorNode->getCollisionObject()->asRigidBody(), Vector3(.5f,0,.5f) );

    doorBody = doorNode->getCollisionObject()->asRigidBody();
    //doorBody->setKinematic(true);//freeze





    //moveable objects--------------------

    Node* hatNode = _scene->findNode("hat");
    //hatNode->setTranslation(0,2.f,1.f);
    hatNode->scale(.9f);
    Model* hatModel = hatNode->getModel();
    hatModel->setMaterial("res/wall.material");
    paramsr.mass = 1.0f;
    hatNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::sphere(),  &paramsr );
    //hatNode->getCollisionObject()->asRigidBody()->setFriction(0.01f);
    //hatNode->getCollisionObject()->asRigidBody()->setAnisotropicFriction(Vector3(1.01,1.01f,1.01f));
    //hatNode->getCollisionObject()->asRigidBody()->setFriction(0.01f);
    hatNode->setTag("name","hat");
    //objectList.push_back(hatNode);
    //hatNode->

    Node* hatNodeB = hatNode->clone();
    hatNodeB->translate(1.f,0,0);
    hatNodeB->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::sphere(),  &paramsr );
    _scene->addNode(hatNodeB);
    //objectList.push_back(hatNodeB);


    Node* ballANode = _scene->findNode("ballA");
    Model* ballAModel = ballANode->getModel();
    ballAModel->setMaterial("res/green.material");
    //ballANode->setScale(1.f,.1f,1.f);
    paramsr.mass = 1.0f;
    ballANode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::sphere(),  &paramsr );
    ballANode->setTag("name","hat");
    //objectList.push_back(ballANode);

    Node* pillowNode = _scene->findNode("pillow");
    Model* pillowModel = pillowNode->getModel();
    pillowModel->setMaterial("res/wall.material");
    paramsr.mass = 1.0f;
    pillowNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );

    Node* clockNode = _scene->findNode("clock");
    Model* clockModel = clockNode->getModel();
    clockModel->setMaterial("res/blue.material");
    paramsr.mass = 1.0f;
    clockNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );

    Node* teddyNode = _scene->findNode("teddy");
    Model* teddyModel = teddyNode->getModel();
    teddyModel->setMaterial("res/brown.material");
    paramsr.mass = 1.0f;
    teddyNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );

    Node* catheadNode = _scene->findNode("cathead");
    Model* catheadModel = catheadNode->getModel();
    catheadModel->setMaterial("res/blue.material");
    paramsr.mass = 1.0f;
    catheadNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );

    Node* eggNode = _scene->findNode("egg");
    Model* eggModel = eggNode->getModel();
    eggModel->setMaterial("res/green.material");
    paramsr.mass = 1.0f;
    eggNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );

    Node* ghostNode = _scene->findNode("ghost");
    Model* ghostModel = ghostNode->getModel();
    ghostModel->setMaterial("res/couch.material");
    paramsr.mass = 1.0f;
    ghostNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );



    for(int i=0; i < 20.f ; i++)
    {
        int randChoice = (int) ( (MATH_RANDOM_0_1() * 3 )  );

        Node *newThing;

        if(randChoice == 0)
        {
            newThing = pillowNode->clone();
            newThing->setScale(0.9f + MATH_RANDOM_0_1() * .2f);
            newThing->setTranslation((MATH_RANDOM_0_1() - .5f) * 3.f, 2.f,(MATH_RANDOM_0_1() - .5f) *  3.f);
            newThing->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );
            _scene->addNode(newThing);
        }
        else if(randChoice == 1)
        {
            newThing = teddyNode->clone();
            newThing->setScale(0.9f + MATH_RANDOM_0_1() * .2f);
            newThing->setTranslation((MATH_RANDOM_0_1() - .5f) * 3.f, 2.f,(MATH_RANDOM_0_1() - .5f) *  3.f);
            newThing->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );
            _scene->addNode(newThing);
        }
        else if(randChoice == 2)
        {
            newThing = hatNode->clone();
            newThing->setScale(0.9f + MATH_RANDOM_0_1() * .2f);
            newThing->setTranslation((MATH_RANDOM_0_1() - .5f) * 3.f, 2.f,(MATH_RANDOM_0_1() - .5f) *  3.f);
            newThing->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsr );
            _scene->addNode(newThing);
        }

        int randMaterial = (int) ( (MATH_RANDOM_0_1() * 3 )  );
        if(randMaterial == 0)
        {
            newThing->getModel()->setMaterial("res/green.material");

        }
        else if(randMaterial == 1)
        {
            newThing->getModel()->setMaterial("res/blue.material");

        }
        if(randMaterial == 2)
        {
            newThing->getModel()->setMaterial("res/couch.material");

        }

    }





    guyNode = Node::create ( "playerA" );
    //guyNode->setTranslation ( _scene->findNode ( "player" )->getTranslation() );
    //guyNode->setTranslation(-13.f,1.f,20.f);
    _scene->addNode ( guyNode );
    guyNode->setTranslation(-1.f,2.1f,-3.4f);
    PhysicsRigidBody::Parameters params ( 50.0f );
    /*
    guyNode->setCollisionObject ( PhysicsCollisionObject::CHARACTER,
                                  PhysicsCollisionShape::capsule ( 0.35f, 1.0f, Vector3 ( 0, 0.5f, 0 ), true ), &params );
    _charController = static_cast<PhysicsCharacter*> ( guyNode->getCollisionObject() );
    */
    guyNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,
                                  PhysicsCollisionShape::capsule(0.4f,2.2f), &params );//.6 1.3
    //PhysicsCollisionShape::sphere(1.f), &params );
    //_charController->getCurrentVelocity();
    //guyNode->setTranslation(2.f,2.f,0);
    _charControllerR = static_cast<PhysicsRigidBody*> ( guyNode->getCollisionObject() );



    _charControllerR->setGravity(Vector3(0, -1.f,0));
    _charControllerR->setDamping(0.87f,0.01f);
    _charControllerR->setAngularFactor(Vector3(0.0,1.0,0.0));

    //_charController->getCurrentVelocity();


    Material* npcMat = Material::create("res/npc.material");
    setupMaterialSpot(_lightNode,npcMat);
    npcA = new Npc( Vector3(.5f,2.2f,8.1f), npcMat, Npc::TALLCAPE );
    _scene->addNode(npcA->getNode());




    _scene->getActiveCamera()->setAspectRatio ( ( float ) Game::getInstance()->getWidth() / ( float ) Game::getInstance()->getHeight() );
    _scene->getActiveCamera()->setFarPlane ( 200.f );

    Camera* sceneCam =  _scene->getActiveCamera();

    SAFE_RELEASE ( _camPitch );
    SAFE_RELEASE ( _camRoot );
    _camRoot = Node::create ( "FirstPersonCamera_root" );
    _camPitch = Node::create ( "FirstPersonCamera_pitch" );
    _camRoot->addChild ( _camPitch );
    _camPitch->setCamera ( _scene->getActiveCamera() );
    _camRoot->setTranslation ( 0,1.f, 0.f );

    //don't want to inherit orientation of guyNode/characterController, so just sync the translations during update
    //guyNode->addChild ( _camRoot );











    Material* planeMaterial = roomAModel->setMaterial ( "res/wall.material" ); //furniture

    setupMaterialSpot(_lightNode, planeMaterial);

    Material* floormat = roomAModel->setMaterial("res/floor.material", 1);
    setupMaterialSpot(_lightNode, floormat);



    //planeMaterial->getTechnique("main")->getParameter("u_fogColor")->setValue(fogColor );//light->getRange()

    Node *lampNode = _scene->findNode ( "lamp" );
    //_scene->addNode ( lampNode );
    lampNode->getModel()->setMaterial ( "res/lamp.material" );
    lampNode->setTranslation(0,3.35f,1.0);
    //lampNode->rotateX(2.1f);
    lampNode->addChild(_lightNode);
    _lightNode->setTranslation(0,-.1f,0);
    PhysicsRigidBody::Parameters paramsb;
    paramsb.mass = 1.0f;
    //lampNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( lampNode->getModel()->getMesh() ), &paramsb );
    float boxsize = 1.0f;
    PhysicsCollisionObject *lampcol = lampNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,
                                      PhysicsCollisionShape::sphere(), &paramsb );
    //lamp = lampcol->asRigidBody();
    lampcol->asRigidBody()->setDamping(0.299f,0.291f);
    //need z coordinate for up somehow, blender export issue?
    PhysicsSocketConstraint* lampConstaint = mPhysicsController->createSocketConstraint(lampcol->asRigidBody(), Vector3(0,0,0.2f));


    //thingConstraint->setTranslationOffsetA(Vector3(0,2.8f,0));
    //thingConstraint->setLinearLowerLimit(Vector3(-1.1,-0.5,-1.1));
    //thingConstraint->setLinearUpperLimit(Vector3(1.1,0.5,1.1));

    //thingConstraint->setAngularLowerLimit(Vector3(-1.1,-0.1,-1.1));
    //thingConstraint->setAngularUpperLimit(Vector3(1.1,0.1,1.1));




    Node *trafficSoundNode = Node::create();

    AudioSource* audioSource = AudioSource::create ( "res/audio/traffic.ogg" );
    assert ( audioSource );
    audioSource->setLooped ( true );
    //audioSource->setVelocity(10.f,10.f,10.f);
    trafficSoundNode->setAudioSource ( audioSource );
    trafficSoundNode->getAudioSource()->play();
    trafficSoundNode->getAudioSource()->setGain ( 2.5f );
    //trafficSoundNode->getAudioSource()->
    audioSource->release();
    trafficSoundNode->setTranslation(4.5f,1.f,2.f);
    //_scene->getNode()->get
    _scene->addNode ( trafficSoundNode );


    knockingSoundNode = Node::create();
 
    AudioSource* audioSourceB = AudioSource::create ( "res/audio/knock.ogg" );
    assert ( audioSourceB );
    audioSourceB->setLooped ( true );
    //audioSourceB->setVelocity(10.f,10.f,10.f);
    knockingSoundNode->setAudioSource ( audioSourceB );
    //knockingSoundNode->getAudioSource()->play();
    knockingSoundNode->getAudioSource()->setGain ( 2.5f );
    //knockingSoundNode->getAudioSource()->
    audioSourceB->release();
    knockingSoundNode->setTranslation(doorNode->getTranslation());
    //_scene->getNode()->get
    _scene->addNode ( knockingSoundNode );




    AudioListener::getInstance()->setCamera ( sceneCam );
    //_scene->bindAudioListenerToCamera(false);
    _scene->bindAudioListenerToCamera(true);
}

void GameModel::setupMaterialSpot( Node *nLightnode, Material *nMaterial)
{
    nMaterial->getTechnique ( "main" )->getParameter ( "u_spotLightPosition" )->bindValue ( nLightnode, &Node::getTranslationView );
    nMaterial->getTechnique("main")->getParameter("u_spotLightRangeInverse")->setValue(1.f/nLightnode->getLight()->getRange()  );
    nMaterial->getTechnique("main")->getParameter("u_spotLightDirection")->bindValue(nLightnode, &Node::getForwardVectorView);
    nMaterial->getTechnique("main")->getParameter("u_spotLightInnerAngleCos")->setValue(nLightnode->getLight()->getInnerAngleCos());
    nMaterial->getTechnique("main")->getParameter("u_spotLightOuterAngleCos")->setValue(nLightnode->getLight()->getOuterAngleCos());
    nMaterial->getTechnique( "main" )->getParameter ( "u_ambientColor" )->setValue ( ambientLight );
    nMaterial->getTechnique( "main" )->getParameter ( "u_lightColor" )->setValue ( Vector3 ( nLightnode->getLight()->getColor() ) );
}



void GameModel::loadHouse()
{





    packagesCount = 0;


    //round item ideas: helmet, drum, wide wheel, gumball machine,

    //fogColor = Vector3(.1f,.1f,.3f);
    fogColor = Vector3::zero();
    //ambientLight = Vector3(.0f,.0f,.07f);
    ambientLight = Vector3(.17f,.2f,.2f);
    //ambientLight = Vector3(.9f,.9f,.97f);

    Bundle* bundleRoom = Bundle::create ( "res/houseA.gpb" );
    _scene =  bundleRoom->loadScene();
    SAFE_RELEASE ( bundleRoom );


    /*
    Scene* cscene;
    Bundle* bundleRoomC = Bundle::create ( "res/box.gpb" );
    cscene =  bundleRoomC->loadScene();
    SAFE_RELEASE ( bundleRoomC );
    Node *boxNode = cscene->findNode ( "box" );
    //_scene->addNode ( boxNode );
    boxNode->getModel()->setMaterial ( "res/wall.material" );
    */



    //point  light test
    _lightNode = Node::create ( "lighta" );
    Light* light = Light::createSpot(Vector3(1.0f,1.0f,1.0f), 11.0f, MATH_DEG_TO_RAD(15.f), MATH_DEG_TO_RAD(15.0f));
    //Light* light = Light::createPoint ( Vector3 ( 1.0f,1.0f,0.99f ), 7.0f );

    _lightNode->setLight ( light );
    //_lightNode->setTranslation(0.f,2.f,0);
    //_lightNode->setTranslation ( _scene->findNode ( "lightA" )->getTranslation() );
    //_lightNode->setTranslation ( _scene->findNode ( "playerA" )->getTranslation() );
    //_lightNode->setRotation(Vector3(1.f,0,0), MATH_DEG_TO_RAD(-180.0f));
    //_scene->addNode ( _lightNode );

    /*
    Node* roomANode = _scene->findNode ( "livingRoom" );
    Model* roomAModel = roomANode->getModel();
    //roomANode->setScale(1.0f);
    //static level physics tree
    PhysicsRigidBody::Parameters paramsr;
    paramsr.mass = 0.0f;
    roomANode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( roomANode->getModel()->getMesh() ), &paramsr );
    */
    Node* roomBNode = _scene->findNode ( "roomB" );
    Model* roomBModel = roomBNode->getModel();
    //roomANode->setScale(1.0f);
    //static level physics tree
    PhysicsRigidBody::Parameters paramsrb;
    paramsrb.mass = 0.0f;
    roomBNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( roomBNode->getModel()->getMesh() ), &paramsrb );



        //setupMaterialSpot(_lightNode, planeMaterial);


    //Material* planeMaterial = roomAModel->setMaterial ( "res/wall.material" ); //furniture
    //setupMaterialSpot(_lightNode, planeMaterial);
    //Material* floormat = roomAModel->setMaterial("res/blue.material", 1);
    //setupMaterialSpot(_lightNode, floormat);


    Bundle* bundleObjects = Bundle::create ( "res/objects.gpb" );
    Scene* objectsScene =  bundleObjects->loadScene();
    SAFE_RELEASE ( bundleObjects );

    PhysicsRigidBody::Parameters paramsObjects;




    /*
    Node* packageNode = objectsScene->findNode("pillow");
    //packageNode->setTranslation(0,2.f,1.f);
    packageNode->scale(.5f);
    Model* packageModel = packageNode->getModel();
    packageModel->setMaterial("res/package.material");
    packageNode->setTranslation(3,2,2);
    paramsObjects.mass = 1.0f;
    packageNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsObjects );
    packageNode->setTag("type","package");
    _scene->addNode(packageNode);
    */

    int mazeCols = 5;
    int mazeRows = 5;
    createMaze(mazeCols,mazeRows, 1);

    /*
    Material* wallMat = roomBModel->setMaterial ( "res/wall.material" );
    setupMaterialSpot(_lightNode, wallMat);
    Material* floorMat = roomBModel->setMaterial ("res/couch.material", 1);
    setupMaterialSpot(_lightNode, floorMat);
    */
    float roomLength = 10.f;

    for(unsigned int i=0; i < mazeNodes.size() ; i++)
	{
		//printf("node:%i,%i: %i %i %i %i\n", i%cols, i/cols, mazeNodes[i].north,
		//	mazeNodes[i].south, mazeNodes[i].east, mazeNodes[i].west );
		int col =  i%mazeCols;
		int row =  i/mazeCols;

        Node* roomNewNode = _scene->findNode ( "roomB" )->clone();
        Model* roomNewModel = roomNewNode->getModel();
        PhysicsRigidBody::Parameters paramsrnewb;

        //roomNewModel->setMaterial ( "res/wall.material" );
        _scene->addNode(roomNewNode);

        paramsrnewb.mass = 0.0f;
        //roomNewNode->translate(-10 * i + -5, 0, 0);
        Vector3 roomCenter(-0.f + col * -roomLength, 0, 0 + row * -roomLength);
        roomNewNode->translate(roomCenter);
        //roomNewNode->setTranslation(1 * i + 5, 0, 0);
        roomNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( roomNewNode->getModel()->getMesh() ), &paramsrnewb );
        roomNewNode->setTag("type","room");
        //Material* newRoomMat = roomNewModel->setMaterial ( "res/wall.material" ); //furniture
        //setupMaterialSpot(_lightNode, newRoomMat);

        Material* wallMat = roomNewModel->setMaterial ( "res/floor.material" );
        setupMaterialSpot(_lightNode, wallMat);
        Material* floorMat = roomNewModel->setMaterial ("res/wall.material", 1);
        setupMaterialSpot(_lightNode, floorMat);
        //Material* woodMat = roomNewModel->setMaterial ("res/wall.material", 2);
        //setupMaterialSpot(_lightNode, woodMat);



        int shelvesOption = (int) (MATH_RANDOM_0_1() * 4.f);
        if(i ==0 || i == mazeNodes.size()-1)
            shelvesOption = -1;
        if(shelvesOption == 0)
        {
            Node* shelvesNewNode = _scene->findNode ( "shelvesA" )->clone();
            Model* shelvesNewModel = shelvesNewNode->getModel();
            PhysicsRigidBody::Parameters paramsrnewbs;
            //shelvesNewModel->setMaterial ( "res/wall.material" );
            _scene->addNode(shelvesNewNode);
            paramsrnewbs.mass = 0.0f;
            //shelvesNewNode->translate(-10 * i + -5, 0, 0);
            shelvesNewNode->translate(roomCenter);
            //shelvesNewNode->setTranslation(1 * i + 5, 0, 0);
            shelvesNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( shelvesNewNode->getModel()->getMesh() ), &paramsrnewbs );
            shelvesNewNode->setTag("type","shelves");
            shelvesNewModel->setMaterial("res/brown.material");
        }
        else if(shelvesOption == 1)
        {
            Node* shelvesNewNode = _scene->findNode ( "shelvesB" )->clone();
            Model* shelvesNewModel = shelvesNewNode->getModel();
            PhysicsRigidBody::Parameters paramsrnewbs;
            //shelvesNewModel->setMaterial ( "res/wall.material" );
            _scene->addNode(shelvesNewNode);
            paramsrnewbs.mass = 0.0f;
            //shelvesNewNode->translate(-10 * i + -5, 0, 0);
            shelvesNewNode->translate(roomCenter);
            //shelvesNewNode->setTranslation(1 * i + 5, 0, 0);
            shelvesNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( shelvesNewNode->getModel()->getMesh() ), &paramsrnewbs );
            shelvesNewNode->setTag("type","shelves");
            shelvesNewModel->setMaterial("res/brown.material");
        }
        else if(shelvesOption == 2)
        {
            Node* shelvesNewNode = _scene->findNode ( "shelvesC" )->clone();
            Model* shelvesNewModel = shelvesNewNode->getModel();
            PhysicsRigidBody::Parameters paramsrnewbs;
            //shelvesNewModel->setMaterial ( "res/wall.material" );
            _scene->addNode(shelvesNewNode);
            paramsrnewbs.mass = 0.0f;
            //shelvesNewNode->translate(-10 * i + -5, 0, 0);
            shelvesNewNode->translate(roomCenter);
            //shelvesNewNode->setTranslation(1 * i + 5, 0, 0);
            shelvesNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( shelvesNewNode->getModel()->getMesh() ), &paramsrnewbs );
            shelvesNewNode->setTag("type","shelves");
            shelvesNewModel->setMaterial("res/brown.material");
        }
        else if(shelvesOption == 3)
        {
            Node* shelvesNewNode = _scene->findNode ( "shelvesD" )->clone();
            Model* shelvesNewModel = shelvesNewNode->getModel();
            PhysicsRigidBody::Parameters paramsrnewbs;
            //shelvesNewModel->setMaterial ( "res/wall.material" );
            _scene->addNode(shelvesNewNode);
            paramsrnewbs.mass = 0.0f;
            //shelvesNewNode->translate(-10 * i + -5, 0, 0);
            shelvesNewNode->translate(roomCenter);
            //shelvesNewNode->setTranslation(1 * i + 5, 0, 0);
            shelvesNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( shelvesNewNode->getModel()->getMesh() ), &paramsrnewbs );
            shelvesNewNode->setTag("type","shelves");
            shelvesNewModel->setMaterial("res/brown.material");
        }

        /*
        Node* deskNewNode = _scene->findNode("desk")->clone();
        Model* deskNewModel = deskNewNode->getModel();
        //PhysicsRigidBody::Parameters paramsdnewb;
        deskNewModel->setMaterial ( "res/wall.material" );
        //Material* newdeskMat = deskNewModel->setMaterial ( "res/wall.material" ); //furniture
        //setupMaterialSpot(_lightNode, newdeskMat);
        _scene->addNode(deskNewNode);
        paramsrnewb.mass = 0.0f;
        //deskNewNode->setScale(0.3f);
        //deskNewNode->translate(-10 * i + -5, 0, 0);
        deskNewNode->translate(-8.f + col * -8.f, 0, 0.f);
        //deskNewNode->setTranslation(roomCenter + Vector3(roomLength * .25f, 0, 0));
        //deskNewNode->setTranslation(1 * i + 5, 0, 0);
        //deskNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::mesh ( deskNewNode->getModel()->getMesh() ), &paramsrnewb );
        deskNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(), &paramsrnewb );
        */





        Node* packageNode = objectsScene->findNode("pillow")->clone();
        //packageNode->setTranslation(0,2.f,1.f);
        packageNode->scale(.5f);
        Model* packageModel = packageNode->getModel();
        packageModel->setMaterial("res/package.material");
        packageNode->setTranslation(roomCenter + Vector3(MATH_RANDOM_MINUS1_1() * 4.f,2.f,MATH_RANDOM_MINUS1_1() * 4.f));
        paramsObjects.mass = 1.0f;
        packageNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsObjects );
        packageNode->setTag("type","package");
        _scene->addNode(packageNode);
        packageNode->getCollisionObject()->asRigidBody()->setDamping(.2f,.2f);

        //random objects
        for(int b = 0; b < 10 ; b++)
        {
            int objectOption  = (int) (MATH_RANDOM_0_1() * 4.f);
            if( objectOption == 0)
            {
                Node* hatNode = objectsScene->findNode("hat")->clone();
                //hatNode->setTranslation(0,2.f,1.f);
                hatNode->scale(.9f);
                Model* hatModel = hatNode->getModel();
                hatModel->setMaterial("res/brown.material");
                hatNode->setTranslation(roomCenter + Vector3(MATH_RANDOM_MINUS1_1() * 4.f,2.f,MATH_RANDOM_MINUS1_1() * 4.f));
                paramsObjects.mass = 1.0f;
                hatNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsObjects );
                hatNode->setTag("type","money");
                _scene->addNode(hatNode);
                hatNode->getCollisionObject()->asRigidBody()->setDamping(.2f,.2f);
            }
            else if( objectOption == 1)
            {
                Node* hatNode = objectsScene->findNode("teddy")->clone();
                //hatNode->setTranslation(0,2.f,1.f);
                hatNode->scale(.6f);
                Model* hatModel = hatNode->getModel();
                hatModel->setMaterial("res/blue.material");
                hatNode->setTranslation(roomCenter + Vector3(MATH_RANDOM_MINUS1_1() * 4.f,2.f,MATH_RANDOM_MINUS1_1() * 4.f));
                paramsObjects.mass = 1.0f;
                hatNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsObjects );
                hatNode->setTag("type","money");
                _scene->addNode(hatNode);
                hatNode->getCollisionObject()->asRigidBody()->setDamping(.2f,.2f);
            }
            else if( objectOption == 2)
            {
                Node* hatNode = objectsScene->findNode("ghost")->clone();
                //hatNode->setTranslation(0,2.f,1.f);
                hatNode->scale(.6f);
                Model* hatModel = hatNode->getModel();
                hatModel->setMaterial("res/brown.material");
                hatNode->setTranslation(roomCenter + Vector3(MATH_RANDOM_MINUS1_1() * 4.f,2.f,MATH_RANDOM_MINUS1_1() * 4.f));
                paramsObjects.mass = 1.0f;
                hatNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsObjects );
                hatNode->setTag("type","money");
                _scene->addNode(hatNode);
                hatNode->getCollisionObject()->asRigidBody()->setDamping(.2f,.2f);
            }
            else if( objectOption == 3)
            {
                Node* hatNode = objectsScene->findNode("clock")->clone();
                //hatNode->setTranslation(0,2.f,1.f);
                hatNode->scale(.9f);
                Model* hatModel = hatNode->getModel();
                hatModel->setMaterial("res/brown.material");
                hatNode->setTranslation(roomCenter + Vector3(MATH_RANDOM_MINUS1_1() * 4.f,2.f,MATH_RANDOM_MINUS1_1() * 4.f));
                paramsObjects.mass = 1.0f;
                hatNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::capsule(),  &paramsObjects );
                hatNode->setTag("type","money");
                _scene->addNode(hatNode);
                hatNode->getCollisionObject()->asRigidBody()->setDamping(.2f,.2f);
            }


        }




        //blocked doors
        if(!mazeNodes[i].east)
		{
            Node* blockNewNode = _scene->findNode("block")->clone();
            Model* blockNewModel = blockNewNode->getModel();
            //PhysicsRigidBody::Parameters paramsdnewb;
            blockNewModel->setMaterial ( "res/wall.material" );
            //Material* newblockMat = blockNewModel->setMaterial ( "res/wall.material" ); //furniture
            //setupMaterialSpot(_lightNode, newblockMat);
            _scene->addNode(blockNewNode);
            paramsrnewb.mass = 0.0f;
            blockNewNode->setTranslation(roomCenter - Vector3(roomLength * .5f, 0, 0));
            blockNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(), &paramsrnewb );

            //second collision so pathfinder can catch
            Node* newCollision = Node::create();
            _scene->addNode(newCollision);
            newCollision->setTranslation(blockNewNode->getTranslationWorld() + Vector3(0,-.15f,0) );
            newCollision->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(Vector3(0.5f,0.5f,0.5f)), &paramsrnewb );
        }


        if(!mazeNodes[i].west)
		{
            Node* blockNewNode = _scene->findNode("block")->clone();
            Model* blockNewModel = blockNewNode->getModel();
            //PhysicsRigidBody::Parameters paramsdnewb;
            blockNewModel->setMaterial ( "res/wall.material" );
            //Material* newblockMat = blockNewModel->setMaterial ( "res/wall.material" ); //furniture
            //setupMaterialSpot(_lightNode, newblockMat);
            _scene->addNode(blockNewNode);
            paramsrnewb.mass = 0.0f;
            blockNewNode->setTranslation(roomCenter + Vector3(roomLength * .5f, 0, 0));
            blockNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(), &paramsrnewb );

            //second collision so pathfinder can catch
            Node* newCollision = Node::create();
            _scene->addNode(newCollision);
            newCollision->setTranslation(blockNewNode->getTranslationWorld() + Vector3(0,-.15f,0) );
            newCollision->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(Vector3(0.5f,0.5f,0.5f)), &paramsrnewb );
        }

        if(!mazeNodes[i].north)
		{
            Node* blockNewNode = _scene->findNode("block")->clone();
            Model* blockNewModel = blockNewNode->getModel();
            //PhysicsRigidBody::Parameters paramsdnewb;
            blockNewModel->setMaterial ( "res/wall.material" );
            //Material* newblockMat = blockNewModel->setMaterial ( "res/wall.material" ); //furniture
            //setupMaterialSpot(_lightNode, newblockMat);
            _scene->addNode(blockNewNode);
            paramsrnewb.mass = 0.0f;
            blockNewNode->setTranslation(roomCenter + Vector3(0, 0, roomLength * .5f));
            blockNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(), &paramsrnewb );

            //second collision so pathfinder can catch
            Node* newCollision = Node::create();
            _scene->addNode(newCollision);
            newCollision->setTranslation(blockNewNode->getTranslationWorld() + Vector3(0,-.15f,0) );
            newCollision->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(Vector3(0.5f,0.5f,0.5f)), &paramsrnewb );
        }


        if(!mazeNodes[i].south)
		{
            Node* blockNewNode = _scene->findNode("block")->clone();
            Model* blockNewModel = blockNewNode->getModel();
            //PhysicsRigidBody::Parameters paramsdnewb;
            blockNewModel->setMaterial ( "res/wall.material" );
            //Material* newblockMat = blockNewModel->setMaterial ( "res/wall.material" ); //furniture
            //setupMaterialSpot(_lightNode, newblockMat);
            _scene->addNode(blockNewNode);
            paramsrnewb.mass = 0.0f;
            blockNewNode->setTranslation(roomCenter - Vector3(0, 0, roomLength * .5f));
            blockNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(), &paramsrnewb );

            //second collision so pathfinder can catch
            Node* newCollision = Node::create();
            _scene->addNode(newCollision);
            newCollision->setTranslation(blockNewNode->getTranslationWorld() + Vector3(0,-.15f,0) );
            newCollision->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box(Vector3(0.5f,0.5f,0.5f)), &paramsrnewb );
        }


    }

    //entrance
    Node* entranceNewNode = _scene->findNode("block")->clone();
    Model* entranceNewModel = entranceNewNode->getModel();
    //PhysicsRigidBody::Parameters paramsdnewb;
    entranceNewModel->setMaterial ( "res/brown.material" );
    //Material* newentranceMat = entranceNewModel->setMaterial ( "res/wall.material" ); //furniture
    //setupMaterialSpot(_lightNode, newentranceMat);
    _scene->addNode(entranceNewNode);
    entranceNewNode->setScale(1.4f, 1.05f, 1.2f);
    entranceNewNode->setTranslation(0.f,0,5.f);
    entranceNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box());

    //exit door
    Node* exitNewNode = _scene->findNode("block")->clone();
    Model* exitNewModel = exitNewNode->getModel();
    //PhysicsRigidBody::Parameters paramsdnewb;
    exitNewModel->setMaterial ( "res/brown.material" );
    //Material* newexitMat = exitNewModel->setMaterial ( "res/wall.material" ); //furniture
    //setupMaterialSpot(_lightNode, newexitMat);
    _scene->addNode(exitNewNode);
    exitNewNode->setScale(1.5f, 1.1f, 1.2f);
    exitNewNode->setTranslation(-40.f,0,-45.f);
    exitNewNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,  PhysicsCollisionShape::box());
    exitNewNode->setTag("type","exit");



    guyNode = Node::create ( "playerA" );
    //guyNode->setTranslation ( _scene->findNode ( "player" )->getTranslation() );
    //guyNode->setTranslation(-13.f,1.f,20.f);
    _scene->addNode ( guyNode );
    guyNode->setTranslation(-1,1.1f,3.f);
    //guyNode->setTranslation(-41,1.2f,-42.f);//goal test
    PhysicsRigidBody::Parameters params ( 50.0f );
    /*
    guyNode->setCollisionObject ( PhysicsCollisionObject::CHARACTER,
                                  PhysicsCollisionShape::capsule ( 0.35f, 1.0f, Vector3 ( 0, 0.5f, 0 ), true ), &params );
    _charController = static_cast<PhysicsCharacter*> ( guyNode->getCollisionObject() );
    */
    guyNode->setCollisionObject ( PhysicsCollisionObject::RIGID_BODY,
                                  PhysicsCollisionShape::capsule(0.4f,1.5f), &params );//.6 1.3
    //PhysicsCollisionShape::sphere(1.f), &params );
    //_charController->getCurrentVelocity();
    //guyNode->setTranslation(2.f,2.f,0);
    _charControllerR = static_cast<PhysicsRigidBody*> ( guyNode->getCollisionObject() );
    guyNode->setTag("type","player");


    _charControllerR->setGravity(Vector3(0, -5.f,0));
    _charControllerR->setDamping(0.87f,0.01f);
    _charControllerR->setAngularFactor(Vector3(0.0,1.0,0.0));

    //_charController->getCurrentVelocity();

    //monstera
    Material* npcMat = Material::create("res/npc.material");
    setupMaterialSpot(_lightNode,npcMat);
    npcA = new Npc( Vector3(-40.f,0.6f,-40.f), npcMat, Npc::TALLCAPE );
    _scene->addNode(npcA->getNode());




    _scene->getActiveCamera()->setAspectRatio ( ( float ) Game::getInstance()->getWidth() / ( float ) Game::getInstance()->getHeight() );
    _scene->getActiveCamera()->setFarPlane ( 30.f );

    Camera* sceneCam =  _scene->getActiveCamera();

    SAFE_RELEASE ( _camPitch );
    SAFE_RELEASE ( _camRoot );
    _camRoot = Node::create ( "FirstPersonCamera_root" );
    _camPitch = Node::create ( "FirstPersonCamera_pitch" );
    _camRoot->addChild ( _camPitch );
    _camPitch->setCamera ( _scene->getActiveCamera() );
    _camRoot->setTranslation ( 0,1.f, 0.f );

    //don't want to inherit orientation of guyNode/characterController, so just sync the translations during update
    //guyNode->addChild ( _camRoot );











    _scene->addNode(_lightNode);
    //_camPitch->addChild(_lightNode);

    //thingConstraint->setTranslationOffsetA(Vector3(0,2.8f,0));
    //thingConstraint->setLinearLowerLimit(Vector3(-1.1,-0.5,-1.1));
    //thingConstraint->setLinearUpperLimit(Vector3(1.1,0.5,1.1));

    //thingConstraint->setAngularLowerLimit(Vector3(-1.1,-0.1,-1.1));
    //thingConstraint->setAngularUpperLimit(Vector3(1.1,0.1,1.1));
    /*
    thingConstraint = mPhysicsController->createSpringConstraint(tcol->asRigidBody(), _charControllerR);

    thingConstraint->setTranslationOffsetA(Vector3(0,0.5f,0));
    //thingConstraint->setBreakingImpulse(100.f);
    thingConstraint->setLinearLowerLimit(Vector3(-.1,-.1,-.1));
    thingConstraint->setLinearUpperLimit(Vector3(.1,.1,.1));

    thingConstraint->setAngularLowerLimit(Vector3(-.1,-.1,-.1));
    thingConstraint->setAngularUpperLimit(Vector3(.1,.1,.1));
    */


    //AudioSource* stepSound;AudioSource* doorSound;AudioSource* driveSound;
    stepSound = AudioSource::create ( "res/audio/step.ogg" );
    assert ( stepSound );
    stepSound->setLooped ( false );
    stepSound->setGain(1.0f);

    flashlightSound = AudioSource::create ( "res/audio/step.ogg" );
    assert ( flashlightSound );
    flashlightSound->setLooped ( false );
    flashlightSound->setGain(1.0f);
    flashlightSound->setPitch(4.f);

    //stepSound->setGain(0.5f);
    doorSound = AudioSource::create ( "res/audio/door.wav" );
    assert ( doorSound );
    doorSound->setLooped ( false );

    packageSound = AudioSource::create ( "res/audio/package.ogg" );
    assert ( packageSound );
    packageSound->setLooped ( false );
    packageSound->setGain(2.f);
    //doorSound->setGain(0.5f);
    driveSound = AudioSource::create ( "res/audio/drive.ogg" );
    assert ( driveSound );
    driveSound->setLooped ( false );
    //driveSound->setGain(0.5f);
    //driveSound->play();

    initPathNodes();
    npcA->setPathNodes(pathNodes);

    //ALWAYS bind audio listener to camera last, after cam nodes or audio listener will lose ref!?
    //ALSO make sure not to load other scenes/bundles after with cameras (such as blender exported models that had a default camera)
    //because that camera will auto-bind to the audio listener instead of the first cam!!
    //_scene->bindAudioListenerToCamera ( true );
    AudioListener::getInstance()->setCamera ( sceneCam );
    //_scene->bindAudioListenerToCamera(false);
    _scene->bindAudioListenerToCamera(true);
}




void GameModel::unloadScene()
{
    //TODO , SAFE_RELEASE needs to be called when using temporary pointers, especially physics collisions,
    //so that reference count will go to zero for proper releasing by scene



    SAFE_RELEASE(guyNode);

    SAFE_RELEASE ( _camPitch );
    SAFE_RELEASE ( _camRoot );
    SAFE_RELEASE ( _scene );



}


void GameModel::initPathNodes()
{
    Scene* cscene;
    Bundle* bundleRoomC = Bundle::create ( "res/box.gpb" );
    cscene =  bundleRoomC->loadScene();
    SAFE_RELEASE ( bundleRoomC );
    Node *boxNode = cscene->findNode ( "box" );
    //_scene->addNode ( boxNode );
    boxNode->getModel()->setMaterial ( "res/wall.material" );

    float gridSize = 1.0f;
    int gridRows = 50;
    int gridCols = 50;
    int rowStart = -5;
    int columnStart = -5;

    for(int i=0; i < gridRows; i++)
    {
        for(int j=0; j < gridCols; j++)
        {

            float rayHeight = 1.5f; //must be below doorway top to avoidhitting 1.5
            float rayOffset = .01f; //offset because of small gap between door when casting ray
            Vector3 startRay(-(i+columnStart)*gridSize + rayOffset, rayHeight, -(j+rowStart)*gridSize + rayOffset);
            AStar::PNode newNode;
            newNode.pos = startRay;
            Vector3 rayDirection(0,-1.f,0);
            float rayLength = 105.f;

            Node* newMark = boxNode->clone();
            newMark->setTranslation(startRay);
            newMark->setScale(0.2f);
            //_scene->addNode(newMark);

            newNode.type = AStar::OBSTACLE;

            PhysicsController::HitResult hitResult;
            if(mPhysicsController->rayTest( Ray(startRay,rayDirection),rayLength, &hitResult ))
            {
                //printf("000\n");


                if(hitResult.object)
                {

                    //_message = hitResult.object->getNode()->getId();
                    //if(std::string(hitResult.object->getNode()->getId() ) == "roomB" )
                    if(hitResult.object->getNode()->getTag("type") )
                    {
                        if(std::string(hitResult.object->getNode()->getTag("type")) == "room" )
                        {
                            newMark->setScale(0.05f);
                            newNode.type = AStar::NEWTYPE;
                        }

                    }

                }

            }


            pathNodes.push_back(newNode);
        }
    }

     //connect nodes by setting path data in each node
    for(int i=0; i < pathNodes.size() ; i++)
	{

        AStar::PNodePath newPath;
        pathNodes[i].paths.clear();

        //left/west
        newPath.pathIndex = i-1;
        newPath.pathLength = gridSize;
        if(newPath.pathIndex >= 0 && newPath.pathIndex < pathNodes.size() )
        {
            if(pathNodes[newPath.pathIndex].type != AStar::OBSTACLE )
                pathNodes[i].paths.push_back(newPath);
        }
        //right east
        newPath.pathIndex = i+1;
        newPath.pathLength = gridSize;
        if(newPath.pathIndex >= 0 && newPath.pathIndex < pathNodes.size() )
        {
            if(pathNodes[newPath.pathIndex].type != AStar::OBSTACLE )
                pathNodes[i].paths.push_back(newPath);
        }
        //up n
        newPath.pathIndex = i+1*gridCols;
        newPath.pathLength = gridSize;
        if(newPath.pathIndex >= 0 && newPath.pathIndex < pathNodes.size() )
        {
            if(pathNodes[newPath.pathIndex].type != AStar::OBSTACLE )
                pathNodes[i].paths.push_back(newPath);
        }
        //down
        newPath.pathIndex = i-1*gridCols;
        newPath.pathLength = gridSize;
        if(newPath.pathIndex >= 0 && newPath.pathIndex < pathNodes.size() )
        {
            if(pathNodes[newPath.pathIndex].type != AStar::OBSTACLE )
                pathNodes[i].paths.push_back(newPath);
        }


    }


    /*
    PhysicsController::HitResult hitResult;
    if(mPhysicsController->rayTest( Ray(_camPitch->getTranslationWorld(), _camPitch->getForwardVectorWorld()), pickingLength * 1.5f, &hitResult ))
    {
        //printf("000\n");

        if(hitResult.object)
        {
            _message = hitResult.object->getNode()->getId();

            if(hitResult.object->isDynamic() )
            {
                //printf("bbb\n");
                pickedBody = hitResult.object->asRigidBody();
                pickedBody->setDamping(.98f,.99f);
            }
        }

    }
    */



}



void GameModel::update( float elapsedTime )
{
    //MathUtil::smooth(float* x, float target, float elapsedTime, float responseTime)




    if (  LevelState == HOUSE && (introTimer > 0.f || hitCounter >= 3) )
    {
        introTimer -= elapsedTime;
        if(introTimer <= 0.f)
            doorSound->play();

        introScrollA += elapsedTime * 240.f;
        if(introScrollA > 256.f)
            introScrollA -= 256.f;
        introScrollB += elapsedTime * 20.f;
        if(introScrollB > 256.f)
            introScrollB -= 256.f;
        walkWave += elapsedTime * 1.2f;
        introHeight = 10.f * std::sin(walkWave);

        npcA->stopSounds();
        if(driveSound->getState() != AudioSource::PLAYING)
            driveSound->play();
    }


    if ( !gameWin && LevelState == HOUSE && introTimer <= 0.f && hitCounter < 3 )
    {
        if(driveSound->getState() == AudioSource::PLAYING)
            driveSound->stop();


        gameTimer += elapsedTime;

        hitCounter = npcA->playerHitCounter;

        npcA->update(_camPitch->getTranslationWorld() , elapsedTime, flashlightOn);

        char text[1024];  sprintf(text, "%i %f %f\n",packagesCount, guyNode->getTranslation().x, guyNode->getTranslation().z);
        _message = npcA->_message + " ..\n" + std::string(text);



        Vector3 moveVector = Vector3::zero();
        float moveSpeed = 4.51f;
        bool moving = false;



        if ( upKeyDown )
        {




            //pitchC += 0.1f * elapsedTime;

            Vector3 v = _camPitch->getForwardVectorWorld();
            v.y = 0.f;
            //_camRoot->translate(v);
            moveVector += v;
            //moveVector = v;
            //_charController->setVelocity(v * 10.f);

            moving = true;


        }
        else if ( downKeyDown )
        {

            //pitchC -=  0.1f * elapsedTime;
            //NextLevelState = DRONELAND;

            Vector3 v = _camPitch->getForwardVectorWorld();
            v.y = 0.f;

            moveVector -= v;
            //moveVector = -v;
            //_charController->setVelocity(v * 10.f);
            moving = true;

        }


        if ( leftKeyDown )
        {
            //pitchB -=  0.1f * elapsedTime;


            Vector3 v = _camPitch->getRightVectorWorld();
            v.y = 0.f;

            moving = true;
            moveVector -= v;
            //moveVector = -v;
            //_charController->setVelocity(v * 10.f);

        }
        else if ( rightKeyDown )
        {

            //NextLevelState = REALM;
            //pitchB +=  0.1f * elapsedTime;
            Vector3 v = _camPitch->getRightVectorWorld();
            v.y = 0.f;

            moveVector += v;
            //moveVector = v;
            //_charController->setVelocity(v * 10.f);
            moving = true;
        }


        if(npcA)
        {
            if(npcA->currentState == Npc::GRAB)
            {
                moving = false;
            }
        }

        if(!moving)
        {
            walkSpeedTarget = 0.f;
            MathUtil::smooth( &walkSpeed, walkSpeedTarget, elapsedTime, 0.4f );
            //_charControllerR->setGravity(Vector3(0, 0.f,0));
        }
        else
        {
            if(runKeyDown)
                walkSpeedTarget = 9.5f;
            else
                walkSpeedTarget = 5.51f;
            MathUtil::smooth( &walkSpeed, walkSpeedTarget, elapsedTime, 0.43f );
            //_charControllerR->setGravity(Vector3(0, -11.f,0));
        }

        //MathUtil::smooth( &walkSpeed, walkSpeedTarget, elapsedTime, 5.13f );
        //bool rayTest(const Ray& ray, float distance, PhysicsController::HitResult* result = NULL, PhysicsController::HitFilter* filter = NULL);
        //PhysicsController::HitResult *feetHit;

        //no falling in current setup
        if(1)//mPhysicsController->rayTest( Ray(guyNode->getTranslation(), Vector3(0,-1.f,0) ),1.5f ))
        {
            //printf("hit");
            //_charControllerR->setGravity(Vector3(0, 0.f,0));
            //currently have footing, so apply walking/bobbing
            //estimate gait(stepping/bobbing) length
            float gaitLength = _charControllerR->getLinearVelocity().lengthSquared();
            if(gaitLength > 11.f)
                gaitLength = 11.f;
            walkWave += elapsedTime * 0.8f * gaitLength;
            rollTarget = -.01f * std::sin(walkWave);
            float bobHeight = .07f;
            bobYTarget = bobHeight * std::sin(walkWave * 2.f);

            if(bobYTarget < (-bobHeight + .005f) && gaitLength > 1.3f)
            {
                if(stepSound->getState() != AudioSource::PLAYING)
                {
                    float stepGain = gaitLength / 11.f;
                    if(stepGain < 0.2f)
                        stepGain = 0.2f;
                    if(stepGain > 1.0f)
                        stepGain = 1.0f;
                    stepSound->setGain(stepGain);
                    stepSound->setPitch(1.f);
                    stepSound->play();
                }

            }
        }
        else//falling
            _charControllerR->setGravity(Vector3(0, -41.f,0));//increased gravity because of dampening





        if(moving )
        {
            moveVector.normalize();
            moveVector.scale(walkSpeed);//walkSpeed moveSpeed

            //float currentSpeed = _charController->getCurrentVelocity().lengthSquared();



            //_charController->setVelocity ( moveVector );
            //_charControllerR->setVelocity ( moveVector );
            _charControllerR->applyImpulse(moveVector);
        }
        else
        {


            //rollTarget = -.11f * st-d::sin(walkWave);
            rollTarget = 0.f;
        }


        grabRollTarget = 0.f;
        if(npcA)
        {
            if(npcA->currentState == Npc::GRAB)
            {
                camRotYTarget -= elapsedTime * 1.2f;
                camRotXTarget -= elapsedTime * 0.2f;
                if(camRotXTarget > 1.5f)
                    camRotXTarget = 1.5f;
                else if(camRotXTarget < -1.35f)
                    camRotXTarget = -1.35f;
                grabWave += elapsedTime * 8.7f;
                grabRollTarget = -.53f * std::sin(grabWave);


                if(hitCounter < 3)
                    headHeightTarget = 0.6f;
                else
                    headHeightTarget = 0.3f;
            }
        }


        MathUtil::smooth( &bobY, bobYTarget, elapsedTime, 0.13f );
        MathUtil::smooth( &camRotZ, rollTarget, elapsedTime, 0.13f );
        MathUtil::smooth( &grabRoll, grabRollTarget, elapsedTime, 0.13f );


        //_charControllerR->setAngularFactor(Vector3(0.0,1.0,0.0));

        //update look rotations
        MathUtil::smooth( &camRotX, camRotXTarget, elapsedTime, 0.053f );
        MathUtil::smooth( &camRotY, camRotYTarget, elapsedTime, 0.053f );


        MathUtil::smooth( &headHeight, headHeightTarget, elapsedTime, 0.3f );

        //
        _camRoot->setTranslation(guyNode->getTranslation());

        //cam moving and bobbing
        //_camRoot->setRotation(std::sin(walkWave),0,0,0); //weird man
        _camRoot->setRotation(Matrix::identity());
        _camPitch->setRotation(Matrix::identity());
        _camPitch->rotateX(camRotX);
        _camRoot->rotateY(camRotY);



        _camRoot->rotateZ( grabRoll );
        _camRoot->rotateZ( camRotZ );//roll
        _camRoot->setTranslationY(guyNode->getTranslationY() + headHeight + bobY);
        _camRoot->translate( _camPitch->getRightVectorWorld() * camRotZ * -10.f );//side bobbing

        //flashlight set
        if(flashlightOn)
        {
            _lightNode->setTranslation(_camPitch->getTranslationWorld() + Vector3(0.02f, -0.2f, 0.01f) );
            _lightNode->setRotation(_camPitch->getWorldMatrix() );
        }
        else
        {
            _lightNode->setTranslation(Vector3(0,-1000.f,0) );

        }
    }



    if(pickedBody)
    {
        Vector3 targetHolding = _camPitch->getTranslationWorld() + _camPitch->getForwardVectorWorld() * pickingLength;
        //if( (pickedBody->getAngularVelocity().lengthSquared() < 18.f )
        // && (pickedBody->getLinearVelocity().lengthSquared() < 8.f )
        //)
        //{

        Vector3 grabDist = (targetHolding - pickedBody->getNode()->getTranslation());

        //21500, 700
        Vector3 grabForce = 9.f *  grabDist;

        Vector3 dampForce = 0.9f * pickedBody->getLinearVelocity();

        //Vector3 dampForce = 212.00f * grabDir * pickedBody->getLinearVelocity().lengthSquared();

        Vector3 holdForce = grabForce - dampForce;
        pickedBody->applyForce(holdForce * 10.f);//3000.f elapsedTime * 700.f
        //pickedBody->applyForce( elapsedTime *  12500.f *  (targetHolding - pickedBody->getNode()->getTranslation())        );
        //}
    }


    //state update
    if ( LevelState != NextLevelState )
    {
        MathUtil::smooth ( &fadeAlpha, 1.0f, elapsedTime, 0.13f );
        if ( fadeAlpha >= .99f )
        {
            fadeAlpha = 1.0f;

            unloadScene();


            if(NextLevelState == PIT)
            {
                loadPit();
            }
            else if(NextLevelState == HOUSE)
            {
                loadHouse();
            }

            LevelState = NextLevelState;
            //currentRoomIndex = nextRoomIndex;
        }
    }
    else if ( fadeAlpha > 0 )
    {
        MathUtil::smooth ( &fadeAlpha, 0.0f, elapsedTime, 0.12f );
        if ( fadeAlpha < 0.01f )
            fadeAlpha = 0.f;
    }



}





void GameModel::collisionEvent ( PhysicsCollisionObject::CollisionListener::EventType type,

                                 const PhysicsCollisionObject::CollisionPair& collisionPair,

                                 const Vector3& contactPointA,

                                 const Vector3& contactPointB )

{


}


void GameModel::animationEvent ( AnimationClip* clip, AnimationClip::Listener::EventType type )

{
}



