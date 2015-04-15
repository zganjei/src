/*
    constraint.hpp

    Created on: Jun 16, 2014

	Copyright (C) 2003 Johann Deneux

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Author: Zeinab Ganjei <zeinab.ganjei@liu.se>
 */

#ifndef CONSTRAINT_HPP_
#define CONSTRAINT_HPP_


#include "linear_expression.hpp"
#include "minset.hpp"

template<class T>
class entailmentOrder{
public:
	entailmentOrder<T>(){}
	//C1 le C2
	bool operator() (const T& C1, const T& C2) const{
		return (C2->entails(C1));//original
//		return (C1->entails(C2));//mine
	}
};

class Constraint{
protected:
	static unsigned counter;
	Linear_Expression lexpr;//multiple rows in the corresponding matrix means conjunction of smaller linear expressions
	unsigned identifier;
	Point point;//only used in the children
	size_t key;//only used in the children
public:
	typedef std::shared_ptr<Constraint> sptr;
	typedef  MinSet< sptr, entailmentOrder<sptr>>  MSet;
	typedef  Minset_Iterator< sptr> MSet_it;
	typedef  MSet::const_iterator const_iterator;

	Constraint(Linear_Expression  _lexpr):lexpr(_lexpr),identifier(counter++){
	}

	Constraint():identifier(counter++){

	}
	size_t get_key(){
		return key;
	}
	unsigned get_weight(){
		return point.get_weight();
	}
	Point get_point(){
		return point;
	}
	bool isEmpty()const {
		assert(false && "Constraint:isEmpty is not implemented yet!");
		return false;
	}

	unsigned get_identifier() const {
		return identifier;
	}

	Linear_Expression get_expression(){
		return lexpr;
	}

	std::string formulaToString() const {
		std::stringstream ss;
		assert(false && "formulaToString is not implemented for Constraint");
		return ss.str();
	}

	bool entails(const Constraint& other)const {
		if(key!=other.key) return false;
		return lexpr.implies(other.lexpr);
	}

	bool entails(const sptr& other)const {
		assert(other);
		return entails(*other);
	}

	friend std::ostream& operator<<
			(std::ostream& out, const sptr& cstr){
		cstr->printOn(out);
		return out;
	}

	friend std::ostream& operator<<
			(std::ostream& out, const MSet& cset){

		MSet_it cset_it = cset.get_iterator();
		const_iterator c = cset_it.get_current_it();

		for(;!cset_it.is_end(); c = cset_it.next_it())
		{
			(*c)->printOn(out);
			out << "\n\n";
		}
		out << "\n\n";
		return out;
	}

	void printOn(std::ostream& o) const {
		o  << lexpr ;
	}
};





#endif /* CONSTRAINT_HPP_ */
