#include "common.h"
#include <map>

bool lookupCommand(const std::string& text, CmdInfo& out)
{
    static const std::map<std::string, CmdInfo> table = {
        {"MPUSH", {Cmd::MPUSH, Family::Array}},   {"MINSERT", {Cmd::MINSERT, Family::Array}},
        {"MGET", {Cmd::MGET, Family::Array}},     {"MSET", {Cmd::MSET, Family::Array}},
        {"MDEL", {Cmd::MDEL, Family::Array}},     {"MLEN", {Cmd::MLEN, Family::Array}},
        {"MPRINT", {Cmd::MPRINT, Family::Array}},

        {"FPUSHH", {Cmd::FPUSHH, Family::SList}}, {"FPUSHT", {Cmd::FPUSHT, Family::SList}},
        {"FPUSHA", {Cmd::FPUSHA, Family::SList}}, {"FPUSHB", {Cmd::FPUSHB, Family::SList}},
        {"FDELH", {Cmd::FDELH, Family::SList}},   {"FDELT", {Cmd::FDELT, Family::SList}},
        {"FDELA", {Cmd::FDELA, Family::SList}},   {"FDELB", {Cmd::FDELB, Family::SList}},
        {"FDEL", {Cmd::FDEL, Family::SList}},     {"FFIND", {Cmd::FFIND, Family::SList}},
        {"FGET", {Cmd::FGET, Family::SList}},     {"FPRINT", {Cmd::FPRINT, Family::SList}},
        {"FPRINTR", {Cmd::FPRINTR, Family::SList}},

        {"LPUSHH", {Cmd::LPUSHH, Family::DList}}, {"LPUSHT", {Cmd::LPUSHT, Family::DList}},
        {"LPUSHA", {Cmd::LPUSHA, Family::DList}}, {"LPUSHB", {Cmd::LPUSHB, Family::DList}},
        {"LDELH", {Cmd::LDELH, Family::DList}},   {"LDELT", {Cmd::LDELT, Family::DList}},
        {"LDELA", {Cmd::LDELA, Family::DList}},   {"LDELB", {Cmd::LDELB, Family::DList}},
        {"LDEL", {Cmd::LDEL, Family::DList}},     {"LFIND", {Cmd::LFIND, Family::DList}},
        {"LGET", {Cmd::LGET, Family::DList}},     {"LPRINT", {Cmd::LPRINT, Family::DList}},
        {"LPRINTR", {Cmd::LPRINTR, Family::DList}},

        {"SPUSH", {Cmd::SPUSH, Family::Stack}},   {"SPOP", {Cmd::SPOP, Family::Stack}},
        {"SPEEK", {Cmd::SPEEK, Family::Stack}},   {"SPRINT", {Cmd::SPRINT, Family::Stack}},

        {"QPUSH", {Cmd::QPUSH, Family::Queue}},   {"QPOP", {Cmd::QPOP, Family::Queue}},
        {"QPEEK", {Cmd::QPEEK, Family::Queue}},   {"QPRINT", {Cmd::QPRINT, Family::Queue}},

        {"TINSERT", {Cmd::TINSERT, Family::Tree}}, {"TDEL", {Cmd::TDEL, Family::Tree}},
        {"TFIND", {Cmd::TFIND, Family::Tree}},     {"ISMEMBER", {Cmd::ISMEMBER, Family::Tree}},
        {"TGET", {Cmd::TGET, Family::Tree}},       {"TPRINT", {Cmd::TPRINT, Family::Tree}},

        {"PRINT", {Cmd::PRINT, Family::Print}},
    };
    auto it = table.find(text);
    if (it == table.end()) return false;
    out = it->second;
    return true;
}

std::string readWord(Args& args, const char* what)
{
    std::string w;
    if (!(args >> w)) throw DbError(std::string("не хватает аргумента: ") + what);
    return w;
}

long readInt(Args& args, const char* what)
{
    std::string w = readWord(args, what);
    try
    {
        size_t pos = 0;
        long v = std::stol(w, &pos);
        if (pos != w.size()) throw std::invalid_argument("");
        return v;
    }
    catch (const std::exception&)
    {
        throw DbError(std::string(what) + " должен быть целым числом, а не '" + w + "'");
    }
}

json* openData(json& db, const std::string& name, const char* type, bool create)
{
    if (!db.contains(name))
    {
        if (!create) throw DbError("нет структуры " + name);
        db[name] = {{"type", type}, {"data", std::string(type) == "tree" ? json(nullptr) : json::array()}};
    }
    else if (db[name].value("type", "") != type)
    {
        throw DbError(name + " это не " + type);
    }
    return &db[name]["data"];
}
