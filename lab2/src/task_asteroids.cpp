#include <sstream>
#include <vector>
#include "handlers.h"
#include "stack.h"

struct Asteroid {
    int size;
    char dir;  // R - летит вправо, L - влево
};

static std::string toText(Asteroid a) {
    return std::to_string(a.size) + " " + a.dir;
}

static Asteroid fromText(const std::string& s) {
    Asteroid a;
    std::istringstream in(s);
    in >> a.size >> a.dir;
    return a;
}

static void printAsteroids(const std::vector<std::string>& list) {
    std::cout << "[";
    for (int i = 0; i < (int)list.size(); i++) {
        Asteroid a = fromText(list[i]);
        std::cout << (i > 0 ? ", " : "") << "('" << a.size << "', '" << a.dir << "')";
    }
    std::cout << "]\n";
}

// Идём слева направо. Астероид, летящий влево, сталкивается с теми, что лежат на вершине стека
// и летят вправо. Меньший взрывается, больший становится равным разнице размеров, равные взрываются оба.
static std::vector<std::string> collide(const std::vector<std::string>& list) {
    Stack stack;
    for (int i = 0; i < (int)list.size(); i++) {
        Asteroid a = fromText(list[i]);
        bool alive = true;
        std::string top;
        while (alive && a.dir == 'L' && stack.peek(top)) {
            Asteroid t = fromText(top);
            if (t.dir != 'R') break;
            stack.pop(top);
            if (t.size < a.size) {
                a.size -= t.size;  // взорвался тот, что в стеке, летим дальше
            } else if (t.size > a.size) {
                t.size -= a.size;  // взорвался текущий
                stack.push(toText(t));
                alive = false;
            } else {
                alive = false;  // взорвались оба
            }
        }
        if (alive) {
            stack.push(toText(a));
        }
    }

    std::vector<std::string> fromTop = stack.fromTop();
    std::vector<std::string> result;
    for (int i = (int)fromTop.size() - 1; i >= 0; i--) {
        result.push_back(fromTop[i]);
    }
    return result;
}

bool handleAsteroids(const std::string& cmd, json& db, const std::string& name, Args& args) {
    if (cmd == "AADD") {
        int size = (int)readInt(args, "размер");
        std::string dir = readWord(args, "направление (R или L)");
        if (size <= 0) {
            throw DbError("размер должен быть положительным");
        }
        if (dir != "R" && dir != "L") {
            throw DbError("направление должно быть R или L, а не " + dir);
        }
        openData(db, name, "asteroids", true)->push_back(toText({size, dir[0]}));
        std::cout << "-> (" << size << ", " << dir << ")\n";
        return true;
    }

    std::vector<std::string> list;
    for (auto& e : *openData(db, name, "asteroids", false)) {
        list.push_back(e.get<std::string>());
    }
    if (cmd == "ARUN") {
        printAsteroids(collide(list));
    } else {
        printAsteroids(list);
    }
    return false;
}
