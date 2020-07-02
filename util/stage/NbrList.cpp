#include <NbrList.hpp>
#include <algorithm>
#include <iostream>

#define ERASE_MULTIPLE_AND_NEXT(Nblist, Iterator, End, Multiple)    if (*Iterator % Multiple == 0) { \
    Iterator = Nblist->erase(Iterator); \
    if (Iterator != End) /* erase next if exist */ \
        Iterator = Nblist->erase(Iterator); \
    continue; \
}

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

template<class T> void NbrList<T>::DisplayNumbersBetweenRemovingMultipleAndNext(const T min, const T max, const T multiple)
{
  typename NbrList::iterator it;
  const typename NbrList::iterator end = this->end();

  for (it = this->begin(); it != end && *it <= min;) {
      // skip numbers <= 50, and remove multiple of 12 and the number after
      ERASE_MULTIPLE_AND_NEXT(this, it, end, multiple);
      it++;
  }
  // Display numbers >= 50
  while (it != end && *it < max) {
      ERASE_MULTIPLE_AND_NEXT(this, it, end, multiple);
      std::cout << *it << "\n";
      it++;
  }
  while (it != end) {
      ERASE_MULTIPLE_AND_NEXT(this, it, end, multiple);
      it++;
  }
}

template<class T> void NbrList<T>::RemoveMultipleAndNext(const T multiple)
{
  typename NbrList::iterator it;
  const typename NbrList::iterator end = this->end();

  for (it = this->begin(); it != end;) {
      // remove multiple of 12 and the number after
      ERASE_MULTIPLE_AND_NEXT(this, it, end, multiple);
      it++;
  }
}

template<class T> void NbrList<T>::DisplayNumbersBetween(const T min, const T max)
{
  typename NbrList::iterator it;
  const typename NbrList::iterator end = this->end();

  // skip numbers <= 50
  for (it = this->begin(); it != end && *it <= min; it++);

  // display numbers < 70
  while (it != end && *it < max)
      std::cout << *(it++) << "\n";
}

template class NbrList<char>;
template class NbrList<short>;
template class NbrList<int>;
template class NbrList<long>;
