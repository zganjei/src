/*
    preorder.hpp

    Created on: Jun 16, 2014

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

#ifndef PREORDER_HPP_
#define PREORDER_HPP_


class Preorder
{
public:
	Linear_Expression preorder_expr;
	std::vector<Linear_Expression> partitions;
	std::vector<std::vector<Linear_Expression>> negated_partitions;

	typedef std::shared_ptr<Preorder> sptr;


	Preorder(const Linear_Expression _expr): preorder_expr(_expr){
	}
	void constrainWith(Linear_Expression frontier)
	{
		partitions.push_back(frontier);
		negated_partitions.push_back(frontier.negate());
	}

	Linear_Expression get_expr(){
		return preorder_expr;
	}

	std::vector<Linear_Expression> get_partitions(){
		return partitions;
	}
	std::vector<std::vector<Linear_Expression>> get_negated_partitions(){
		return negated_partitions;
	}
	std::set <unsigned> get_domain_indices(){
		assert(!preorder_expr.get_domain_indices().empty());
		return preorder_expr.get_domain_indices();
	}

	friend std::ostream& operator<< (std::ostream& out, const sptr& up);

protected:
	void printOn(std::ostream& o) const{
		o << "[original: " ;
		preorder_expr.printOn(o,true);
		o<< "| partition: \n";
		int i=0;
		for(Linear_Expression p: partitions){
			o << "\t[p"<<i <<"] " ;
			p.printOn(o,true);
			o<< "\n ";
			i++;
		}
		o<< "| negated_partitions: \n";
		for(std::vector<Linear_Expression> negation_row: negated_partitions){
			for(Linear_Expression neg:negation_row){
			o << "\t[!p"<<i <<"] " ;
			neg.printOn(o,true);
			o<< "\n ";
			i++;
			}
		}
		o << "\n";
	}

};

inline std::ostream& operator<<(std::ostream& o, const Preorder::sptr& rel){
	rel->printOn(o);
	return o;
};




#endif /* PREORDER_HPP_ */
