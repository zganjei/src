/*
    bw_constraint.cpp

    Created on: Jun 26, 2014

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

#include "bw_constraint.hpp"
#include "rule.hpp"

//#define debug_cplex_constraint

std::vector<IloConstraintArray>  BW_Constraint:: get_cplex_constraint ()const{

	std::vector<IloConstraintArray> temp, result;
	//partition indices
	std::vector<Linear_Expression> partitions=g_preorder->get_partitions();
	std::vector<std::vector<Linear_Expression>> negated_partitions=g_preorder->get_negated_partitions();

	temp.push_back(lexpr.get_cplex_constraint());//lexpr, e.g. guard

	assert(temp.size());

	IloConstraintArray point_cnstr = Cplex::get_point_cnstr(point);
	for (IloConstraintArray cnstr :temp){
		cnstr.add(point_cnstr);
	}

//	not needed, everything is in temp
//	if(!result.empty()){
//		temp = result;
//		result.clear();
//	}


#ifdef debug_cplex_constraint
//	std::cout<<"bwc lexpr constraint: "<<result[0]<<std::endl;
#endif

	for(int index:partn_indcs){
		assert(false && "not tested yet");
		IloConstraintArray partial_result = partitions[index].get_cplex_constraint ();
		for (IloConstraintArray cnstr :temp)
			cnstr.add(partial_result);//we want to avoid OR in the iloconstraints, thus in the upcomming bw_constraints

//		not needed, everything is in temp
//		if(!result.empty()){
//			temp.clear();
//			for(IloConstraintArray lexpr: result)
//				temp.push_back(lexpr);
//			result.clear();
//		}
	}
	for(int index:negated_partn_indcs){
		for(Linear_Expression neg_partn:negated_partitions[index]){
			IloAnd partial_result(Cplex::env);
			partial_result.add(neg_partn.get_cplex_constraint ());
#ifdef debug_cplex_constraint
			std::cout<<"partition neg cnstr: "<<partial_result<<std::endl;
#endif

			for (IloConstraintArray cnstr :temp){
				IloConstraintArray copy(Cplex::env);//*** very important. don't use copy constructor or it will mess everything up!
				copy.add(cnstr);
				copy.add(partial_result);
				result.push_back(copy);
			}
		}
		if(!result.empty()){
			temp.clear();
			for(IloConstraintArray lexpr: result)
				temp.push_back(lexpr);
			result.clear();
		}
	}
//////
	//pre_partition indices
	if(my_rule!=0){

		for(int index:pre_partn_indcs){
#ifdef debug_cplex_constraint
				std::cout<<"index: "<<index<<std::endl;
				std::cout<<"size of pre: "<<my_rule->get_pre(index,pre_id).size()<<std::endl;
#endif
			for(Linear_Expression lexpr: my_rule->get_pre(index)){
				IloAnd partial_result(Cplex::env);
				partial_result.add(lexpr.get_cplex_constraint());
#ifdef debug_cplex_constraint
				std::cout<<"pre partition cnstr: "<<partial_result<<std::endl;
#endif

				for (IloConstraintArray cnstr :temp){
					IloConstraintArray copy(Cplex::env);//*** very important. don't use copy constructor or it will mess everything up!
					copy.add(cnstr);
					copy.add(partial_result);
					result.push_back(copy);
				}
			}

			if(!result.empty()){
				temp.clear();
				for(IloConstraintArray lexpr: result)
					temp.push_back(lexpr);
				result.clear();
			}
		}


		for(int index:pre_negated_partn_indcs){

			for(Linear_Expression lexpr: my_rule->get_pre_neg(index)){
				IloAnd partial_result(Cplex::env);
				partial_result.add(lexpr.get_cplex_constraint ());
#ifdef debug_cplex_constraint
				std::cout<<"index: "<<index<<std::endl;
				std::cout<<"size of pre: "<<my_rule->get_pre_neg(index,pre_id).size()<<std::endl;
				std::cout<<"pre partition neg cnstr: "<<partial_result<<std::endl;
#endif

				for (IloConstraintArray cnstr :temp){
					IloConstraintArray copy(Cplex::env);//*** very important. don't use copy constructor or it will mess everything up!
					copy.add(cnstr);
					copy.add(partial_result);
					result.push_back(copy);
				}
			}
			if(!result.empty()){
				temp.clear();
				for(IloConstraintArray lexpr: result)
					temp.push_back(lexpr);
				result.clear();
			}
		}

	}

	if( result.empty() && !temp.empty())
		{
		for(IloConstraintArray lexpr: temp)
			result.push_back(lexpr);
		}


#ifdef debug_cplex_constraint
	for(IloConstraintArray r:result)
		std::cout<<"*************constraint of bwc: "<<r<<std::endl;
#endif
	return result;
}

std::vector<Linear_Expression> BW_Constraint::get_all_expressions()const{
	std::vector<Linear_Expression> result,temp;

	std::vector<Linear_Expression> partitions=g_preorder->get_partitions();
	std::vector<std::vector<Linear_Expression>> negated_partitions=g_preorder->get_negated_partitions();

	temp.push_back(lexpr);
#ifdef debug_lexpr
std::cout<<"BW_Constraint::get_all_expressions -added  lexpr\n";
#endif

	Linear_Expression point_lexpr = Linear_Expression::get_expression(point);
	for (Linear_Expression linexpr :temp){
		Linear_Expression intersect = linexpr.intersect(point_lexpr);
		intersect.update_z3();//updatez3 is for debugging
		result.push_back(intersect);
	}
	if(!result.empty()){
		temp.clear();
		for(Linear_Expression lexpr: result)
			temp.push_back(lexpr);
		result.clear();
	}
	for(Linear_Expression lexpr: result)
	{
		lexpr.update_z3();
		std::cout<<"after point and lexpr : "<<lexpr<<std::endl;
	}
	/////////
#ifdef debug_lexpr
std::cout<<"BW_Constraint::get_all_expressions -added point lexpr\n";
#endif


	for(int index:partn_indcs){
		for(Linear_Expression linexpr : temp)
			result.push_back(linexpr.intersect(partitions[index]));

		if(!result.empty()){
				temp.clear();
				for(Linear_Expression lexpr: result)
					temp.push_back(lexpr);
				result.clear();
			}
	}
////

	for(int index:negated_partn_indcs){
		for(Linear_Expression neg_partn:negated_partitions[index]){
			for (Linear_Expression linexpr :temp){
				result.push_back(linexpr.intersect(neg_partn));
			}
		}
		if(!result.empty()){
			temp.clear();
			for(Linear_Expression lexpr: result)
				temp.push_back(lexpr);
			result.clear();
		}
	}


	if(my_rule!=0){
		for(int index:pre_partn_indcs){
			for(Linear_Expression lexpr: my_rule->get_pre(index)){
				for (Linear_Expression linexpr :temp){
					result.push_back(linexpr.intersect(lexpr));
				}
			}

			if(!result.empty()){
				temp.clear();
				for(Linear_Expression lexpr: result)
					temp.push_back(lexpr);
				result.clear();
			}
		}
//		std::cout<<"BW_Constraint::get_all_expressions -added pre-partitions lexpr\n";

		for(int index:pre_negated_partn_indcs){
			for(Linear_Expression lexpr: my_rule->get_pre_neg(index)){
#ifdef debug_lexpr
				std::cout<<"pre_negated_partn_indcs.size="<<pre_negated_partn_indcs.size()<<std::endl;
				std::cout<<"my_rule->get_pre_neg(index,pre_id) size: "<<my_rule->get_pre_neg(index).size()<<std::endl;
				std::cout<<"temp result size: "<<temp.size()<<std::endl;
				std::cout<<"pre of constraint neg partition: "<<lexpr<<std::endl;
#endif
				for (Linear_Expression linexpr :temp){
					linexpr.update_z3();
//					std::cout<<"index= "<<index<<",rule id= "<<my_rule->getRuleIdentifier()<<std::endl;
					Linear_Expression intersect = linexpr.intersect(lexpr);
					intersect.update_z3();
#ifdef debug_lexpr
					std::cout<<"partial intersection:::"<<intersect<<std::endl;//<<==== check THIIIIIIIIIIIIIS !!!
#endif
					result.push_back(linexpr.intersect(lexpr));

				}
			}

			if(!result.empty()){
				temp.clear();
				for(Linear_Expression lexpr: result)
					temp.push_back(lexpr);
				result.clear();
			}
		}

//		std::cout<<"BW_Constraint::get_all_expressions -added pre-neg-partitions lexpr\n";

	}
	try{
		if( result.empty() && !temp.empty()){
//			std::cout<<"vector size: "<<temp.size()<<std::endl;

			result.clear();
			int i=0;
			for(Linear_Expression lexpr: temp){
				result.push_back(lexpr);
//				std::cout<<i<<"th element coppied\n";
				i++;
			}
			//		result = temp;
		}
//		std::cout<<"BW_Constraint::get_all_expressions -out of if clause\n";
		for(Linear_Expression le:result){
			//sometimes gives such error: terminate called after throwing an instance of 'std::bad_alloc' why?????XXX
			le.update_z3();
			//		std::cout<<"final intersect result::::: "<<le<<std::endl;
		}
	}
	catch(std::bad_alloc& ba)
	{
		std::cerr << "bad_alloc caught: " << ba.what() << '\n';
		exit(-1);
	}
//	std::cout<<"return get_all_expr\n";
	return result;
}


