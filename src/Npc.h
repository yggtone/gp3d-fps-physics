#ifndef NPC_H_
#define NPC_H_


#include "gameplay.h"
#include <vector>
#include "Astar.h"


using namespace gameplay;
using namespace std;




class Npc: public AnimationClip::Listener, public PhysicsCollisionObject::CollisionListener
{
public:

    enum NPCSTATE
    {
        IDLE,
        SEARCH,
        CHASE,
        GRAB
    };
    NPCSTATE currentState;


    //public while debugging
    std::string _message;







    enum NPCTYPE
    {
        TALLCAPE,
        GHOSTA

    };

    Npc(Vector3 initpos, Material *npcmat, NPCTYPE ntype);
    ~Npc();

    void stopSounds();

    Node* getNode()
    {
        return npcNode;
    };


    void update( Vector3 targetPos, float elapsedTime, bool flashlightOn);





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

    void setPathNodes(std::vector<AStar::PNode> pathNodes);

    int playerHitCounter;

private:

    Vector3 patrolA, patrolB;
    bool patrolForward;
    int playerSeenCount;
    int hideCount;
    float monsterLookTimer;
    float moveSpeed;


    NPCTYPE _npcType;

    Node* npcNode;
    AnimationClip* currentAnim;
    AnimationClip* walkAnim;
    AnimationClip* idleAnim;
    //PhysicsCharacter* npcController;
    PhysicsRigidBody* npcController;
    Node* controllerNode;
    float walkTimeout;

    Node* neckBone;


    float changeDirectionTimer;
    Vector3 newHeading;

    AStar *pathFinder;
    vector<Vector3> currentPath;
    unsigned int pathIndex;
    float pathUpdateTimer;



    Node *searchSoundNode;

    Node *yellSoundNode;

    AudioSource *grabSound;
};



#endif
