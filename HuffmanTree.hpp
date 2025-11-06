#pragma once
#include <string>
#include <vector>
#include <istream>
#include <ostream>

class HuffmanTree {
public:
    HuffmanTree();
    ~HuffmanTree();

    // Build from (word, code) pairs read from .hdr
    bool buildFromHeader(const std::vector<std::pair<std::string,std::string>>& pairs,
                         std::string& err);

    // Stream-decode .code -> out, writing one token per line
    bool decode(std::istream& in, std::ostream& out, std::string& err) const;

private:
    struct Node {
        std::string word;   // non-empty iff leaf
        Node* left{nullptr};
        Node* right{nullptr};
    };

    Node* root_{nullptr};

    void free_(Node* n);
    static bool isLeaf(const Node* n) { return n && !n->left && !n->right; }
};
