// TURADD имя a b (впереди a черепах, позади b), TURRUN имя, TURPRINT имя.
// Черепаха на позиции p (с нуля) видит впереди p черепах, а позади N-1-p. Значит, говорить правду
// она может только если a + b = N - 1, и тогда её позиция равна a. Две черепахи не могут стоять
// на одном месте, поэтому ответ - сколько разных значений a среди подходящих черепах.
#include "handlers.h"
#include "set.h"

bool handleTurtles(const std::string& cmd, json& db, const std::string& name, Args& args) {
    if (cmd == "TURADD") {
        int a = (int)readInt(args, "a (сколько впереди)");
        int b = (int)readInt(args, "b (сколько позади)");
        if (a < 0 || b < 0) {
            throw DbError("a и b не могут быть отрицательными");
        }
        openData(db, name, "turtles", true)->push_back(json::array({a, b}));
        std::cout << "-> (" << a << ", " << b << ")\n";
        return true;
    }

    json* data = openData(db, name, "turtles", false);
    if (cmd == "TURPRINT") {
        for (auto& e : *data) {
            std::cout << "(" << e[0] << ", " << e[1] << ") ";
        }
        std::cout << "(черепах: " << data->size() << ")\n";
        return false;
    }

    int n = (int)data->size();
    Set positions;
    for (auto& e : *data) {
        int a = e[0].get<int>();
        int b = e[1].get<int>();
        if (a + b == n - 1) {
            positions.setAdd(std::to_string(a));
        }
    }
    std::cout << "-> " << positions.count() << "\n";
    return false;
}
