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

#include <cstdlib>

#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <set>
#include <string.h>

//############# not needed after parsing
//#include "linear_expression.hpp"
//#include "aux.hpp"
//#include "fw_constraint.hpp"
//#include "bw_constraint.hpp"
//#include "ideal.hpp"
//##############

int line_no = 0;
extern int yyparse();
extern FILE *yyin;
extern int yy_scan_string(const char *);



int main(int argc, char** argv)
{

	//	find_min_elems();
//		int data4[] = {0,0,0};
//		Point point4(3);
//		point4.data = data4;
	//	double coef3[] = {0.5,1,0};
	//	Partition p3(coef3, LARGER_EQUAL, 3,3);//0.5x+y>=3
	//	cplex_find_min_elems(point4,p3);
	//
	//	return 0;

	if(argc!=2){
		std::cout << "./zaama file_name" << std::endl;
		exit(1);
	}
	yyin = fopen(argv[1], "r");
	if(yyin!=NULL){
		yyparse();
	}
	else
		std::cerr << "could not open file: " << argv[1] << "\n";
	return 0;
}

