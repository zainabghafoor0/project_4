#include "HuffmanTree.hpp"
#include <cctype>
#include <string>

HuffmanTree::HuffmanTree() : root_(new Node{}) {}
HuffmanTree::~HuffmanTree() { free_(root_); }

void HuffmanTree::free_(Node* n) {
    if (!n) return;
    free_(n->left);
    free_(n->right);
    delete n;
}

bool HuffmanTree::buildFromHeader(
    const std::vector<std::pair<std::string,std::string>>& pairs,
    std::string& err
) {
    // Start fresh each time
    free_(root_);
    root_ = new Node{};

    for (const auto& kv : pairs) {
        const std::string& word = kv.first;
        const std::string& code = kv.second;

        if (word.empty()) { err = "Header contains an empty word."; return false; }
        if (code.empty()) { err = "Empty code for word: " + word; return false; }

        Node* cur = root_;
        for (size_t i = 0; i < code.size(); ++i) {
            char b = code[i];
            if (b != '0' && b != '1') {
                err = "Invalid bit '" + std::string(1,b) + "' in code for word: " + word;
                return false;
            }
            bool last = (i + 1 == code.size());
            if (b == '0') {
                if (!cur->left) cur->left = new Node{};
                cur = cur->left;
            } else {
                if (!cur->right) cur->right = new Node{};
                cur = cur->right;
            }
            if (last) {
                // Assign leaf
                if (!cur->word.empty()) {
                    err = "Duplicate code: multiple words share the same code (at '" + word + "').";
                    return false;
                }
                if (cur->left || cur->right) {
                    err = "Prefix conflict: code for '" + word + "' is a prefix of another code.";
                    return false;
                }
                cur->word = word;
            }
        }
    }
    return true;
}

bool HuffmanTree::decode(std::istream& in, std::ostream& out, std::string& err) const {
    if (!root_) { err = "Decoder tree not initialized."; return false; }

    const Node* cur = root_;
    char c;
    while (in.get(c)) {
        // Ignore newlines and carriage returns; ignore other whitespace unless it's 0/1
        if (c == '\n' || c == '\r') continue;
        if (std::isspace(static_cast<unsigned char>(c)) && c!='0' && c!='1') continue;

        if (c != '0' && c != '1') {
            err = std::string("Encountered non-binary character in .code: '") + c + "'";
            return false;
        }

        cur = (c == '0') ? cur->left : cur->right;
        if (!cur) {
            err = "Traversal hit a null branch (code stream inconsistent with header).";
            return false;
        }
        if (isLeaf(cur)) {
            if (cur->word.empty()) {
                err = "Leaf without word (corrupt tree).";
                return false;
            }
            out << cur->word << '\n';  // one token per line
            cur = root_;               // reset for next token
        }
    }

    // If we end not at root, stream ended mid-symbol
    if (cur != root_) {
        err = "Code stream ended mid-symbol (stopped at an internal node).";
        return false;
    }
    return true;
}
