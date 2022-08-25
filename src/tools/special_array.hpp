/*
Minimum class for circular array
Use: This class allows to push elements at the beginning and at the end of the array with a complexity in O(1)
Usage: to be included in the C++ program
Remark : this class is minimal (incomplete) and can cause errors in case of specific use
Author: Aurélien Schwab <aurelien.schwab+dev@gmail.com> for immersiveadventure.net
Updated on 27/05/2017
*/

#ifndef SPECIAL_ARRAY_H
#define SPECIAL_ARRAY_H

#include <stdlib.h>

template<typename T>
class SpecialArray {

private:

	T* array; //Pointer of the array that will store the elements
	unsigned int asize; //Size of the array
	unsigned int begin; //Start of the array

	void backward() {
		if(begin == 0) begin = asize; //If we are at the beginning we loop to the end
		begin--; //We go backwards
	}

public:

	SpecialArray() = delete; //TODO
	SpecialArray(const SpecialArray<T> &sa) = delete; //TODO

	SpecialArray(const unsigned int size) : asize(size), begin(0) {
		array = new T[size];    //Constructor
	}
	~SpecialArray() {
		delete array;    //Destructor TODO delete[]
	}

	const unsigned int size() const {
		return asize;
	}; //Delete the size of the array


	T& push(int move) { //Add from the front
		if(move < 0) return pushFront();
		return pushBack();
	}

	T& pushFront() { //Add by front
		backward();
		return array[begin]; //Assign value
	}

	T& pushBack() { //Adding from behind
		if(++begin == asize) {
			begin = 0;
			return array[asize-1];
		} else return array[begin-1];
	}

	void pushFront(const T &value) { //Add in front
		backward();
		array[begin] = value; //We assign the value
	}

	void pushBack(const T &value) { //Add from behind
		array[begin] = value; //Assign the value
		if(++begin == asize) begin = 0; //We go forward and if we are at the end we loop back to the beginning
	}

	T& operator[](const unsigned int index) { //Random access to an element //TODO const
		if(index < 0 || index >= asize) throw; //TODO //Outside the array
		if(begin + index >= asize) return array[begin + index - asize]; //Before begin
		return array[index + begin]; //After begin
	}

};

#endif

