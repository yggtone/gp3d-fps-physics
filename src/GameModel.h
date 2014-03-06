#ifndef GAMEMODEL_H_
#define GAMEMODEL_H_


#include "gameplay.h"
#include <vector>
#include <string>
#include <list>

#include "Npc.h"

#include "Astar.h"

using namespace gameplay;
using namespace std;




class GameModel: public AnimationClip::Listener, public PhysicsCollisionObject::CollisionListener
{
public:
    GameModel(PhysicsController * physicsControl);
    ~GameModel();




    enum LEVELSTATETYPE
    {
        INIT,
        TITLE,
        PIT,
        HOUSE

    };

    enum SCENERYTYPE
    {
        TREES,
        GROVE,
        HILL,
        TOWERSA,
        ORBA,
        PILLAR,
        SROOM,
        BRIDGE


    };


    static GameModel* getInstance();

    LEVELSTATETYPE getLevelState()
    {
        return LevelState;
    };



    float getFadeAlpha()
    {
        return fadeAlpha;
    };


    void update(float elapsedTime);

    Scene* getScene()
    {
        return _scene;
    };



    void unloadScene();

    void camMove(float pitch, float yaw);



    void primaryAction();
    void leftMouseRelease();
    void rightMouseDown();



    /**
     *
     * @see AnimationClip::Listener::animationEvent
     *
     */

    void animationEvent(AnimationClip* clip, AnimationClip::Listener::EventType type);


    /**
     *
     * @see PhysicsCollisionObject::CollisionListener::collisionEvent
     *
     */

    void collisionEvent(PhysicsCollisionObject::CollisionListener::EventType type,
                        const PhysicsCollisionObject::CollisionPair& collisionPair,
                        const Vector3& contactPointA = Vector3::zero(),
                        const Vector3& contactPointB = Vector3::zero());



    //key down status edited by controller(or main class input listeners)
    bool upKeyDown;
    bool leftKeyDown;
    bool rightKeyDown;
    bool downKeyDown;

    bool runKeyDown;
    bool flashlightOn;

    Vector3 getFogColor()
    {
        return fogColor;
    };

    Node* particleNodeA;

    std::string getMessage()
    {
        return _message;
    };


    void duckPressed();
    void duckReleased();


    Npc * npcA;

    float introTimer;
    float introScrollA;
    float introScrollB;
    float introHeight;
    int packagesCount;
    int hitCounter;
    bool gameWin;
    float gameTimer;



    void flashlightButton(){
        flashlightOn = !flashlightOn;
        flashlightSound->play();
    };


private:


    void setupMaterialSpot( Node *nLightnode, Material *nMaterial);

    Vector3 fogColor;
    Vector3 ambientLight;

    void setNextState(LEVELSTATETYPE newState)
    {
        NextLevelState = newState;
    };

    float fadeAlpha;



    float waveB;
    //Vector3 moveVector;

    Scene* _scene;

    Animation* _animation;
    //AnimationClip* _currentClip;

    //PhysicsCharacter* _charController;
    PhysicsRigidBody* _charControllerR;

    Node* _camBackup;
    Node* _camPitch;
    Node* _camRoot;
    Node* _lightNode;
    Node* _globe;

    //Node* camPitchTarget;
    //Node* camRootTarget;
    //Quaternion camPitchOri;
    //Quaternion camRootOri;

    Node* guyNode;

    //warps char controller between two frames
    bool playerWarpFrame;





    MaterialParameter *matColorParam;
    float colorWave;

    AudioSource *droneA;






    float walkSpeed;
    float walkSpeedTarget;
    float walkWave;
    float rollTarget;
    float bobY;
    float bobYTarget;
    float camRotX;
    float camRotY;
    float camRotZ;

    float camRotXTarget;
    float camRotYTarget;



    LEVELSTATETYPE LevelState, NextLevelState;





    ParticleEmitter* pEmitterA;



    void loadPit();
    void loadHouse();

    void initPathNodes();

    PhysicsController* mPhysicsController;


    PhysicsRigidBody * pickedBody;

    float pickingLength;

    std::string _message;

    PhysicsRigidBody * doorBody;

    Node *knockingSoundNode;
    bool knockStarted;

    //float gameTimer;
    float checkTimer;
    std::list<Node*> objectList;

    //path nodes to initialize, and pass to the pathfinders of npcs
    std::vector<AStar::PNode> pathNodes;


    void createMaze(int cols, int rows, int floors);
	struct MazeNode
	{
		bool north, south, east, west, up, down;

		bool partOfMaze;//for algorithm building maze

		MazeNode()
		{

			north = false;
			south = false;
			east = false;
			west = false;
			up = false;
			down = false;

			partOfMaze = false;

		}

	};
	enum WallDirectionType
	{
		NORTH,SOUTH,EAST,WEST,UP,DOWN
	};
	struct MazeWall
	{
		unsigned int firstCellIndex;
		unsigned int secondCellIndex;

		WallDirectionType wallDirection;

		MazeWall()
		{
			firstCellIndex = 0;
			secondCellIndex = 0;

		}
	};
	std::vector<MazeNode> mazeNodes;
	std::vector<MazeWall> mazeWalls;
	int mazeCols;
	int mazeRows;
	int mazeFloors;

    float headHeight;
    float headHeightTarget;




    float grabWave;
    float grabRoll;
    float grabRollTarget;


    AudioSource* stepSound;
    AudioSource* doorSound;
    AudioSource* driveSound;
    AudioSource* packageSound;
    AudioSource* flashlightSound;

};



#endif
