#define USE_RAYLIB
#ifdef USE_RAYLIB
// ----------------------------------------------------------------------------
// TODO -- Follow these directions to get started
// If you're using Visual Studio you'll have to 
//
//
//
//
// All that should mean that this works
#include "raylib.h"
// ----------------------------------------------------------------------------
#endif // USE_RAYLIB

#include "Helper.h"
#include <vector>
#include <stdio.h>
#include <thread>

#include <math.h>
#include <cmath>
#include <random>

// ----------------------------------------------------------------------------
// Your variables for this project should go here
// If you want to wrap them in other classes please help yourself
const int screenWidth = 1600;
const int screenHeight = 1000;

const int cellWidth = 4;
const int cellHeight = 4;
const int numRows = screenHeight / cellHeight;
const int numCols = screenWidth / cellWidth;

CPSC30000::Cell colorGrid[numRows][numCols]; // 2D array of cells
float redTrailGrid[numRows][numCols]; // 2D array of RED trail residue values (the ATTRACTIVE trail)
float greenTrailGrid[numRows][numCols]; // 2D array of GREEN trail residue values (the REPULSIVE trail)
float blueTrailGrid[numRows][numCols]; // 2D array of BLUE trail residue values (the REPULSIVE trail)
const float redTrailDecayRate = 0.01f; // Moderate
const float greenTrailDecayRate = 0.002f; // VERY Slow
const float blueTrailDecayRate = 0.07f; // Fast
const float redTrailDepositAmount = 1.0f; // Moderate
const float greenTrailDepositAmount = 0.1; // Very Low
const float blueTrailDepositAmount = 2.0f; // High

const float attractionCap = 5.0f; // maximum attraction value from trails
const float repulsionCap = 5 * attractionCap; // maximum repulsion value from trails

struct AgentState
{
    float xPos, yPos;
    float xVel, yVel;
};

const int numAgents = 3000;
const int agentSpeed = 4; // should be comparable to cellWidth and cellHeight (APPROXIMATE SPEED)
const int randomMovementBias = 0.5f; // adds extra random movement
const int agentVision = 2; // in cells
const float trailWeight = 0.9f; // how much the trail influences agent movement
AgentState agentArr[numAgents]; // an array containing the states of all agents

const Color agentColor = { 0, 255, 0, 255 };
const float trailColorSaturation = 25.0f;

const int threadCount = 8; // number of threads to use for multithreading

// ----------------------------------------------------------------------------


static void InitializeVariables()
{
    Color cellColor = { 0, 0, 0, 255 };

    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols; col++)
        {
            colorGrid[row][col] = CPSC30000::Cell(col * cellWidth, row * cellHeight, cellWidth, cellHeight, cellColor);
            // Initialize trail residue to zero
            redTrailGrid[row][col] = 0.0f; 
            greenTrailGrid[row][col] = 0.0f;
            blueTrailGrid[row][col] = 0.0f;
        }
    }
    
    for (int i = 0; i < numAgents; i++)
    {
        agentArr[i].xPos = static_cast<float>(GetRandomValue(agentSpeed * 5, screenWidth - (agentSpeed * 5)));
        agentArr[i].yPos = static_cast<float>(GetRandomValue(agentSpeed * 5, screenHeight - (agentSpeed * 5)));
        agentArr[i].xVel = static_cast<float>(agentSpeed);
		agentArr[i].yVel = static_cast<float>(0);
    }
}

static void DeallocateVariables()
{

}

// THIS FUNCTION WAS TAKEN FROM CHATGPT!!!
// I needed help figuring out how to generate a random float between two values in a fast and thread-safe way
float randomFloat(float a, float b) {
    thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(a, b);
    return dist(gen);
}

static void UpdateAgent(AgentState& state)
{
    int agentRow = static_cast<int>(state.yPos) / cellHeight;
    int agentCol = static_cast<int>(state.xPos) / cellWidth;

    // Sense trail residues in vision range + random movement
    float rowSum = 0.0f;
    float colSum = 0.0f;

    for (int rowDif = -1 * agentVision; rowDif <= agentVision; rowDif++)
    {
        for (int colDif = -1 * agentVision; colDif <= agentVision; colDif++)
        {
            int checkRow = (agentRow + rowDif + numRows) % numRows;
            int checkCol = (agentCol + colDif + numCols) % numCols;

            // Logic to sense trail residue and adjust velocity accordingly
            float trailValue = std::min(redTrailGrid[checkRow][checkCol], attractionCap) - std::min(greenTrailGrid[checkRow][checkCol] + blueTrailGrid[checkRow][checkCol], repulsionCap);
            rowSum += trailValue * ((rowDif > 0) - (rowDif < 0));
            colSum += trailValue * ((colDif > 0) - (colDif < 0));
        }
    }
    // Update velocity based on sensed trails
    state.yVel += (rowSum * trailWeight) + randomFloat(-randomMovementBias * agentSpeed, randomMovementBias * agentSpeed);
    state.xVel += (colSum * trailWeight) + randomFloat(-randomMovementBias * agentSpeed, randomMovementBias * agentSpeed);

    // Normalize velocity to maintain constant speed
	float xSpeed = fabs(state.xVel);
	float ySpeed = fabs(state.yVel);
	float currentSpeed = std::max(xSpeed, ySpeed) + 0.5 * std::min(xSpeed, ySpeed); // APPROXIMATION TO AVOID SLOW SQRT
    //float currentSpeed = sqrt(state.xVel * state.xVel + state.yVel * state.yVel); // sqrt IS SLOW !!!!! 
    state.yVel *= agentSpeed / currentSpeed;
    state.xVel *= agentSpeed / currentSpeed;

    // Update position based on velocity
    state.xPos = fmodf((state.xPos + state.xVel + screenWidth), screenWidth);
    state.yPos = fmodf((state.yPos + state.yVel + screenHeight), screenHeight);
}

static void UpdateAgentThreaded(AgentState* states, int start, int end)
{
    for (int i = start; i < end; i++)
    {
        UpdateAgent(states[i]);
    }
}

static void Update()
{
	// decay trail residues
    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols; col++)
        {
            redTrailGrid[row][col] *= (1.0f - redTrailDecayRate);
			greenTrailGrid[row][col] *= (1.0f - greenTrailDecayRate);
            blueTrailGrid[row][col] *= (1.0f - blueTrailDecayRate);
        }
	}

	// deposit trail residues
    for (int i = 0; i < numAgents; i++)
    {
        int agentRow = static_cast<int>(agentArr[i].yPos) / cellHeight;
        int agentCol = static_cast<int>(agentArr[i].xPos) / cellWidth;
        redTrailGrid[agentRow][agentCol] += redTrailDepositAmount;
		greenTrailGrid[agentRow][agentCol] += greenTrailDepositAmount;
        blueTrailGrid[agentRow][agentCol] += blueTrailDepositAmount;
	}

    // update all agents (MULTITHREADED)
	std::thread threads[threadCount];
    for (int i = 0; i < threadCount; i++)
    {
        int start = i * (numAgents / threadCount);
        int end = (i == threadCount - 1) ? numAgents : start + (numAgents / threadCount);
		threads[i] = std::thread(UpdateAgentThreaded, agentArr, start, end);
    }

    for (int i = 0; i < threadCount; i++)
    {
		threads[i].join();
    }
}

static void Draw()
{
    BeginDrawing();

    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols; col++)
        {
			int redValue = static_cast<int>((redTrailGrid[row][col] / trailColorSaturation) * 255);
			int greenValue = static_cast<int>((greenTrailGrid[row][col] / trailColorSaturation) * 255);
			int blueValue = static_cast<int>((blueTrailGrid[row][col] / trailColorSaturation) * 255);

			redValue = std::min(redValue, 255);
			greenValue = std::min(greenValue, 255);
			blueValue = std::min(blueValue, 255);

            Color cellColor = { redValue, greenValue, blueValue, 255 };
			colorGrid[row][col].ChangeColor(cellColor);
			colorGrid[row][col].Draw();
        }
    }

    EndDrawing();
}

int main()
{
    InitWindow(screenWidth, screenHeight, "raylib multithread simulation");
    SetTargetFPS(60);

    InitializeVariables();

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        Update();
        Draw();
    }

    DeallocateVariables();

    CloseWindow();
    return 0;
}
