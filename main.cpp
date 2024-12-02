#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// Function to parse a CSV line considering quoted fields
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

int main() {
    string csvFile = "../data/openchargemap_data.csv";
    ifstream file(csvFile);
    string line;

    // Check if the file opens successfully
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << csvFile << endl;
        return 1;
    }

    cout << "File opened successfully!" << endl;

    vector<pair<double, double>> coordinates; // To store latitude and longitude

    // Skip the header row
    getline(file, line);

    // Process each record
    while (file) {
        string record;
        int quoteCount = 0;
        bool recordStarted = false;

        // Read lines until a complete record is formed
        while (getline(file, line)) {
            if (recordStarted) {
                record += "\n"; // Preserve newline character within quoted fields
            }
            record += line;
            recordStarted = true;

            // Count the number of double quotes in the current record
            quoteCount = count(record.begin(), record.end(), '"');

            // If quoteCount is even, we have a complete record
            if (quoteCount % 2 == 0) {
                break;
            }
        }

        if (record.empty()) {
            // End of file
            break;
        }

        vector<string> row = parseCSVLine(record);

        // Handle incomplete rows
        if (row.size() < 11) {
            cerr << "Warning: Incomplete or malformed row with ID: " << row[0] << endl;
            continue;
        }

        try {
            // Validate and parse Latitude and Longitude
            if (!row[9].empty() && !row[10].empty()) {
                double latitude = stod(row[9]);   // Convert Latitude to double
                double longitude = stod(row[10]); // Convert Longitude to double
                coordinates.push_back({latitude, longitude});
            } else {
                cerr << "Warning: Missing latitude/longitude in row ID: " << row[0] << endl;
            }
        } catch (const invalid_argument& e) {
            cerr << "Warning: Invalid latitude/longitude in row ID: " << row[0] << endl;
        }
    }

    file.close();

    // Print extracted latitude and longitude
    cout << "Extracted Coordinates:" << endl;
    for (const auto& coord : coordinates) {
        cout << "Latitude: " << coord.first << ", Longitude: " << coord.second << endl;
    }

    return 0;
}
