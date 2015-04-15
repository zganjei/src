/*
    utility.hpp

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

#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <math.h>

#include "ideal.hpp"
#include "bw_constraint.hpp"
#include "fw_constraint.hpp"
#include "cplex.hpp"
#include "linear_expression.hpp"
#include "refine_info.hpp"

//#define debug_intersect
//#define debug_intersect_2
//#define debug_close

//#define lazy_refinement

class Utility{
public:
	static Ideal::MSet close(BW_Constraint::sptr bwc,Refine_Info::MSet refine_objs){//from bw_
#ifdef debug_close
		std::cout<<"Utility::close()\n";
		std::cout<<"input bwc: "<<bwc<<std::endl;
#endif
		Ideal::MSet result;

		std::vector<Linear_Expression> all_partitions = g_preorder->get_partitions();
		std::vector<std::vector<Linear_Expression>> negated_partitions=g_preorder->get_negated_partitions();

#ifdef lazy_refinement
		Refine_Info::sptr refine_obj=Refine_Info::sptr(new Refine_Info(bwc->get_point()));
		std::set<int> lazy_partition_indcs;
		std::list<Refine_Info::sptr> corresponding_refine_points =refine_objs.get_implied_by(refine_obj);
#ifdef debug_refine
		std::cout<<"size of implied by:"<<corresponding_refine_points.size()<<std::endl;
#endif

		if(corresponding_refine_points.empty()){//doesn't need any partitioning, cause there is no refinement point corresponding to this bwc
			Ideal::sptr closure =Ideal::sptr(new Ideal(bwc->get_point()));
			result.insert(closure);
			return result;
		}

		else{

			for(Refine_Info::sptr ref_info : corresponding_refine_points){

				std::set<int> indcs = ref_info->get_indcs();//*
				lazy_partition_indcs.insert(indcs.begin(),indcs.end());
#ifdef debug_refine
				std::cout<<"refine w.r.t: "<<ref_info<<std::endl;
#endif
			}
		}
		//limit the partitions being used for refinement in case of lazy refinement XXXXX *****
#endif

#ifdef lazy_refinement
		for(int combination_id=0;combination_id<pow(2,lazy_partition_indcs.size());combination_id++)//correct???
#else
			for(int combination_id=0;combination_id<pow(2,all_partitions.size());combination_id++)
#endif
			{
				std::set<int> pos_partitions;
				std::set<int> neg_partitions;

				IloOr disjunction(Cplex::env);
				for(IloConstraintArray elem:bwc->get_cplex_constraint()){
					IloAnd conjunction(Cplex::env);
					conjunction.add(elem);
					disjunction.add(conjunction);
				}

				IloConstraintArray all_cplex_ctrs(Cplex::env);
				all_cplex_ctrs.add(disjunction);


				unsigned int mask = 1 ;
#ifdef lazy_refinement
//				for(int j:lazy_partition_indcs)
				for(int j=0;j<all_partitions.size();j++)//let's consider all partitions for lazy refinement points. good?????
#else
					for(int j=0;j<all_partitions.size();j++)
#endif
					{
#ifdef debug_close
						std::cout<<"combination_id="<<combination_id<<std::endl;
						std::cout<<"mask="<<mask<<std::endl;
						std::cout<<"j="<<j<<std::endl;
						std::cout<<"combination_id & mask= "<<(combination_id & mask)<<std::endl;
#endif
						/*&& pow(2,mask)<=count*/
						if((combination_id & mask)!= 0){
							pos_partitions.insert(j);
							all_cplex_ctrs.add(all_partitions[j].get_cplex_constraint());
#ifdef debug_close
							std::cout<<"partition condition:"<<all_partitions[j].get_cplex_constraint()<<std::endl;
#endif
						}
						else{
							neg_partitions.insert(j);

							IloOr ors(Cplex::env);
							for(Linear_Expression neg:negated_partitions[j])
								ors.add(neg.get_cplex_constraint());

							all_cplex_ctrs.add(ors);
#ifdef debug_close
							std::cout<<"neg_partition condition:"<<ors<<std::endl;
#endif
						}

						mask  <<= 1;//just 1 bit shift

					}

				std::vector<Point> min_points=Cplex::find_min_elems(all_cplex_ctrs,g_preorder->get_domain_indices());

				//if point is in a different partition than all_cplex_cstrs, min_points would be empty
				for(Point min:min_points){
					Ideal::sptr closure =Ideal::sptr(new Ideal(min,pos_partitions,neg_partitions));
					result.insert(closure);

				}

			}
#ifdef debug_close
		std::cout<<"close results: \n"<<result<<std::endl;
#endif
		//		if(all_partitions.size()>=2)

		return result;
	}

	static Ideal::MSet close(BW_Constraint::MSet bw_minset,Refine_Info::MSet refine_objs){//from bw_
		Ideal::MSet result;

		BW_Constraint::MSet_it set_it = bw_minset.get_iterator();
		BW_Constraint::const_iterator it = set_it.get_current_it();

		for(;!set_it.is_end(); it = set_it.next_it())
		{
			result.insert(close((*it),refine_objs));
		}
		return result;
	}

		static FW_Constraint::sptr intersect(const FW_Constraint::sptr& fwc_1,const FW_Constraint::sptr& fwc_2) {//from ideal
#ifdef debug_intersect
		std::cout<<"intersect (fwc,fwc)\n";
		std::cout<<"input fwc1: "<<fwc_1<<std::endl;
		std::cout<<"input fwc2: "<<fwc_2<<std::endl;
#endif
		IloConstraintArray cnstr(Cplex::env);
		IloConstraintArray fwc_cnstr1(fwc_1->get_cplex_constraint());
		IloConstraintArray fwc_cnstr2(fwc_2->get_cplex_constraint());

		for(int i=0;i<fwc_cnstr1.getSize();i++)
			cnstr.add(fwc_cnstr1[i]);

		for(int i=0;i<fwc_cnstr2.getSize();i++)
			cnstr.add(fwc_cnstr2[i]);

		std::vector<Point>points= Cplex::find_min_elems(cnstr,g_preorder->get_domain_indices());
		if(points.empty()){
			std::cout<<"return intersect (fwc,ideal): false\n";

			return FW_Constraint::sptr(new FW_Constraint());//empty
		}
		std::vector<Linear_Expression> lexprs;
		std::vector<Linear_Expression> fwc_lexprs1;
		std::vector<Linear_Expression>fwc_lexprs2;

		fwc_lexprs1=fwc_1->get_all_expressions();
		fwc_lexprs2=fwc_2->get_all_expressions();

		lexprs.insert(lexprs.end(),fwc_lexprs1.begin(),fwc_lexprs1.end());
		lexprs.insert(lexprs.end(),fwc_lexprs2.begin(),fwc_lexprs2.end());

#ifdef debug_intersect
		std::cout<<"return intersect (fwc,ideal): true\n";
#endif
		return FW_Constraint::sptr(new FW_Constraint(Linear_Expression::intersect(lexprs)));
	}


	static FW_Constraint::MSet intersect(const FW_Constraint::sptr& fw_c,const Ideal::sptr& ideal) //from ideal
	{

		FW_Constraint::MSet result;

#ifdef debug_intersect_2
		std::cout<<"intersect (fwc,ideal)\n";
		std::cout<<"input fwc: "<<fw_c<<std::endl;
		std::cout<<"input ideal: "<<ideal<<std::endl;
#endif
		std::vector<Linear_Expression> this_lexprs=ideal->get_all_expressions();
		std::vector<Linear_Expression>other_lexprs=fw_c->get_all_expressions();

		int ideal_cnstr_counter=-1;//will be increased in the following for loop
		IloConstraintArray fwc_cnstr=fw_c->get_cplex_constraint();
		for(IloConstraintArray ideal_cnstr:ideal->get_cplex_constraint()){
			IloConstraintArray cnstr(Cplex::env);
			ideal_cnstr_counter++;

			std::vector<Linear_Expression> lexprs;
			for(int i=0;i<ideal_cnstr.getSize();i++){
#ifdef debug_intersect_2
				std::cout<<"ideal_cnstr[i]: "<<ideal_cnstr[i]<<std::endl;
#endif
				cnstr.add(ideal_cnstr[i]);
			}

			for(int i=0;i<fwc_cnstr.getSize();i++){
#ifdef debug_intersect_2
				std::cout<<"fw_cnstr[i]: "<<fwc_cnstr[i]<<std::endl;
#endif
				cnstr.add(fwc_cnstr[i]);
			}

			std::vector<Point>points= Cplex::find_min_elems(cnstr,g_preorder->get_domain_indices(),true);
			if(points.empty()){
				continue;
			}

			lexprs.push_back(this_lexprs[ideal_cnstr_counter]);//riskyXXXXXX
			lexprs.insert(lexprs.end(),other_lexprs.begin(),other_lexprs.end());

			result.insert(FW_Constraint::sptr(new FW_Constraint(Linear_Expression::intersect(lexprs))));

		}

#ifdef debug_intersect_2
		if(result.size()>0){
			std::cout<<"return intersect (fwc,ideal): true\n";
			//				getchar();
		}
		else
			std::cout<<"return intersect (fwc,ideal): false\n";
#endif
		Cplex::reset_environment();//****
		return result;//FW_Constraint::sptr(new FW_Constraint(Linear_Expression::intersect(lexprs)));
	}


static FW_Constraint::MSet intersect(const BW_Constraint::sptr& bwc,const Ideal::sptr& ideal) //from ideal
	{
		FW_Constraint::MSet result;
#ifdef debug_intersect
		std::cout<<"intersect (bwc,ideal)\n";
//	getchar();
		std::cout<<" bwc: "<<bwc<<std::endl;
		std::cout<<" ideal: "<<ideal<<std::endl;
#endif
		if(ideal->get_key()!= bwc->get_key())
			return result;//empty

		std::vector<Linear_Expression> this_lexprs=ideal->get_all_expressions();
//		std::cout<<"ideal lexpr ready\n";
		std::vector<Linear_Expression>other_lexprs=bwc->get_all_expressions();

#ifdef debug_intersect
		for(Linear_Expression ilexpr: this_lexprs){
			ilexpr.update_z3();
			std::cout<<"ideal lexpr:"<<ilexpr<<std::endl;
		}

		for(Linear_Expression item:other_lexprs){
			item.update_z3();
			std::cout<<"bwc lexpr: "<<item<<std::endl;
		}
#endif

		assert(bwc->get_cplex_constraint().size()==other_lexprs.size());

		int ideal_cnstr_counter=0;
		for(IloConstraintArray ideal_cnstr: ideal->get_cplex_constraint()){
			int bwc_cnstr_counter=-1;//will be increased inside the next loop
			for (IloConstraintArray bwc_cnstr: bwc->get_cplex_constraint()){
				bwc_cnstr_counter++;
				IloConstraintArray cnstr(Cplex::env);
				std::vector<Linear_Expression> lexprs;

				for(int i=0;i<ideal_cnstr.getSize();i++)
					cnstr.add(ideal_cnstr[i]);

				for(int i=0;i<bwc_cnstr.getSize();i++)
					cnstr.add(bwc_cnstr[i]);

				std::vector<Point>points= Cplex::find_min_elems(cnstr,g_preorder->get_domain_indices(),true);
				if(points.empty()){
//					return result;//FW_Constraint::sptr(new FW_Constraint());//empty
					continue;
				}
#ifdef debug_intersect
				this_lexprs[ideal_cnstr_counter].update_z3();
				other_lexprs[bwc_cnstr_counter].update_z3();

				std::cout<<"ideal lexpr at index "<<ideal_cnstr_counter<<"is\n"<<this_lexprs[ideal_cnstr_counter]<<std::endl;
				std::cout<<"bwc lexpr at index "<<bwc_cnstr_counter<<"is\n"<<other_lexprs[bwc_cnstr_counter]<<std::endl;

#endif
				lexprs.push_back(this_lexprs[ideal_cnstr_counter]);//riskyXXXXXX
				lexprs.push_back(other_lexprs[bwc_cnstr_counter]);

				Linear_Expression intrsct= Linear_Expression::intersect(lexprs);
				intrsct.update_z3();//**** we need this for insertion in Minset which requires checking entailment which requires updated z3
				result.insert(FW_Constraint::sptr(new FW_Constraint(intrsct)));

				//because this function is just used by sequence::frontier(), I decided to generate more precise FWCs, I'm not sure
				//if it is correct
//				BW_Constraint::MSet bwcs = BW_Constraint::get_constraints(Linear_Expression::intersect(lexprs));
//				BW_Constraint::MSet_it f_it = bwcs.get_iterator();
//				BW_Constraint::const_iterator f = f_it.get_current_it();
//				for(;!f_it.is_end(); f=f_it.next_it()){
//					Linear_Expression point_expr = Linear_Expression::get_exact_expression((*f)->get_point());
//					Linear_Expression fwc_expr = point_expr.intersect(bwc->get_expression());
//					fwc_expr.update_z3();
//					result.insert(FW_Constraint::sptr(new FW_Constraint(point_expr)));
//				}

			}
			ideal_cnstr_counter++;
		}

#ifdef debug_intersect
		std::cout<<"return intersect (bwc,ideal)\n";
#endif
		Cplex::reset_environment();//*****
		return result;//FW_Constraint::sptr(new FW_Constraint(Linear_Expression::intersect(lexprs)));
	}
	static FW_Constraint::MSet intersect(const BW_Constraint::MSet& cset,const Ideal::sptr& ideal) {
		std::cout<<"intersect(bwc::MSet,ideal)\n";
		FW_Constraint::MSet result;

		BW_Constraint::MSet_it cset_it = cset.get_iterator();
		BW_Constraint::const_iterator c = cset_it.get_current_it();

		for(;!cset_it.is_end(); c = cset_it.next_it())

		{
				result.insert(intersect(*c,ideal));
		}

		return result;
	}
};
#endif /* UTILITY_HPP_ */
