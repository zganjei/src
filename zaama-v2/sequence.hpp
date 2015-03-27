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

/**@file sequence.hpp
 * Whatever is needed for tracing
 *
 *@author Ahmed Rezine
 */

#ifndef _ZAAMA_SEQUENCE_HPP_
#define _ZAAMA_SEQUENCE_HPP_

#include <iostream>
#include <stdlib.h>
#include <map>
#include <memory>
#include <sstream>

#include "mathsat.h"

#include "linear_expression.hpp"
#include "constraint.hpp"
#include "preorder.hpp"
#include "ideal.hpp"
#include "rule.hpp"
#include "utility.hpp"
#include "refine_info.hpp"

#define debug_fronter
//#define stop_per_step

extern Preorder::sptr g_preorder;
extern Point refinement_point;
class Sequence 
{
private:
	std::list<FW_Constraint::sptr > exact_constraints;
	std::list<Ideal::sptr > approximated_ideals;
	std::list<Rule::sptr> rules;
public:
	typedef std::shared_ptr<Sequence> sptr;

public:
	Sequence(){}

	bool addExactConstraint(const FW_Constraint::sptr& cstr)
	{exact_constraints.push_back(cstr);}

	bool addApproximatedIdeal(const Ideal::sptr& uc)
	{approximated_ideals.push_back(uc);}

	bool addRule(const Rule::sptr& rule)
	{rules.push_back(rule); }

	void clear(){
		exact_constraints.clear();
		approximated_ideals.clear();
		rules.clear();
	}
	int size(){
		assert(exact_constraints.size() == approximated_ideals.size());
		assert(exact_constraints.size() == 1 + rules.size());
		return approximated_ideals.size();
	}

	Ideal::sptr get_last_approx(){
		return (approximated_ideals.back());
	}
	FW_Constraint::sptr get_last_exact(){
		return (exact_constraints.back());
	}
	bool isEmpty()const{return exact_constraints.empty();}

	std::vector<Linear_Expression> getFrontier()
						  {
		std::cout<< "sequence::getFrontier()"<<std::endl;

#ifdef debug_fronter
		std::list<FW_Constraint::sptr>::const_iterator exact_it= exact_constraints.begin();
		std::list<Ideal::sptr>::const_iterator approx_it= approximated_ideals.begin();
		std::list<Rule::sptr>::const_iterator rule_it= rules.begin();

		int i=0;
		if(exact_it!=exact_constraints.end()){
			assert(approx_it!=approximated_ideals.end());
			std::cout << "exact:" << *exact_it << "\n approx:" << *approx_it << std::endl;
			++exact_it; ++approx_it;
#ifdef stop_per_step
			getchar();
#endif
			i++;
		}
		while(exact_it!=exact_constraints.end()) {
			assert(approx_it!=approximated_ideals.end());
			assert(rule_it!=rules.end());
			std::cout << "\nrule: " << (*rule_it)->getRuleIdentifier() << std::endl ;
			std::cout << "exact:" << *exact_it << "\n approx:" << *approx_it << std::endl;
			++exact_it; ++approx_it;++rule_it;
#ifdef stop_per_step
			getchar();
#endif
			i++;
		}
#endif
		assert(exact_constraints.size()>=2);
		assert(exact_constraints.size() == approximated_ideals.size());
		assert(exact_constraints.size() == 1 + rules.size());

		std::list<FW_Constraint::sptr>::const_reverse_iterator  rit_exact=exact_constraints.rbegin();
		std::list<Ideal::sptr>::const_reverse_iterator  rit_approx=approximated_ideals.rbegin();
		std::list<Rule::sptr>::const_reverse_iterator  rit_rule=rules.rbegin();

#ifdef debug_fronter
		std::cout << "last added approx ideal:\n" << *rit_approx << "\n";//0, 0, 0, 0, 0, 0, 1, 1,
		std::cout << "last added exact constraint:\n" << *rit_exact << "\n";//false
		std::cout << "last added rule: " << *rit_rule << "\n";//13
#endif
		FW_Constraint::sptr plus,minus;

		BW_Constraint::MSet pre_exact = (*rit_rule)->pred(*rit_approx);
//getchar();

#ifdef debug_fronter
		std::cout << "exact predecessor of the last added approx ideal (eplaai/exact pre): " << pre_exact << "\n";//f2'
#endif

		++rit_approx;

#ifdef debug_fronter
		std::cout << "the before last added approx ideal (blaai/approx pre): " << *rit_approx << "\n";// ideal
#endif

		//    pre_exact = g_preorder->intersect(*rit_approx,pre_exact);
		FW_Constraint::MSet pre_exact_2 = Utility::intersect(pre_exact,*rit_approx);//std::bad_aloc error/ assertion false

#ifdef debug_fronter
		std::cout << "intersection of eplaai and blaai: (" << pre_exact_2 << ")\n";//minus
#endif

		assert(!pre_exact_2.empty());

		rit_exact++;
		plus= *rit_exact;//fwc:


		FW_Constraint::MSet_it pre_it = pre_exact_2.get_iterator();//thisone
std::cout<<"getting minus\n";
		minus= *(pre_it.get_current_it());
		std::cout<<"updating z3\n";
		minus->update_z3();
		assert(!plus->isEmpty());
		assert(!minus->isEmpty());


#ifdef debug_fronter
		std::cout<< "<<< plus >>> " << plus << "\n"
				<< "<<< minus>>>" << minus << "\n";
//		getchar();
#endif
#ifdef lazy_refinement
		std::vector<Point> min_points =  Cplex::find_min_elems(minus->get_cplex_constraint(),g_preorder->get_domain_indices());//**
		for(Point refp:min_points)
			Point::print_point(refp,"refinement point i: ");

		assert(min_points.size()>0 && "there must exist some refinement point!!");
		assert(min_points.size()==1 && "I have not implemented the case where there exists several refinement points");
		refinement_point= min_points[0];
		Point::print_point(refinement_point,"refinement point: ");
#endif
		assert((Utility::intersect(plus,minus))->isEmpty());

		std::vector<Linear_Expression> temp_result= interpolant(plus,minus);//original
//		std::vector<Linear_Expression> temp_result= interpolant(minus,plus);//correct??????XXXXX


		std::vector<Linear_Expression> result;
		result.push_back(temp_result[0]);//start the implication checking process
//good???
		for(Linear_Expression a:temp_result){
			bool repetitive = false;
			for(Linear_Expression b:result){
				std::vector<Linear_Expression> neg= b.negate();
				for(Linear_Expression negg:neg){

					if(!a.get_z3_updated())
						a.update_z3();
					if(!negg.get_z3_updated())
						negg.update_z3();

					if(!b.get_z3_updated())
						b.update_z3();

					if(a.implies(negg) || negg.implies(a) || a.implies(b) || b.implies(a))
						repetitive=true;
				}
			}
			if(!repetitive)
				result.push_back(a);
		}

		return result;

 }

	std::vector<Linear_Expression> interpolant(FW_Constraint::sptr plus, FW_Constraint::sptr minus){

		msat_config cfg = msat_create_config();
		msat_set_option(cfg, "interpolation", "true");
		msat_env env = msat_create_env(cfg);
		assert(!MSAT_ERROR_ENV(env));
		msat_destroy_config(cfg);

		//get bool type
		msat_type int_type = msat_get_integer_type(env);
		assert(!MSAT_ERROR_TYPE(int_type));

		//collect the boolean variables in the predicate
		msat_decl vars[Linear_Expression::vars_names.size()];
		for (int i=0; i<Linear_Expression::vars_names.size(); ++i) {
			vars[i] = msat_declare_function(env, Linear_Expression::vars_names[i].c_str(), int_type);
			assert(!MSAT_ERROR_DECL(vars[i]));
		}

		// create the interpolation groups
		int groupA = msat_create_itp_group(env);
		int groupB = msat_create_itp_group(env);
		assert(groupA != -1 && groupB != -1);

		// create and assert formula A
		//print predicate to a string in the SmtLib format
		std::string pstrA= plus->formulaToString();
#ifdef debug_fronter
		std::cout << "pstrA: " << pstrA << std::endl;
#endif
		//finally add the predicate to the environment
		msat_term formulaA = msat_from_string(env, pstrA.c_str());
		assert(!MSAT_ERROR_TERM(formulaA));
		// tell MathSAT that all subsequent formulas belong to group A
		int res = msat_set_itp_group(env, groupA);
		assert(res == 0);
		res = msat_assert_formula(env, formulaA);
		assert(res == 0);

		// create and assert formula B
		//print predicate to a string in the SmtLib format
		std::string pstrB = minus->formulaToString();
		//finally add the predicate to the environment
		msat_term formulaB = msat_from_string(env, pstrB.c_str());
		assert(!MSAT_ERROR_TERM(formulaB));
		// tell MathSAT that all subsequent formulas belong to group B
		res = msat_set_itp_group(env, groupB);
		assert(res == 0);
		res = msat_assert_formula(env, formulaB);
		assert(res == 0);

		if (msat_solve(env) == MSAT_UNSAT) {
			int groups_of_a[1];
			msat_term interpolant;
			char *s;
			groups_of_a[0] = groupA;
			interpolant = msat_get_interpolant(env, groups_of_a, 1);
			assert(!MSAT_ERROR_TERM(interpolant));
			s = msat_term_repr(interpolant);
			assert(s);
			std::string result(s);

			result=msat2smt(result);

			msat_destroy_env(env);

#ifdef debug_fronter
			std::cout << "interpolant: \n"
					<< "\t plus: " << plus << "\n"
					<< "\t minus:" << minus << "\n"
					<< "\t inter:" << result << std::endl;
//			getchar();
#endif


			return (Linear_Expression(result)).split();

		}
	}

	void print_rules(std::ostream& os)const{
		int i=0;
		for(auto& rule: rules){
			//    	if(i!=0) std::cout<< " ";
			os << rule->getRuleIdentifier()<< " ";
			i++;
		}
	}

	friend std::ostream& operator<< (std::ostream& o, const sptr& sequence);

private:
	std::string msat2smt(const std::string& source)const;


};

inline std::ostream& operator<<
		(std::ostream& o, const Sequence::sptr& sequence){

	std::list<FW_Constraint::sptr>::const_iterator exact_it= sequence->exact_constraints.begin();
	std::list<Ideal::sptr>::const_iterator approx_it= sequence->approximated_ideals.begin();
	std::list<Rule::sptr>::const_iterator  rule_it= sequence->rules.begin();

	//the initial state
	if(exact_it!=sequence->exact_constraints.end()){
		assert(approx_it!=sequence->approximated_ideals.end());
		o << /*"exact:" <<*/ *exact_it /*<< "\n approx:" << *approx_it */<< std::endl;
		++exact_it; ++approx_it;
	}
	//next states
	while(exact_it!=sequence->exact_constraints.end()) {
		assert(approx_it!=sequence->approximated_ideals.end());
		assert(rule_it!=sequence->rules.end());
		o << "\nrule: " << (*rule_it)->getRuleIdentifier() << std::endl ;
		o << /*"exact:" <<*/ *exact_it /*<< "\n approx:" << *approx_it */<< std::endl;
		++exact_it; ++approx_it;++rule_it;
	}
	return o;
}

std::vector<std::string> from_msat2smt={"_int","`"};
std::vector<std::string> to_msat2smt={"",""};

std::string Sequence::msat2smt(const std::string& source)const{
	std::string result;
	for(std::string var_name: Linear_Expression::vars_names){
		result+="(declare-const ";
		result+=var_name;
		result+=" Int) ";
	}
	result+="(assert ";
	result+= source;
	result+=")";

	for(int c=0; c<from_msat2smt.size(); ++c){
		size_t index=0;
		while(true){
			index = result.find(from_msat2smt[c]);
			if(index == std::string::npos)
				break;
			result.replace(index, (from_msat2smt[c]).size(), to_msat2smt[c]);
		}
	}
	return result;
}


#endif /* _ZAAMA_SEQUENCE_HPP_ */

