// Verify that the production CIN parser preserves every Windows association.
#include "OVCINDataTable.h"
#include <fstream>
#include <sstream>
#include <memory>
int main(int argc, char **argv) {
    if (argc != 2) return 2;
    std::ifstream input(argv[1]);
    if (!input) return 2;
    std::map<std::string, std::vector<std::string>> expected;
    std::string line;
    bool inDefinitions = false;
    size_t count = 0;
    while (std::getline(input, line)) {
        if (line == "%chardef begin") { inDefinitions = true; continue; }
        if (line == "%chardef end") { inDefinitions = false; break; }
        if (!inDefinitions || line.empty() || line[0] == '#') continue;
        std::istringstream row(line);
        std::string key, value;
        if (!(row >> key >> value)) return 2;
        expected[key].push_back(value);
        ++count;
    }
    OpenVanilla::OVCINDataTableParser parser;
    std::unique_ptr<OpenVanilla::OVCINDataTable> table(parser.CINDataTableFromFileName(argv[1]));
    if (!table || expected.empty() || inDefinitions) return 2;
    for (const auto &entry : expected) {
        if (table->findChardef(entry.first) != entry.second) {
            std::cerr << "Order mismatch: " << entry.first << '\n';
            return 1;
        }
    }
    std::cout << "PASS: " << count << " associations across " << expected.size() << " keys retain file order\n";
}
