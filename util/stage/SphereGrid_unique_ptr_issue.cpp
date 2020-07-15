#include <memory>
#include <list>
#include <vector>

int main() {
    typedef std::list<std::unique_ptr<int>> dataType_t;
    std::vector<std::pair<dataType_t, bool>> me;
    std::vector<std::pair<dataType_t, bool>*> mine;

    for (int i = 0; i < 2; i++) {
        me.push_back(std::pair<dataType_t, bool>({}, false));
        mine.push_back(&(*me.rbegin()));
    }

    auto e = std::make_unique<int>(5);
    me[0].first.push_back(std::move(e));

    return (0);
}
