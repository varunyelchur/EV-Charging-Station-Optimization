#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

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
    cout << "Building graph with threshold distance: " << thresholdDistance << " km" << endl;

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
    dist[Id] = 0.0;
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        int node = it->first;
        Ids.push_back(node);
        dist[node] = numeric_limits<double>::infinity();
    }

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

    //skip header since its column headers
    getline(file, line);

    int nodeId = 0;

    // Set of U.S. state abbreviations
    unordered_set<string> usStates = {
            "AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA",
            "HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD",
            "MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ",
            "NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC",
            "SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY",
            // Include full state names if necessary
            "Alabama", "Alaska", "Arizona", "Arkansas", "California", "Colorado", "Connecticut", "Delaware", "Florida", "Georgia",
            "Hawaii", "Idaho", "Illinois", "Indiana", "Iowa", "Kansas", "Kentucky", "Louisiana", "Maine", "Maryland",
            "Massachusetts", "Michigan", "Minnesota", "Mississippi", "Missouri", "Montana", "Nebraska", "Nevada", "New Hampshire", "New Jersey",
            "New Mexico", "New York", "North Carolina", "North Dakota", "Ohio", "Oklahoma", "Oregon", "Pennsylvania", "Rhode Island", "South Carolina",
            "South Dakota", "Tennessee", "Texas", "Utah", "Vermont", "Virginia", "Washington", "West Virginia", "Wisconsin", "Wyoming"
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

            // Extract stateOrProvince
            string stateOrProvince = row[6];

            // Check if the stateOrProvince is a U.S. state
            if (usStates.find(stateOrProvince) != usStates.end()) {
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
        }
        else {
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
    cout << "Enter the U.S. state you would like the stations in (e.g., 'CA' or 'California'): ";
    getline(cin, inputState);

    bool running = true;

    while(running){
        // Convert input to uppercase for comparison (for state abbreviations)
        string inputStateUpper = inputState;
        transform(inputStateUpper.begin(), inputStateUpper.end(), inputStateUpper.begin(), ::toupper);

        // Convert input to title case for comparison (for state names)
        string inputStateTitle = inputState;
        transform(inputStateTitle.begin(), inputStateTitle.begin() + 1, inputStateTitle.begin(), ::toupper);
        transform(inputStateTitle.begin() + 1, inputStateTitle.end(), inputStateTitle.begin() + 1, ::tolower);

        // Clear previous graph and nodes for old state
        graph.clear();
        vector<Node> nodes;

        // Filter nodes based on the input state
        for (const auto& node : allNodes) {
            string nodeState = node.stateOrProvince;

            if (nodeState == inputStateUpper || nodeState == inputStateTitle) {
                nodes.push_back(node);
                graph[node.id] = node;
            }
        }

        if (nodes.empty()) {
            cout << "No stations found in the specified state: " << inputState << endl;
            // Prompt for a new state
            cout << "Enter the U.S. state you would like the stations in (e.g., 'CA' or 'California'): ";
            getline(cin, inputState);
            continue;
        }

        if (nodes.empty()) {
            cout << "No stations found in the specified state: " << inputState << endl;
            return 1;
        }

        cout << "Number of stations in " << inputState << ": " << nodes.size() << endl;

        double thresholdDistance = 2.0;

        // Build the graph
        cout << "Building graph..." << endl;
        buildGraph(nodes, thresholdDistance, graph);
        cout << "Graph built successfully!" << endl;
        cout << "Number of nodes: " << nodes.size() << endl;


        // User menu
        while (true) {
            cout << "\nMenu:" << endl;
            cout << "1. Change state" << endl;
            cout << "2. Find best location for new node" << endl;
            cout << "3. Exit" << endl;
            cout << "Enter your choice: ";
            int choice;
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear input buffer

            if (choice == 1) {
                // Change state
                cout << "Enter the U.S. state you would like the stations in (e.g., 'CA' or 'California'): ";
                getline(cin, inputState);
                // Break inner loop to rebuild graph with new state
                break;
            } else if (choice == 2) {
                if (nodes.empty()) {
                    cout << "No nodes available in the graph!" << endl;
                } else {
                    int sourceId = -1;
                    for (int i = 0; i < nodes.size(); ++i) {
                        if (!graph.at(nodes[i].id).neighbors.empty()) {
                            sourceId = nodes[i].id;
                            break;
                        }
                    }
                    if (sourceId == -1) {
                        cout << "No suitable source node with neighbors found!" << endl;
                        return 0;
                    }

                    cout << "Using source node: " << sourceId << endl;

                    // Call the updated Dijkstra's function
                    pair<int, double> result = dijkstra_alg(sourceId, graph);

                    // Display the results
                    if (result.first != -1) {
                        cout << "The farthest node from node " << sourceId
                             << " is node " << result.first
                             << " with a distance of " << result.second << " km." << endl;
                    }
                    else {
                        cout << "No reachable nodes found from the source node." << endl;
                    }
                }
            }
            else if (choice == 3) {
                // Exit
                running = false;
                cout << "Exiting" << endl;
                break;
            } else {
                cout << "Invalid choice. Please enter 1, 2, or 3." << endl;
            }
        }
    }

    return 0;
}
