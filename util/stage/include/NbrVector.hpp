#pragma once

#include <vector>
#include <iostream>
#include <algorithm>

template <typename T>
class NbrVector : public std::vector<T>
{
    public:
        void Fill(const T size);
        void DisplayMultiple(const T multiple);
        void RemoveMultiple(const T multiple);
        void RemoveMultipleAndNext(const T multiple);
        void DisplayNumbersBetweenExclusive(const T min, const T max); // min and max are both exclusive
};

#include "../templates/NbrVector.tpp"
