/*
    aux.hpp
 
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

#ifndef AUX_HPP_
#define AUX_HPP_

#include <cstdlib>

#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <string.h>
#include <ilcplex/ilocplex.h>

ILOSTLBEGIN//for cplex
typedef enum {EQUAL, NOT_EQUAL, SMALLER, SMALLER_EQUAL, LARGER, LARGER_EQUAL} Comparator;
//typedef int* Point;


class Point{
public:
	int size;
	int* data;
	Point(int _size):size(_size){
		data = (int*)malloc(size*sizeof(int));
	}

	Point(const Point& other){
		size = other.size;
		data = (int*)malloc(size*sizeof(int));
		for (int i=0;i<size;i++)
			data[i] = other.data[i];
	}

	bool operator<(const Point& other) const{

		return ! (*this == other);//i did this on purpose
	}

	bool operator==(const Point& other) const{

		assert(size==other.size);

		for(int i=0;i<size;i++){
			if(data[i]!=other.data[i])
				return false;

		}
		return true;
	}

	bool operator>=(const Point& other) const{

		assert(size==other.size);

		for(int i=0;i<size;i++){
			if(data[i]<other.data[i])
				return false;

		}
		return true;
	}

	bool operator>(const Point& other) const{

		assert(size==other.size);

		for(int i=0;i<size;i++){
			if(data[i]<=other.data[i])
				return false;

		}
		return true;
	}


	static void print_point(Point point,std::string msg =""){
		std::cout<<msg<<" ";
		for(int i=0;i<point.size;i++)
			std::cout<<point.data[i]<<", ";
		std::cout<<std::endl;
	}

	static void print_points(std::set<Point> points,std::string msg =""){
		std::cout<<"sizeeee: "<<points.size()<<std::endl;
		std::cout<<msg<<" \n";

		std::cout<<"{\n";
		for(std::set<Point>::iterator it = points.begin();it!=points.end();it++){
			print_point(*it);
		}
		std::cout<<"}\n";
	}



	void find_min_elems_test(){//works only for octadedra, i.e. the coefficients are just 1,0,-1
		//	int size=3;
		//	int data[] = {2,10,3};
		//	Point point(3);
		//	point.data = data;
		//
		//	int coef[] = {1,0,0};
		//	Partition p1 (coef, LARGER_EQUAL, 2,size);//x>=2
		//	std::set<Point> result=p1.get_min_elem(point);
		//
		//	if(result.empty())
		//		std::cout<<"There is no result for the given point.\n";
		//	else
		//		Point::print_points(result,"resulted Points: ");
		//
		//	/////
		//	std::cout<<"*********************\n";
		//	size=2;
		//	int data2[] = {2,1};
		//	Point point2(2);
		//	point2.data = data2;
		//
		//	int coef2[] = {1,-1};
		//	Partition p2(coef2, LARGER_EQUAL, 5,size);//x-y>=5
		//	std::set<Point> result2=p2.get_min_elem(point2);
		//
		//	if(result2.empty())
		//		std::cout<<"There is no result for the given point.\n";
		//	else{
		//
		//		Point::print_points(result2,"resulted Points:");
		//	}
		//	/////
		//	std::cout<<"*********************\n";
		//	size=3;
		//	int data3[] = {0,0,0};
		//	Point point3(3);
		//	point3.data = data3;
		//	int coef3[] = {1,1,1};
		//	Partition p3(coef3, LARGER_EQUAL, 3,size);//x+y+z>=3
		//	std::set<Point> result3=p3.get_min_elem(point3);
		//
		//	if(result3.empty())
		//		std::cout<<"There is no result for the given point.\n";
		//	else{
		//
		//		Point::print_points(result3,"resulted Points:");
		//	}
		//
		//	/////
		//	std::cout<<"*********************\n";
		//	size=3;
		//	int data4[] = {0,0,0};
		//	Point point4(size);
		//	point4.data = data4;
		//	int coef4[] = {1,-1,1};
		//	Partition p4(coef4, LARGER_EQUAL, 3,size);//x-y+z>=3
		//	std::set<Point> result4=p4.get_min_elem(point4);
		//
		//	if(result4.empty())
		//		std::cout<<"There is no result for the given point.\n";
		//	else{
		//
		//		Point::print_points(result4,"resulted Points:");
		//	}
		//
		//	/////
		//	std::cout<<"*********************\n";
		//	size=4;
		//	int data5[] = {1,0,0,0};
		//	Point point5(size);
		//	point5.data = data5;
		//	int coef5[] = {-1,-1,0,1};
		//	Partition p5(coef5, LARGER_EQUAL, 2,size);//-x-y+z-w>=2
		//	std::set<Point> result5=p5.get_min_elem(point5);
		//
		//	if(result5.empty())
		//		std::cout<<"There is no result for the given point.\n";
		//	else{
		//
		//		Point::print_points(result5,"resulted Points:");
		//	}

	}

};

class Partition {
public:
	double* coefficinets;
	Comparator comparator;
	int constant;
	int size;


	Partition(double _coefficinets[],Comparator _comparator,int _constant,int _size):size(_size){
		coefficinets=_coefficinets;
		comparator=_comparator;
		constant =_constant;
	}

	bool is_in_partition(Point input){
		int total_sum = 0;
		for(int i=0;i<size;i++){
			if(coefficinets[i]!=0)
				total_sum+=input.data[i]*coefficinets[i];
		}

		if(comparator==LARGER_EQUAL)
			return (total_sum >=constant);
		else if(comparator==SMALLER_EQUAL)
			return (total_sum <=constant);
		else
			assert(false && "I didn't consider this comparator!");
	}

	std::set<Point> get_min_elem(Point input){


		std::set<Point> all_points;
		int total_sum = 0;
		for(int i=0;i<size;i++){
			if(coefficinets[i]!=0)
				total_sum+=input.data[i]*coefficinets[i];
		}

		if(( comparator == LARGER_EQUAL && total_sum >=constant) ||
				(comparator == SMALLER_EQUAL && total_sum <=constant) ){//regardless of is_multi_uniform or !is_multi_uniform

			all_points.insert(input);
			return all_points;
		}

		for (int index =0; index<size;index++){

			if(coefficinets[index]==0)//when coef is zero, we must disregard that axis
				continue;

			Point mapped_point(input);

			mapped_point.data[index] = (constant - total_sum + input.data[index])/coefficinets[index];

			if(mapped_point.data[index] < 0)//values must be non-negative because they correspond to the number of threads
				continue;
			all_points.insert(mapped_point);

			Point::print_point(input,"input:");
			Point::print_point(mapped_point,"mapped point:");

			if(mapped_point >= input){//we want to close upwards, so we don't want points smaller that input
				int mask[size];
				for(int j=0;j<size;j++)
					if(coefficinets[j]!=0)
						mask[j]=1;
					else
						mask[j]=0;

				mask[index]=0;//means: don't change x0, add the value i to other xj s


				for(int value=1;value<=(mapped_point.data[index]-input.data[index]);value++){
					Point new_input(mapped_point);
					//				copy (mapped_point.data,new_input.data, size );
					new_input.data[0]-=value;
					if(new_input.data[0]<0)//values must be non-negative because they correspond to the number of threads
						continue;
					//				print_point(new_input,size,"now increase for this: ");
					std::set<Point> ith_result= increase(new_input,mask,value);
					for(Point p:ith_result){
						int old_size = all_points.size();
						all_points.insert(p);
						int new_size = all_points.size();

						//						if(new_size > old_size)
						//							Point::print_point(p,"inserted:");
					}
				}

				break;//??? i think doing the computations with just one point would be enough
			}//endif

		}

	}


	std::set<Point> increase(Point input, int mask[], int total){

		std::set<Point> result;

		if(total==1){

			for(int i=0;i<size;i++){
				Point new_point(input);

				if(mask[i]==1){
					new_point.data[i]++;
					if(is_in_partition(new_point))
						result.insert(new_point);

				}

			}
		}
		else{//total >=2
			for(int i=0;i<size;i++){
				Point new_point(input);

				if(mask[i]==1){
					new_point.data[i]++;

					std::set<Point> result1= increase(new_point,mask,total-1);
					int *new_mask =(int*)malloc(size*sizeof(int));
					copy(mask,new_mask,size);
					new_mask[i]=0;
					std::set<Point> result2= increase(input,new_mask,total);

					for(Point p1:result1)
						result.insert(p1);

					for(Point p2:result2)
						result.insert(p2);

				}

			}

		}

		return result;

	}

	void cplex_find_min_elems(Point point,Partition partition){
			IloEnv env;
			try {
				IloModel model(env);
				//		IloNumVarArray vars(env);
				IloIntVarArray vars(env);
				IloExpr expr(env);
				IloExpr min_point_expr(env);
				for(int i=0;i<point.size;i++){
					vars.add(IloIntVar(env, point.data[i]));//define min value

					expr += (vars[i]*partition.coefficinets[i]);
					min_point_expr +=vars[i];//minimization criteria
				}
				IloConstraint cnst;
				if(partition.comparator==LARGER_EQUAL)
					cnst = expr>=partition.constant;
				else if (partition.comparator==SMALLER_EQUAL)
					cnst = expr<=partition.constant;

				model.add(IloMinimize(env, min_point_expr));
				model.add(cnst);


				IloCplex cplex(model);
				if ( !cplex.solve() ) {
					env.error() << "Failed to optimize LP." << endl;
					throw(-1);
				}
				while(cplex.solve()){
					IloNumArray vals(env);
					env.out() << "Solution status = " << cplex.getStatus() << endl;
					env.out() << "Solution value = " << cplex.getObjValue() << endl;
					cplex.getValues(vals, vars);
					env.out() << "Values = " << vals << endl;

					IloAnd old_cnst(env);
					for(int i=0;i<point.size;i++){
						old_cnst .add(vars[i]>=vals[i]);
					}
					model.add(IloNot(old_cnst));
					cplex = IloCplex (model);
					//			getchar();

				}
			}
			catch (IloException& e) {
				cerr << "Concert exception caught: " << e << endl;
			}
			catch (...) {
				cerr << "Unknown exception caught" << endl;
			}

			env.end();
		}
	void copy (int input[],int output[], int size){
		for(int i=0;i<size;i++)
			output[i] = input[i];
	}
};










#endif /* AUX_HPP_ */
