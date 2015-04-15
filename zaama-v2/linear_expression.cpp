/*
    linear_expression.cpp

    Created on: Jul 8, 2014

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

#include "linear_expression.hpp"

//#define debug_matrix

int row; //row of matrix to be constructed XXXX
void Linear_Expression::visit(z3::expr const & e/*,int row*/, int sign,bool has_not) {
	int row_length = 2*(vars.size()+aux_vars.size())+2;
	int all_vars_length = vars.size()+aux_vars.size();
	if (e.is_app()) {

		z3::func_decl f = e.decl();
		std::stringstream ss;
		ss<<f.name();
		std::string name = ss.str();
		unsigned num_of_args = e.num_args();

#ifdef debug_matrix
		std::cout << "application of " << f.name() << ": " << e << "\n";
#endif


		if(num_of_args==0){//it's a variable or constant, i.e. leaf node
			int index=get_index(e);
			if(name=="Int"){//it must be a constant
				int number= get_value(e);
#ifdef debug_matrix
				std::cout<<"number is "<<number<<std::endl;
#endif
				index = row_length-2;
#ifdef debug_matrix
				std::cout<<"matrix["<<row<<"]["<<index<<"]="<<sign*number<<std::endl;//test
#endif
				matrix[row][index] = matrix[row][index] + sign*number;
			}
			else if(name=="true"){
				//do nothing?
			}
			else{//it's a constant
#ifdef debug_matrix
				std::cout<<"matrix["<<row<<"]["<<index<<"]="<<sign<<std::endl;//test
#endif
				matrix[row][index] =matrix[row][index] + sign*1;//XXX 1 is not always the case
			}
		}
		else if(name=="*"){// :)
			int coefficient = get_value(e.arg(0));
			int var_index = get_index(e.arg(1));
#ifdef debug_matrix
			std::cout<<"coeff= "<<coefficient<<std::endl;
			std::cout<<"matrix["<<row<<"]["<<var_index<<"]="<<coefficient*sign<<std::endl;//test
#endif
			matrix[row][var_index]=coefficient*sign;
		}
		//node is a comparator or minus
		else if(name==">=" | name=="<=" | name=="=" | name==">" | name=="<" |name=="!=" |name=="-" ){//i.e num_args ==2

			if(name==">=") {
#ifdef debug_matrix
				std::cout<<"matrix["<<row<<"]["<<row_length-1<<"]="<<LARGER_EQUAL<<std::endl;//test
#endif
				if(!has_not )
					matrix[row][row_length-1] = LARGER_EQUAL;
				else
					matrix[row][row_length-1] = SMALLER;
			}
			else if(name=="<=") {
#ifdef debug_matrix
				std::cout<<"matrix["<<row<<"]["<<row_length-1<<"]="<<SMALLER_EQUAL<<std::endl;//test
#endif
				if(!has_not )
					matrix[row][row_length-1] = SMALLER_EQUAL;
				else
					matrix[row][row_length-1] = LARGER;
			}

			else if(name=="="){
#ifdef debug_matrix
				std::cout<<"matrix["<<row<<"]["<<row_length-1<<"]="<<EQUAL<<std::endl;//test
#endif
				if(!has_not )
					matrix[row][matrix[row].size()-1] = EQUAL;
				else
					matrix[row][matrix[row].size()-1] = NOT_EQUAL;
			}
			else if(name==">"){
				if(!has_not )
					matrix[row][matrix[row].size()-1] = LARGER;
				else
					matrix[row][matrix[row].size()-1] = SMALLER_EQUAL;
			}
			else if(name=="<"){
				if(!has_not )
					matrix[row][matrix[row].size()-1] = SMALLER;
				else
					matrix[row][matrix[row].size()-1] = LARGER_EQUAL;
			}
			else if(name=="!="){
				if(!has_not )
					matrix[row][matrix[row].size()-1] = NOT_EQUAL;
				else
					matrix[row][matrix[row].size()-1] = EQUAL;
			}

			visit(e.arg(0)/*,row*/,sign,has_not);
			visit(e.arg(1)/*,row*/,(-1)*sign,has_not);//go to the right hand
		}
		else if(name=="not"){
			visit(e.arg(0)/*,row*/,sign,true);
		}
		else{//node has 2 or more arguments
			bool incresead = false;
#ifdef debug_matrix
			std::cout<<"num_of_args = "<<num_of_args<<std::endl;
#endif
			for (unsigned i = 0; i < num_of_args; i++) {
				if( name=="and" ){//XXXX
//					sign = 1;//reseting sign in each branch
					if(has_not ){
						assert(false && "we don't handle such expression having not!");
					}
					std::stringstream ss;
					ss<<e.arg(i).decl().name();

					if( ss.str()!="and"){
#ifdef debug_matrix
						std::cout<<"************ row is "<<row<<" / available rows is "<<matrix.size()<<std::endl;
#endif
						visit(e.arg(i)/*,row*/,sign,has_not);

						matrix.push_back(std::vector<int>());
						matrix[matrix.size()-1].assign (row_length,0);
						row++;
					}
					else{//ss.str()=="and"
						visit(e.arg(i)/*,row*/,sign,has_not);
					}

				}
				else if(name=="or"){
					matrix_updated = false;
				}
				else{//e.g. +, or, ...
					if(has_not && name!="+"){//we can handle "+" having "not"
						assert(false && "we don't handle such expression having not!");
					}
					visit(e.arg(i)/*,row*/,sign,has_not);
				}
			}
		}
	}
	else if (e.is_quantifier()) {
		//	        visit(e.body(),depth+1);
		// do something
		assert(false && "we don't expect quantifiers!");
	}
	else {//I don't know why we never go to this branch!!
		assert(e.is_var());
		// do something
		std::cout<<"hello! I'm a variable\n";
	}
}
void Linear_Expression::reset_row_counter(){
	row = 0;
}
int Linear_Expression::get_value(z3::expr const & e){
	std::stringstream ss;
	ss<<e;
	std::string num_str = ss.str();
	int number;
	if(num_str.find("(")==std::string::npos)//positive number
		number = std::atoi(num_str.c_str());
	else{
		num_str.replace(num_str.find("("),1,"");
		num_str.replace(num_str.find(")"),1,"");
		num_str.replace(num_str.find("-"),1,"");
		//						std::cout<<"num_str: "<<num_str<<std::endl;
		number = std::atoi(num_str.c_str())*(-1);
	}
	return number;
}

int Linear_Expression::get_index(z3::expr const & e){
	int all_vars_length = vars.size()+aux_vars.size();

	z3::func_decl f = e.decl();
	std::stringstream ss;
	ss<<f.name();
	std::string name = ss.str();
	int index=-1;
	for(int i=0; i < vars_names.size(); ++i){
		if(vars_names[i].compare(name) == 0)
			index = i;
		else if((vars_names[i]+"_p").compare(name) == 0)
			index = all_vars_length+i;
	}
	for(int i=0; i < aux_vars_names.size(); ++i){
		if(aux_vars_names[i].compare(name) == 0)
			index = vars.size()+i;
		else if((aux_vars_names[i]+"_p").compare(name) == 0)
			index = all_vars_length+vars.size()+i;
	}

	return index;

}



