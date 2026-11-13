#include "array.h"
#include "dlist.h"
#include "handlers.h"
#include "hash_table.h"

struct People {
    HashTable ids;             // имя -> номер человека
    DynArray names;            // имена в порядке появления
    DList** children = nullptr;  // children[номер] - дети в порядке ввода
    int* parent = nullptr;       // номер родителя или -1
    int* desc = nullptr;         // число потомков
    int visited = 0;

    void init() {
        int n = names.size();
        children = new DList*[n];
        parent = new int[n];
        desc = new int[n];
        for (int i = 0; i < n; i++) {
            children[i] = new DList();
            parent[i] = -1;
            desc[i] = 0;
        }
    }

    ~People() {
        if (children != nullptr) {
            for (int i = 0; i < (int)names.size(); i++) {
                delete children[i];
            }
        }
        delete[] children;
        delete[] parent;
        delete[] desc;
    }

    void addName(const std::string& name) {
        if (!ids.search(name)) {
            ids.add(name, names.size());
            names.push(name);
        }
    }

    // Потомки человека = дети + потомки каждого из детей.
    int countDesc(const std::string& name) {
        int id = *ids.get(name);
        visited++;
        int total = 0;
        std::vector<std::string> kids = children[id]->items();
        for (int i = 0; i < (int)kids.size(); i++) {
            total += countDesc(kids[i]) + 1;
        }
        desc[id] = total;
        return total;
    }

    void printTree(const std::string& name, int depth) {
        std::cout << std::string(depth * 2, ' ') << name << "\n";
        std::vector<std::string> kids = children[*ids.get(name)]->items();
        for (int i = 0; i < (int)kids.size(); i++) {
            printTree(kids[i], depth + 1);
        }
    }
};

static void sortNames(DynArray& names) {
    for (int i = 1; i < (int)names.size(); i++) {
        std::string key = names.get(i);
        int j = i - 1;
        while (j >= 0 && names.get(j) > key) {
            names.set(j + 1, names.get(j));
            j--;
        }
        names.set(j + 1, key);
    }
}

bool handleGenealogy(const std::string& cmd, json& db, const std::string& name, Args& args) {
    if (cmd == "GADD") {
        std::string child = readWord(args, "потомок");
        std::string parent = readWord(args, "родитель");
        if (child == parent) {
            throw DbError("человек не может быть родителем самому себе");
        }
        openData(db, name, "genealogy", true)->push_back(json::array({child, parent}));
        std::cout << "-> " << child << " " << parent << "\n";
        return true;
    }

    json* data = openData(db, name, "genealogy", false);
    if (cmd == "GPRINT") {
        for (auto& p : *data) {
            std::cout << p[0].get<std::string>() << " " << p[1].get<std::string>() << "\n";
        }
        return false;
    }
    if (data->empty()) {
        throw DbError("в " + name + " нет пар, добавьте их через GADD");
    }

    People family;
    for (auto& p : *data) {
        family.addName(p[0].get<std::string>());
        family.addName(p[1].get<std::string>());
    }
    family.init();
    for (auto& p : *data) {
        std::string child = p[0].get<std::string>();
        std::string parent = p[1].get<std::string>();
        int childId = *family.ids.get(child);
        if (family.parent[childId] != -1) {
            throw DbError("у " + child + " указано два родителя");
        }
        family.parent[childId] = *family.ids.get(parent);
        family.children[*family.ids.get(parent)]->pushTail(child);
    }

    int roots = 0;
    int root = 0;
    for (int i = 0; i < (int)family.names.size(); i++) {
        if (family.parent[i] == -1) {
            root = i;
            roots++;
        }
    }
    if (roots != 1) {
        throw DbError("родоначальник должен быть один, а найдено " + std::to_string(roots));
    }

    family.countDesc(family.names.get(root));
    if (family.visited != (int)family.names.size()) {
        throw DbError("в данных есть цикл");
    }

    DynArray sorted;
    for (int i = 0; i < (int)family.names.size(); i++) {
        sorted.push(family.names.get(i));
    }
    sortNames(sorted);
    for (int i = 0; i < (int)sorted.size(); i++) {
        std::cout << sorted.get(i) << " " << family.desc[*family.ids.get(sorted.get(i))] << "\n";
    }

    std::cout << "\nДерево:\n";
    family.printTree(family.names.get(root), 0);
    return false;
}
