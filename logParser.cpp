#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <regex>
#include <algorithm>

using namespace std;

const string LOG_VERSION = "2";

const regex LOG_REGEX(R"(^(\d+)\s+\d+\s+\S+\s+[0-9\.]+\s+[0-9\.]+\s+(\d+)\s+\d+\s+(\d+)\s+\d+\s+\d+\s+\d+\s+\d+\s+(\w+)\s+(\w+)\s*$)");

unordered_map<string, string> genProtocolMappings() {
    return {
        {"6", "tcp"},
        {"17", "udp"},
        {"1", "icmp"}
    };
}

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

unordered_map<string, string> parseLookup(const string& filePath) {
    unordered_map<string, string> lookup;
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening lookup file: " << filePath << endl;
        exit(1);
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string dstport, protocol, tag;
        if (!(getline(ss, dstport, ',') && getline(ss, protocol, ',') && getline(ss, tag, ','))) {
            cerr << "Skipping malformed line in lookup file: " << line << endl;
            continue;
        }

        transform(protocol.begin(), protocol.end(), protocol.begin(), ::tolower);
        lookup[dstport + "," + protocol] = trim(tag);
    }
    return lookup;
}

pair<map<string, int>, map<string, int>> parseLog(const string& filePath, const unordered_map<string, string>& lookup) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening log file: " << filePath << endl;
        exit(1);
    }

    map<string, int> tagCounts, portProtocolCounts;
    unordered_map<string, string> protocolMappings = genProtocolMappings();
    int untaggedCount = 0;

    string line;
    while (getline(file, line)) {
        smatch match;
        if (!regex_match(line, match, LOG_REGEX)) {
            cerr << "Skipping malformed log line: " << line << endl;
            continue;
        }

        if (match[1] != LOG_VERSION) continue;

        string dstport = match[2];
        string protocolNum = match[3];
        string action = match[4];
        string status = match[5];

        if (protocolMappings.find(protocolNum) == protocolMappings.end()) {
            cerr << "Skipping unknown protocol number: " << protocolNum << endl;
            continue;
        }
        string protocol = protocolMappings[protocolNum];

        if (action != "ACCEPT" || status != "OK") continue;

        string key = dstport + "," + protocol;
        portProtocolCounts[key]++;

        if (lookup.find(key) != lookup.end()) {
            tagCounts[lookup.at(key)]++;
        } else {
            untaggedCount++;
        }
    }

    tagCounts["Untagged"] = untaggedCount; 
    return {tagCounts, portProtocolCounts};
}

void writeOutput(const string& filePath, const map<string, int>& tagCounts, const map<string, int>& portProtocolCounts) {
    ofstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening output file: " << filePath << endl;
        exit(1);
    }

    file << "Tag Counts:\nTag,Count\n";
    
    if (tagCounts.find("Untagged") != tagCounts.end()) {
        file << "Untagged," << tagCounts.at("Untagged") << "\n";
    }

    vector<pair<string, int>> sortedTags;
    for (const auto& pair : tagCounts) {
        if (pair.first != "Untagged") {
            sortedTags.push_back(pair);
        }
    }

    sort(sortedTags.begin(), sortedTags.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.first < b.first;
    });

    for (const auto& pair : sortedTags) {
        file << pair.first << "," << pair.second << "\n";
    }
    file << "\n";

    file << "Port/Protocol Combination Counts:\nPort,Protocol,Count\n";
    vector<pair<int, string>> sortedPorts;

    for (const auto& pair : portProtocolCounts) {
        stringstream ss(pair.first);
        int port;
        string protocol;
        ss >> port;
        getline(ss, protocol, ',');
        sortedPorts.push_back({port, pair.first});
    }

    sort(sortedPorts.begin(), sortedPorts.end());

    for (const auto& pair : sortedPorts) {
        stringstream ss(pair.second);
        string port, protocol;
        getline(ss, port, ',');
        getline(ss, protocol, ',');
        file << port << "," << protocol << "," << portProtocolCounts.at(pair.second) << "\n";
    }

    cout << "Output written to: " << filePath << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <log_file> <lookup_file> <output_file>" << endl;
        return 1;
    }

    string logPath = argv[1];
    string lookupPath = argv[2];
    string outputPath = argv[3];

    unordered_map<string, string> lookup = parseLookup(lookupPath);
    auto result = parseLog(logPath, lookup);
    map<string, int> tagCounts = result.first;
    map<string, int> portProtocolCounts = result.second;
    writeOutput(outputPath, tagCounts, portProtocolCounts);

    return 0;
}
