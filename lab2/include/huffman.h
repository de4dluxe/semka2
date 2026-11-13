#pragma once
#include <string>

struct HuffNode {
    char symbol;
    int weight;
    HuffNode* left;
    HuffNode* right;
};

class HuffTree {
public:
    HuffTree();
    ~HuffTree();
    HuffTree(const HuffTree&) = delete;
    HuffTree& operator=(const HuffTree&) = delete;

    void build(const std::string& text);                 // дерево по тексту (по частоте символов)
    void addCode(char symbol, const std::string& code);  // дерево по готовой таблице кодов
    std::string encode(const std::string& text);
    bool decode(const std::string& bits, std::string& result);  // false, если код неверный
    std::string getCode(char symbol);

private:
    std::string codes_[256];  // код каждого символа из 0 и 1
    HuffNode* root_;
    HuffNode* takeMin(HuffNode* nodes[], int& count);
    void buildCodes(HuffNode* node, std::string code);
    void freeTree(HuffNode* node);
};
