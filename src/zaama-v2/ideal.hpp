/*
    ideal.hpp

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

#ifndef IDEAL_HPP_
#define IDEAL_HPP_

#include <set>
#include <algorithm>   // for includes()
#include <boost/functional/hash.hpp>

#include "preorder.hpp"


extern Preorder::sptr g_preorder;


class Ideal: public Constraint
{
protected:
	/*
	 we keep the indices to partitions used in the ideal.
	 in the positive_partitions we keep the indices of partitions used originally
	 and in the negative_partitions we keep the indices of partitions whose negation is contributing to the ideal
	 */
	std::set<int> partn_indcs;
	std::set<int> negated_partn_indcs;

public:

	Ideal(Point _point,std::set<int> _positive,std::set<int> _negative)
		:Constraint(),partn_indcs(_positive),negated_partn_indcs(_negative){
		point = _point;
		key = point.get_key();
//		std::cout<<hash()<<std::endl;getchar();
	}

	Ideal(Point _point):Constraint(){
			point = _point;
			key = point.get_key();
			assert(key>=0);
//			printOn(std::cout);
//			std::cout<<hash()<<std::endl;getchar();
		}
	typedef std::shared_ptr<Ideal> sptr;

	typedef  MinSet< Ideal::sptr, entailmentOrder<Ideal::sptr>>  MSet;
	typedef  Minset_Iterator< sptr> MSet_it;
	typedef  MSet::const_iterator const_iterator;
	typedef  MSet::iterator iterator;

	std::set<int> get_partition_indices(){
		return partn_indcs;
	}
	std::set<int> get_negated_partition_indices(){
		return negated_partn_indcs;
	}

	size_t hash(){

		int i=0;
		int total_size = point.size+partn_indcs.size()+negated_partn_indcs.size();
		int all_data[total_size];
		for(;i<point.size;i++){
			all_data[i]=point.data[i];
		}
		for(int p:partn_indcs){
			all_data[i]=p;
			i++;
		}

		for(int p:negated_partn_indcs){
			all_data[i]=p;
			i++;
		}
		size_t hash = boost::hash_range(all_data, all_data+total_size);
//		std::cout<<"ideal.hash= "<<hash<<std::endl;
		return hash;
	}

	std::vector<IloConstraintArray> get_cplex_constraint ()const{

		std::vector<IloConstraintArray> temp;
		std::vector<IloConstraintArray> result;

		std::vector<Linear_Expression> partitions=g_preorder->get_partitions();
		std::vector<std::vector<Linear_Expression>> negated_partitions=g_preorder->get_negated_partitions();

		temp.push_back(IloConstraintArray(Cplex::env));//correct????

		for(int index:partn_indcs){

			IloConstraintArray partial_result = partitions[index].get_cplex_constraint ();
//			for(int i=0;i<partial_result.getSize();i++)
//				result.add(partial_result[i]);
			for (IloConstraintArray cnstr :temp){
				cnstr.add(partial_result);//we want to avoid OR in the iloconstraints
			std::cout<<"after adding partition: "<<cnstr<<std::endl;
			}


//			if(!result.empty()){
//				temp.clear();
//				for(IloConstraintArray lexpr: result)
//					temp.push_back(lexpr);
//				result.clear();
//			}
		}


		for(int index:negated_partn_indcs){
			for(Linear_Expression neg_partn:negated_partitions[index]){
				IloConstraintArray partial_result =neg_partn.get_cplex_constraint ();
//				for(int i=0;i<partial_result.getSize();i++)
//					result.add(partial_result[i]);

				for (IloConstraintArray cnstr :temp){
					IloConstraintArray copy(Cplex::env);//*** very important. don't use copy constructor or it will mess everything up!
					copy.add(cnstr);
					copy.add(partial_result);
					result.push_back(copy);
					std::cout<<"after adding neg partition: "<<copy<<std::endl;
				}
			}
			if(!result.empty()){
				temp.clear();
				for(IloConstraintArray lexpr: result)
					temp.push_back(lexpr);
				result.clear();
			}
		}


//		result.add(Cplex::get_point_cnstr(point));
		IloConstraintArray point_cnstr = Cplex::get_point_cnstr(point);
		for (IloConstraintArray cnstr :temp)
			cnstr.add(point_cnstr);


		if( result.empty() && !temp.empty())
		{
			for(IloConstraintArray lexpr: temp)
				result.push_back(lexpr);
		}


#ifdef debug_ideal
		for(IloConstraintArray r: result)
		std::cout<<"ideal cplex constraint: "<<r<<std::endl;
#endif
		return result;
	}

	//an expression off all possible lexprs of an ideal, each one is OR-free!
	std::vector<Linear_Expression> get_all_expressions()const{
		std::vector<Linear_Expression> result,temp;

		std::vector<Linear_Expression> partitions=g_preorder->get_partitions();
		std::vector<std::vector<Linear_Expression>> negated_partitions=g_preorder->get_negated_partitions();

		temp.push_back(Linear_Expression(true));

		for(int index:partn_indcs){
//			result.push_back(partitions[index]);

			for(Linear_Expression linexpr : temp)
				result.push_back(linexpr.intersect(partitions[index]));

//			if(!result.empty()){
//				temp = result;
//				result.clear();
//			}

		}

		for(int index:negated_partn_indcs){
			for(Linear_Expression neg_partn:negated_partitions[index]){
//				result.push_back(neg_partn);
				for (Linear_Expression lexpr :temp){
					result.push_back(lexpr.intersect(neg_partn));
				}
			}
			if(!result.empty()){
				temp = result;
				result.clear();
			}
		}

//		result.push_back(Linear_Expression::get_expression(point));
		Linear_Expression point_lexpr = Linear_Expression::get_expression(point);
		for (Linear_Expression lexpr :temp){
			result.push_back(lexpr.intersect(point_lexpr));
		}
		//std::cout <<"return ideal::get_all_expressions()\n";
		return result;
	}
	bool entails(const Ideal& other)const{//check this,ask!

		//point of this ideal must be smaller than point of the other, if it wants to imply it
		if(key!=other.key )
			return false;
		if(!(point <= other.point) && !( other.point <= point))/*unrelated points*/
			return false;

		if(point < other.point)
			return false;

		//this ideal must be stronger than the other, i.e. its predicates must be subset of the other
		//i.e. "other" inclues "this"
		if(!std::includes(partn_indcs.begin(),partn_indcs.end(),other.partn_indcs.begin(),other.partn_indcs.end()))
			return false;
		if(!std::includes(negated_partn_indcs.begin(),negated_partn_indcs.end(),other.negated_partn_indcs.begin(),other.negated_partn_indcs.end()))
			return false;
#ifdef debug_entail
		std::cout<<"ideal: apparently an entailment!\n";
		Point::print_point(point,"\n");
		Point::print_point(other.point,"\n");
#endif
		//correct????????
//		if(point == other.point)
//			return false;//zeinab: intentionaly i want to add all ideals which are equal, because they may result in different traces

		return true;
	}

	bool entails(const sptr& other)const {
		assert(other);
		return entails(*other);
	}

	void add_partition(int index,bool positive){
		if(positive)
			partn_indcs.insert(index);
		else
			negated_partn_indcs.insert(index);
	}

	Ideal intersect(const Ideal& other)const {
		assert(false && "not implemented yet!");
		//		return Ideal(cstr_expr.intersect(other.get_expression()));
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
		o<<"key: "<<key<<std::endl;
		Point::print_point(point);
		o<<"positive partitions:";
		for(int i:partn_indcs)
			o<<i<<" ";
		std::cout<<std::endl;

		o<<"negative partn_indcs:";
		for(int i:negated_partn_indcs)
			o<<i<<" ";
		std::cout<<std::endl;

		//		o  << lexpr ;//maybe I totally remove lexpr from class ideal...
	}
	friend class BW_Constraint;
	friend class FW_Constraint;

};


#endif /* IDEAL_HPP_ */
