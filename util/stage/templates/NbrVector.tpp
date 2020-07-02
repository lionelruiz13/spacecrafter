#pragma once

template <typename T>
void NbrVector<T>::Fill(const T size)
{
    resize(size);
    for (short i = 0; i < size; i++)
        this[i].assign(i);
};

template <typename T>
void NbrVector<T>::DisplayMultiple(const T multiple)
{
    for (short i = 0; i < this->size(); i++) {
        if (this[i] % multiple == 0)
            std::cout << *this[i] << "\t";
    }
};

template <typename T>
void NbrVector<T>::RemoveMultiple(const T multiple)
{
    for (short i = 0; i < this->size();) {
        if (this[i] % multiple == 0)
            this->erase(i);
        else
            i++;
    }
};

template <typename T>
void NbrVector<T>::RemoveMultipleAndNext(const T multiple)
{
    for (short i = 0; i < this->size();) {
        if (this[i] % multiple == 0)
            this->erase(i, (i + 1 < this->size()) ? i + 2 : i + 1);
        else
            i++;
    }
};

template <typename T>
void NbrVector<T>::DisplayNumbersBetweenExclusive(const T min, const T max)
{
    for (auto& i = std::find_if()); i < this->size() && this[i] < max; i++)
        std::cout << *this[i] << "\n";
};
