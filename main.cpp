#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cmath>

const int GLOBAL_CONST_WINDOW_WIDTH = 700;
const int GLOBAL_CONST_WINDOW_HEIGHT = 700;
const float GLOBAL_CONST_GRID_SIZE = 0.05;

struct Vector2 {
	float x;
	float y;
};

class Node
{
public:
	// SDL_Rect corresponds to node location and size
	Node(SDL_Rect rect, int id);

	SDL_Rect* GetRect() { return &mRect; };
	Vector2 GetPosition() { return mPosition; };
	int GetId() { return mID; }

	// Used for to calculate A* algorithm
	int gCost;
	int hCost;
	int fCost() { return gCost + hCost; };
	bool walkable;
	std::vector<Node> parent;

	bool operator==(const Node &node)
	{
		return (this->mID == node.mID);
	}
	bool operator!=(const Node &node)
	{
		return (this->mID != node.mID);
	}

private:
	SDL_Rect mRect;
	int mID;
	Vector2 mPosition;
};

Node::Node(SDL_Rect rect, int id)
{
	mRect = rect;
	mID = id;
	mPosition.x = static_cast<int>(rect.x + rect.w/2);
	mPosition.y = static_cast<int>(rect.y + rect.h/2);
	walkable = true;
}


class Pathfinding
{
public:
	Pathfinding();
	
	bool Initialize();
	
	void RunLoop();
	
	void Shutdown();
private:
	void ProcessInput();
	void GenerateOutput();

	void MakeNodes(int windowWidth, int windowHeight);
	void DrawGrid(SDL_Renderer* renderer, int windowWidth, int windowHeight);
	void FindPath(Node start, Node target);
	void RetracePath(Node start, Node target);
	std::vector<Node> GetNeighbors(Node node);
	int GetDistance(Node nodeA, Node nodeB);
	SDL_Window* mWindow;
	// Renderer to draw graphics created by SDL
	SDL_Renderer* mRenderer;

	// Pathfinding should continue to run
	bool mIsRunning;

	bool mMouseDown;
	bool mRightMouseDown;
	bool mLeftMouseDown;
	// Current mouse location within the window
	int mXMouse;
	int mYMouse;

	// Clears the following vectors if true
	bool mErase;

	// All the nodes in the program
	std::vector<Node> mNodes;
	// Nodes selected to be !walkable (walls, white)
	std::vector<Node> mSelectedNodes;
	// Nodes selected to make a path from (start, target)
	std::vector<Node> mPathNodes;
	// Nodes calculated as neighbors in the A* algorithm
	std::vector<Node> mNeighbors;
	// The final path retraced from finish to start
	std::vector<Node> mPath;
};

Pathfinding::Pathfinding()
{
	mWindow = nullptr;
	mRenderer = nullptr;
	mIsRunning = true;
	mMouseDown = false;
	mRightMouseDown = false;
	mLeftMouseDown = false;
	mErase = false;
}

// The Initialization function returns true 
//if initialization succeeds and false otherwise
bool Pathfinding::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);

	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	mWindow = SDL_CreateWindow(
				"A* Pathfinding Example",
				100,  // Top left x-coordinate of window
				100,  // Top left y-coordinate of window
				GLOBAL_CONST_WINDOW_WIDTH, // Width of window
				GLOBAL_CONST_WINDOW_HEIGHT,  // Height of window
				0     // Flags (0 for no flags set)
			);

	if (!mWindow)
	{ 
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	mRenderer = SDL_CreateRenderer(
		mWindow, // Window to create renderer for
		-1,      // Usually -1
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	MakeNodes(GLOBAL_CONST_WINDOW_WIDTH, GLOBAL_CONST_WINDOW_HEIGHT);
	return true;
}

// Runloop keeps running iterations of the pathfinding  until mIsRunning becomes false
void Pathfinding::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		GenerateOutput();
	}
}

void Pathfinding::Shutdown()
{
	SDL_DestroyWindow(mWindow);
	SDL_DestroyRenderer(mRenderer);
	SDL_Quit();
}

void Pathfinding::ProcessInput()
{
	SDL_Event event;
	const Uint8* state = SDL_GetKeyboardState(NULL);
	// While there are still events in the queue
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
			mIsRunning = false;
			break;

			case SDL_MOUSEMOTION:
			SDL_GetMouseState(&mXMouse,&mYMouse);
			break;

			case SDL_MOUSEBUTTONDOWN:
			mMouseDown = true;
			if (event.button.button  == SDL_BUTTON_RIGHT)
			{
				mRightMouseDown = true;
			}
			if (event.button.button  == SDL_BUTTON_LEFT)
			{
				mLeftMouseDown = true;
			}
			break;

			case SDL_MOUSEBUTTONUP:
			mMouseDown = false;
			mRightMouseDown = false;
			mLeftMouseDown = false;
			break;
		}
	}

	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}
	
	if (state[SDL_SCANCODE_E])
	{
		mErase = true;
	}
	else
	{ 
		mErase = false;
	}

}

void Pathfinding::GenerateOutput()
{
	SDL_SetRenderDrawColor(
		mRenderer,
		12,
		12,
		13,
		255
	);

	// Clear the back buffer to the current draw color
	SDL_RenderClear(mRenderer);

	SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);

	SDL_SetRenderDrawColor(
		mRenderer,
		100,
		101,
		103,
		150
	);
	
	for ( auto node:mNodes)
	{
		if (node.GetRect()->x < mXMouse && (node.GetRect()->x + node.GetRect()->w) > mXMouse
			&& node.GetRect()->y < mYMouse && (node.GetRect()->y + node.GetRect()->h) > mYMouse)
		{

			SDL_RenderFillRect(mRenderer, node.GetRect());
		}

	}

	if (mMouseDown == true && mLeftMouseDown == true)
	{

		for ( auto node:mNodes)
		{
			if (node.GetRect()->x < mXMouse && (node.GetRect()->x + node.GetRect()->w) > mXMouse
				&& node.GetRect()->y < mYMouse && (node.GetRect()->y + node.GetRect()->h) > mYMouse)
			{
				auto iterator = std::find(mSelectedNodes.begin(), mSelectedNodes.end(), node);
				if (iterator == mSelectedNodes.end())
				{
					node.walkable = false;
					mSelectedNodes.push_back(node);
				}
			}
		}
	}

	if (mErase == true)
	{
		mSelectedNodes.clear();
		mPathNodes.clear();
		mPath.clear();
	}


	for (auto node:mSelectedNodes)
	{
		
		SDL_SetRenderDrawColor(
					mRenderer,
					255,
					255,
					255,
					255
				);

		SDL_RenderFillRect(mRenderer, node.GetRect());
	}

	if (mRightMouseDown == true)
		{

			for ( auto node:mNodes)
			{
				if (node.GetRect()->x < mXMouse && (node.GetRect()->x + node.GetRect()->w) > mXMouse
					&& node.GetRect()->y < mYMouse && (node.GetRect()->y + node.GetRect()->h) > mYMouse)
				{
					auto iterator = std::find(mPathNodes.begin(), mPathNodes.end(), node);
					if (iterator == mPathNodes.end() && mPathNodes.size() < 2)
					{
						mPathNodes.push_back(node);
					}
				}
			}
		}

	for (auto node:mPathNodes)
	{
		
		SDL_SetRenderDrawColor(
					mRenderer,
					255,
					0,
					0,
					255
				);

		SDL_RenderFillRect(mRenderer, node.GetRect());
	}

	if (mPathNodes.size() > 1)
	{
		FindPath(mPathNodes[0], mPathNodes[1]);
	}

	for (auto node:mPath)
	{
		SDL_SetRenderDrawColor(
					mRenderer,
					0,
					255,
					0,
					255
				);
		if ( std::find(mPathNodes.begin(), mPathNodes.end(), node) == mPathNodes.end() && mPathNodes.size() > 1)
		{
			SDL_RenderFillRect(mRenderer, node.GetRect());
		}
	}

	DrawGrid(mRenderer, GLOBAL_CONST_WINDOW_WIDTH,GLOBAL_CONST_WINDOW_HEIGHT);
	// Swap the front and back buffers
	SDL_RenderPresent(mRenderer);
}

// Draws a grid of lines over the nodes at the end of generate output
void Pathfinding::DrawGrid(SDL_Renderer* renderer, int windowWidth, int windowHeight)
{

		SDL_SetRenderDrawColor(
		renderer,
		100,
		101,
		103,
		255
	);
	
	for ( int i = 0; i < windowWidth; i += windowWidth * GLOBAL_CONST_GRID_SIZE)
	{
		SDL_RenderDrawLine(renderer, i, 0, i, windowHeight);
	} for ( int i = 0; i < windowHeight; i += windowHeight * GLOBAL_CONST_GRID_SIZE) { SDL_RenderDrawLine(renderer, 0, i, windowWidth, i); } }

// Makes nodes row by row and assigns an ID to each node, node size is dependent on the window and the grid size
void Pathfinding::MakeNodes(int windowWidth, int windowHeight)
{
	int id = 0;
	for ( int i = 0; i < windowWidth; i += windowWidth * GLOBAL_CONST_GRID_SIZE)
	{
		for ( int j = 0; j < windowHeight; j += windowHeight * GLOBAL_CONST_GRID_SIZE)
		{
			SDL_Rect rect{i, j, static_cast<int>(windowWidth * GLOBAL_CONST_GRID_SIZE), static_cast<int>(windowHeight * GLOBAL_CONST_GRID_SIZE)};
			auto node = new Node(rect, id);
			mNodes.push_back(*node);
			id++;
		}
	}
}

// Basic implementation of the A* pathfinding algorithm
void Pathfinding::FindPath(Node start, Node target)
{
	std::vector<Node> openSet;
	std::vector<Node> closedSet;

	openSet.push_back(start);

	while (openSet.size() > 0)
	{
		Node currentNode = openSet[0];
		for (int i = 1; i < openSet.size(); i++)
		{
			if (openSet[i].fCost() < currentNode.fCost() || openSet[i].fCost() == currentNode.fCost() && openSet[i].hCost < currentNode.hCost)
			{
				currentNode = openSet[i];
			}
		}
		// openSet remove current node
		auto iterator = std::find(openSet.begin(), openSet.end(), currentNode);
		if (iterator != openSet.end())
		{
			openSet.erase(iterator);
		}
		// closedSet add current node
		closedSet.push_back(currentNode);

		if (currentNode == target)
		{
			target.parent = currentNode.parent;
			RetracePath(start, target);
			
			return;
		}

		for (Node neighbor:GetNeighbors(currentNode))
		{
			if (/*!neighbor.walkable*/ std::find(mSelectedNodes.begin(), mSelectedNodes.end(), neighbor) != mSelectedNodes.end()|| 
				std::find(closedSet.begin(), closedSet.end(), neighbor) != closedSet.end())
			{
				continue;
			}

			int newMovementCostToNeighbor = currentNode.gCost + GetDistance(currentNode, neighbor);

			if (newMovementCostToNeighbor < neighbor.gCost || std::find(openSet.begin(), openSet.end(), neighbor) == openSet.end())
			{
				neighbor.gCost = newMovementCostToNeighbor;
				neighbor.hCost = GetDistance(neighbor, target);
				neighbor.parent.push_back(currentNode);

				if (std::find(openSet.begin(), openSet.end(), neighbor) == openSet.end())
				{
					openSet.push_back(neighbor);
				}

			}
		}
	}
}

// Used to draw the shortest path after find path completes (if a path is available)
void Pathfinding::RetracePath(Node start, Node target)
{
	std::vector<Node> path;
	Node currentNode = target;
	Node tmp = target;

	while (currentNode != start)
	{

		path.push_back(currentNode);
		if (currentNode.parent.size() != 0)
		{
			tmp = currentNode.parent[0];
		}

		currentNode = tmp;
	}

	std::reverse(path.begin(), path.end());
	mPath = path;
}

// Used within the A* algorithm to get the surrounding nodes of the current node being analyzed
std::vector<Node> Pathfinding::GetNeighbors(Node currentNode)
{
	std::vector<Node> neighbors;
	for (auto node:mNodes)
	{
		if (node.GetPosition().x == currentNode.GetPosition().x && node.GetPosition().y == currentNode.GetPosition().y)
		{
			continue;
		}

		if (node.GetPosition().x > 0 && node.GetPosition().x < GLOBAL_CONST_WINDOW_WIDTH && 
			node.GetPosition().x > (currentNode.GetPosition().x - (GLOBAL_CONST_WINDOW_WIDTH*GLOBAL_CONST_GRID_SIZE+GLOBAL_CONST_WINDOW_WIDTH*0.015)) &&
			node.GetPosition().x < (currentNode.GetPosition().x + (GLOBAL_CONST_WINDOW_WIDTH*GLOBAL_CONST_GRID_SIZE+GLOBAL_CONST_WINDOW_WIDTH*0.015)) &&
			node.GetPosition().y >= 0 && node.GetPosition().y < GLOBAL_CONST_WINDOW_HEIGHT &&
			node.GetPosition().y > (currentNode.GetPosition().y - (GLOBAL_CONST_WINDOW_HEIGHT*GLOBAL_CONST_GRID_SIZE+GLOBAL_CONST_WINDOW_WIDTH*0.015)) &&
			node.GetPosition().y < (currentNode.GetPosition().y + (GLOBAL_CONST_WINDOW_HEIGHT*GLOBAL_CONST_GRID_SIZE+GLOBAL_CONST_WINDOW_WIDTH*0.015)))

		{
			neighbors.push_back(node);
		}
	}
	
	return neighbors;
}

// Used within the A* algorithm to calculate distance between selected nodes
int Pathfinding::GetDistance(Node nodeA, Node nodeB)
{
	int distanceX = std::abs(nodeA.GetPosition().x - nodeB.GetPosition().x);
	int distanceY = std::abs(nodeA.GetPosition().y - nodeB.GetPosition().y);

	if (distanceX > distanceY)
	{
		return 14*distanceY + 10*(distanceX-distanceY);
	}
	else
	{
		return 14*distanceX + 10*(distanceY-distanceX);
	}
}

	
int main(int argc, char** argv)
{
	Pathfinding pathfinding;
	bool success = pathfinding.Initialize();
	if (success)
	{
		pathfinding.RunLoop();
	}
	pathfinding.Shutdown(); 
	return 0;
}

















