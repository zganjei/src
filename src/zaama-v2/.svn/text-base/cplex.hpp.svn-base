/*
    cplex.hpp

    Created on: Jun 24, 2014

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

#ifndef CPLEX_HPP_
#define CPLEX_HPP_

#include <cstdlib>

#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <string.h>
#include <ilcplex/ilocplex.h>
#include <boost/functional/hash.hpp>

#include "linear_expression.hpp"

//#define debug_does_intersect
//#define debug_min

ILOSTLBEGIN//for cplex
typedef enum {EQUAL, NOT_EQUAL, SMALLER, SMALLER_EQUAL, LARGER, LARGER_EQUAL} Comparator;

class Point{
public:
	int size;
	int* data;
	std::set<unsigned> domain_indices;//pc indices that we need for comparing two points, computing weight , etc
	bool is_empty;
	Point(int _size,std::set<unsigned> _domain_indices):size(_size),domain_indices(_domain_indices),is_empty(true){
		assert(!domain_indices.empty());
		data = (int*)malloc(size*sizeof(int));
	}

	Point(const Point& other):is_empty(false){
		size = other.size;
		data = (int*)malloc(size*sizeof(int));
		for (int i=0;i<size;i++)
			data[i] = other.data[i];
		domain_indices = other.domain_indices;
	}

	Point():is_empty(true),size(0){}

	int get_weight()const{
		int result=0;
		for(unsigned i:domain_indices)
		{
			if(i<size)
				result +=data[i];
		}

		return result;
	}

	size_t get_key() const{
//		int key=0;
//		int count=0;
		int key_vars_size = size-domain_indices.size();
		int key_vals[key_vars_size];

		int j=0;
		for(int i=0;i<size;i++){
			if(domain_indices.find(i)==domain_indices.end()){//key var index
				key_vals[j]=data[i];
				j++;
			}
		}

//		getchar();

		size_t hash = boost::hash_range(key_vals, key_vals+key_vars_size);
//		for(unsigned i=0;i<size;i++){
//			if(domain_indices.find(i)==domain_indices.end()){
//				key = key*1000+data[i];
//				if(log10 (data[i])>3){
//					std::cout<<"var at index "<<i<<"= "<<data[i]<<std::endl;
//				}
//				assert(log10 (data[i])<=3);
//				//it works for cases where for example s is always in 0-999
//			}
//		}
		//		std::cout<<"key is "<<key<<std::endl;
//		return key;

		return hash;

	}


	bool operator<(const Point& other) const{

		for (int i =0;i<size;i++){
			if(domain_indices.find(i)!=domain_indices.end()){
				if(data[i]>=other.data[i]){
					return false;
				}
			}
			else{
				if(data[i]!=other.data[i]){
					return false;
				}
			}
		}
		return true;
	}

	bool operator==(const Point& other) const{

		assert(size==other.size);

		for (int i =0;i<size;i++){
			if(data[i]!=other.data[i])
				return false;

		}
		return true;
	}

	bool operator>=(const Point& other) const{

		assert(size==other.size);

		for (int i =0;i<size;i++){
			if(domain_indices.find(i)!=domain_indices.end()){//pc variable
				if(data[i]<other.data[i])
					return false;
			}
			else{
				if(data[i]!=other.data[i])
					return false;
			}
		}
		return true;
	}

	bool operator<=(const Point& other) const{

		assert(size==other.size);

		for (int i =0;i<size;i++){
			if(domain_indices.find(i)!=domain_indices.end()){//pc variable
				if(i<size && data[i]>other.data[i])
					return false;

			}
			else{
				if(data[i]!=other.data[i])
					return false;
			}
		}
		return true;
	}

	bool operator>(const Point& other) const{

		assert(size==other.size);

		for (int i =0;i<size;i++){
			if(domain_indices.find(i)!=domain_indices.end()){//pc variable
				if(i<size && data[i]<=other.data[i])
					return false;
			}
			else{
				if(data[i]!=other.data[i])
					return false;
			}
		}
		return true;
	}


	static void print_point(Point point,std::string msg =""){
		std::cout<<msg<<" ";
		for(int i=0;i<point.size;i++)
			std::cout<<point.data[i]<<", ";
		std::cout<<std::endl;
	}

	static void print_points(std::vector<Point> points,std::string msg =""){
		std::cout<<"sizeeee: "<<points.size()<<std::endl;
		std::cout<<msg<<" \n";

		std::cout<<"{\n";
		for(Point p: points){
			print_point(p);
		}
		std::cout<<"}\n";
	}

	friend std::ostream& operator<<(std::ostream& out, const Point& point){
		point.printOn(out);
		return out;
	}


	void printOn(std::ostream& o) const {
		for(int i=0;i<size;i++)
			o<<data[i]<<", ";
		o<<std::endl;
	}
};

class Cplex{

public:
	static IloEnv env;
	static IloIntVarArray vars;
	static IloIntVarArray aux_vars;
	static std::vector<std::string>vars_names;
	static std::vector<std::string>aux_vars_names;


	static IloConstraintArray get_point_cnstr(Point point){
		IloConstraintArray result(env);
		for(int i=0;i<point.size;i++){
			IloInt val = point.data[i];
			IloConstraint cnstr;
			if(point.domain_indices.find(i)!=point.domain_indices.end()){//pc
				cnstr = vars[i]>=val;
			}
			else{//stick. e.g. s,pcmain :)
				cnstr = vars[i]==val;
			}
			result.add(cnstr);
		}
		return result;
	}

	static std::vector<Point> find_min_elems(IloConstraintArray cnsts,std::set<unsigned>pc_indices,bool reset_later=false){
#ifdef debug_min
		std::cout<<"find_min_elems()\n";
		std::cout<<"constraints size: "<<cnsts.getSize()<<std::endl;
		std::cout<<"input constraints: \n";
		for(int j=0;j<cnsts.getSize();j++){
			std::cout<<cnsts[j]<<"\n ";
		}std::cout<<std::endl;
#endif
		std::vector<Point> result;

		try {
			IloModel model(env);
			IloExpr min_point_expr(env);
			IloIntVarArray vars_copy(env);
			vars_copy = vars;
			for(int i=0;i<vars.getSize();i++){
				vars_copy[i].setBounds(0,IloIntMax);

				min_point_expr +=vars_copy[i];//minimization criteria
			}

			model.add(IloMinimize(env, min_point_expr));

			model.add(cnsts);

			IloCplex cplex(model);
			std::ofstream logfile("cplex.log");
			cplex.setOut(env.getNullStream());
			cplex.setWarning(env.getNullStream());
			cplex.setError(env.getNullStream());

			//MIPInterval MIPDisplay SimDisplay BarDisplay NetDisplay
//			cplex.setParam(IloCplex::MIPInterval, 1000);//Controls the frequency of node logging when MIPDISPLAY is set higher than 1.
//			cplex.setParam(IloCplex::MIPDisplay, 0);//MIP node log display information-No display until optimal solution has been found
//			cplex.setParam(IloCplex::SimDisplay, 0);//No iteration messages until solution
//			cplex.setParam(IloCplex::BarDisplay, 0);//No progress information
//			cplex.setParam(IloCplex::NetDisplay, 0);//No display-Network logging display indicator
//

			if ( !cplex.solve() ) {
#ifdef debug_min
				std::cout<<"************* the intersection of constraints is empty\n";
#endif
				if(!reset_later)
					reset_environment();
				return result;//empty
			}
			while(cplex.solve()){
				IloNumArray vals(env);
#ifdef debug_min
				env.out() << "Solution status = " << cplex.getStatus() << endl;
				env.out() << "Solution object value = " << cplex.getObjValue() << endl;
#endif
				cplex.getValues(vals, vars);
#ifdef debug_min
				env.out() << "Values = " << vals << endl;
#endif
				int size= vals.getSize();
				Point point(size,pc_indices);
				IloAnd old_cnst(env);
				for(int i=0;i<vars.getSize();i++){
					old_cnst.add(vars[i]>=vals[i]);
					point.data[i]=vals[i];
				}
				model.add(IloNot(old_cnst));
				cplex = IloCplex (model);

				result.push_back(point);

			}

		}
		catch (IloException& e) {
			cerr << "Concert exception caught: " << e << endl;
		}
		catch (...) {
			cerr << "Unknown exception caught" << endl;
		}
		if(!reset_later)
			reset_environment();
		return result;
	}

	static std::vector<Point> find_min_elems(std::vector<IloConstraintArray> cnsts,std::set<unsigned>pc_indices,bool reset_later=false){
		std::vector<Point> result;
		for(IloConstraintArray cnst: cnsts){
			std::vector<Point> temp_result = find_min_elems(cnst,pc_indices,true);

			for(Point t:temp_result){
				bool repetitiv=false;
				for(Point p:result){
					if(t==p){
						repetitiv=true;
						break;
					}
				}
				if(!repetitiv)//add only unique points
					result.push_back(t);
			}
		}

		if(!reset_later)
			reset_environment();
		std::cout<<"points size: "<<result.size()<<std::endl;
//		getchar();
		return result;

	}
	static bool does_intersect(IloConstraintArray cnsts,std::set<unsigned>pc_indices){
		try {
#ifdef debug_does_intersect
			std::cout<<"does_intersect()\n";
			std::cout<<"input constraints: "<<cnsts<<std::endl;
			std::cout<<"constraints size: "<<cnsts.getSize()<<std::endl;
#endif
			IloIntVarArray vars_copy(env);

			IloExpr min_point_expr(env);
			vars_copy = vars;
			for(int i=0;i<vars.getSize();i++){
				vars_copy[i].setBounds(0,IloIntMax);
				min_point_expr +=vars_copy[i];//minimization criteria
			}
#ifdef debug_does_intersect
			std::cout<<"min expr: "<<min_point_expr<<std::endl;
#endif

			IloModel check_intersection_model(env);
			check_intersection_model.add(IloMinimize(env, min_point_expr));


#ifdef inefficient_intersect
			for(int k=0;k<cnsts.getSize();k++){
				check_intersection_model.add(cnsts[k]);
				std::cout<<"......... constraint "<<k<<": "<<cnsts[k]<<std::endl;
				IloCplex temp(check_intersection_model);

				if(temp.solve()){
					std::cout<<"...........non-empty\n";
					IloNumArray vals(env);
					temp.getValues(vals, vars_copy);
					env.out() << " Values = " << vals << endl;
				}
				else{
					std::cout<<"............empty\n";
					env.out() << "Solution status = " << temp.getStatus() << endl;
				}
			}

#endif

#ifndef debug_does_intersect
			check_intersection_model.add(cnsts);
#endif
			IloCplex cplex_check(check_intersection_model);
			std::ofstream logfile("cplex.log");
			cplex_check.setOut(env.getNullStream());
			cplex_check.setWarning(env.getNullStream());
//			cplex_check.setParam(IloCplex::MIPDisplay,0);//No node log display until optimal solution has been found

			if (cplex_check.solve()) {
				std::cout << "non-empty intersection" << endl;
#ifdef debug_does_intersect
				getchar();
#endif
				reset_environment();
				return true;

			}
			else{
				std::cout << "empty intersection" << endl;
				env.out() << "Solution status = " << cplex_check.getStatus() << endl;

				for(int j=0;j<cnsts.getSize();j++){
					std::cout<<"conflict: "<<cplex_check.getConflict(cnsts[j])<<std::endl;
				}

				reset_environment();
				return false;
			}
		}
		catch (IloException& e) {
			cerr << "Concert exception caught: " << e << endl;
		}
		catch (...) {
			cerr << "Unknown exception caught" << endl;
		}

	}

	//this function empties "environment" , thus frees memory
 static void reset_environment(){
//		std::cout<<"mem usage before: "<<env.getMemoryUsage()<<std::endl;
		env.end();
		env =IloEnv();
		vars = IloIntVarArray (env);
		aux_vars = IloIntVarArray (env);
		for(std::string name: vars_names)
			vars.add(IloIntVar(env,name.c_str()));
		for(std::string name: aux_vars_names)
			aux_vars.add(IloIntVar(env,name.c_str()));

//		std::cout<<"mem usage after freeing memory: "<<env.getMemoryUsage()<<std::endl;
//		if(env.getMemoryUsage()>40000)
//		getchar();
 }
};








#endif /* CPLEX_HPP_ */
