
#include "Astar.h"

using namespace gameplay;
using namespace std;

AStar::AStar() : currentIndex(0), startIndex(0), endIndex(0)
{

	currentIndex = 0;


	horizontalD = 1.f;
	diagonalD = std::sqrt(horizontalD * horizontalD + horizontalD * horizontalD );
}

AStar::~AStar()
{

}

void AStar::setPathNodes(std::vector<AStar::PNode> pathNodes)
{
    allPNodes = pathNodes;




}

std::vector<Vector3> AStar::FindPath(Vector3 start, Vector3 end)
{


	//2d test
	start.y = 0;
	end.y = 0;


	float closestStart = 1000000.f;
	float closestEnd = 1000000.f;
	startIndex = 0;
	endIndex = 0;

	//init/reset all PNodes, also find closest PNodes to start and end
	for(unsigned int i=0; i < allPNodes.size() ; i++)
	{
		if(allPNodes[i].type != AStar::OBSTACLE)
		{
			allPNodes[i].type = AStar::NEWTYPE;

			float startMeasure = (start - allPNodes[i].pos).lengthSquared();
			if(startMeasure < closestStart)
			{
				closestStart = startMeasure;
				startIndex = i;

			}

			float endMeasure = (end - allPNodes[i].pos).lengthSquared();
			if(endMeasure < closestEnd)
			{
				closestEnd = endMeasure;
				endIndex = i;
			}
		}
	}

	//printf("start: %f  %f end: %f  %f \n", allPNodes[startIndex].pos.x, allPNodes[startIndex].pos.z, allPNodes[endIndex].pos.x, allPNodes[endIndex].pos.z );
	//printf("starti: %i endi: %i \n", startIndex, endIndex);

	//start PNode will be first current PNode
	currentIndex = startIndex;

	//A* specific-implementation for format
	/*
	//1) add current PNode to closed list.(remove from open list if not the starting PNode)
	//2) check all adjacent PNodes (check paths list)
	//  for each adjacent PNode(that is not 'closed' or obstacle):
		a)if 'new', add to open list, and make the adjacent PNode's parent the current PNode index
		b)if 'open', compare the (open PNode's G score) with (current-PNodes G-score + path length to open PNode)
		   -if (open PNode's G score) is less(better), then do nothing
		   -if (open PNode's G score) is more, then replace the open PNode's G value and F value,
		    and update the open PNode's parentIndex to current PNode index

	  3) after all adjacent PNodes are checked, choose the next current PNode:
	   pick the open PNode with the lowest F value.
	   (eventually keep an open list as a min-heap, for finding the lowest value quickly)
	*/

	bool endFound = false;

	//init first PNode
	if(currentIndex >= 0 && currentIndex < allPNodes.size() )
	{
		allPNodes[currentIndex].type = AStar::OPENTYPE;
		allPNodes[currentIndex].F = 0;
		allPNodes[currentIndex].G = 0;
		allPNodes[currentIndex].H = 0;
		allPNodes[currentIndex].parentIndex = currentIndex;
	}

	for(int i = 0; i < MAXSEARCH && !endFound ; i++)
	{
		if(currentIndex >= 0 && currentIndex < allPNodes.size() )
		{
			//printf("current: %i \n", currentIndex );
			allPNodes[currentIndex].type = AStar::CLOSEDTYPE;

			if(currentIndex == endIndex)
			{
				endFound = true;
				//printf("iter: %i\n",i);
				break;
			}
			if(!endFound)
			{
				//check all adjacent PNodes
				for(unsigned int p = 0; p < allPNodes[currentIndex].paths.size() ; p++)
				{
					unsigned int pIndex = allPNodes[currentIndex].paths[p].pathIndex;
					if(pIndex >= 0 && pIndex < allPNodes.size() )
					{
						if(allPNodes[pIndex].type == AStar::NEWTYPE)
						{
							allPNodes[pIndex].type = AStar::OPENTYPE;
							allPNodes[pIndex].parentIndex = currentIndex;

							//calculate F,G,H
							allPNodes[pIndex].G = allPNodes[currentIndex].G + allPNodes[currentIndex].paths[p].pathLength;
							//allPNodes[pIndex].H = (allPNodes[endIndex].pos - allPNodes[pIndex].pos).length();
							//manhatten distance H hueuristic
							//allPNodes[pIndex].H = std::fabs((allPNodes[endIndex].pos.x - allPNodes[pIndex].pos.x) ) + std::fabs((allPNodes[endIndex].pos.z - allPNodes[pIndex].pos.z) );
							//chebyshev, for diagonal paths h(n) = D * max(abs(n.x-goal.x), abs(n.y-goal.y))
							//allPNodes[pIndex].H = std::max(std::fabs((allPNodes[endIndex].pos.x - allPNodes[pIndex].pos.x) ), std::fabs((allPNodes[endIndex].pos.z - allPNodes[pIndex].pos.z))) ;
							//h_diagonal(n) = min(abs(n.x-goal.x), abs(n.y-goal.y))
							//h_straight(n) = (abs(n.x-goal.x) + abs(n.y-goal.y))
							//h(n) = D2 * h_diagonal(n) + D * (h_straight(n) - 2*h_diagonal(n)))
							float diagonal = std::min(std::fabs((allPNodes[endIndex].pos.x - allPNodes[pIndex].pos.x) ), std::fabs((allPNodes[endIndex].pos.z - allPNodes[pIndex].pos.z))) ;
							float straight = (std::fabs((allPNodes[endIndex].pos.x - allPNodes[pIndex].pos.x) ) + std::fabs((allPNodes[endIndex].pos.z - allPNodes[pIndex].pos.z)));
							allPNodes[pIndex].H = diagonalD * diagonal + horizontalD * straight;
							//allPNodes[pIndex].H = 0;
							allPNodes[pIndex].F = allPNodes[pIndex].G + allPNodes[pIndex].H;
							allPNodes[pIndex].parentIndex = currentIndex;
						}
						/*
						b)if 'open', compare the (open PNode's G score) with (current-PNodes G-score + path length to open PNode)
					   -if (open PNode's G score) is less(better), then do nothing
					   -if (open PNode's G score) is more, then replace the open PNode's G value and F value,
						and update the open PNode's parentIndex to current PNode index

						*/
						else if(allPNodes[pIndex].type == AStar::OPENTYPE)
						{
							float newLength = allPNodes[currentIndex].paths[p].pathLength + allPNodes[currentIndex].G;
							if( allPNodes[pIndex].G > newLength)
							{
								allPNodes[pIndex].parentIndex = currentIndex;
								allPNodes[pIndex].G = newLength;
								allPNodes[pIndex].F = allPNodes[pIndex].G + allPNodes[pIndex].H;

							}

						}
					}

				}
			}




			//get next current, find open PNode with least F value
			float FMIN = 10000000;

			for(unsigned int n=0; n < allPNodes.size(); n++)
			{
				if(allPNodes[n].type == AStar::OPENTYPE)
				{
					if(allPNodes[n].F < FMIN)
					{
						FMIN = allPNodes[n].F;
						currentIndex = n;

					}
				}
			}



		}

	}

	//printf("current: %i \n", currentIndex );

	//trace back path
	vector<Vector3> path;
	path.clear();
	bool pathFound = false;
	for(int i = 0; i < MAXSEARCH && !pathFound ; i++)
	{
		if(currentIndex >= 0 && currentIndex < allPNodes.size() )
		{
			path.push_back(allPNodes[currentIndex].pos);
			unsigned int newIndex = allPNodes[currentIndex].parentIndex;
			//printf("cIndex,parent %i %i\n", currentIndex, newIndex );
			currentIndex = newIndex;
			if(currentIndex == startIndex)
			{
				//printf("startIndex == currentIndex: %i ", currentIndex );
				path.push_back(allPNodes[currentIndex].pos);
				break;
			}
			//printf("x,y: %f  %f\n",allPNodes[currentIndex].pos.x,  allPNodes[currentIndex].pos.z );
		}
	}

	vector<Vector3> realPath;


	//reverse
	if(path.size() > 0)
	{

		for(int i = (int)path.size() - 1 ; i >= 0 ; i--)
		{
			//printf("i: %i \n", i);
			realPath.push_back(path[i]);
			//printf("x,y: %f %f %f\n",path[i].x, path[i].y, path[i].z );
		}
	}


        //printf("origpathsize: %i\n", realPath.size());

	return realPath;

}
