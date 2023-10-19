#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <algorithm>
#include <unordered_set>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;


nlohmann::json generateMetadata(const nlohmann::json& traceData) {
    struct metadata_t {
        size_t tid;
        size_t pid;
        std::string cat;

        // Equality comparator for metadata_t
        bool operator==(const metadata_t& other) const {
            return tid == other.tid && pid == other.pid && cat == other.cat;
        }
    };

    // Custom hash function for metadata_t
    struct metadata_hash {
        size_t operator()(const metadata_t& meta) const {
            return std::hash<size_t>()(meta.tid) ^
                   std::hash<size_t>()(meta.pid) ^
                   std::hash<std::string>()(meta.cat);
        }
    };

    std::unordered_set<metadata_t, metadata_hash> metadata;

    for (const auto& event : traceData) {
        metadata_t data;
        data.tid = event["tid"].get<size_t>();
        data.pid = event["pid"].get<size_t>();
        data.cat = event["cat"].get<std::string>();
        metadata.insert(data);
    }

    nlohmann::json metadataList = nlohmann::json::array();
    for (const auto& entry : metadata) {
        metadataList.push_back({
            {"cat", "__metadata"},
            {"name", "thread_name"},
            {"ph", "M"},
            {"pid", entry.pid},
            {"tid", entry.tid},
            {"args", {{"name", entry.cat}}}
        });
    }

    return metadataList;
}

void mergeFiles(const std::vector<std::string>& filePaths, const std::string& outputPath)
{
    nlohmann::json masterList = nlohmann::json::array();

    for (const auto& path : filePaths)
    {
        if (!fs::exists(path))
        {
            std::cerr << "File does not exist: " << path << '\n';
            continue;
        }

        std::ifstream inputFile(path);
        if (!inputFile)
        {
            std::cerr << "Failed to open file: " << path << '\n';
            continue;
        }

        nlohmann::json currentList;
        inputFile >> currentList;
        masterList.insert(masterList.end(), currentList.begin(), currentList.end());
    }

    // Sort masterList by the "ts" timestamp
    std::sort(masterList.begin(), masterList.end(),
              [](const nlohmann::json& a, const nlohmann::json& b) { return a["ts"] < b["ts"]; });
    nlohmann::json metadata = generateMetadata(masterList);
    masterList.insert(masterList.begin(), metadata.begin(), metadata.end());

    // Write the merged content to the output file
    std::ofstream outputFile(outputPath);
    if (!outputFile)
    {
        std::cerr << "Failed to open output file: " << outputPath << '\n';
        return;
    }
    outputFile << masterList.dump(4);
}


int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <output file path> <input file pattern1> [<input file pattern2> ...]" << std::endl;
        return 1;
    }

    std::string outputPath = argv[1];

    std::vector<std::string> filePaths;
    for (int i = 2; i < argc; ++i)
    {
        filePaths.push_back(argv[i]);
       
    }
    std::cout << "Merging the following files:\n";
    for(const auto& path: filePaths){
        std::cout <<"\t" <<path << std::endl;
    }

    mergeFiles(filePaths, outputPath);

    return 0;
}
