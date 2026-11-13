//   HTEXT имя текст     запомнить текст          HIST имя   нарисовать гистограмму
//   HUFFENC имя текст   закодировать, сохранить таблицу кодов и код
//   HUFFDEC имя         расшифровать сохранённый код     HUFFPRINT имя   показать таблицу и код
#include <cctype>
#include "array.h"
#include "handlers.h"
#include "huffman.h"

// Различные символы лежат в массиве из лабы 1 по возрастанию кода, пробелы и переводы строк не считаются.
static void printHistogram(const std::string& text) {
    int count[256] = {0};
    DynArray symbols;
    for (char c : text) {
        unsigned char code = (unsigned char)c;
        if (isspace(code)) continue;
        if (count[code] == 0) {
            int pos = 0;
            while (pos < (int)symbols.size() && (unsigned char)symbols.get(pos)[0] < code) {
                pos++;
            }
            symbols.insert(pos, std::string(1, c));
        }
        count[code]++;
    }
    if (symbols.size() == 0) {
        throw DbError("в тексте нет символов кроме пробелов");
    }

    int maxCount = 0;
    for (int i = 0; i < (int)symbols.size(); i++) {
        int c = count[(unsigned char)symbols.get(i)[0]];
        if (c > maxCount) maxCount = c;
    }

    // строки сверху вниз: столбик на этой высоте есть у символов, которых не меньше row
    for (int row = maxCount; row >= 1; row--) {
        std::string line = "";
        for (int i = 0; i < (int)symbols.size(); i++) {
            line += (count[(unsigned char)symbols.get(i)[0]] >= row) ? '#' : ' ';
        }
        line.erase(line.find_last_not_of(' ') + 1);
        std::cout << line << "\n";
    }
    for (int i = 0; i < (int)symbols.size(); i++) {
        std::cout << symbols.get(i);
    }
    std::cout << "\n";
}

static void printTable(const json& huff) {
    std::cout << "Таблица:\n";
    for (int i = 0; i < 256; i++) {
        std::string key = std::to_string(i);
        if (huff["table"].contains(key)) {
            std::cout << "'" << (char)i << "' " << huff["table"][key].get<std::string>() << "\n";
        }
    }
    std::cout << "Код: " << huff["bits"].get<std::string>() << "\n";
}

bool handleText(const std::string& cmd, json& db, const std::string& name, Args& args) {
    if (cmd == "HTEXT") {
        std::string text = readRest(args, "текст");
        *openData(db, name, "text", true) = text;
        std::cout << "-> символов: " << text.size() << "\n";
        return true;
    }
    if (cmd == "HIST") {
        json* data = openData(db, name, "text", false);
        printHistogram(data->is_string() ? data->get<std::string>() : "");
        return false;
    }

    if (cmd == "HUFFENC") {
        std::string text = readRest(args, "текст");
        HuffTree tree;
        tree.build(text);

        // в файл: таблица кодов (ключ - номер символа, чтобы JSON остался корректным) и сам код
        json huff = {{"table", json::object()}, {"bits", tree.encode(text)}};
        for (int i = 0; i < 256; i++) {
            if (tree.getCode((char)i) != "") {
                huff["table"][std::to_string(i)] = tree.getCode((char)i);
            }
        }
        *openObject(db, name, "huffman", true) = huff;
        printTable(huff);
        return true;
    }

    // HUFFDEC и HUFFPRINT
    json* data = openObject(db, name, "huffman", false);
    if (!data->contains("table") || !data->contains("bits")) {
        throw DbError("в " + name + " ничего нет, сначала HUFFENC");
    }
    if (cmd == "HUFFPRINT") {
        printTable(*data);
        return false;
    }

    HuffTree tree;
    for (auto it = (*data)["table"].begin(); it != (*data)["table"].end(); ++it) {
        tree.addCode((char)std::stoi(it.key()), it.value().get<std::string>());
    }
    std::string text;
    if (!tree.decode((*data)["bits"].get<std::string>(), text)) {
        throw DbError("код не подходит к таблице");
    }
    std::cout << "-> " << text << "\n";
    return false;
}
