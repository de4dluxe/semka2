//   CHSET имя ключ число, CHGET имя ключ, CHDEL имя ключ, CHPRINT имя     (цепочки)
//   CKSET имя ключ число, CKGET имя ключ, CKDEL имя ключ, CKPRINT имя     (кукушка)
// В файле таблица хранится объектом {"ключ": число}.
#include "handlers.h"
#include "hash_table.h"

template <typename Table>
static bool runTable(const std::string& op, const char* type, json& db, const std::string& name, Args& args) {
    json* data = openObject(db, name, type, op == "SET");
    Table table;
    for (auto it = data->begin(); it != data->end(); ++it) {
        table.add(it.key(), it.value().get<int>());
    }

    if (op == "PRINT") {
        DynArray keys;
        table.getKeys(keys);
        std::cout << "{";
        for (int i = 0; i < (int)keys.size(); i++) {
            std::cout << (i > 0 ? ", " : "") << keys.get(i) << ": " << *table.get(keys.get(i));
        }
        std::cout << "} (размер=" << table.count() << ")\n";
        return false;
    }

    std::string key = readWord(args, "ключ");
    if (op == "GET") {
        int* value = table.get(key);
        if (value == nullptr) {
            throw DbError("нет ключа " + key);
        }
        std::cout << "-> " << *value << "\n";
        return false;
    }
    if (op == "DEL") {
        if (!table.remove(key)) {
            throw DbError("нет ключа " + key);
        }
        std::cout << "-> " << key << "\n";
    } else { // SET
        int value = (int)readInt(args, "число");
        table.add(key, value);
        std::cout << "-> " << key << " " << value << "\n";
    }

    DynArray keys;
    table.getKeys(keys);
    json obj = json::object();
    for (int i = 0; i < (int)keys.size(); i++) {
        obj[keys.get(i)] = *table.get(keys.get(i));
    }
    *data = obj;
    return true;
}

bool handleChain(const std::string& cmd, json& db, const std::string& name, Args& args) {
    return runTable<HashTable>(cmd.substr(2), "hashchain", db, name, args);
}

bool handleCuckoo(const std::string& cmd, json& db, const std::string& name, Args& args) {
    return runTable<Cuckoo>(cmd.substr(2), "hashcuckoo", db, name, args);
}
