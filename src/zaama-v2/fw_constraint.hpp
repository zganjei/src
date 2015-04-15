/*
    fw_constraint.hpp

    Created on: Jun 5, 2014

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

#ifndef FW_CONSTRAINT_HPP_
#define FW_CONSTRAINT_HPP_

#include "linear_expression.hpp"
#include "minset.hpp"
#include "constraint.hpp"
#include "cplex.hpp"


class FW_Constraint:public Constraint{
public:
	typedef std::shared_ptr<FW_Constraint> sptr;
	typedef  MinSet< sptr, entailmentOrder<sptr>>  MSet;
	typedef  Minset_Iterator< sptr> MSet_it;
	typedef  MSet::const_iterator const_iterator;

	FW_Constraint(Linear_Expression _lexpr):Constraint(_lexpr){
		key = 0;
	}
	FW_Constraint():Constraint(){
		lexpr = Linear_Expression(false);
		key=0;
	}

	bool isEmpty() {
		return lexpr.isEmpty();
	}

	void update_z3(){
		lexpr.update_z3();
	}

	bool entails(const FW_Constraint& other)const {
		return lexpr.implies(other.lexpr);
	}

	bool entails(const sptr& other)const {
		assert(other);
		return entails(*other);
	}
	IloConstraintArray get_cplex_constraint(){
		return lexpr.get_cplex_constraint();
	}

	std::vector<Linear_Expression> get_all_expressions()const{
		std::vector<Linear_Expression> result;
		result.push_back(lexpr);
		return result;
	}

	std::string formulaToString()  {
		std::stringstream ss;
		lexpr.update_z3();
		lexpr.printOn(ss,true);
		return ss.str();
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





#endif /* FW_CONSTRAINT_HPP_ */
