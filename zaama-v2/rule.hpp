/*
    Copyright (C) 2009 Rezine Ahmed

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

    Contact information: Rezine Ahmed <rezine.ahmed@gmail.com>  
 */


/** @file rule.hpp
 * Header for an abstract rule denoting a possibly infinite set of configurations of the system
 *
 * @author Rezine Ahmed
 */

#ifndef _ZAAMA_RULE_HPP_
#define _ZAAMA_RULE_HPP_

#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <string>

#include <memory>

//#include "minset.hpp"

#include "printable.hpp"
#include "linear_expression.hpp"
#include "bw_constraint.hpp"
#include "fw_constraint.hpp"
#include "preorder.hpp"
#include "ideal.hpp"

//#define debug_post
//#define debug_pred
//#define debug_rule

extern Preorder::sptr g_preorder;

class Rule{
private:
	Linear_Expression rule_expr;
	std::vector<std::vector<Linear_Expression>> pre_of_partitions;
	std::vector<std::vector<Linear_Expression>> pre_of_neg_partitions;
	unsigned rule_identifier;
	std::vector<size_t> compatible_keys;
	bool isBroadcast;

public:
	typedef std::shared_ptr<Rule> sptr;

	Rule(int _identifier, const Linear_Expression& _expr):
		rule_identifier(_identifier), rule_expr(_expr)
	{
		compute_pre_partitions();
		isBroadcast = Linear_Expression::is_broadcast(rule_expr);

//		if(rule_identifier<=11){
//			std::cout<<"rule::is_broadcast= "<<is_broadcast<<std::endl;
//			getchar();
//		}

	}

	void compute_pre_partitions(){
		//in case no new partition is added, for example sometimes in lazy refinement
		if(pre_of_partitions.size()==g_preorder->get_partitions().size())
			return;

		int old_size= pre_of_partitions.size();
		int new_size = g_preorder->get_partitions().size();

		assert(old_size<new_size);

//		pre_of_partitions.clear();
//		pre_of_neg_partitions.clear();

		std::vector<Linear_Expression> partitions = g_preorder->get_partitions();
		std::vector<std::vector<Linear_Expression>> negated_partitions = g_preorder->get_negated_partitions();

		assert(partitions.size()==negated_partitions.size());

		//don't clear previous "pre_of_..." arrays, only add pre of the new members to them
		for(int i=old_size/*0*/;i<new_size/*partitions.size()*/;i++){
			std::vector<Linear_Expression> pre_of_pos;
			std::vector<Linear_Expression> pre_of_neg;
			//partitions
#ifdef debug_rule
			std::cout<<"pre of partition\n"<<partitions[i]<<"\n";
#endif
			std::vector<Linear_Expression> pre=Linear_Expression::pre(rule_expr,partitions[i]);
			for(Linear_Expression lexpr:pre){//split to remove OR
				pre_of_pos.push_back(lexpr);
#ifdef debug_rule
				std::cout<<"is \n"<<lexpr<<"\n";
#endif
			}
			pre_of_partitions.push_back(pre_of_pos);
			//////////////////////////////////////////////////////////////////////
			//negation of partitions
			for(Linear_Expression neg:negated_partitions[i]){
#ifdef debug_rule
				std::cout<<"pre of partition negation\n"<<neg<<"\n";
#endif
				std::vector<Linear_Expression> pre=Linear_Expression::pre(rule_expr,neg);
				if(!pre.empty())
					for(Linear_Expression lexpr:pre){
						pre_of_neg.push_back(lexpr);
#ifdef debug_rule
						std::cout<<"is \n"<<lexpr<<"\n";
						std::cout<<"size of preneg = "<<pre_of_neg.size()<<std::endl;
#endif
					}
			}

			pre_of_neg_partitions.push_back(pre_of_neg);
		}
#ifdef debug_rule
		if(rule_identifier==58)
			getchar();
#endif

	}

	std::vector<Linear_Expression> get_pre(int partn_index)const{
		assert(partn_index<pre_of_partitions.size());
		return pre_of_partitions[partn_index];
	}

	std::vector<Linear_Expression> get_pre_neg(int partn_index)const{
		assert(partn_index<pre_of_neg_partitions.size());
		return pre_of_neg_partitions[partn_index];
	}



	void find_compatible_keys(Preorder::sptr order){
		compatible_keys = rule_expr.get_compatible_keys(g_preorder->preorder_expr);
	}

	std::vector<size_t> get_compatible_keys(){
		return compatible_keys;
	}

	unsigned getRuleIdentifier()const{return rule_identifier;}

	Linear_Expression get_guard()const{
		Linear_Expression result=Linear_Expression::get_guard(rule_expr);
		return result;
	}

	Linear_Expression get_expression ()const{
		return rule_expr;
	}
	bool is_broadcast()const{
		return isBroadcast;
	}
	FW_Constraint::MSet post(const FW_Constraint::sptr& from)const{//uses z3!
		FW_Constraint::MSet result;
#ifdef debug_post
		std::cout<<"rule::post()\n";
		Linear_Expression from_expr = from->get_expression();
		from_expr.update_z3();
		std::cout<<"from->expression: "<<from_expr<<std::endl;
#endif
		std::list<Linear_Expression> post_expressions =
				Linear_Expression::post(rule_expr,from->get_expression());
		std::cout<<"actual pre size: "<<post_expressions.size()<<std::endl;
		for(Linear_Expression post_expression: post_expressions){
//#ifdef debug_post
			std::cout<<"****expression:post output****\n";
			std::cout<<post_expression<<std::endl;
//#endif
			FW_Constraint::sptr pp=FW_Constraint::sptr(new FW_Constraint(post_expression));
			result.insert(pp);
			std::cout<<"size after insertion: "<<result.size()<<std::endl;
		}
#ifdef debug_post
		std::cout<<"****rule:post output****\n";
		std::cout<<result<<std::endl;
#endif
		assert(!(!post_expressions.empty()&&result.empty()));
		return result;
	}

	FW_Constraint::MSet post(const FW_Constraint::MSet& from)const{
		FW_Constraint::MSet result;

		FW_Constraint::MSet_it f_it = from.get_iterator();
		FW_Constraint::const_iterator f = f_it.get_current_it();
		////********

		for(;!f_it.is_end(); f=f_it.next_it()){
			result.insert(post(*f));
		}

		return result;
	}



	BW_Constraint::MSet pred(const Ideal::sptr& from)const{//FIXME!!
#ifdef debug_pred
		std::cout<<"rule["<<rule_identifier<<"], pred()\n";// of \n"<<from<<std::endl;
#endif
		BW_Constraint::MSet constraint_set;

//		if(!isBroadcast){//is it correct to comment???
			//if rule doesn't have pre for any of the partitions involved in the ideal, the result of pre would be empty
			for(int i: from->get_partition_indices()){
				if(get_pre(i).empty()){
					return constraint_set;//empty
				}
			}

			for(int i: from->get_negated_partition_indices()){
				if(get_pre_neg(i).empty()){
					return constraint_set;//empty
				}
			}
//		}

		std::vector<Linear_Expression> pred_exprs = Linear_Expression::pre(rule_expr,from->get_point());
		if(pred_exprs.empty()) return constraint_set;//empty result


		IloConstraintArray cplex_cnstr(Cplex::env);
		for(int i=0;i<pred_exprs.size();i++){/// XXXX AND? OR??
			cplex_cnstr.add(pred_exprs[i].get_cplex_constraint());
		}
		cplex_cnstr.add(get_guard().get_cplex_constraint());
		///add pre partition constraints
		for(int i:from->get_partition_indices()){
			IloOr pre_cnstr(Cplex::env);
			for(Linear_Expression prelexpr:get_pre(i)){
				IloAnd partital(Cplex::env);
				partital.add(prelexpr.get_cplex_constraint());
				pre_cnstr.add(partital);
			}
			cplex_cnstr.add(pre_cnstr);
		}

		for(int i:from->get_negated_partition_indices()){
			IloOr pre_neg_cnstr(Cplex::env);
			for(Linear_Expression negprelexpr:get_pre_neg(i)){
				IloAnd partital(Cplex::env);
				partital.add(negprelexpr.get_cplex_constraint());
				pre_neg_cnstr.add(partital);
			}
			cplex_cnstr.add(pre_neg_cnstr);
		}
		///

		std::vector<Point> min_points = Cplex::find_min_elems(cplex_cnstr,g_preorder->get_domain_indices());

		for(int j=0;j<min_points.size();j++){


//			if(isBroadcast){//broadcast is like any other rule????
//
//				Point::print_point(min_points[j],"a broadcast pre point");
//				Linear_Expression bwc_lexpr(true);
//				for(Linear_Expression lexpr: pred_exprs)
//					bwc_lexpr = bwc_lexpr.intersect(lexpr);
//				BW_Constraint* brdcst_bwc =new BW_Constraint(min_points[j],std::set<int>(),std::set<int>(),this,-1,bwc_lexpr);
//				constraint_set.insert(BW_Constraint::sptr(brdcst_bwc));
//				continue;//you don't need to check min points again!
//				//					getchar();
//			}

			//FIXME - i think the following call to cplex can be removed by just checking the previous min_points against guard, agree???? XXXX
			BW_Constraint* bwc =new BW_Constraint(min_points[j],from->get_partition_indices(),from->get_negated_partition_indices(),this,get_guard());

//			I commented the following lines. correct????
			/*
			std::vector<Point> test_points = Cplex::find_min_elems(bwc->get_cplex_constraint(),g_preorder->get_domain_indices());

			if(test_points.size()!=1){
				std::cout<<"second call to cplex. result size= "<<test_points.size()<<std::endl;

				Point::print_points(test_points,"all result points");
				//				getchar();
			}*/

			//			assert(test_points.size()<=1);//correct ??? 0 or 1

#ifdef debug_pred
//			if(test_points.size()!=0){
//				Point::print_point(test_points[0],"test_points[0]:");
//				Point::print_point(min_points[j],"min_points[j]:");
//			}
#endif
//			if(!test_points.empty())//don't insert empty constraints - ic commented it . correct???
				constraint_set.insert(BW_Constraint::sptr(bwc));


		}
#ifdef debug_pred
		std::cout<<"return rule:pred\n";
#endif
		return constraint_set;
	}




	friend std::ostream& operator<<
			(std::ostream& out, const sptr& rule){
		rule->print(out);
		return out;
	}

public:

	virtual void print(std::ostream& o) const {

		o << "[" << rule_identifier << "]: " ;
		rule_expr.printOn(o,true);
	}



};

typedef std::list<Rule::sptr> Rules;

#endif /* _ZAAMA_RULE_HPP_ */
