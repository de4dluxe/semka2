#pragma once
#include "common.h"

// Возвращают true, если данные изменились и файл надо записать.
bool handleSet(const std::string& cmd, json& db, const std::string& name, Args& args);        // задание 2
bool handleAsteroids(const std::string& cmd, json& db, const std::string& name, Args& args);  // задание 1
bool handleTurtles(const std::string& cmd, json& db, const std::string& name, Args& args);    // задание 3
bool handleText(const std::string& cmd, json& db, const std::string& name, Args& args);       // задания 4 и 5
bool handleChain(const std::string& cmd, json& db, const std::string& name, Args& args);      // задание 6.1
bool handleCuckoo(const std::string& cmd, json& db, const std::string& name, Args& args);     // задание 6.1
bool handleGenealogy(const std::string& cmd, json& db, const std::string& name, Args& args);  // задание 6.2
bool handleLFU(const std::string& cmd, json& db, const std::string& name, Args& args);        // задание 7
void compareHash();

inline json* openObject(json& db, const std::string& name, const char* type, bool create) {
    json* data = openData(db, name, type, create);
    if (data->is_array() && data->empty()) {
        *data = json::object();
    }
    return data;
}

inline std::string readRest(Args& args, const char* what) {
    std::string rest;
    std::getline(args >> std::ws, rest);
    if (rest.empty()) {
        throw DbError(std::string("не хватает аргумента: ") + what);
    }
    return rest;
}
