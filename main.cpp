#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cmath>

const int window_width = 700;
const int window_height = 700;
const float grid_size = 0.05;

struct Vector2 {
	float x;
	float y;
};

class Node
{
public:
	std::vector<Node> parent;
	Node(SDL_Rect rect, int id);
	SDL_Rect* getRect() { return &mRect; };
	Vector2 getPosition() { return mPosition; };
	int GetId() { return mID; }
	int gCost;
	int hCost;
	int fCost() { return gCost + hCost; };
	bool walkable = true;
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
}


			


class Game
{
public:
	Game();
	// Initialize the game
	bool Initialize();
	// Runs the game loop until the game is over
	void RunLoop();
	// Shutdown the game
	void Shutdown();
private:
	// Helper functions for the game loop
	void ProcessInput();
	void UpdateGame() {};
	void GenerateOutput();
	void drawGrid(SDL_Renderer* renderer, int windowWidth, int windowHeight);
	void makeNodes(int windowWidth, int windowHeight);
	void findPath(Node start, Node target);
	void RetracePath(Node start, Node target);
	std::vector<Node> GetNeighbors(Node node);
	int GetDistance(Node nodeA, Node nodeB);

	// Window created by SDL
	SDL_Window* mWindow;
	// Renderer to draw graphics created by SDL
	SDL_Renderer* mRenderer;
	// Game should continue to run
	bool mIsRunning;
	bool mMouseDown;
	bool mErase;
	std::vector<Node> mNodes;
	std::vector<Node> mSelectedNodes;
	std::vector<Node> mNeighbors;
	std::vector<Node> mPath;
	int mXMouse;
	int mYMouse;
};

Game::Game()
{
	mWindow = nullptr;
	mRenderer = nullptr;
	mIsRunning = true;
	mMouseDown = false;
	mErase = false;
}

// The Initialization function returns true 
//if initialization succeeds and false otherwise
bool Game::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);

	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	mWindow = SDL_CreateWindow(
				"Game Programming (Chapter 1)",
				100,  // Top left x-coordinate of window
				100,  // Top left y-coordinate of window
				window_width, // Width of window
				window_height,  // Height of window
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

	makeNodes(window_width, window_height);


	return true;
}

// Runloop keeps running iterations of the game until mIsRunning becomes false
void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::Shutdown()
{
	SDL_DestroyWindow(mWindow);
	SDL_DestroyRenderer(mRenderer);
	SDL_Quit();
}

void Game::ProcessInput()
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
			break;

			case SDL_MOUSEBUTTONUP:
			mMouseDown = false;
			break;
		}
	}

	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}
	
	if (state[SDL_SCANCODE_LSHIFT])
	{
		mErase = true;
	}
	else
	{ 
		mErase = false;
	}

}

void Game::GenerateOutput()
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

	// Draw game scene

	

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
		if (node.getRect()->x < mXMouse && (node.getRect()->x + node.getRect()->w) > mXMouse
			&& node.getRect()->y < mYMouse && (node.getRect()->y + node.getRect()->h) > mYMouse)
		{

			SDL_RenderFillRect(mRenderer, node.getRect());
		}

	}

	if (mMouseDown == true && mErase == false)
	{

		for ( auto node:mNodes)
		{
			if (node.getRect()->x < mXMouse && (node.getRect()->x + node.getRect()->w) > mXMouse
				&& node.getRect()->y < mYMouse && (node.getRect()->y + node.getRect()->h) > mYMouse)
			{
				auto iterator = std::find(mSelectedNodes.begin(), mSelectedNodes.end(), node);
				if (iterator == mSelectedNodes.end() && mSelectedNodes.size() < 2)
				{
					mSelectedNodes.push_back(node);

					std::cout << mNeighbors.size() << std::endl;
					std::cout << "node added" << std::endl;

				}
			}
		}
	}

	if (mErase == true && mMouseDown == true)
	{

		for ( auto node:mNodes)
		{
			if (node.getRect()->x < mXMouse && (node.getRect()->x + node.getRect()->w) > mXMouse
				&& node.getRect()->y < mYMouse && (node.getRect()->y + node.getRect()->h) > mYMouse)
			{
				auto iterator = std::find(mSelectedNodes.begin(), mSelectedNodes.end(), node);
				if (iterator != mSelectedNodes.end())
				{
					mSelectedNodes.erase(iterator);
					std::cout << "node deleted" << std::endl;

				}
			}
		}
		mErase = false;
	}


	for (auto node:mSelectedNodes)
	{
		
		SDL_SetRenderDrawColor(
					mRenderer,
					255,
					0,
					0,
					255
				);

		SDL_RenderFillRect(mRenderer, node.getRect());
	}
 	
	if (mSelectedNodes.size() > 1)
	{
		findPath(mSelectedNodes[0], mSelectedNodes[1]);
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
		if ( std::find(mSelectedNodes.begin(), mSelectedNodes.end(), node) == mSelectedNodes.end() && mSelectedNodes.size() > 1)
		{

			SDL_RenderFillRect(mRenderer, node.getRect());
		}
	}

	drawGrid(mRenderer, window_width,window_height);
	// Swap the front and back buffers
	SDL_RenderPresent(mRenderer);
}

void Game::drawGrid(SDL_Renderer* renderer, int windowWidth, int windowHeight)
{

		SDL_SetRenderDrawColor(
		renderer,
		100,
		101,
		103,
		255
	);
	
	for ( int i = 0; i < windowWidth; i += windowWidth * grid_size)
	{
		SDL_RenderDrawLine(renderer, i, 0, i, windowHeight);
	}

	for ( int i = 0; i < windowHeight; i += windowHeight * grid_size)
	{
		SDL_RenderDrawLine(renderer, 0, i, windowWidth, i);
	}
}

void Game::makeNodes(int windowWidth, int windowHeight)
{
	int id = 0;
	for ( int i = 0; i < windowWidth; i += windowWidth * grid_size)
	{
		for ( int j = 0; j < windowHeight; j += windowHeight * grid_size)
		{
			SDL_Rect rect{i, j, static_cast<int>(windowWidth * grid_size), static_cast<int>(windowHeight * grid_size)};
			auto node = new Node(rect, id);
			mNodes.push_back(*node);
			id++;
		}
	}
}

void Game::findPath(Node start, Node target)
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
			if (!neighbor.walkable || std::find(closedSet.begin(), closedSet.end(), neighbor) != closedSet.end())
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

void Game::RetracePath(Node start, Node target)
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

std::vector<Node> Game::GetNeighbors(Node currentNode)
{
	std::vector<Node> neighbors;
	for (auto node:mNodes)
	{
		if (node.getPosition().x == currentNode.getPosition().x && node.getPosition().y == currentNode.getPosition().y)
		{
			continue;
		}

		if (node.getPosition().x > 0 && node.getPosition().x < window_width && 
			node.getPosition().x > (currentNode.getPosition().x - (window_width*grid_size+window_width*0.015)) &&
			node.getPosition().x < (currentNode.getPosition().x + (window_width*grid_size+window_width*0.015)) &&
			node.getPosition().y >= 0 && node.getPosition().y < window_height &&
			node.getPosition().y > (currentNode.getPosition().y - (window_height*grid_size+window_width*0.015)) &&
			node.getPosition().y < (currentNode.getPosition().y + (window_height*grid_size+window_width*0.015)))

		{
			neighbors.push_back(node);
		}
	}
	
	return neighbors;
}

int Game::GetDistance(Node nodeA, Node nodeB)
{
	int distanceX = std::abs(nodeA.getPosition().x - nodeB.getPosition().x);
	int distanceY = std::abs(nodeA.getPosition().y - nodeB.getPosition().y);

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
	Game game;
	bool success = game.Initialize();
	if (success)
	{
		game.RunLoop();
	}
	game.Shutdown(); 
	return 0;
}

















