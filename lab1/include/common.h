#pragma once
// Общие определения: команды, разбор запроса, вспомогательные функции.
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using Args = std::istringstream;

// К какой структуре относится команда (по первой букве имени команды).
enum class Family { Array, SList, DList, Stack, Queue, Tree, Print };

enum class Cmd
{
    // массив
    MPUSH, MINSERT, MGET, MSET, MDEL, MLEN, MPRINT,
    // односвязный список
    FPUSHH, FPUSHT, FPUSHA, FPUSHB, FDELH, FDELT, FDELA, FDELB, FDEL,
    FFIND, FGET, FPRINT, FPRINTR,
    // двусвязный список
    LPUSHH, LPUSHT, LPUSHA, LPUSHB, LDELH, LDELT, LDELA, LDELB, LDEL,
    LFIND, LGET, LPRINT, LPRINTR,
    // стек
    SPUSH, SPOP, SPEEK, SPRINT,
    // очередь
    QPUSH, QPOP, QPEEK, QPRINT,
    // дерево (АВЛ)
    TINSERT, TDEL, TFIND, ISMEMBER, TGET, TPRINT,
    // вывод любой структуры
    PRINT
};

struct CmdInfo
{
    Cmd cmd;
    Family family;
};

// false, если такой команды нет.
bool lookupCommand(const std::string& text, CmdInfo& out);

// Ошибка пользователя (неверный индекс, пустой стек и т.п.) — ловится в main.
struct DbError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

// Чтение аргументов команды.
std::string readWord(Args& args, const char* what);
long readInt(Args& args, const char* what);

// Находит структуру name типа type и возвращает указатель на её поле "data".
// Если структуры нет: create == true создаёт пустую, иначе DbError.
// Если тип не совпал — DbError.
json* openData(json& db, const std::string& name, const char* type, bool create);
