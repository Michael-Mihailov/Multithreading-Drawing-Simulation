# Multithreading-Drawing-Simulation
A project to practice multithreading using a graphics simulation in raylib.

RAYLIB MUST BE INSTALLED FOR THE PROJECT TO FUNCTION!!!

Part 1 (Colors):

The colors represent “pheromones”. Agents are attracted to red pheromones and repealed by blue and green pheromones.
Red: MODERATE lifespan, MODERATE attraction
Green: LONG lifespan, WEAK repulsion
Blue: SHORT lifespan, STRONG repulsion
The colors all mix when they are rendered.
The brighter the color the greater the pheromone concentration.

Part 2 (Thread Interactions):

The simulation has a few thousand agents (you can see them moving around and leaving purple colored streaks behind them). The agents interact with each other’s pheromones as it influences their movements. The simulation uses 8 threads and splits the simulation of the agents between the threads. Unfortunately, drawing onto the screen cannot be multithreaded so it serves as a major bottleneck.
