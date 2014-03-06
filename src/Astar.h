//3D astar path finding algorithm and containers for nodes and paths
#ifndef ASTAR_H_
#define ASTAR_H_


#include <vector>
#include <iostream>

//#include "utMath.h"
#include "gameplay.h"

using namespace gameplay;
using namespace std;



class AStar
{


public:
        enum PNodeType {
        NEWTYPE,
        OPENTYPE,
        CLOSEDTYPE,
        OBSTACLE
        };
	struct PNodePath{
		unsigned int pathIndex;//index (into allPNodes) of PNodes that have paths to this one
		float pathLength;

	};
	struct PNode
	{
		//std::vector<PNode*> paths;
		std::vector<PNodePath> paths;
		float F, G, H;//G-local dist H-hueristic F=G+H
		Vector3 pos;

		PNodeType type; //new,open,closed, obstacle

		unsigned int parentIndex;//the index of the parent this PNode points to (for tracing back path)

		PNode()
		{

			type = NEWTYPE;
			F = 0;
			G = 0;
			H = 0;

		}
	};


	AStar();
	~AStar();



	std::vector<Vector3> FindPath(Vector3 start, Vector3 end);

  void setPathNodes(std::vector<AStar::PNode> pathNodes);

private:


	static const int MAXSEARCH = 500;//5000 500

	//initialize beforehand based on map or generate
	//set path lists and fill in indexes and the path lengths
	std::vector<PNode> allPNodes;
	unsigned int currentIndex;

	unsigned int startIndex;
	unsigned int endIndex;

	float diagonalD;
	float horizontalD;

	

};






#endif



