#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cctype>
#include "HuffmanTree.hpp"

// Split path into {dir, base, ext}
static void splitPath(const std::string& path, std::string& dir, std::string& base, std::string& ext) {
    size_t slash = path.find_last_of("/\\");
    dir = (slash == std::string::npos) ? "" : path.substr(0, slash + 1);
    std::string file = (slash == std::string::npos) ? path : path.substr(slash + 1);

    size_t dot = file.find_last_of('.');
    if (dot == std::string::npos) { base = file; ext = ""; }
    else { base = file.substr(0, dot); ext = file.substr(dot); }
}

static bool readHeader(const std::string& hdrPath,
                       std::vector<std::pair<std::string,std::string>>& pairs,
                       std::string& err)
{
    std::ifstream in(hdrPath);
    if (!in) { err = "Cannot open header file: " + hdrPath; return false; }

    std::string line;
    size_t lineno = 0;
    bool checked_first_line = false;

    while (std::getline(in, line)) {
        ++lineno;
        // skip fully blank
        size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
        if (i == line.size()) continue;

        if (!checked_first_line) {
            checked_first_line = true;
            if (line[i] == '0' || line[i] == '1') {
                err = "Header seems to start with a bit on line " + std::to_string(lineno) + ".";
                return false;
            }
        }

        // parse: WORD <spaces> CODE
        size_t j = i;
        while (j < line.size() && !std::isspace(static_cast<unsigned char>(line[j]))) ++j;
        if (j == i) { err = "Missing word on header line " + std::to_string(lineno); return false; }
        std::string word = line.substr(i, j - i);

        while (j < line.size() && std::isspace(static_cast<unsigned char>(line[j]))) ++j;
        if (j >= line.size()) { err = "Missing code on header line " + std::to_string(lineno); return false; }

        size_t k = j;
        while (k < line.size() && (line[k] == '0' || line[k] == '1')) ++k;
        std::string code = line.substr(j, k - j);
        if (code.empty()) { err = "Empty code on header line " + std::to_string(lineno); return false; }

        // allow trailing spaces only
        while (k < line.size()) {
            if (!std::isspace(static_cast<unsigned char>(line[k]))) {
                err = "Garbage after code on header line " + std::to_string(lineno);
                return false;
            }
            ++k;
        }

        pairs.emplace_back(std::move(word), std::move(code));
    }

    if (pairs.empty()) { err = "Header is empty."; return false; }
    return true;
}

static bool checkCodeLooksBinary(const std::string& codePath, std::string& err) {
    std::ifstream in(codePath);
    if (!in) { err = "Cannot open code file: " + codePath; return false; }
    int ch;
    while ((ch = in.get()) != EOF) {
        char c = static_cast<char>(ch);
        if (c == '\n' || c == '\r' || std::isspace(static_cast<unsigned char>(c))) continue;
        if (c != '0' && c != '1') {
            err = std::string("First non-whitespace character in code is not 0/1: '") + c + "'";
            return false;
        }
        return true;
    }
    err = "Code file appears empty.";
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage:\n  " << argv[0] << " input_output/<name>.hdr input_output/<name>.code\n";
        return 1;
    }

    const std::string hdrPath  = argv[1];
    const std::string codePath = argv[2];

    // Build output name in the SAME directory as the .code file,
    // with the BASE of the .hdr (e.g., TheBells.tokens_decoded)
    std::string codeDir, codeBase, codeExt;
    splitPath(codePath, codeDir, codeBase, codeExt);

    std::string hdrDir, hdrBase, hdrExt;
    splitPath(hdrPath, hdrDir, hdrBase, hdrExt);

    const std::string outPath = codeDir + hdrBase + ".tokens_decoded";

    // Read header
    std::vector<std::pair<std::string,std::string>> pairs;
    std::string err;
    if (!readHeader(hdrPath, pairs, err)) {
        std::cerr << "Header error: " << err << "\n";
        return 2;
    }

    // Sanity check for code
    if (!checkCodeLooksBinary(codePath, err)) {
        std::cerr << "Code file error: " << err << "\n";
        return 3;
    }

    // Build tree
    HuffmanTree tree;
    if (!tree.buildFromHeader(pairs, err)) {
        std::cerr << "Build error: " << err << "\n";
        return 4;
    }

    // Open I/O
    std::ifstream codeIn(codePath);
    if (!codeIn) {
        std::cerr << "Cannot open code file for reading: " << codePath << "\n";
        return 5;
    }
    std::ofstream out(outPath);
    if (!out) {
        std::cerr << "Cannot create output file: " << outPath << "\n";
        return 6;
    }

    if (!tree.decode(codeIn, out, err)) {
        std::cerr << "Decode error: " << err << "\n";
        return 7;
    }

    std::cerr << "Decoded -> " << outPath << "\n";
    return 0;
}
