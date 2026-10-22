#include <fstream>
#include "array.h"
#include "dlist.h"
#include "queue.h"
#include "slist.h"
#include "stack.h"
#include "tree.h"

// PRINT <имя>: определяем тип по файлу и вызываем нужную команду печати.
static bool handlePrint(json& db, const std::string& name, Args& args)
{
    if (!db.contains(name)) throw DbError("нет структуры " + name);
    std::string type = db[name].value("type", "");
    if (type == "array") return handleArray(Cmd::MPRINT, db, name, args);
    if (type == "flist") return handleSList(Cmd::FPRINT, db, name, args);
    if (type == "dlist") return handleDList(Cmd::LPRINT, db, name, args);
    if (type == "stack") return handleStack(Cmd::SPRINT, db, name, args);
    if (type == "queue") return handleQueue(Cmd::QPRINT, db, name, args);
    if (type == "tree") return handleTree(Cmd::TPRINT, db, name, args);
    throw DbError("неизвестный тип структуры: " + type);
}

static bool run(const CmdInfo& info, json& db, const std::string& name, Args& args)
{
    switch (info.family)
    {
        case Family::Array: return handleArray(info.cmd, db, name, args);
        case Family::SList: return handleSList(info.cmd, db, name, args);
        case Family::DList: return handleDList(info.cmd, db, name, args);
        case Family::Stack: return handleStack(info.cmd, db, name, args);
        case Family::Queue: return handleQueue(info.cmd, db, name, args);
        case Family::Tree: return handleTree(info.cmd, db, name, args);
        case Family::Print: return handlePrint(db, name, args);
    }
    return false;
}

static void saveDb(const json& db, const std::string& fileName)
{
    std::ofstream out(fileName);
    if (!out) throw DbError("не удалось записать файл " + fileName);
    out << db.dump(4) << "\n";
}

// Выполняет одну команду; если данные изменились - сразу пишет файл.
static void execute(json& db, const std::string& fileName, const std::string& query)
{
    Args args(query);
    std::string word;
    args >> word;
    CmdInfo info;
    if (!lookupCommand(word, info)) throw DbError("неизвестная команда " + word);
    std::string name = readWord(args, "имя структуры");
    if (run(info, db, name, args)) saveDb(db, fileName);
}

static void printHelp()
{
    std::cout << "Формат: КОМАНДА имя [аргументы], например SPUSH mystack 5\n"
                 "  M: MPUSH MINSERT MGET MSET MDEL MLEN MPRINT\n"
                 "  F/L (списки): PUSHH PUSHT PUSHA PUSHB DELH DELT DELA DELB DEL FIND GET PRINT PRINTR (с буквой F или L)\n"
                 "  S: SPUSH SPOP SPEEK SPRINT    Q: QPUSH QPOP QPEEK QPRINT\n"
                 "  T: TINSERT TDEL TFIND ISMEMBER TGET TPRINT    PRINT имя\n"
                 "  help - эта справка, exit - выход\n";
}

int main(int argc, char* argv[])
{
    std::string fileName, query;
    for (int i = 1; i + 1 < argc; i += 2)
    {
        std::string key = argv[i];
        if (key == "--file") fileName = argv[i + 1];
        else if (key == "--query") query = argv[i + 1];
    }
    if (fileName.empty())
    {
        std::cout << "Использование: ./dbms --file <файл> [--query '<команда>']\n"
                     "Без --query запускается ввод команд с клавиатуры.\n";
        return 1;
    }

    json db = json::object();
    try
    {
        std::ifstream in(fileName);
        if (in.is_open() && in.peek() != std::ifstream::traits_type::eof())
        {
            db = json::parse(in);
            if (!db.is_object()) throw DbError("файл " + fileName + " имеет неверный формат");
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "ошибка: не удалось прочитать " << fileName << ": " << e.what() << "\n";
        return 1;
    }

    // Один запрос из командной строки
    if (!query.empty())
    {
        try
        {
            execute(db, fileName, query);
        }
        catch (const std::exception& e)
        {
            std::cout << "ошибка: " << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    // Ввод с клавиатуры: команды по одной в строке, пока не exit / Ctrl+D
    std::cout << "Команды вводятся с клавиатуры (help - справка, exit - выход)\n";
    std::string line;
    while (std::cout << "> " << std::flush, std::getline(std::cin, line))
    {
        if (line.find_first_not_of(" \t\r") == std::string::npos) continue;
        if (line == "exit" || line == "quit") break;
        if (line == "help")
        {
            printHelp();
            continue;
        }
        try
        {
            execute(db, fileName, line);
        }
        catch (const std::exception& e)
        {
            std::cout << "ошибка: " << e.what() << "\n";   // ошибка не прерывает сеанс
        }
    }
    std::cout << "\n";
    return 0;
}
