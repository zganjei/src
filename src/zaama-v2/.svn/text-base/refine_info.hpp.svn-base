/*
    refine_info.hpp
 
    Created on: Jul 9, 2014

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

#ifndef REFINE_INFO_HPP_
#define REFINE_INFO_HPP_

#include "cplex.hpp"
#include "minset.hpp"



class Refine_Info{
private:
	Point point;
	std::set<int> partition_indices;
	int key;
	int weight;

public:
	Refine_Info(Point _point, std::set<int> _partition_indices=std::set<int>())
	:point(_point),partition_indices(_partition_indices){

//		key = point.get_key();
		key = 0;//let's put all partitioning points in one place
		weight = point.get_weight();
	}

	typedef std::shared_ptr<Refine_Info> sptr;
	typedef  MinSet< sptr, entailmentOrder<sptr>>  MSet;
	typedef  Minset_Iterator< sptr> MSet_it;
	typedef  MSet::const_iterator const_iterator;

	int get_key(){
//		return key;
		return 0;//let's put all partitioning points in one place
	}

	int get_weight(){
		return weight;
	}

	std::set<int> get_indcs(){
		return partition_indices;
	}

	Point get_point(){
		return point;
	}

	bool entails(const Refine_Info& other)const {
		if(key!=other.key) return false;

//		if(!(point <= other.point) && !( other.point <= point))/*unrelated points*/ ///correct???
//			return false;
//
//		if(point < other.point)
//			return false;
//
//		return true;
		std::set<unsigned> domain_indices = point.domain_indices;

		for(unsigned i:domain_indices){//key vars (i.e. s,pcmain,) don't matter at this part
			if(point.data[i]<other.point.data[i])
				return false;
		}

		if(!std::includes(other.partition_indices.begin(),other.partition_indices.end(),partition_indices.begin(),partition_indices.end()))
		return false;

		return true;
	}

	bool entails(const sptr& other)const {
		assert(other);
		return entails(*other);
	}

	friend std::ostream& operator<<
			(std::ostream& out, const sptr& ref){
		ref->printOn(out);
		return out;
	}

	friend std::ostream& operator<<
			(std::ostream& out, const MSet& refset){

		MSet_it refset_it = refset.get_iterator();
		const_iterator r = refset_it.get_current_it();

		for(;!refset_it.is_end(); r = refset_it.next_it())
		{
			(*r)->printOn(out);
			out << "\n\n";
		}
		out << "\n\n";
		return out;
	}

	void printOn(std::ostream& o) const {
		o<<"ref_info key: "<<key<<"\n";
		Point::print_point(point);
		o<<"partn_indcs:";
		for(int i:partition_indices)
			o<<i<<", ";
		std::cout<<std::endl;
	}

};





#endif /* REFINE_INFO_HPP_ */
