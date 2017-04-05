////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// STL A* Search implementation
// (C)2001 Justin Heyes-Jones
//
// Finding a path on a simple grid maze
// This shows how to do shortest path finding using A*

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stlastar.h" // See header for copyright and usage information

#include <iostream>
#include <math.h>

#define DEBUG_LISTS 0
#define DEBUG_LIST_LENGTHS_ONLY 0

using namespace std;
//using namespace astar;

// map helper functions

static uint8_t* _map = NULL;
static unsigned int _map_width=0, _map_height=0;

uint8_t GetMap( unsigned int x, unsigned int y )
{
  assert(_map);

  if(x >= _map_width || y >= _map_height)
		return 9;

	return _map[(y*_map_width)+x];
}

// Definitions

class MapSearchNode
{
public:
	unsigned int x;	 // the (x,y) positions of the node
	unsigned int y;

	MapSearchNode() { x = y = 0; }
	MapSearchNode( unsigned int px, unsigned int py ) { x=px; y=py; }

	float GoalDistanceEstimate( MapSearchNode &nodeGoal );
	bool IsGoal( MapSearchNode &nodeGoal );
	bool GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node );
	float GetCost( MapSearchNode &successor );
	bool IsSameState( MapSearchNode &rhs );

	void PrintNodeInfo();


};

bool MapSearchNode::IsSameState( MapSearchNode &rhs )
{

	// same state in a maze search is simply when (x,y) are the same
	if( (x == rhs.x) &&
		(y == rhs.y) )
	{
		return true;
	}
	else
	{
		return false;
	}

}

void MapSearchNode::PrintNodeInfo()
{
  //cout << x << "\t" << y << endl;
  cout << "Node position : (" << x << ", " << y << ")" << endl;
}

// Here's the heuristic function that estimates the distance from a Node
// to the Goal.

float MapSearchNode::GoalDistanceEstimate( MapSearchNode &nodeGoal )
{
  float xd = fabs(float(((float)x - (float)nodeGoal.x)));
  float yd = fabs(float(((float)y - (float)nodeGoal.y)));
  return xd + yd;
  //return 1.001 * (xd + yd );
}

bool MapSearchNode::IsGoal( MapSearchNode &nodeGoal )
{

	if( (x == nodeGoal.x) &&
		(y == nodeGoal.y) )
	{
		return true;
	}

	return false;
}

// This generates the successors to the given Node. It uses a helper function called
// AddSuccessor to give the successors to the AStar class. The A* specific initialisation
// is done for each node internally, so here you just set the state information that
// is specific to the application
bool MapSearchNode::GetSuccessors( AStarSearch<MapSearchNode> *astarsearch,
											  MapSearchNode *parent_node )
{

	int parent_x = -1;
	int parent_y = -1;

	if( parent_node )
	{
		parent_x = parent_node->x;
		parent_y = parent_node->y;
	}

	MapSearchNode NewNode;

	int ix = (int)x;
	int iy = (int)y;

	// push each possible move except allowing the search to go backwards

	if( (GetMap( ix-1, iy ) < 9u)
			&& !((parent_x == ix-1) && (parent_y == iy))
	  )
	{
		NewNode = MapSearchNode( ix-1, iy );
		astarsearch->AddSuccessor( NewNode );
	}

	if( (GetMap( ix, iy-1 ) < 9u)
		&& !((parent_x == ix) && (parent_y == iy-1))
	  )
	{
		NewNode = MapSearchNode( ix, iy-1 );
		astarsearch->AddSuccessor( NewNode );
	}

	if( (GetMap( ix+1, iy ) < 9u)
		&& !((parent_x == ix+1) && (parent_y == iy))
	  )
	{
		NewNode = MapSearchNode( ix+1, iy );
		astarsearch->AddSuccessor( NewNode );
	}


	if( (GetMap( ix, iy+1 ) < 9u)
		&& !((parent_x == ix) && (parent_y == iy+1))
		)
	{
		NewNode = MapSearchNode( ix, iy+1 );
		astarsearch->AddSuccessor( NewNode );
	}

	return true;
}

// given this node, what does it cost to move to successor. In the case
// of our map the answer is the map terrain value at this node since that is
// conceptually where we're moving

float MapSearchNode::GetCost( MapSearchNode & )
{
	return (float) GetMap( x, y );

}


// Main

#include "astar.h"
using namespace ast;

bool ast::astar(uint8_t* map,
        uint32_t width,
        uint32_t height,
        const point_t &start,
        const point_t &goal,
        std::vector<point_t>& path )
{
  //cout << "STL A* Search implementation\n(C)2001 Justin Heyes-Jones\n";

	// set the static vars
	_map = map;
	_map_width = width;
	_map_height = height;

	// Our sample problem defines the world as a 2d array representing a terrain
	// Each element contains an integer from 0 to 5 which indicates the cost
	// of travel across the terrain. Zero means the least possible difficulty
	// in travelling (think ice rink if you can skate) whilst 5 represents the
	// most difficult. 9 indicates that we cannot pass.

	// Create an instance of the search class...

	AStarSearch<MapSearchNode> astarsearch;

	unsigned int SearchCount = 0;

	const unsigned int NumSearches = 1;

	bool path_found = false;

	while(SearchCount < NumSearches)
	{

		// Create a start state
		MapSearchNode nodeStart;
		nodeStart.x = start.x;
		nodeStart.y = start.y;

		// Define the goal state
		MapSearchNode nodeEnd;
		nodeEnd.x = goal.x;
		nodeEnd.y = goal.y;

		// Set Start and goal states

		astarsearch.SetStartAndGoalStates( nodeStart, nodeEnd );

		unsigned int SearchState;
		unsigned int SearchSteps = 0;

		do
		{
			SearchState = astarsearch.SearchStep();

			SearchSteps++;

	#if DEBUG_LISTS

			cout << "Steps:" << SearchSteps << "\n";

			int len = 0;

			cout << "Open:\n";
			MapSearchNode *p = astarsearch.GetOpenListStart();
			while( p )
				{
					len++;
#if !DEBUG_LIST_LENGTHS_ONLY
					((MapSearchNode *)p)->PrintNodeInfo();
#endif
					p = astarsearch.GetOpenListNext();

				}

			cout << "Open list has " << len << " nodes\n";

			len = 0;

			cout << "Closed:\n";
			p = astarsearch.GetClosedListStart();
			while( p )
				{
					len++;
#if !DEBUG_LIST_LENGTHS_ONLY
					p->PrintNodeInfo();
#endif
					p = astarsearch.GetClosedListNext();
				}

			cout << "Closed list has " << len << " nodes\n";
#endif

		}
		while( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING );

		if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED )
		{
		  //cout << "Search found goal state\n";

				MapSearchNode *node = astarsearch.GetSolutionStart();

#if DISPLAY_SOLUTION
				cout << "Displaying solution\n";
#endif
				int steps = 0;

				//node->PrintNodeInfo();

				path.push_back( point_t( node->x, node->y ) );

				for( ;; )
				  {
					 node = astarsearch.GetSolutionNext();

					 if( !node )
						{
						  break;
						}

					 //node->PrintNodeInfo();

					 path.push_back( point_t( node->x, node->y ) );

					 steps ++;

				  };

				//cout << "Solution steps " << steps << endl;

				// Once you're done with the solution you can free the nodes up
				astarsearch.FreeSolutionNodes();

				path_found = true;

		}
		else if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED )
		  {
			 cout << "Search terminated. Did not find goal state\n";

		  }

		// Display the number of loops the search went through
		//cout << "SearchSteps : " << SearchSteps << "\n";

		SearchCount ++;

		astarsearch.EnsureMemoryFreed();
	}

	return path_found;
}

//   // STL container iterator macros
// #define VAR(V,init) __typeof(init) V=(init)
// #define FOR_EACH(I,C) for(VAR(I,(C).begin());I!=(C).end();I++)

// int main( int argc, char *argv[] )
// {
//   std::vector<point_t> path;

//   bool result = astar( map, 30, 30, point_t( 1,1 ), point_t( 25,20 ), path );

//   printf( "#%s:\n", result ? "PATH" : "NOPATH" );

//   FOR_EACH( it, path )
// 	 printf( "%d, %d\n", it->x, it->y );
//   puts("");
// }

