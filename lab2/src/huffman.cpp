#include "huffman.h"

HuffTree::HuffTree() {
    root_ = nullptr;
}

HuffTree::~HuffTree() {
    freeTree(root_);
}

void HuffTree::freeTree(HuffNode* node) {
    if (node == nullptr) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// Достаёт из списка свободных узлов узел с наименьшим весом.
HuffNode* HuffTree::takeMin(HuffNode* nodes[], int& count) {
    int mini = 0;
    for (int i = 1; i < count; i++) {
        if (nodes[i]->weight < nodes[mini]->weight) {
            mini = i;
        }
    }
    HuffNode* result = nodes[mini];
    nodes[mini] = nodes[count - 1];
    count--;
    return result;
}

void HuffTree::build(const std::string& text) {
    freeTree(root_);
    root_ = nullptr;
    for (int i = 0; i < 256; i++) {
        codes_[i] = "";
    }

    int freq[256] = {0};
    for (char c : text) {
        freq[(unsigned char)c]++;
    }

    // сначала у каждого символа свой лист
    HuffNode* nodes[256];
    int count = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            nodes[count] = new HuffNode{(char)i, freq[i], nullptr, nullptr};
            count++;
        }
    }
    if (count == 0) return;

    if (count == 1) {
        // в тексте один символ: ему достаётся код "0"
        root_ = new HuffNode{'\0', nodes[0]->weight, nodes[0], nullptr};
    } else {
        // два самых лёгких узла склеиваем в один, пока не останется один узел
        while (count > 1) {
            HuffNode* first = takeMin(nodes, count);
            HuffNode* second = takeMin(nodes, count);
            nodes[count] = new HuffNode{'\0', first->weight + second->weight, first, second};
            count++;
        }
        root_ = nodes[0];
    }
    buildCodes(root_, "");
}

// Влево идём по 0, вправо по 1. Дошли до листа - запомнили код символа.
void HuffTree::buildCodes(HuffNode* node, std::string code) {
    if (node == nullptr) return;
    if (node->left == nullptr && node->right == nullptr) {
        codes_[(unsigned char)node->symbol] = code;
        return;
    }
    buildCodes(node->left, code + "0");
    buildCodes(node->right, code + "1");
}

void HuffTree::addCode(char symbol, const std::string& code) {
    if (root_ == nullptr) {
        root_ = new HuffNode{'\0', 0, nullptr, nullptr};
    }
    HuffNode* current = root_;
    for (char bit : code) {
        HuffNode** next = (bit == '0') ? &current->left : &current->right;
        if (*next == nullptr) {
            *next = new HuffNode{'\0', 0, nullptr, nullptr};
        }
        current = *next;
    }
    current->symbol = symbol;
    codes_[(unsigned char)symbol] = code;
}

std::string HuffTree::getCode(char symbol) {
    return codes_[(unsigned char)symbol];
}

std::string HuffTree::encode(const std::string& text) {
    std::string result = "";
    for (char c : text) {
        result += codes_[(unsigned char)c];
    }
    return result;
}

// Стоим в корне, читаем бит: 0 - влево, 1 - вправо. Попали в лист - записали символ и вернулись в корень.
bool HuffTree::decode(const std::string& bits, std::string& result) {
    result = "";
    if (root_ == nullptr) return false;
    HuffNode* current = root_;
    for (char bit : bits) {
        if (bit == '0')
            current = current->left;
        else if (bit == '1')
            current = current->right;
        else
            return false;

        if (current == nullptr) return false;
        if (current->left == nullptr && current->right == nullptr) {
            result += current->symbol;
            current = root_;
        }
    }
    return current == root_;  // код не должен обрываться на середине символа
}
