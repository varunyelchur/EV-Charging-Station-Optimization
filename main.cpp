#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <chrono>

using namespace std;

const double earthR = 6371.0;

double haversine(double latitude, double longitude, double latitude_two, double longitude_two) {
    const double DEG_TO_RAD = M_PI / 180.0;
    latitude *= DEG_TO_RAD;
    longitude *= DEG_TO_RAD;
    latitude_two *= DEG_TO_RAD;
    longitude_two *= DEG_TO_RAD;
    double dlat = latitude_two - latitude;
    double dlon = longitude_two - longitude;
    double a = sin(dlat / 2) * sin(dlat / 2) +
               cos(latitude) * cos(latitude_two) *
               sin(dlon / 2) * sin(dlon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = earthR * c;
    return distance;
}

// Struct to represent a node in the graph
struct Node {
    int id;
    double latitude;
    double longitude;
    string stateOrProvince;
    vector<pair<int, double>> neighbors;
};

// Function to build the graph
void buildGraph(const vector<Node>& nodes, double thresholdDistance, unordered_map<int, Node>& graph) {
    int numNodes = nodes.size();
    int edgeCount = 0;
    cout << "Building graph... (threshold: " << thresholdDistance << " km)" << endl;

    for (int i = 0; i < numNodes; i++) {
        Node node = nodes[i];
        for (int j = i + 1; j < numNodes; j++) {
            Node otherNode = nodes[j];
            double distance = haversine(node.latitude, node.longitude, otherNode.latitude, otherNode.longitude);
            if (distance <= thresholdDistance) {
                graph[node.id].neighbors.push_back({otherNode.id, distance});
                graph[otherNode.id].neighbors.push_back({node.id, distance});
                edgeCount++;
            }
        }
    }
}
//go through input and discard quoted sections
vector<string> parseCSVLine(const string& line) {
    vector<string> result;
    bool inQuotes = false;
    string field;
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                // Escaped quote
                field += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            result.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    result.push_back(field);
    return result;
}

//Djikstr'a Alg
pair<int, double> dijkstra_alg(int Id, const unordered_map<int, Node>& graph) {
    unordered_map<int, double> dist;
    vector<int> Ids;
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        int node = it->first;
        Ids.push_back(node);
        dist[node] = numeric_limits<double>::infinity();
    }
    dist[Id] = 0.0;
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<>> pq;
    pq.push({0.0, Id});
    while (!pq.empty()) {
        pair<double, int> top = pq.top();
        double currDist = top.first;
        int currNode = top.second;
        pq.pop();
        if (!(currDist > dist[currNode])) {
            const auto& neighbors = graph.at(currNode).neighbors;
            for (int i = 0; i < static_cast<int>(neighbors.size()); ++i) {
                int neighbor = neighbors[i].first;
                double weight = neighbors[i].second;
                double newDist = currDist + weight;

                if (newDist < dist[neighbor]) {
                    dist[neighbor] = newDist;
                    pq.push({newDist, neighbor});
                }
            }
        }
    }
    int farthestNodeId = -1;
    double maxDistance = 0.0;
    for (int i = 0; i < static_cast<int>(Ids.size()); ++i) {
        int nodeId = Ids[i];
        double distance = dist[nodeId];
        if (distance > maxDistance && distance < numeric_limits<double>::infinity()) {
            maxDistance = distance;
            farthestNodeId = nodeId;
        }
    }

    return {farthestNodeId, maxDistance};
}

pair<int, double> bellman_ford(int ID, const unordered_map<int, Node>& graph) {
    unordered_map<int, double> distanceMap;
    vector<int> nodeIDs;

    // Initialize distances to all nodes as infinity and collect node IDs
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        int nodeID = it->first;
        nodeIDs.push_back(nodeID);
        distanceMap[nodeID] = numeric_limits<double>::infinity();
    }
    distanceMap[ID] = 0.0;

    int size = graph.size();

    for (int i = 0; i < size - 1; i++) {
        bool updated = false;
        for (auto it = graph.begin();it!= graph.end(); it++) {
            int currentNode= it->first;
            vector<pair<int, double>> neighbors= it->second.neighbors;
            for (auto neighbor = neighbors.begin(); neighbor != neighbors.end(); neighbor++) {
                int neighborID = neighbor->first;
                double edgeWeight = neighbor->second;
                if (distanceMap[currentNode] != numeric_limits<double>::infinity() && distanceMap[currentNode] + edgeWeight < distanceMap[neighborID]) {
                    distanceMap[neighborID] = distanceMap[currentNode] + edgeWeight;
                    updated = true;
                }
            }
        }
        if (!updated) {
            break;
        }
    }

    // Find the farthest node from the source
    int farthestNodeID = -1;
    double maxDistance = 0.0;
    for (auto it = nodeIDs.begin(); it != nodeIDs.end(); ++it) {
        int nodeID = *it;
        double distance = distanceMap[nodeID];
        if (distance > maxDistance && distance != numeric_limits<double>::infinity()) {
            maxDistance = distance;
            farthestNodeID = nodeID;
        }
    }

    return {farthestNodeID, maxDistance};
}

vector<int> runDijkstras(const vector<Node>& nodes, const unordered_map<int, Node>& graph, const vector<Node>& newlyAdded, string inputS) {
    if (nodes.empty()) {
        cout << "No nodes." << endl;
        return {-1, -1};
    } else {
        int sourceId = -1;
        for (int i = 0; i < nodes.size(); ++i) {
            if (!graph.at(nodes[i].id).neighbors.empty()) {
                sourceId = nodes[i].id;
                break;
            }
        }
        if (sourceId == -1) {
            cout << "No suitable source." << endl;
            return {-1, -1};
        }

        //if already added in this state
        if(newlyAdded.size() > 0) {
            for (int i = newlyAdded.size() - 1; i >= 0; i--) {
                if (newlyAdded[i].stateOrProvince == inputS) {
                    sourceId = newlyAdded[i].id;
                    break;
                }
            }
        }
        cout << "Using source node: " << sourceId << endl;

        // Start timer
        auto start = chrono::high_resolution_clock::now();

        pair<int, double> result = dijkstra_alg(sourceId, graph);
        // End timer
        auto end = chrono::high_resolution_clock::now();

        // Calculate duration in nanoseconds
        auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
        cout << "Dijkstra's algorithm took " << duration << " nanoseconds." << endl;

        // Display the results
        if (result.first != -1) {
            cout << "The farthest node from node " << sourceId
                 << " is node " << result.first
                 << " with a distance of " << result.second << " km." << endl;

            return {sourceId, result.first};
        } else {
            cout << "No reachable nodes found from the source node." << endl;
            return {-1, -1};
        }
    }
}

vector<int> runBellmanFord(const vector<Node>& nodes, const unordered_map<int, Node>& graph, const vector<Node>& newlyAdded, const string& inputS) {
    if (nodes.empty()) {
        cout << "No nodes." << endl;
        return {-1, -1};
    } else {
        int sourceID = -1;

        for (int i = 0; i < nodes.size(); i++) {
            if (!graph.at(nodes[i].id).neighbors.empty()) {
                sourceID = nodes[i].id;
                break;
            }
        }
        if (sourceID == -1) {
            cout << "No suitable source node." << endl;
            return {-1, -1};
        }

        if(newlyAdded.size() > 0) {
            for (int i = newlyAdded.size() - 1; i >= 0; i--) {
                if (newlyAdded[i].stateOrProvince == inputS) {
                    sourceID = newlyAdded[i].id;
                    break;
                }
            }
        }

        cout << "Using source node: " << sourceID << endl;

        // Start timer
        auto start = chrono::high_resolution_clock::now();

        pair<int, double> result = bellman_ford(sourceID, graph);

        // End timer after function is called
        auto end = chrono::high_resolution_clock::now();

        // Calculate duration in nanoseconds
        auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
        cout << "Bellman-Ford algorithm took " << duration << " nanoseconds." << endl;

        // Display the results
        if (result.first != -1) {
            cout << "The farthest node from node " << sourceID << " is node " << result.first
                 << " with a distance of " << result.second << " km." << endl;
            // Return sourceID and farthest node ID as a vector
            return {sourceID, result.first};
        } else {
            cout << "No reachable nodes found from the source node." << endl;
            return {-1, -1};
        }
    }
}

vector<double> cMidpoint(const Node& one, const Node& two){
    double oneLat = one.latitude;
    double oneLon = one.longitude;
    double twoLat = two.latitude;
    double twoLon = two.longitude;
    double avgx = (oneLat + twoLat)/2;
    double avgy = (oneLon + twoLon)/2;
    vector<double> result;
    result.push_back(avgx);
    result.push_back(avgy);
    return result;
}

int main() {

    string csvFile = "../data/openchargemap_data.csv";
    ifstream file(csvFile);
    string line;


    if (!file.is_open()) {
        cout << "Error: Could not open file " << csvFile << endl;
        return 1;
    }

    cout << "File opened successfully!" << endl;

    vector<Node> allNodes;
    unordered_map<int, Node> graph;
    vector<Node> newlyAdded;

    //skip header since its column headers
    getline(file, line);

    int nodeId = 0;

    unordered_map<string, string> stateToAbbreviation = {
            // Full names
            {"ALABAMA", "AL"}, {"ALASKA", "AK"}, {"ARIZONA", "AZ"}, {"ARKANSAS", "AR"}, {"CALIFORNIA", "CA"},
            {"COLORADO", "CO"}, {"CONNECTICUT", "CT"}, {"DELAWARE", "DE"}, {"FLORIDA", "FL"}, {"GEORGIA", "GA"},
            {"HAWAII", "HI"}, {"IDAHO", "ID"}, {"ILLINOIS", "IL"}, {"INDIANA", "IN"}, {"IOWA", "IA"},
            {"KANSAS", "KS"}, {"KENTUCKY", "KY"}, {"LOUISIANA", "LA"}, {"MAINE", "ME"}, {"MARYLAND", "MD"},
            {"MASSACHUSETTS", "MA"}, {"MICHIGAN", "MI"}, {"MINNESOTA", "MN"}, {"MISSISSIPPI", "MS"}, {"MISSOURI", "MO"},
            {"MONTANA", "MT"}, {"NEBRASKA", "NE"}, {"NEVADA", "NV"}, {"NEW HAMPSHIRE", "NH"}, {"NEW JERSEY", "NJ"},
            {"NEW MEXICO", "NM"}, {"NEW YORK", "NY"}, {"NORTH CAROLINA", "NC"}, {"NORTH DAKOTA", "ND"},
            {"OHIO", "OH"}, {"OKLAHOMA", "OK"}, {"OREGON", "OR"}, {"PENNSYLVANIA", "PA"}, {"RHODE ISLAND", "RI"},
            {"SOUTH CAROLINA", "SC"}, {"SOUTH DAKOTA", "SD"}, {"TENNESSEE", "TN"}, {"TEXAS", "TX"}, {"UTAH", "UT"},
            {"VERMONT", "VT"}, {"VIRGINIA", "VA"}, {"WASHINGTON", "WA"}, {"WEST VIRGINIA", "WV"}, {"WISCONSIN", "WI"},
            {"WYOMING", "WY"},
            // Abbreviations mapped to themselves for easier lookup
            {"AL", "AL"}, {"AK", "AK"}, {"AZ", "AZ"}, {"AR", "AR"}, {"CA", "CA"}, {"CO", "CO"}, {"CT", "CT"},
            {"DE", "DE"}, {"FL", "FL"}, {"GA", "GA"}, {"HI", "HI"}, {"ID", "ID"}, {"IL", "IL"}, {"IN", "IN"},
            {"IA", "IA"}, {"KS", "KS"}, {"KY", "KY"}, {"LA", "LA"}, {"ME", "ME"}, {"MD", "MD"}, {"MA", "MA"},
            {"MI", "MI"}, {"MN", "MN"}, {"MS", "MS"}, {"MO", "MO"}, {"MT", "MT"}, {"NE", "NE"}, {"NV", "NV"},
            {"NH", "NH"}, {"NJ", "NJ"}, {"NM", "NM"}, {"NY", "NY"}, {"NC", "NC"}, {"ND", "ND"}, {"OH", "OH"},
            {"OK", "OK"}, {"OR", "OR"}, {"PA", "PA"}, {"RI", "RI"}, {"SC", "SC"}, {"SD", "SD"}, {"TN", "TN"},
            {"TX", "TX"}, {"UT", "UT"}, {"VT", "VT"}, {"VA", "VA"}, {"WA", "WA"}, {"WV", "WV"}, {"WI", "WI"},
            {"WY", "WY"}
    };

    // process each section of data
    //the sections sometimes will span multiple lines so process it until the section is finished
    while (file) {
        string section;
        int quoteCount = 0;
        bool sectionStarted = false;

        // Read lines until a complete section is formed
        while (getline(file, line)) {
            if (sectionStarted) {
                // Preserve newline character within quoted fields
                section += "\n";
            }
            section += line;
            sectionStarted = true;

            // Count the number of double quotes in the current section
            quoteCount = count(section.begin(), section.end(), '"');

            // If quoteCount is even, we have a complete section
            if (quoteCount % 2 == 0) {
                break;
            }
        }

        if (section.empty()) {
            // End of file
            break;
        }

        vector<string> row = parseCSVLine(section);

        // Handle incomplete rows
        if (row.size() < 11) {
            cout << "Warning: Incomplete or malformed row with ID: " << row[0] << endl;
            continue;
        }

        if (!row[9].empty() && !row[10].empty()) {
            // Convert Lat and Long to double
            double latitude = stod(row[9]);
            double longitude = stod(row[10]);

            // Extract stateOrProvince and normalize to abbreviation
            string stateOrProvince = row[6];
            transform(stateOrProvince.begin(), stateOrProvince.end(), stateOrProvince.begin(), ::toupper);

            if (stateToAbbreviation.find(stateOrProvince) != stateToAbbreviation.end()) {
                stateOrProvince = stateToAbbreviation[stateOrProvince];
            }

            // Check if the stateOrProvince is a U.S. state
            if (stateToAbbreviation.find(stateOrProvince) != stateToAbbreviation.end()) {
                // Create a node and add it to the list
                Node node;
                node.id = nodeId++;
                node.latitude = latitude;
                node.longitude = longitude;
                node.stateOrProvince = stateOrProvince;

                allNodes.push_back(node);
            } else {
                // Skip nodes not in the U.S.
                continue;
            }
        } else {
            cout << "Warning: Missing latitude/longitude in row ID: " << row[0] << endl;
        }

    }

    file.close();

    if (allNodes.empty()) {
        cout << "No stations found in the United States." << endl;
        return 1;
    }

    // Prompt the user for the U.S. state
    string inputState;
    cout << "Enter the U.S. state you would like the stations in (e.g., 'CA' or 'California'): " << endl;
    getline(cin, inputState);

    bool running = true;

    while (running) {

        // Normalize input state to abbreviation
        transform(inputState.begin(), inputState.end(), inputState.begin(), ::toupper);

        // Check if the input is a full state name
        if (stateToAbbreviation.find(inputState) != stateToAbbreviation.end()) {
            inputState = stateToAbbreviation[inputState]; // Convert to abbreviation
        }

        // Validate if inputState is now a valid U.S. state abbreviation
        if (stateToAbbreviation.find(inputState) == stateToAbbreviation.end()) {
            cout << "Invalid state, enter a US state:" << endl;
            getline(cin, inputState);
            continue;
        }


        // Clear previous graph and nodes for old state
        graph.clear();
        vector<Node> nodes;

        // Filter nodes based on the input state
        for (const auto &node: allNodes) {
            string nodeState = node.stateOrProvince;

            if (node.stateOrProvince == inputState) {
                nodes.push_back(node);
                graph[node.id] = node;
            }
        }

        if (nodes.empty()) {
            cout << "No stations found in the specified state: " << inputState << endl;
            // Prompt for a new state
            cout << "Enter the U.S. state you would like the stations in (e.g., 'CA' or 'California'): " << endl;
            getline(cin, inputState);
            continue;
        }

        if (nodes.empty()) {
            cout << "No stations found in the specified state: " << inputState << endl;
            return 1;
        }

        cout << "\nNumber of stations in " << inputState << ": " << nodes.size() << endl;

        double thresholdDistance = 2.0;

        // Build the graph
        buildGraph(nodes, thresholdDistance, graph);
        cout << "Graph built successfully!" << endl;

        // User menu
        while (true) {
            cout << "\nMenu:" << endl;
            cout << "1. Change state" << endl;
            cout << "2. Find best location for new node using Dijkstra's algorithm" << endl;
            cout << "3. Find best location for new node using Bellman Ford's Algorithm" << endl;
            cout << "4. Place a new optimized station(node) in the graph using an Algorithm" << endl;
            cout << "5. Exit" << endl;
            cout << "Enter your choice: " << endl;


            string choice1;
            cin >> choice1;
            int choice;

            //input validation
            try {
                choice = stoi(choice1);
            } catch (const invalid_argument&) {
                cout << "Invalid input. Please enter a number between 1 and 5." << endl;
                continue;
            }

            if (choice == 1) {
                // Change state
                cout << "Enter the U.S. state you would like the stations in (e.g., 'CA' or 'California'): " << endl;
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                getline(cin, inputState);
                // Break inner loop to rebuild graph with new state
                break;
            } else if (choice == 2) {
                runDijkstras(nodes, graph, newlyAdded, inputState);
            }
            else if (choice == 3) {
                runBellmanFord(nodes, graph, newlyAdded, inputState);
            }
            else if(choice == 4){
                cout << "\nWhich algorithm would you like to use?" << endl;
                cout << "1. Dijkstra's" << endl;
                cout << "2. Bellman Ford" << endl;
                cout << "Enter your choice: " << endl;
                string choice1;
                cin >> choice1;
                int choice2;

                //input validation
                try {
                    choice2 = stoi(choice1);
                } catch (const invalid_argument&) {
                    cout << "Invalid input. Please enter a number between 1 and 5." << endl;
                    continue;
                }


                if(choice2 == 1){
                    vector<int> values = runDijkstras(nodes, graph, newlyAdded, inputState);
                    int source = values[0];
                    int farthest = values[1];
                    if (source != -1 && farthest != -1) {
                        if (graph.find(source) != graph.end() && graph.find(farthest) != graph.end()) {
                            Node sourceNode = graph.find(source)->second;
                            Node farthestNode = graph.find(farthest)->second;
                            vector<double> midpoint = cMidpoint(sourceNode, farthestNode);
                            cout << "New Station Location Added! (midpoint):" << endl;
                            cout << "Latitude: " << midpoint[0] << ", Longitude: " << midpoint[1] << endl;
                            Node add;
                            add.latitude = midpoint[0];
                            add.longitude = midpoint[1];
                            add.id = allNodes.size();
                            add.stateOrProvince = inputState;
                            allNodes.push_back(add);
                            newlyAdded.push_back(add);

                            // Clear previous graph and nodes for old state
                            graph.clear();
                            vector<Node> nodes;

                            // Filter nodes based on the input state
                            for (const auto &node: allNodes) {
                                string nodeState = node.stateOrProvince;

                                if (node.stateOrProvince == inputState) {
                                    nodes.push_back(node);
                                    graph[node.id] = node;
                                }
                            }

                            cout << "\nNumber of stations in " << inputState << ": " << nodes.size() << endl;

                            thresholdDistance = 2.0;

                            // Build the graph
                            buildGraph(nodes, thresholdDistance, graph);
                            cout << "Graph built successfully!" << endl;
                        }
                        else{
                            cout << "Not in graph" << endl;
                        }
                    }
                }
                else if(choice2 == 2) {
                    vector<int> values = runBellmanFord(nodes, graph, newlyAdded, inputState);
                    int source = values[0];
                    int farthest = values[1];
                    if (source != -1 && farthest != -1) {
                        if (graph.find(source) != graph.end() && graph.find(farthest) != graph.end()) {
                            Node sourceNode = graph.find(source)->second;
                            Node farthestNode = graph.find(farthest)->second;
                            vector<double> midpoint = cMidpoint(sourceNode, farthestNode);
                            cout << "Suggested location (midpoint):" << endl;
                            cout << "Latitude: " << midpoint[0] << ", Longitude: " << midpoint[1] << endl;
                            Node add;
                            add.latitude = midpoint[0];
                            add.longitude = midpoint[1];
                            add.id = allNodes.size();
                            add.stateOrProvince = inputState;
                            allNodes.push_back(add);
                            newlyAdded.push_back(add);

                            // Clear previous graph and nodes for old state
                            graph.clear();
                            vector<Node> nodes;

                            // Filter nodes based on the input state
                            for (const auto &node: allNodes) {
                                string nodeState = node.stateOrProvince;

                                if (node.stateOrProvince == inputState) {
                                    nodes.push_back(node);
                                    graph[node.id] = node;
                                }
                            }

                            cout << "Number of stations in " << inputState << ": " << nodes.size() << endl;

                            thresholdDistance = 2.0;

                            // Build the graph
                            buildGraph(nodes, thresholdDistance, graph);
                            cout << "Graph built successfully!" << endl;
                        }
                        else{
                            cout << "Not in graph" << endl;
                        }
                    }
                }
                else {
                    cout << "Choose valid option" << endl;
                }
            }
            else if (choice == 5) {
                // Exit
                running = false;
                cout << "Exiting" << endl;
                break;
            } else {
                //fix
                cout << "Invalid choice. Please enter 1, 2, 3, 4, or 5." << endl;
            }
        }
    }
    return 0;
}