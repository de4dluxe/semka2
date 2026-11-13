//   LFUINIT имя ёмкость, LFUSET имя ключ значение, LFUGET имя ключ, LFUPRINT имя
// В файле: {"cap": 2, "items": [[ключ, значение, сколько раз обращались, когда в последний раз], ...]}
#include "handlers.h"
#include "lfu.h"

bool handleLFU(const std::string& cmd, json& db, const std::string& name, Args& args) {
    if (cmd == "LFUINIT") {
        int cap = (int)readInt(args, "ёмкость");
        if (cap < 0) {
            throw DbError("ёмкость не может быть отрицательной");
        }
        *openObject(db, name, "lfu", true) = {{"cap", cap}, {"items", json::array()}};
        std::cout << "-> ёмкость " << cap << "\n";
        return true;
    }

    json* data = openObject(db, name, "lfu", false);
    if (!data->contains("cap")) {
        throw DbError("кэш " + name + " не создан, сначала LFUINIT");
    }

    int cap = (*data)["cap"].get<int>();
    LFUCache cache(cap);
    for (auto& e : (*data)["items"]) {
        cache.restore(e[0].get<int>(), e[1].get<int>(), e[2].get<int>(), e[3].get<int>());
    }

    if (cmd == "LFUPRINT") {
        for (auto& e : (*data)["items"]) {
            std::cout << e[0] << ":" << e[1] << " (обращений: " << e[2] << ")  ";
        }
        std::cout << "[" << cache.size() << "/" << cap << "]\n";
        return false;
    }

    int key = (int)readInt(args, "ключ");
    if (cmd == "LFUGET") {
        std::cout << "-> " << cache.get(key) << "\n";
    } else { // LFUSET
        int value = (int)readInt(args, "значение");
        cache.set(key, value);
        std::cout << "-> ok\n";
    }

    json items = json::array();
    for (int i = 0; i < cache.size(); i++) {
        int k, v, f, t;
        cache.getItem(i, k, v, f, t);
        items.push_back(json::array({k, v, f, t}));
    }
    *data = {{"cap", cap}, {"items", items}};
    return true;
}
