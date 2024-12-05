# EV-Charging-Station-Optimization
Final Project Group 169

Varun Yelchur
Jacob Schreck
Nicholas Levy

Problem:
The problem that we are trying to solve is where exactly charging stations should be built for Electric Vehicles based on the location and amount of charging stations.

Motivation:
This is a problem because as more Electric Vehicles are bought and driven which is increasing every day, charging stations are needed in more places. Using this problem would allow us to develop a solution that would optimize the placement of these charging stations based on existing stations.

Features:
The user will be able to input a state where they would like to place new station. The algorithm will decide an optimal placement for a new charging station based on existing station location data. The new stations will be added to the graph of nodes and will be saved to the graph whenever that state is accessed again. The two algorithms used to compare it would be Bellman Ford's and Dijkstra's Algorithm.

Data
Open Charge Map dataset of global electric vehicle charging station data, specifically the latitude and longitude of the stations. https://openchargemap.org/site : https://github.com/openchargemap/ocm-data

Tools:
We will use C++, Clion, and Github in our project, for the UI maybe SFML.
We will use Djkstra's algorithm and Bellman Ford's Algorithm to find the shortest and optimal path in our graph.

Visuals:
Through the terminal in the code, it will show a menu with five options: 1. Allows you to change the state, 2. Runs Dijkstra's Algorithm and gives you the two nodes being used and the distance, 3. It does the same as #2 but using Bellman Ford's Algorithm, 4. Actually places the node in the graph either using Bellman Ford or Dijkstra's which is chosen by the user, and 5. Exit. It displays the Latitude and Longitude of the new station and displays the time in nanoseconds when ran through either algorithm. 

Strategy:
We will represent the data using hashmaps for accessing data and put it as a graph to ultimately manage station locations. We will utilize both algorithms and the user can choose which one they either want to see or actually place the node. Both will display the time so by doing step 2 and 3, the time can be compared.

Distribution of Responsibility and Roles
Varun: Djikstra's Algorithm, executing algorithms in the main, and execution of placing the node into the graph while saving it.
Jacob: Preproccesses data, saves it, implements a graph and works on a lot of the main, and getting the code to be cohesive.
Nicholas: Bellman Ford's Algorithm, executing algorithms in the main, and the timer to compare the two.


