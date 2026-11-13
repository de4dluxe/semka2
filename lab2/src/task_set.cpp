// Задание 2. АТД Множество: SETADD, SETDEL, SET_AT, SETPRINT.
#include "handlers.h"
#include "set.h"

bool handleSet(const std::string& cmd, json& db, const std::string& name, Args& args) {
    json* data = openData(db, name, "set", cmd == "SETADD");

    Set set;
    for (auto& e : *data) {
        set.setAdd(e.is_string() ? e.get<std::string>() : e.dump());
    }

    if (cmd == "SETPRINT") {
        DynArray all;
        set.getAll(all);
        std::cout << "{";
        for (int i = 0; i < (int)all.size(); i++) {
            std::cout << (i > 0 ? ", " : "") << all.get(i);
        }
        std::cout << "} (размер=" << set.count() << ")\n";
        return false;
    }

    std::string x = readWord(args, "элемент");
    if (cmd == "SET_AT") {
        std::cout << "-> " << (set.setAt(x) ? "true" : "false") << "\n";
        return false;
    }
    if (cmd == "SETADD") {
        if (set.setAt(x)) {
            std::cout << "-> " << x << " (уже есть)\n";
            return false;
        }
        set.setAdd(x);
    } else { // SETDEL
        if (!set.setDel(x)) {
            throw DbError("элемента " + x + " нет в множестве " + name);
        }
    }
    std::cout << "-> " << x << "\n";

    DynArray all;
    set.getAll(all);
    json arr = json::array();
    for (int i = 0; i < (int)all.size(); i++) {
        arr.push_back(all.get(i));
    }
    *data = arr;
    return true;
}
