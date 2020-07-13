/*
 * Tree
 *
 * Copyright 2020 AssociationSirius
 * Copyright 2020 Association Andromède
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#ifndef _TREE_HPP_
#define _TREE_HPP_

/*
 * Arborescent container
 */
template <typename T>
class Tree {
public:
	Tree() {}
	Tree(T &data) : value(data) {}
	Tree &operator[](int index) {return *tree[index];}
	auto begin() {return tree.begin();}
	auto end() {return tree.end();}
	int size() const {return tree.size();}
	void push_back(T &data) {tree.push_back(std::make_unique<Tree>(data));}
	void clear() {tree.clear();}

	T value; // Value stored by the container
private:
	std::vector<std::unique_ptr<Tree>> tree;
};


#endif /* _TREE_HPP_ */
