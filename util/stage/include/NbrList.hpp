#include <list>

using namespace std;

template <typename T>
class NbrList : public list<T>
{
    public:
        void Fill(T size);
        void DisplayMultiple11(void);
        void RemoveMultipleAndNext(const T multiple);
        void DisplayNumbersBetween(const T min, const T max); // min and max are both exclusive
        void DisplayNumbersBetweenRemovingMultipleAndNext(const T min, const T max, const T multiple);
};
