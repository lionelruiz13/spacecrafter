#include <list>

using namespace std;

template <typename T>
class NbrList : public list<T>
{
    public:
        void Fill(T size);
        void DisplayMultiple11(void);
};