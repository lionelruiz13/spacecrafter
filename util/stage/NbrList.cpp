#include <NbrList.hpp>
#include <algorithm>
#include <iostream>

template<class T> void NbrList<T>::Fill(T size)
{
    for (T i = 0; i < size;)
        // fill list of numbers from 1 to "size" included
        this->push_back(++i);
}

template<class T> void NbrList<T>::DisplayMultiple11(void)
{
    std::for_each(this->begin(), this->end(), [](T value) {
        if ((value % 11) == 0)
            std::cout << value << "\n";
    });
}

template class NbrList<char>;
template class NbrList<short>;
template class NbrList<int>;
template class NbrList<long>;
