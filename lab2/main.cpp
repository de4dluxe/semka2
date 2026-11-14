#include <fstream>
#include <map>
#include "array.h"
#include "dlist.h"
#include "handlers.h"
#include "queue.h"
#include "slist.h"
#include "stack.h"
#include "tree.h"

// Какому заданию лабы 2 принадлежит команда.
enum class Task { Set, Asteroids, Turtles, Text, Chain, Cuckoo, Genealogy, LFU };

static const std::map<std::string, Task> TASKS = {
    {"SETADD", Task::Set}, {"SETDEL", Task::Set}, {"SET_AT", Task::Set}, {"SETPRINT", Task::Set},
    {"AADD", Task::Asteroids}, {"ARUN", Task::Asteroids}, {"APRINT", Task::Asteroids},
    {"TURADD", Task::Turtles}, {"TURRUN", Task::Turtles}, {"TURPRINT", Task::Turtles},
    {"HTEXT", Task::Text}, {"HIST", Task::Text}, {"HUFFENC", Task::Text}, {"HUFFDEC", Task::Text}, {"HUFFPRINT", Task::Text},
    {"CHSET", Task::Chain}, {"CHGET", Task::Chain}, {"CHDEL", Task::Chain}, {"CHPRINT", Task::Chain},
    {"CKSET", Task::Cuckoo}, {"CKGET", Task::Cuckoo}, {"CKDEL", Task::Cuckoo}, {"CKPRINT", Task::Cuckoo},
    {"GADD", Task::Genealogy}, {"GRUN", Task::Genealogy}, {"GPRINT", Task::Genealogy},
    {"LFUINIT", Task::LFU}, {"LFUSET", Task::LFU}, {"LFUGET", Task::LFU}, {"LFUPRINT", Task::LFU},
};

// Какой командой печатается структура данного типа (для PRINT <имя>).
static const std::map<std::string, std::string> PRINT_COMMAND = {
    {"set", "SETPRINT"}, {"asteroids", "APRINT"}, {"turtles", "TURPRINT"}, {"huffman", "HUFFPRINT"},
    {"hashchain", "CHPRINT"}, {"hashcuckoo", "CKPRINT"}, {"genealogy", "GPRINT"}, {"lfu", "LFUPRINT"},
};

// Передаёт команду обработчику её задания.
static bool runTask(Task task, const std::string& word, json& db, const std::string& name, Args& args) {
    switch (task) {
        case Task::Set: return handleSet(word, db, name, args);
        case Task::Asteroids: return handleAsteroids(word, db, name, args);
        case Task::Turtles: return handleTurtles(word, db, name, args);
        case Task::Text: return handleText(word, db, name, args);
        case Task::Chain: return handleChain(word, db, name, args);
        case Task::Cuckoo: return handleCuckoo(word, db, name, args);
        case Task::Genealogy: return handleGenealogy(word, db, name, args);
        case Task::LFU: return handleLFU(word, db, name, args);
    }
    return false;
}

static bool printLab1(json& db, const std::string& name, Args& args) {
    std::string type = db[name].value("type", "");
    if (type == "array") return handleArray(Cmd::MPRINT, db, name, args);
    if (type == "flist") return handleSList(Cmd::FPRINT, db, name, args);
    if (type == "dlist") return handleDList(Cmd::LPRINT, db, name, args);
    if (type == "stack") return handleStack(Cmd::SPRINT, db, name, args);
    if (type == "queue") return handleQueue(Cmd::QPRINT, db, name, args);
    if (type == "tree") return handleTree(Cmd::TPRINT, db, name, args);
    throw DbError("неизвестный тип структуры: " + type);
}

static bool runLab1(const std::string& word, json& db, Args& args) {
    CmdInfo info;
    if (!lookupCommand(word, info)) {
        throw DbError("неизвестная команда " + word);
    }
    std::string name = readWord(args, "имя структуры");
    switch (info.family) {
        case Family::Array: return handleArray(info.cmd, db, name, args);
        case Family::SList: return handleSList(info.cmd, db, name, args);
        case Family::DList: return handleDList(info.cmd, db, name, args);
        case Family::Stack: return handleStack(info.cmd, db, name, args);
        case Family::Queue: return handleQueue(info.cmd, db, name, args);
        case Family::Tree: return handleTree(info.cmd, db, name, args);
        case Family::Print: return printLab1(db, name, args);
    }
    return false;
}

static void saveDb(const json& db, const std::string& fileName) {
    std::ofstream out(fileName);
    if (!out) {
        throw DbError("не удалось записать файл " + fileName);
    }
    out << db.dump(4) << "\n";
}

// Выполняет одну команду. Если данные изменились, сразу записывает файл.
static void execute(json& db, const std::string& fileName, const std::string& query) {
    Args args(query);
    std::string word;
    args >> word;
    bool changed = false;

    if (word == "PRINT") {
        std::string name = readWord(args, "имя структуры");
        if (!db.contains(name)) {
            throw DbError("нет структуры " + name);
        }
        std::string type = db[name].value("type", "");
        auto printCmd = PRINT_COMMAND.find(type);
        if (type == "text") {
            std::cout << db[name]["data"].get<std::string>() << "\n";
        } else if (printCmd != PRINT_COMMAND.end()) {
            changed = runTask(TASKS.at(printCmd->second), printCmd->second, db, name, args);
        } else {
            changed = printLab1(db, name, args);
        }
    } else if (TASKS.count(word) > 0) {
        std::string name = readWord(args, "имя структуры");
        changed = runTask(TASKS.at(word), word, db, name, args);
    } else {
        changed = runLab1(word, db, args);
    }

    if (changed) {
        saveDb(db, fileName);
    }
}

static void printHelp() {
    std::cout << "Формат: КОМАНДА имя [аргументы], например SETADD s 5\n"
                 "Лаба 2:\n"
                 "  множество (2):     SETADD SETDEL SET_AT SETPRINT   имя x\n"
                 "  астероиды (1):     AADD имя размер R|L, ARUN имя, APRINT имя\n"
                 "  черепахи (3):      TURADD имя a b, TURRUN имя, TURPRINT имя\n"
                 "  гистограмма (4):   HTEXT имя текст, HIST имя\n"
                 "  Хаффман (5):       HUFFENC имя текст, HUFFDEC имя, HUFFPRINT имя\n"
                 "  хеш (6.1):         CHSET/CKSET имя ключ число, CHGET/CKGET имя ключ,\n"
                 "                     CHDEL/CKDEL имя ключ, CHPRINT/CKPRINT имя (CH - цепочки, CK - кукушка)\n"
                 "  родословная (6.2): GADD имя потомок родитель, GRUN имя, GPRINT имя\n"
                 "  LFU-кэш (7):       LFUINIT имя ёмкость, LFUSET имя ключ значение, LFUGET имя ключ, LFUPRINT имя\n"
                 "Лаба 1: команды структур (MPUSH, SPUSH, QPUSH, TINSERT и другие), PRINT имя\n"
                 "help - эта справка, exit - выход\n";
}

int main(int argc, char* argv[]) {
    std::string fileName;
    std::string query;
    for (int i = 1; i + 1 < argc; i += 2) {
        std::string key = argv[i];
        if (key == "--file") fileName = argv[i + 1];
        else if (key == "--query") query = argv[i + 1];
    }
    if (fileName.empty()) {
        std::cout << "Использование: ./lab2 --file <файл> [--query '<команда>']\n"
                     "Без --query команды вводятся с клавиатуры.\n";
        return 1;
    }

    // загружаем базу из файла (если файла нет, база пустая)
    json db = json::object();
    try {
        std::ifstream in(fileName);
        if (in.is_open() && in.peek() != std::ifstream::traits_type::eof()) {
            db = json::parse(in);
            if (!db.is_object()) {
                throw DbError("файл " + fileName + " имеет неверный формат");
            }
        }
    } catch (const std::exception& e) {
        std::cout << "ошибка: не удалось прочитать " << fileName << ": " << e.what() << "\n";
        return 1;
    }

    // один запрос из командной строки
    if (!query.empty()) {
        try {
            execute(db, fileName, query);
        } catch (const std::exception& e) {
            std::cout << "ошибка: " << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    // ввод с клавиатуры: по одной команде в строке, пока не exit
    std::cout << "Введите команду (help - справка, exit - выход)\n";
    std::string line;
    while (std::cout << "> " << std::flush, std::getline(std::cin, line)) {
        if (line.find_first_not_of(" \t\r") == std::string::npos) continue;
        if (line == "exit") break;
        if (line == "help") {
            printHelp();
            continue;
        }
        try {
            execute(db, fileName, line);
        } catch (const std::exception& e) {
            std::cout << "ошибка: " << e.what() << "\n";  // ошибка не завершает программу
        }
    }
    return 0;
}
