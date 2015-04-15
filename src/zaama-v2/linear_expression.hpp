/*
    linear_expression.hpp

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

#ifndef LINEAR_EXPRESSION_HPP_
#define LINEAR_EXPRESSION_HPP_

#include<map>
#include<list>
#include<vector>
#include<math.h>
#include <boost/functional/hash.hpp>
#include "z3++.h"

#include "cplex.hpp"


using namespace std;

//#define debug_matrix
//#define debug_expression_split
//#define debug_expression_smtlib2_constructor
//#define debug_cplex_generation
//#define debug_sticks
//#define debug_rule
//#define debug_post
//#define debug_pre_point
//#define debug_z3_update


class Linear_Expression{
	//	int** matrix;//vector of vectors
	std::vector<std::vector<int> > matrix;
public:
	//	static std::vector<std::string>vars_names_;//maps var names to indices

	static z3::context cxt;

	static std::vector<std::string> vars_names;

	static std::vector<z3::expr> vars;
	static std::vector<z3::expr> primed_vars;

	static std::vector<Z3_ast> vars_ast;
	static std::vector<Z3_ast> primed_vars_ast;

	static std::vector<std::string> aux_vars_names;
	static std::vector<z3::expr> aux_vars;

	static z3::tactic qe;
	static z3::tactic simplify;
	static z3::tactic propagate;
	static z3::tactic nnf;
	static z3::tactic skip;
	static z3::tactic split_clause;
	static z3::tactic eq2ineq;
	static z3::tactic cnf;

	static z3::expr top_expr;
	static z3::expr identity_expr;


private:
	z3::expr z3_expr;
	std::set<unsigned> domain_indices;
	std::set<unsigned> image_indices;
	std::set<unsigned> aux_indices;
	bool matrix_updated;//shows whether matrix is "correct" (e.g. if a lexpr has or, it's matrix is not correct and will be after split) and updated compared to z3_expr
	bool z3_updated;//shows whether z3_expr is updated compared to matrix

public:
	bool get_z3_updated(){
		return z3_updated;
	}
	Linear_Expression(z3::expr _expr):z3_expr(_expr),z3_updated(true){
		make_matrix();
	}

	Linear_Expression(z3::expr _expr,
			std::set<unsigned> domain,
			std::set<unsigned> image,
			std::set<unsigned> aux)
	:z3_expr(_expr),
	 domain_indices(domain),
	 image_indices(image),
	 aux_indices(aux),z3_updated(true){

		make_matrix();
#ifdef debug_matrix
		printOn(std::cout);
#endif

	}

	Linear_Expression(bool universe=true):z3_expr(cxt.bool_val(universe)),z3_updated(true),matrix_updated(true){
		if(!universe)
		{
			return;//empty
		}
		make_matrix();}

	Linear_Expression(std::string smtlib2):z3_expr(cxt),z3_updated(true){
#ifdef debug_expression_smtlib2_constructor
		std::cout << "expression constructor: smtlib2 input " << smtlib2 << "\n";
#endif
		Z3_ast f=Z3_parse_smtlib2_string(cxt.operator Z3_context(),
				smtlib2.c_str(), 0,0,0,0,0,0);
		z3_expr = to_expr(cxt, f);
		make_matrix();
#ifdef debug_expression_smtlib2_constructor
		std::cout << "expression constructor: obtained expr: " << z3_expr << "\n";
#endif
	}
		Linear_Expression (std::vector<std::vector<int> > _matrix):matrix(_matrix),z3_expr(cxt.bool_val(true)),z3_updated(false),matrix_updated(true){
//			update_z3();
		}

	std::set<unsigned> get_domain_indices(){
		return domain_indices;
	}

	void make_matrix(){
		assert(z3_updated);
		std::stringstream ss;
		ss<<z3_expr;
		if(ss.str()=="false")
			return;//matrix should be left empty

		int row_length = 2*(vars_names.size()+aux_vars_names.size())+2;
		matrix.push_back(std::vector<int>());
		matrix[0].assign (row_length,0);//create one empty row at matrix with zero values

		//		std::cout<<z3_expr<<std::endl;
//		row=0;
		reset_row_counter();
		visit(z3_expr/*,0*/,1,false);
#ifdef debug_matrix
		std::cout<<"matrix after update:\n";
		printOn(std::cout);
#endif

		matrix_updated = true;
	}

	/*
 row: the row in the matrix we will fill at this iteration
 depth: the current traversing depth in the AST
 left_hand: wether or not we are on the left hand of the comparation statement
 	 	 (for variables that appear on the right hand, we must negate their sign)
	 */
	void visit(z3::expr const & e/*,row*/, int sign,bool has_not);
	int get_value(z3::expr const & e);
	int get_index(z3::expr const & e);
	void reset_row_counter();


	std::vector<Linear_Expression> negate() const{
		assert(z3_updated);
#ifdef debug_negate
		std::cout << "negating: input: " << z3_expr << "\n";
		std::cout << "negating: with top: " << top_expr << "\n";
#endif
		std::vector<Linear_Expression> result;
		z3::goal g(cxt);
		g.add(!z3_expr);
		z3::apply_result r = ( // eq2ineq &
				propagate & simplify
				& nnf & repeat (split_clause | skip)
		& simplify) (g);
#ifdef debug_negate
		std::cout << "apply_result: " << r << "\n";
#endif
		for(int i=0; i<r.size(); ++i){
			if(r[0].size()!=0){
				z3::expr e = r[i][0];
				for(int j=1; j < r[i].size(); ++j)
					e = e && r[i][j];

				z3::solver s(cxt);
				s.add(e && top_expr);

				if(s.check()==z3::sat){
					std::vector<Linear_Expression> splitted = Linear_Expression(e).split();//we split because OR is not supported in lexpr
					for(Linear_Expression lexpr: splitted){
#ifdef debug_negate
						std::cout << "\tone of negation results: " << lexpr << "\n";
						getchar();
#endif
						result.push_back(lexpr);
					}
				}
			}
		}
		return result;

	}

	bool isEmpty() 	{
		assert(matrix_updated);
		if(matrix.empty()){
#ifdef debug_empty
			std::cout<<"case 1\n";
#endif
			return true;//obviously empty!
		}
		else{
			update_z3();//****
			std::stringstream ss;
			ss<<z3_expr;
			if(ss.str()=="false"){
#ifdef debug_empty
			std::cout<<"case 2\n";
#endif
			return true;
			}
		}
		z3::solver s(cxt);
//z3 is already updated
#ifdef debug_empty
		std::cout<<"case 3\n";
		std::cout<<"z3 expr: "<<z3_expr<<std::endl;
#endif

		s.add(z3_expr&&top_expr);
		switch(s.check()){
		case z3::unsat: return true;
		case z3::sat: return false;
		case z3::unknown: assert(false && "z3 failed to check emptiness of an expression!");
		}
	}
	void update_z3(){
//		std::cout<<"update_z3\n";
		assert(matrix_updated);
//		if(z3_expr == cxt.bool_val(false) && matrix.empty())
//			return;
		assert(!matrix.empty());
		std::string z3_str;

		int all_vars_length = vars.size()+aux_vars.size();
		int row_length = 2*all_vars_length+2;

		if(matrix.size()>1)
			z3_str="(and\n";

//		std::cout<<"matrix size: "<<matrix.size()<<std::endl;

		for(int i=0;i<matrix.size();i++){
			assert(matrix[i].size()==row_length);
			std::string row_str="(+";
			bool var_found=false;
			for(int index=0;index<2*all_vars_length;index++){
				std::string var_name;
				if(index<vars.size()){
					if(matrix[i][index]!=0){
						var_name=vars_names[index];
						var_found = true;

						//					else if (index<all_vars_length)
						//						var_name=aux_vars_names[index-vars.size()];//
						//					else if (index<(all_vars_length+vars.size()))
						//						var_name=vars_names[index-all_vars_length]+"_p";//
						//					else if(index<2*all_vars_length)
						//						var_name=aux_vars_names[index-(all_vars_length+vars.size())]+"_p";//

						if(matrix[i][index]==1){
							row_str += (" "+var_name);
						}
						else{
							row_str = row_str + "(* "+std::to_string(matrix[i][index])+" "+var_name+")";
						}
					}
				}
				else{//but guards have E !!!
					//there must not be any primed variable or auxiliaries in z3 of idea/bwc/fwc which is going to be updated
					if(matrix[i][index]!=0){
//						printmatrix(std::cout);
					}
//					assert(matrix[i][index]==0);
				}

			}

//			std::cout<<i<<"-row_str: "<<row_str<<std::endl;

			if(var_found){
				std::string comp;
				switch(matrix[i][2*all_vars_length+1]){
				case(EQUAL):comp="=";
				break;
				case(NOT_EQUAL):comp="!=";
				break;
				case(SMALLER):comp="<";
				break;
				case(SMALLER_EQUAL):comp="<=";
				break;
				case (LARGER):comp=">";
				break;
				case(LARGER_EQUAL):comp=">=";
				break;

				}
				z3_str =z3_str+ "("+comp+" "+row_str +" "+std::to_string(matrix[i][2*all_vars_length])+") 0)";
//				std::cout<<i<<"-row: "<<"("<<comp<<" "<<row_str <<" "<<std::to_string(matrix[i][2*all_vars_length])<<") 0)"<<std::endl;
			}

		}

		if(matrix.size()>1)
			z3_str+="\n)";

		std::string var_decl="";

		for(std::string s:vars_names){
			var_decl= var_decl +"(declare-fun "+s+"() Int)";
			var_decl= var_decl +"(declare-fun "+s+"_p() Int)";
		}
		for(std::string s:aux_vars_names){
			var_decl= var_decl +"(declare-fun "+s+"() Int)";
			var_decl= var_decl +"(declare-fun "+s+"_p() Int)";
		}

		z3_str= var_decl + "(assert "+z3_str+")";
//		std::cout<<"-z3_str is ready to be given to z3 parser";
#ifdef debug_z3_update
		std::cout<<z3_str<<std::endl;
#endif
		try{
		Z3_ast f=Z3_parse_smtlib2_string(cxt.operator Z3_context(),
				z3_str.c_str(), 0,0,0,0,0,0);
		z3_expr = to_expr(cxt, f);
#ifdef debug_z3_update
		std::cout<<"z3_expr after update: "<<z3_expr<<std::endl;
#endif

		z3_updated = true;
		}
		catch(z3::exception ex){
			std::cout << "failed: " << ex << "\n";
			exit(-1);
		}
	}


	std::set <z3::expr> get_notused_variables()const{
		std::set <unsigned> used_indices= domain_indices;
		std::set <z3::expr> notused_vars;

		for(int i=0;i<vars.size();i++){
			if(used_indices.find(i)==used_indices.end())
				notused_vars.insert(vars[i]);
		}
		return notused_vars;
	}
	//we compute this function by z3 , because it's just called once for each input
	std::vector<Linear_Expression> get_sticks(const Linear_Expression& preorder_expr)const{
		assert(z3_updated);
		std::vector<Linear_Expression> results;
#ifdef debug_sticks
		std::cout << "\nget_sticks: input:\n" << z3_expr << "\n";
#endif

		std::set <z3::expr> notused_inpreorder =preorder_expr.get_notused_variables();

		z3::solver s(cxt);
		s.add(z3_expr);
		z3::expr result = cxt.bool_val(false);
		while(s.check()==z3::sat){
			z3::model m= s.get_model();
			z3::expr min = cxt.bool_val(true);
			//      Marking marking;
			for(z3::expr var : notused_inpreorder){
				std::stringstream ss;
				ss<<m.eval(var);
				unsigned val=std::stoi(ss.str());
				min = min && (var == cxt.int_val(val));
				//	marking[var]=val;//?
#ifdef debug_sticks
				std::cout << var << "---->" << val << "\n";
				//	getchar();
#endif
			}
			result = result || min;
			Linear_Expression stick(min);
			results.push_back(stick);
			s.add(!min);
#ifdef debug_sticks
			std::cout << "\n";
#endif
		}

#ifdef debug_sticks
		std::cout << result<<"\n";

		for(Linear_Expression lexpr: results)
			std::cout<<"--->"<<lexpr<<std::endl;
#endif


		return results;
	}
	//this function is for computing pre of a partition, or can be used in case of a broadcast
	static std::vector<Linear_Expression> pre(Linear_Expression rule, Linear_Expression from){
		from.update_z3();
		//-------------------------------------------
#ifdef debug_pred
		std::cout << "pred: input " << from << std::endl;
		std::cout << "pred: rule " << rule << std::endl;
#endif /* debug_pred */

		z3::expr cstr_expr = from.z3_expr && top_expr;
		//todo: no need to rename all variables, tune renaming per rule.
		z3::expr primed_cstr_expr(cxt,
				z3::ast(cxt,
						Z3_substitute(cxt,
								z3::ast(cstr_expr),
								vars_names.size(),
								vars_ast.data(),
								primed_vars_ast.data())));
		primed_cstr_expr = primed_cstr_expr && top_expr;

#ifdef debug_pred
		std::cout << "renamed & conjuncted with top " << primed_cstr_expr << std::endl;
#endif /* debug_pred */

		z3::goal g(cxt);
		g.add(rule.z3_expr && primed_cstr_expr);
		z3::apply_result simplified_expr= (simplify
				& propagate
				& simplify) (g);
		z3::expr image = simplified_expr[0][0];
		for(int j=1; j < simplified_expr[0].size(); ++j)
			image = image && simplified_expr[0][j];
		for(int i=1; i<simplified_expr.size(); ++i)
			for(int j=0; j < simplified_expr[i].size(); ++j)
				image = image && simplified_expr[i][j];

#ifdef debug_pred
		std::cout << "image: " << image << std::endl;
#endif /* debug_pred */

		for(unsigned i: rule.image_indices)
			image = z3::exists(primed_vars[i], image);
		for(unsigned a: rule.aux_indices)
			image = z3::exists(aux_vars[a], image);

#ifdef debug_pred
		std::cout << "existential: " << image << std::endl;
#endif /* debug_pred */

		z3::goal domain(cxt);
		domain.add(image);
		z3::apply_result  domain_ap = (qe
				& simplify
				& propagate)(domain);
#ifdef debug_pred
		std::cout << "apply_result: " << domain_ap << std::endl;
#endif /* debug_pred */

		z3::expr r = domain_ap[0][0];
		for(int j=1; j<domain_ap[0].size(); ++j)
			r = r && domain_ap[0][j];
		for(int i=1; i<domain_ap.size(); ++i)
			for(int j=0; j<domain_ap[i].size();++j)
				r = r && domain_ap[i][j];

		r= z3::expr(cxt,
				z3::ast(cxt,
						Z3_substitute(cxt,
								z3::ast(r),
								vars_names.size(),
								primed_vars_ast.data(),
								vars_ast.data())));



		 std::vector<Linear_Expression> result = Linear_Expression(r).split();//in split, the satisfiability is checked :)
#ifdef debug_pred
		std::cout << "pre result: " << r << std::endl;
		std::cout << "pre: detailed result "<< std::endl;
				for(Linear_Expression r:result){
					std::cout<<r<<std::endl<<std::endl;
				}

#endif /* debug_pred */

		return result;

	}
	/*
 because we are computing the pre, we consider the point values as primed values and try
 to find the value for unprimed variables
	 */
	static std::vector<Linear_Expression> pre(Linear_Expression rule_lexpr, Point point){
#ifdef debug_pre_point
		std::cout<<"rule:"<<rule_lexpr<<std::endl;
#endif
		assert(point.size == vars.size());
		std::vector<Linear_Expression> result;

		int row_len = 2*(vars.size()+aux_vars.size())+2;
		int vars_count =vars.size()+aux_vars.size();
		if(is_broadcast(rule_lexpr)){
		//call z3 (for pre of broadcast)
			Linear_Expression point_lexpr= get_expression(point);
			std::vector<Linear_Expression> brdcst_pre = pre(rule_lexpr,point_lexpr);
#ifdef debug_pre_point
			for(Linear_Expression bp: brdcst_pre)
				std::cout<<"pre with z3: "<<bp<<std::endl;
//			if(!brdcst_pre.empty())
//			getchar();
#endif
			return brdcst_pre;
		}
		else{
			Point single_result(point);
			for(int row=0;row<rule_lexpr.matrix.size();row++){
				int computed =0;
				bool unkown_found = false;
				int index_of_unknown;
				if(rule_lexpr.matrix[row][row_len-1]/*comparator*/==EQUAL ){
				bool is_a_guard = is_guard(rule_lexpr.matrix[row],vars_count);//this boolean variable distinguishes a guard (pc00 = 0) from a formula (pc00'=pc00 -1)
#ifdef debug_pre_point
					std::cout<<"equation found at row "<<row<<std::endl;
#endif
					for(int column=0;column<row_len;column++)
					{
						if(column<vars.size() && rule_lexpr.matrix[row][column]!=0){
#ifdef debug_pre_point
							std::cout<<"isguard: "<<is_a_guard<<std::endl;
#endif
							if(unkown_found && !is_a_guard){//bad event!
								std::cout<<"row "<<row<<std::endl;
								std::cout<<rule_lexpr<<std::endl;
								rule_lexpr.printmatrix(std::cout);
								std::cout<<"(!unkown_found || is_a_guard) &&we just solve equations with one unknown! we'll call the other pre function"<<std::endl;
//								getchar();
								Linear_Expression point_lexpr= get_expression(point);
								std::vector<Linear_Expression> brdcst_pre = pre(rule_lexpr,point_lexpr);
								return brdcst_pre;
							}
							assert( (!unkown_found || is_a_guard) &&"we just solve equations with one unknown!");//XXX
							index_of_unknown = column;
							unkown_found = true;
						}
						else if(column>=vars_count && column!=(row_len-2)/*constant*/ && rule_lexpr.matrix[row][column]!=0 )//primed var coefficient
						{
							computed += (rule_lexpr.matrix[row][column]*point.data[column-vars_count]);

						}
						else if (column==(row_len-2)){//constant
							computed += (rule_lexpr.matrix[row][column]);
						}
					}
					if(unkown_found){
#ifdef debug_pre_point
						std::cout<<"unknown found at index "<<index_of_unknown<<std::endl;
						std::cout<<"result.data["<<index_of_unknown<<"]="<<max(0,computed/(rule_lexpr.matrix[row][index_of_unknown]*(-1)))<<std::endl;
#endif
						if(!is_a_guard || index_of_unknown==0 /*s*/){
#ifdef debug_pre_point
							std::cout<<"computed: "<<computed<<" matrix val: "<<rule_lexpr.matrix[row][index_of_unknown]<<std::endl;
#endif
//							single_result.data[index_of_unknown]=max(0,computed/(rule_lexpr.matrix[row][index_of_unknown]*(-1)));//no negative result
							single_result.data[index_of_unknown]=computed/(rule_lexpr.matrix[row][index_of_unknown]*(-1));//which one?????
						}
					}
				}
				else if(rule_lexpr.matrix[row][row_len-1]/*comparator*/!=EQUAL && !is_guard(rule_lexpr.matrix[row],vars_count)){
					assert(false && "we dont handle non-equalities!");
				}

			}//end of rows

#ifdef debug_pre_point
			Point::print_point(single_result,"result of pre (guard not checked at this step) : ");
#endif
			///check guards
			//WRONGXXXXX
//			for(int row=0;row<rule_lexpr.matrix.size();row++){
//				if(!is_guard(rule_lexpr.matrix[row],vars_count))
//					continue;
//				int computed =0;
//				for(int column=0;column<vars_count;column++)//only non-primed variables are checked in guard
//				{
//					computed += (rule_lexpr.matrix[row][column]*single_result.data[column]);
//				}
//				computed += (rule_lexpr.matrix[row][row_len-2]);//constant
//
//				bool satisfies_guard;
//				switch(rule_lexpr.matrix[row][row_len-1]){//comparator
//				case(EQUAL): satisfies_guard = computed ==0;
//				break;
//				case(NOT_EQUAL):satisfies_guard = computed !=0;
//				break;
//				case(SMALLER):satisfies_guard = computed <0;
//				break;
//				case(SMALLER_EQUAL):satisfies_guard = computed <=0;
//				break;
//				case (LARGER):satisfies_guard = computed >0;
//				break;
//				case(LARGER_EQUAL):satisfies_guard = computed >=0;
//				break;
//
//				}
//				if(!satisfies_guard){
//					std::cout<<"guard in row "<<row<<" is not satisfied.\n";
////					getchar();
//					return result;//empty
//				}
//			}

//			getchar();
			result.push_back(get_expression(single_result));

		}
		return result;
	}

	static bool is_guard(std::vector<int> row,int vars_count){
		assert(row.size()==(2*vars_count+2));
		for(int i=vars_count;i<2*vars_count;i++){///primed variables part
			if(row[i]!=0)//if there is a prime variable involved, then it's not a guard
				return false;
		}
		return true;
	}

	bool implies(const Linear_Expression& other)const {

//		z3::expr first ,second;
		assert(z3_updated /*|| matrix_updated*/);
//		if(!z3_updated){
//			Linear_Expression copy1(*this);
//			copy.update_z3();
//		}
		assert(other.z3_updated /*|| other.matrix_updated*/);
//		if(!other.z3_updated){
//			Linear_Expression copy2(other);
//			copy2.update_z3();
//		}
#ifdef debug_entails
		std::cout << "checking whether: \n"
				<< z3_expr << "\nentails:\n"
				<< other.z3_expr << " ... ";
#endif
		z3::solver s(cxt);
		s.add(z3_expr && !other.z3_expr);
		bool result=false;
		switch(s.check()){
		case z3::unsat: result= true; break;
		case z3::sat: result= false;break;
		case z3::unknown: assert(false && "z3 failed to check expressions' entailment!");
		}
#ifdef debug_entails
		std::cout << (result? "true\n": "false\n");
#endif
		return result;
	}

	static bool is_broadcast(Linear_Expression lexpr){
		assert(lexpr.matrix_updated);
		int all_vars_size =vars.size()+aux_vars.size();

		if(aux_vars.size()==0) return false;

		for(int row=0;row<lexpr.matrix.size();row++){
			//			if(lexpr.matrix[row][row_len-1]/*comparator*/==EQUAL)
			for(int column=vars.size();column<all_vars_size;column++)
			{
				if( lexpr.matrix[row][column]!=0)
					return true;
			}
		}

		return false;

	}
	std::vector<size_t> get_compatible_keys(Linear_Expression order){
		assert(order.z3_updated);
		std::set<unsigned> all_key_var_indices;//vars used in key
		std::set<unsigned> rule_vars = domain_indices;//vars used in rule pre/post condition
#ifdef debug_rule
		std::cout<<"find keys of: "<<z3_expr<<std::endl;
#endif
		for(int i=0;i<vars.size();i++){
#ifdef debug_rule
			//			std::cout<<"check var: "<<vars[i]<<" "<<(order.domain_indices.find(i)==order.domain_indices.end())<<std::endl;
#endif
			if(order.domain_indices.find(i)==order.domain_indices.end()){// variable not mentioned in preorder
				all_key_var_indices.insert(i);
			}
		}

		std::vector<std::vector<int>> old_all_keys;
		std::vector<std::vector<int>> new_all_keys;

		std::vector <size_t> old_keys;
//		std::vector <size_t> new_keys;

		std::vector <z3::expr> key_exprs;
		std::vector <z3::expr> new_key_exprs;

		z3::solver exact(cxt);
		exact.add(z3_expr);
		exact.check();

		if(exact.check()==z3::unsat){//unsat rule,this rule should not exist

			return std::vector<size_t>();
		}

//		old_keys.push_back(0);
//		old_all_keys.push_back({0});

		key_exprs.push_back(cxt.bool_const("true"));

		int key_array_counter=0;
		for(int index:all_key_var_indices){
//			assert(key_array_counter<=all_key_var_indices.size());
//		for(int index=0;index<vars.size();index++){
//			if(order.domain_indices.find(index)!=order.domain_indices.end())//var is not in key (is not mentioned in preorder)
//				continue;
#ifdef debug_rule
			std::cout<<"key var: "<<vars[index]<<std::endl;
#endif
			if(rule_vars.find(index)!=rule_vars.end()){//a key variable which is mentioned in rule
				std::stringstream ss;
				ss<<exact.get_model().eval(primed_vars[index]);
#ifdef debug_rule
				std::cout<<"eval result:"<<primed_vars[index]<<" = "<<ss.str()<<std::endl;
#endif
//				for(size_t key : old_keys){
//					key = key*1000+ std::stoi(ss.str());
//					new_keys.push_back(key);
//				}
//				old_keys = new_keys;
//				new_keys.clear();
				int val = std::stoi(ss.str());
				for(std::vector<int> key_vector: old_all_keys){
					key_vector.push_back(val);
//					key_array_counter++;
					new_all_keys .push_back(key_vector);
				}
				if(old_all_keys.empty())
					new_all_keys .push_back({val});

				old_all_keys = new_all_keys;
				new_all_keys.clear();

				for(z3::expr exp:key_exprs){
					exp = exp && (vars[index]==std::stoi(ss.str()));
					new_key_exprs.push_back(exp);
				}
				key_exprs = new_key_exprs;
				new_key_exprs.clear();
			}
			else{//a key variable not mentioned in the precondition of rule, we must consider all its possible values
				z3::solver range(cxt);
				range.add(top_expr);
				while(range.check()){/// FIXME what if we have pc0>=0 ???? infinit...
					std::stringstream ss;
					ss<<range.get_model().eval(vars[index]);
#ifdef debug_rule
					std::cout<<vars[index]<<" "<<ss.str()<<std::endl;
#endif
					int val = std::stoi(ss.str());
//					for(unsigned key : old_keys){
//						key = key*1000 + val;
//						new_keys.push_back(key);
//					}

					for(std::vector<int> key_vector: old_all_keys){
						key_vector.push_back(val);
//						key_array_counter++;
						new_all_keys .push_back(key_vector);
					}
					if(old_all_keys.empty())
						new_all_keys .push_back({val});

					for(z3::expr exp:key_exprs){
						exp = exp && (vars[index]==std::stoi(ss.str()));
						new_key_exprs.push_back(exp);
					}

					range.add(!(vars[index]==cxt.int_val(val)));
				}
//				old_keys = new_keys;
//				new_keys.clear();

				old_all_keys = new_all_keys;
				new_all_keys.clear();

				key_exprs = new_key_exprs;
				new_key_exprs.clear();
			}
		}

		vector <vector<int> >::iterator itr = old_all_keys.begin();
		int i=0;
		assert(old_all_keys.size()==key_exprs.size());
		while(itr!=old_all_keys.end()){
			z3::solver valid(cxt);
			valid.add(key_exprs.at(i) && top_expr);
			if(valid.check()==z3::unsat){
				//				old_keys.erase(old_keys.begin()+i);
				itr=old_all_keys.erase(itr);
			}
			else{
				++itr;
			}

			i++;
		}

		for(std::vector<int> key_array:old_all_keys){
			assert(key_array.size()==all_key_var_indices.size());
			int key_vals[key_array.size()];
			for(int i=0;i<key_array.size();i++)//copy from vector to int* for the upcomming hash function
				key_vals[i]=key_array[i];
			old_keys.push_back(boost::hash_range(key_vals, key_vals+key_array.size()));
		}

#ifdef debug_rule

		std::cout<<"all generated keys: \n";
		for(size_t k:old_keys)
			std::cout<<k<<std::endl;
		for(z3::expr exp:key_exprs)
			std::cout<<exp<<std::endl;


//		getchar();
#endif
		return old_keys;

	}


	static std::list<Linear_Expression> post( Linear_Expression rule, Linear_Expression from){

//		std::cout<<"linear_expr::post\n";
		assert(from.matrix_updated);
		from.update_z3();
		//"this" = a rule
		std::list<Linear_Expression> result;

#ifdef debug_post_detail
		std::cout << "post: input rule " << rule << std::endl;
		std::cout << "post: input from " << from << std::endl;
		std::cout << "post: dom indices: ";
		for(unsigned index: rule.domain_indices)
			std::cout << index ;
		std::cout << "\npost: ima indices: ";
		for(unsigned index: rule.image_indices)
			std::cout << index;
		std::cout << std::endl;
#endif

		z3::expr domain = rule.z3_expr && from.z3_expr;

#ifdef debug_post
		std::cout << "post: domain " << domain << std::endl;
#endif

		//now propagate and simplify before quantifier elimination
		z3::goal g(cxt);
		g.add(domain);
		z3::apply_result simplified_domain= (simplify
				& propagate
				& simplify) (g);
		domain = simplified_domain[0][0];
		for(int j=1; j < simplified_domain[0].size(); ++j)
			domain = domain && simplified_domain[0][j];
		for(int i=1; i<simplified_domain.size(); ++i)
			for(int j=0; j < simplified_domain[i].size(); ++j)
				domain = domain && simplified_domain[i][j];

		//only project away domain variables whose images are mentioned.
		for(unsigned d: rule.domain_indices)
			if(rule.image_indices.find(d)!=rule.image_indices.end())
				domain = z3::exists(vars[d], domain);
		for(unsigned a: rule.aux_indices)
			domain = z3::exists(aux_vars[a], domain);

#ifdef debug_post
		std::cout << "post: e dom " << domain << std::endl;
#endif
try{
		z3::goal im(cxt);
		im.add(domain);
		z3::apply_result  image_ap = (  simplify
				& propagate
				& qe
				& simplify
				& propagate)(im);
		z3::expr r = image_ap[0][0];
		for(int j=1; j<image_ap[0].size(); ++j)
			r = r && image_ap[0][j];
		for(int i=1; i<image_ap.size(); ++i)
			for(int j=0; j<image_ap[i].size();++j)
				r = r && image_ap[i][j];

#ifdef debug_post_detail
		std::cout << "post: app r: " << r << std::endl;
#endif

		z3::expr ri(cxt,
				z3::ast(cxt,
						Z3_substitute(cxt,
								z3::ast(r),
								vars_names.size(),
								primed_vars_ast.data(),
								vars_ast.data())));

		for (Linear_Expression lexpr:Linear_Expression(ri).split())
			result.push_back(lexpr);
#ifdef debug_post
		std::cout << "post: detailed result "<< std::endl;
		for(Linear_Expression r:result){
			std::cout<<r<<std::endl<<std::endl;
		}
		std::cout<<"********************\n";
#endif
		return result;
	}
		catch(z3::exception ex){
			std::cout << "failed: " << ex << "\n";
			exit(-1);
		}

	}

	Linear_Expression intersect(Linear_Expression other_lexpr){
		assert(matrix_updated);
		assert(other_lexpr.matrix_updated);
//		if(z3_expr==cxt.bool_val(false) || other_lexpr.z3_expr==cxt.bool_val(false))
//			return Linear_Expression(false);

		Linear_Expression result = *this;
		//do it in a wiser way, i.e. don't add repetitive rows!
		result.matrix.insert(result.matrix.end(),other_lexpr.matrix.begin(),other_lexpr.matrix.end());
#ifdef debug_intersect
		std::cout<<"intersect(lexpr) result: \n"<<result<<std::endl;
#endif

		result.z3_updated = false;//because we have changed the matrix, but have not updated z3 accordingly
		return result;

	}
	static Linear_Expression intersect(std::vector<Linear_Expression> all_lexpr){

		assert(all_lexpr.size()!=0);
		Linear_Expression result=all_lexpr[0];
#ifdef debug_intersect
		std::cout<<"intersect(vector lexpr) input0: \n"<<result<<std::endl;
#endif
		for(int i=1;i<all_lexpr.size();i++){
			result=result.intersect(all_lexpr[i]);//implicitely result.z3_updated = false;
		}
#ifdef debug_intersect
		std::cout<<"intersect(vector lexpr) result: \n"<<result<<std::endl;
#endif


		return result;

	}
	Linear_Expression intersect(z3::expr other_z3_expr){
		assert(matrix_updated);
		update_z3();

		z3::goal g(cxt);
		g.add(z3_expr && other_z3_expr);
		z3::apply_result r = (simplify & propagate & simplify)(g);
		z3::expr z3_result = r[0][0];
		for(int j=1; j < r[0].size(); ++j)
			z3_result = z3_result && r[0][j];
		for(int i=1; i<r.size(); ++i)
			for(int j=0; j < r[i].size(); ++j)
				z3_result = z3_result && r[i][j];


		Linear_Expression result(z3_result);
		return result;

	}


	std::vector<Linear_Expression> split(){
#ifdef debug_expression_split
		std::cout << "expression split input: " << *this ;
#endif
		assert(z3_updated);
		//don't update z3 here because matrix may be trash!
		std::vector<Linear_Expression> result;
		z3::goal g(cxt);
		g.add(z3_expr);
		z3::apply_result r = ( (
				eq2ineq
				&propagate//this is the one that transforms (x>=0 AND x<=0) to x=0 ,
				& simplify
				& nnf
				& repeat (split_clause | skip)
		& simplify) | skip) (g);
		assert(g.precision()==Z3_GOAL_PRECISE);

		for(int i=0; i<r.size(); ++i){
			if(r[i].size()!=0){
				z3::expr e = r[i][0];
				for(int j=1; j < r[i].size(); j++){
					e = e && r[i][j];
				}
				z3::solver s(cxt);
				s.add(e && top_expr);

				if(s.check()==z3::sat){

					result.push_back(Linear_Expression(e));//only add the satisfiable ones
#ifdef debug_expression_split
				std::cout <<"split result: "<< "\t" << e << "\n";
#endif
				}
				else{
//					std::cout<<"this is not added to split() result because it's not sat: "<<e<<std::endl;
				}

			}
		}
		return result;

	}


	static Linear_Expression get_guard(Linear_Expression rule_lexpr){
		assert(rule_lexpr.matrix_updated);
		int row_len = 2*(vars.size()+aux_vars.size())+2;
		int vars_count =vars.size()+aux_vars.size();
		std::vector<std::vector<int> > temp;

		for(int row=0;row<rule_lexpr.matrix.size();row++){

//			if(rule_lexpr.matrix[row][row_len-1]/*comparator*/==EQUAL){//just equality guards matter
				bool is_guard = true;//this boolean variable distinguishes between a guard (pc00 = 0)and a formula (pc00'=pc00 -1)
				for(int column=vars.size();column<(row_len-2)/*constant*/ ;column++)
				{
					if( rule_lexpr.matrix[row][column]!=0 )//auxiliary or primed var coefficient found
					{
						is_guard = false;

					}
				}
				if(is_guard && !is_zero(rule_lexpr.matrix[row])){
					temp.push_back(rule_lexpr.matrix[row]);
				}
//			}
		}
		if(temp.empty())
			return Linear_Expression(true);
		Linear_Expression result(temp);
#ifdef debug_guard
		std::cout<<"guard of rule: "<<result<<std::endl;
		getchar();
#endif
		return result;
	}
	static bool is_zero(std::vector<int> input){
		for(int i=0;i<input.size();i++)
			if(input[i]!=0)
				return false;
		return true;
	}

//WARNING: BUGYY function !!!!!
//static Point get_corresponding_point(Linear_Expression fw_lexpr,std::set<unsigned>_domain_indices){
//	assert(fw_lexpr.matrix_updated);
//	fw_lexpr.update_z3();
//	fw_lexpr.matrix.clear();
//	fw_lexpr.make_matrix();//now the matrix should have only vars.size() rows at most
//	//////***************************
//	assert(!fw_lexpr.isEmpty());
//
//	int row_len = 2*(vars.size()+aux_vars.size())+2;
//	int vars_count =vars.size()+aux_vars.size();
//	Point point(vars.size(),_domain_indices);
//	for(int row=0;row<fw_lexpr.matrix.size();row++){
//		assert((fw_lexpr.matrix[row][row_len-1]/*comparator*/==EQUAL || fw_lexpr.matrix[row][row_len-1]/*comparator*/==LARGER_EQUAL )&& "a point must just have == or >=");//
//		//		if(fw_lexpr.matrix[row][row_len-1]/*comparator*/==EQUAL){
//		bool data_found = false;//each matrix row must give info about just one variable
//		for(int column=0;column<(row_len-2)/*constant*/ ;column++)
//		{
//			if(column < vars_count && fw_lexpr.matrix[row][column]!=0){
//				assert(!data_found);
//				point.data[column]=fw_lexpr.matrix[row][row_len-2]*(-1);
//				data_found=true;
//			}
//			if( column>=vars_count  && fw_lexpr.matrix[row][column]!=0 )//primed var coefficient found
//			{
//				assert(false && "this lexpr doesn't look like a point!");
//
//			}
//		}
//
//		//		}
//	}
//
//	point.print_point(point,"point corresponding to fwc: ");
//	point.is_empty = false;
//			getchar();
//	return point;
//
//
//}

static Linear_Expression get_expression(Point point){
	assert(!point.is_empty);
	std::vector<std::vector<int> > point_matrix;
	int row_length = 2*(vars_names.size()+aux_vars_names.size())+2;

	for(int i=0;i<point.size;i++){
		point_matrix.push_back(std::vector<int>());
		point_matrix[i].assign (row_length,0);//create one empty row at matrix with zero values

		point_matrix[i][i]=1;//coefficient
		point_matrix[i][row_length-2]=point.data[i]*(-1);//constant
		if(point.domain_indices.find(i)==point.domain_indices.end())//key var
			point_matrix[i][row_length-1]=EQUAL;//comparator
		else
			point_matrix[i][row_length-1]=LARGER_EQUAL;//comparator
	}

	return Linear_Expression(point_matrix);

}
//gives the expression corresponding to only the point itself and not the closure
static Linear_Expression get_exact_expression(Point point){
	assert(!point.is_empty);
	std::vector<std::vector<int> > point_matrix;
	int row_length = 2*(vars_names.size()+aux_vars_names.size())+2;

	for(int i=0;i<point.size;i++){
		point_matrix.push_back(std::vector<int>());
		point_matrix[i].assign (row_length,0);//create one empty row at matrix with zero values

		point_matrix[i][i]=1;//coefficient
		point_matrix[i][row_length-2]=point.data[i]*(-1);//constant
		point_matrix[i][row_length-1]=EQUAL;//comparator
	}

	return Linear_Expression(point_matrix);

}

	IloConstraintArray get_cplex_constraint ()const{
		assert(matrix_updated);
		int row_length = 2*(vars_names.size()+aux_vars_names.size())+2;

#ifdef debug_cplex_generation
		std::cout<<"get_cplex_constraint () 1"<<std::endl;
#endif
		IloConstraintArray result(Cplex::env);

		if(matrix.empty()){
			IloConstraint empty_cnst  = IloFalse;
			result.add(empty_cnst);//false!
			return result;
		}
		for(int i=0;i<matrix.size();i++){//for each row
			IloConstraint row_cnst;
			IloExpr expr(Cplex::env);

			for(int j=0;j<(Cplex::vars.getSize());j++){//we neither consider aux vars nor primed vars in this partXXX

				if(matrix[i][j]!=0){
					expr += (Cplex::vars[j]*matrix[i][j]);
#ifdef debug_cplex_generation
					std::cout<<"an expression: "<<Cplex::vars[j]<<std::endl;
#endif
				}
			}
			expr += matrix[i][row_length-2];//constant value
			switch(matrix[i][row_length-1]){//comparator
			case(EQUAL):
					row_cnst = expr ==0;
			break;
			case(NOT_EQUAL):
					row_cnst = expr !=0;
			break;
			case(SMALLER):
					row_cnst = expr <0;
			break;
			case(SMALLER_EQUAL):
					row_cnst = expr <=0;
			break;
			case (LARGER):
					row_cnst = expr >0;
			break;
			case(LARGER_EQUAL):
					row_cnst = expr >=0;
			break;

			}
#ifdef debug_cplex_generation
			std::cout<<"row_cnst: "<<row_cnst<<std::endl;
#endif
			result.add(row_cnst);

		}
#ifdef debug_cplex_generation
		std::cout<<"returned from 1"<<std::endl;
#endif
		return result;

	}
	void printOn(std::ostream& out,bool print_z3=false)const{
		int row_length = 2*(vars_names.size()+aux_vars_names.size())+2;
		int all_vars_length = vars_names.size()+aux_vars_names.size();

		if(false/*!print_z3*/){
			out  << z3_expr <<std::endl;
			std::string label;
			std::string indentation;
			for(int column=0;column< row_length;column++){
				if(column<vars_names.size()){
					label=vars_names[column];
				}
				else if(column<all_vars_length){
					label=aux_vars_names[column-vars_names.size()];
				}
				else if(column<all_vars_length+vars_names.size()){
					label=vars_names[column-all_vars_length]+"_p";
				}
				else if(column<2*all_vars_length){
					label=aux_vars_names[column-all_vars_length-vars_names.size()]+"_p";
				}
				else if(column == 2*all_vars_length){
					label="const";
				}
				else{
					label="comp";
				}

				if(label.length()<=7)
					indentation = "\t\t";
				else
					indentation = "\t";
				//////////////////////

				std::cout<<label<<indentation;
				for(int row = 0;row<matrix.size();row++)
					out<<matrix[row][column]<<"\t";
				std::cout<<std::endl<<"__________";
				for(int i=0;i<matrix.size();i++)
					out<<"________";
				out<<std::endl;
			}
		}
		else{
			out  << z3_expr <<std::endl;
		}
	}

	void printmatrix(std::ostream& out)const{
	int row_length = 2*(vars_names.size()+aux_vars_names.size())+2;
	int all_vars_length = vars_names.size()+aux_vars_names.size();

	out  << z3_expr <<std::endl;
	std::string label;
	std::string indentation;
	for(int column=0;column< row_length;column++){
		if(column<vars_names.size()){
			label=vars_names[column];
		}
		else if(column<all_vars_length){
			label=aux_vars_names[column-vars_names.size()];
		}
		else if(column<all_vars_length+vars_names.size()){
			label=vars_names[column-all_vars_length]+"_p";
		}
		else if(column<2*all_vars_length){
			label=aux_vars_names[column-all_vars_length-vars_names.size()]+"_p";
		}
		else if(column == 2*all_vars_length){
			label="const";
		}
		else{
			label="comp";
		}

		if(label.length()<=7)
			indentation = "\t\t";
		else
			indentation = "\t";
		//////////////////////

		std::cout<<label<<indentation;
		for(int row = 0;row<matrix.size();row++)
			out<<matrix[row][column]<<"\t";
		std::cout<<std::endl<<"__________";
		for(int i=0;i<matrix.size();i++)
			out<<"________";
		out<<std::endl;
	}

}

	friend std::ostream& operator<<(std::ostream& out, const Linear_Expression& expr){
		expr.printOn(out);
		return out;
	}

	friend std::ostream& operator<<(std::ostream& out, const std::list<Linear_Expression>& exprs){
		for(Linear_Expression expr: exprs)
			expr.printOn(out);
		return out;
	}

};





#endif /* LINEAR_EXPRESSION_HPP_ */
