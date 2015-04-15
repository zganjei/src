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

#ifndef BW_CONSTRAINT_HPP_
#define BW_CONSTRAINT_HPP_

#include <algorithm>   // for set.includes()

#include "linear_expression.hpp"
#include "minset.hpp"
#include "preorder.hpp"
#include "constraint.hpp"
//#include "rule.hpp"
#include "cplex.hpp"

//#define detailed_print

extern Preorder::sptr g_preorder;

class Rule;

class BW_Constraint: public Constraint{
protected:

	std::set<int> partn_indcs;//partition indices
	std::set<int> negated_partn_indcs;

	std::set<int> pre_partn_indcs;
	std::set<int> pre_negated_partn_indcs;

	const Rule* my_rule;

public:
	typedef std::shared_ptr<BW_Constraint> sptr;
	typedef  MinSet< sptr, entailmentOrder<sptr>>  MSet;
	typedef  Minset_Iterator< sptr> MSet_it;
	typedef  MSet::const_iterator const_iterator;

	BW_Constraint(Point _point,std::set<int> _pre_partn_indcs,std::set<int> _pre_negated_partn_indcs,const Rule* _my_rule,Linear_Expression  _lexpr=Linear_Expression(true)/*e.g. guard*/)
	:Constraint(_lexpr),pre_partn_indcs(_pre_partn_indcs),pre_negated_partn_indcs(_pre_negated_partn_indcs),my_rule(_my_rule){
		point = _point;
		key = point.get_key();

	}

	BW_Constraint(/*Linear_Expression  _lexpr,*/Point _point,const Rule* _my_rule=0):Constraint(),my_rule(_my_rule)//for "bad" constraint
	{

		//lexpr = _lexpr;//for "bad" state we don't need lexpr, just the point is enough

		std::cout<<"let's create a bwc!\n";

		point = _point;
		key = point.get_key();

	}

	static BW_Constraint::MSet get_constraints(Linear_Expression input){//somehow a constructor
		BW_Constraint::MSet result;

		std::cout<<"top: "<<Linear_Expression::top_expr<<std::endl;
		std::cout<<"before conjunct with top: \n"<<input<<std::endl;
		input = input.intersect(Linear_Expression::top_expr);//z3 is used - z3_updated=true
		std::cout<<"after conjunct with top: \n"<<input<<std::endl;

		std::vector<Linear_Expression> sticks = input.get_sticks(g_preorder->get_expr());


		int i=0;
		for(Linear_Expression stick: sticks){
			std::cout<<"stick: "<<stick<<std::endl;
			i++;
			Linear_Expression intrsct = input.intersect(stick);
			intrsct.update_z3();
			std::vector<Linear_Expression> splitted = intrsct.split();//assertion violation in here

			for(Linear_Expression split:splitted){
//				std::cout<<"intersection with top\n"<<split<<std::endl;
				std::vector<Point> points=Cplex::find_min_elems(split.get_cplex_constraint(),g_preorder->get_domain_indices());
				for(Point p:points)
					result.insert(BW_Constraint::sptr(new BW_Constraint(/*split,*/p)));
			}
		}
//getchar();
		return result;
	}

	std::vector<IloConstraintArray> get_cplex_constraint ()const;

	std::vector<Linear_Expression> get_all_expressions()const;



	bool entails(const BW_Constraint& other)const {
		if(key!=other.key) return false;
		if(!(point <= other.point) && !( other.point <= point))/*unrelated points*/
			return false;

		if(point < other.point)
			return false;

		if(!std::includes(partn_indcs.begin(),partn_indcs.end(),other.partn_indcs.begin(),other.partn_indcs.end()))
			return false;
		if(!std::includes(negated_partn_indcs.begin(),negated_partn_indcs.end(),other.negated_partn_indcs.begin(),other.negated_partn_indcs.end()))
			return false;

		if(!std::includes(pre_partn_indcs.begin(),pre_partn_indcs.end(),other.pre_partn_indcs.begin(),other.pre_partn_indcs.end()))
			return false;
		if(!std::includes(pre_negated_partn_indcs.begin(),pre_negated_partn_indcs.end(),other.pre_negated_partn_indcs.begin(),other.pre_negated_partn_indcs.end()))
			return false;

#ifdef debug_entail
		std::cout<<"bwc: apparently an entailment!\n";
		Point::print_point(point,"\n");
		Point::print_point(other.point,"\n");
#endif
		return true;
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
		o<<"key: "<<key<<"\n";
		Point::print_point(point);
//		std::cout<<"rule-"<<my_rule->getRuleIdentifier()<<std::endl;//XX fw declaration problem
		o<<"pre_partn_indcs:";
		for(int i:pre_partn_indcs)
			o<<i<<" ";
		std::cout<<std::endl;

		o<<"negative pre_partn_indcs:";
		for(int i:pre_negated_partn_indcs)
			o<<i<<" ";
		std::cout<<std::endl;
#ifdef detailed_print
		o  << lexpr ;
#endif
	}
	friend class Rule;
};





#endif /* BW_CONSTRAINT_HPP_ */
