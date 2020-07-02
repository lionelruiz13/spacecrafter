#include <NbrVector.hpp>
#include <dynamic_printer.h>

using namespace std;

#define NB_MAX 100

/*
short *just_find_result(void)
{
    // Create array, element in position -1 is the array size.
    static short array[NB_MAX + 1];
    short i = 1; // array position

    for (register short j = 0; j++ < 100;) { // j : next value to insert into array
        if (j % 5 == 0)
            continue;
        if (j % 12 == 0) {
            if (++j % 5 == 0) // In case next number is 5, it would already been deleted with step-by-step treatment
                j++;
            continue;
        }
        array[i++] = j;
        if (j % 11 == 0)
            std::cout << j << "\n";
    }

    // note : i is now the array size
    register short j = 0; // array position
    while (j++ < i && array[j] <= 50); // skip number <= 50 (begin with 1)
    while (j < i && array[j] < 70) // Display numbers < 70
        std::cout << array[j++] << "\n";

    array[0] = i; // Write array size
    return (array + 1); // Move array so that it begin at 0, and -1 is array size.
}
*/

int main()
{
    NbrVector<short> nbvec();

    nbvec.Fill(100); // fill with numbers from 0 to 99 both included

    std::cout << "\ec";
    draw_cadre(80, 2);
    my_move(1, 1);
    std::cout << "Multiples of 11\n ";

    nbvec.DisplayMultiple(11); // display multiple of 11

    nbvec.RemoveMultiple(5);

    nbvec.RemoveMultipleAndNext(12);

    nbvec.DisplayNumbersBetweenExclusive(50, 70);

    return 0;
}
