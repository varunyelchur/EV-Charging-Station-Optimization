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
The user will be able to input a maximum distance for placement. The algorithm will decide an optimal placement for a new charging station based on existing station location data. The new stations will be added to the graph of nodes and visually shown after input.
Data
Open Charge Map dataset of global electric vehicle charging station data, specifically the latitude and longitude of the stations. https://openchargemap.org/site : https://github.com/openchargemap/ocm-data

Tools:
We will use C++, Clion, and Github in our project, for the UI maybe SFML.

Visuals


Strategy:
We will represent the data using hashmaps for accessing data and graphs for managing station locations.

Distribution of Responsibility and Roles
Varun: Parsing through and saving data into sub-sections based on country.
Jacob: Saving data to hashmaps for easy access for later use.
Nicholas: Using data saved to calculate new charging station locations based on existing graph nodes.


