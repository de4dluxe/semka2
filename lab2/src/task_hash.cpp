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

static double elapsed(std::chrono::high_resolution_clock::time_point start) {
    std::chrono::duration<double, std::milli> d = std::chrono::high_resolution_clock::now() - start;
    return d.count();
}

void compareHash() {
    int sizes[] = {1000, 10000, 50000, 200000};
    srand((unsigned)time(nullptr));

    std::cout << "Элементов | Цепочки: вставка, поиск (мс) | Кукушка: вставка, поиск (мс)\n";
    for (int n : sizes) {
        std::string* keys = new std::string[n];
        for (int i = 0; i < n; i++) {
            keys[i] = std::to_string(rand()) + "_" + std::to_string(i);  // случайные разные ключи
        }

        HashTable chain;
        Cuckoo cuckoo;

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < n; i++) chain.add(keys[i], i);
        double chainInsert = elapsed(start);

        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < n; i++) chain.search(keys[i]);
        double chainSearch = elapsed(start);

        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < n; i++) cuckoo.add(keys[i], i);
        double cuckooInsert = elapsed(start);

        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < n; i++) cuckoo.search(keys[i]);
        double cuckooSearch = elapsed(start);

        std::cout << n << " | " << chainInsert << ", " << chainSearch << " | " << cuckooInsert << ", "
                  << cuckooSearch << "\n";
        delete[] keys;
    }
}
