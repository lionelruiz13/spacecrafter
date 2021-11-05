#ifndef INSERT_ALL_H_
#define INSERT_ALL_H_

#include <vector>

//! insert all Ts ...ts in vector<T>
template <typename T, typename ... Ts>
void insert_all(std::vector<T> &vec, Ts ... ts)
{
    (vec.push_back(ts), ...);
}

#endif
