
/*
    fixpoint.hpp

    Created on: Jun 25, 2014

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

#ifndef FIXPOINT_HPP_
#define FIXPOINT_HPP_

#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>

#include <sys/time.h>
#include <time.h>

#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>

#include "bw_constraint.hpp"
#include "fw_constraint.hpp"
#include "preorder.hpp"
#include "ideal.hpp"
#include "rule.hpp"
#include "trace.hpp"
#include "sequence.hpp"
#include "utility.hpp"

//#define stop_per_insertion
//#define stop_per_iteration
//#define stop_per_rule_per_item
//#define debug_witness
//#define debug_reachable


#define time_measuring_mode


Point refinement_point;
typedef unsigned long long timestamp_t;
static timestamp_t get_timestamp ()
{
	struct timeval now;
	gettimeofday (&now, NULL);
	return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}
std::string file_name= ("1.zaama_execution_info_"+std::to_string(get_timestamp())+".txt");
std::ofstream execution_info(file_name);

using namespace boost::logic;
extern Preorder::sptr g_preorder;
int preorder_refinement_iteration =1;
int reachability_iteration =1;



class Fixpoint{
private:
	FW_Constraint::MSet  inits;
	BW_Constraint::MSet  bad;
	Rules rules;
	Preorder::sptr preorder;
	Refine_Info::MSet refine_objects;

public:
	Fixpoint(const FW_Constraint::MSet& _init,
			const Rules _rules,
			const BW_Constraint::MSet& _bad,
			const Preorder::sptr& _preceq)
	: inits(_init), rules(_rules), bad(_bad), preorder(_preceq)
	{
		g_preorder = preorder;
		refinement_point=Point();

	}

	 //! Checks reachability of bad states.
	    /*!
	      \param sequence an empty sequence to contain the trace at the end (if any).
	      \return 0 there is no trace, 1 a trace found, otherwise unknown
	      \sa Fixpoint()
	    */

	tribool isReachable(Sequence::sptr& sequence){

		// at each iteration, compute pre_up for each cstr in old_set,
		// if not in backward_reach (that contains all generated)
		// then add it to backward_reach and new_set.
		// when done with all in old_set, empty old_set, copy new_set
		// to old_set, and empty old_set.

		Ideal::MSet old_set, new_set, backward_reach;
		Trace trace;
		FW_Constraint::sptr exact_fwd_witness;
		Ideal::sptr approx_bck_witness;
		sequence = Sequence::sptr(new Sequence);


#ifdef debug_reachable
		std::cout << " bad(BW_Constraint) " << bad << "\n";
//		getchar();
		std::cout << "init " << inits << "\n";
#endif
		while(true){


			old_set= Utility::close(bad,refine_objects);
			backward_reach= old_set;
			trace.clear();
			new_set.clear();

			Ideal::MSet_it old_it = old_set.get_iterator();
			Ideal::const_iterator uc_from_old_set = old_it.get_current_it();

			for(;!old_it.is_end(); uc_from_old_set = old_it.next_it())
				trace.add(*uc_from_old_set, Rule::sptr(), Ideal::sptr());

#ifdef debug_reachable
			std::cout << "\n\n\nstarting reachability"
					<< "\nwith preceq: " << preorder << "\n";//XXXXXXXX error for *preceq
#endif

			//reachability analysis

			while(!find_intersection_witnesses(inits,
					exact_fwd_witness,
					old_set,
					approx_bck_witness))
			{
#ifdef debug_reachable
				std::cout << "\nno intersection, starting new reachability iteration" << std::endl;

#endif

				if(old_set.empty()){
					std::cout << "\nold_set is empty! can't go any further" << std::endl;
					if(sequence)
						sequence->clear();
					return false;
				}

				std::set<int> keys = old_set.get_existing_keys();
				std::cout<<"map size is "<<old_set.size()<<std::endl;

				for (int k: keys){
					std::cout<<k<<"  "<<old_set.get(k).size()<<std::endl;
				}


				std::cout<<"***********round "<<preorder_refinement_iteration<<" of iterating rules****************\n";

				for(Rule::sptr rule: rules)
				{

					int rule_iteration =1;

					std::cout << "*******************************\n"<<"rule: " << rule->getRuleIdentifier()
																																					<<"\n********************************"<< std::endl;
					for(unsigned key: rule->get_compatible_keys()){

#ifdef debug_reachable
						std::cout<<"rule iteration "<<rule_iteration<<" with key "<<key<<std::endl;
#endif

						rule_iteration++;

						if(keys.find(key)==keys.end()){
//							std::cout<<"nothing with this key exists!! \n";
							continue;
						}
						Ideal::MSet::ContainerT oldset_key = old_set.get(key);

						for(Ideal::iterator uc_from_old_set = oldset_key.begin();uc_from_old_set!=oldset_key.end();++uc_from_old_set )
						{
#ifdef debug_reachable
							std::cout << "\tuc_from_old_set: " << *uc_from_old_set << std::endl;
#endif

							BW_Constraint::MSet pre=rule->pred(*uc_from_old_set);
#ifdef debug_reachable
							std::cout << "\tpre : "<< pre << std::endl;
#endif

							Ideal::MSet uppred=Utility::close(pre,refine_objects);

#ifdef debug_reachable
							std::cout << "\tuppred : "<< uppred << std::endl;

#endif
							Ideal::MSet_it up_it = uppred.get_iterator();
							Ideal::const_iterator uc_from_pre_up = up_it.get_current_it();
							for(;!up_it.is_end(); uc_from_pre_up = up_it.next_it())
							{
//// up to here ***********
								int size_before=backward_reach.size();
								bool result =  backward_reach.insert(*uc_from_pre_up);

								if(result){
									result = new_set.insert(*uc_from_pre_up); //,true);
									trace.add(*uc_from_pre_up,rule,*uc_from_old_set);//child --rule--> parent
//#ifdef debug_reachable
									std::cout<<"bwreach size after insertion is "<<backward_reach.size()<<std::endl;
									std::cout<<"new_set size after insertion is "<<new_set.size()<<std::endl;
									std::cout<<"FW:pre: \n"<<*uc_from_pre_up<<"== rule: "<<rule->getRuleIdentifier()<<" ===>\npost:\n"<<*uc_from_old_set<<std::endl;
//#endif

#ifdef stop_per_insertion
									if(rule->getRuleIdentifier()==1)
									getchar();
#endif
								}



							}
#ifdef stop_per_rule_per_item
							if(reachability_iteration==2)
								getchar();
#endif
						}

					}

				}

				old_set= new_set;
				new_set.clear();

				for(int print=0;print<10;print++)
					std::cout<<"************************************\n";

				std::cout<< "iteration:"<<reachability_iteration<<", new old_set("<<old_set.size()<<"): " << old_set << std::endl;
//////////////////////////////////////////////////////////////
#ifdef stop_per_iteration

				getchar();
#endif

				reachability_iteration=reachability_iteration+1;
			}
			//*************************************************************************************************
			//			std::vector<std::pair<Ideal::sptr,std::vector<FW_Constraint::sptr>>> all_exact_fwd_witness;
			//					find_all_intersection_witnesses(init,
			//							all_exact_fwd_witness,
			//							old_set);
			//
			//			analyze_counterexample(all_exact_fwd_witness,trace);
			//			exit(-1);
			//*************************************************************************************************

#ifdef debug_reachable
			std::cout << "done reachability\n";
			getchar();
			//the old set hasn't been emptied until the very last iteration
#endif
			sequence->clear();
			//***** counterexample analyzer *****
			//build the sequence in case of a non empty intersection init/old_set
			std::cout<<"exact fwd witness "<<exact_fwd_witness<<std::endl;
			sequence->addExactConstraint(exact_fwd_witness);
			sequence->addApproximatedIdeal(approx_bck_witness);
			Rule::sptr rule= trace.generatingRuleOf(approx_bck_witness);
			std::cout<<"----------this rule is responsible: "<<rule->getRuleIdentifier()<<std::endl;
			//			getchar();
			while(rule){
				std::cout<<"this rule: "<<rule->getRuleIdentifier()<<std::endl;
				sequence->addRule(rule);
				FW_Constraint::MSet post= rule->post(Utility::intersect(exact_fwd_witness,approx_bck_witness));
				approx_bck_witness= trace.parentOf(approx_bck_witness);

				if(!find_an_intersection_witness(post, exact_fwd_witness, approx_bck_witness)){
						sequence->addApproximatedIdeal(approx_bck_witness);
					if(post.empty()){
						std::cout<<"post is empty!\n"<<std::endl;
#ifndef time_measuring_mode
						std::cout<<"incomplete sequence"<<sequence<<std::endl;
//						getchar();
#endif
						sequence->addExactConstraint(FW_Constraint::sptr (new FW_Constraint()));//empty constraint
					}
					else{
						std::cout<<"post doesn't intersect with approx_back_witness\n"<<std::endl;
//						getchar();
						FW_Constraint::MSet_it post_it = post.get_iterator();
						FW_Constraint::const_iterator cstr_from_post = post_it.get_current_it();//XXXwhy only first one?
						sequence->addExactConstraint(*cstr_from_post);

					}

					break;
				}
				std::cout<<"exact fwd witness "<<exact_fwd_witness<<std::endl;

				sequence->addApproximatedIdeal(approx_bck_witness);
				sequence->addExactConstraint(exact_fwd_witness);
				std::cout<<"add:exact: \n"<<exact_fwd_witness<<" rule: "<<rule->getRuleIdentifier()<<" approx:\n"<<approx_bck_witness<<std::endl;

				rule= trace.generatingRuleOf(approx_bck_witness);//rule for next iteration
//											getchar();

			}
#ifdef debug_reachable
			std::cout << "sequence: \n" << sequence << "\n";
#endif
			if(!rule){
				std::ofstream zaama_trace_file("zaama-trace.txt");
				sequence->print_rules(zaama_trace_file);
#ifdef debug_reachable
				std::cout << "zaama says it's a real trace! round:" <<preorder_refinement_iteration<<std::endl;
				getchar();
#endif
				return true;

			}
			else{
				std::cout<<".....................................no trace?????!!!!\n";
			}
			//			trace.clear();//***** zeinab: empty the parent-child relations before starting next reachablity iteration

			std::vector<Linear_Expression> frontiers= sequence->getFrontier();
//			for(std::vector<Linear_Expression>)
#ifdef debug_reachable
			std::cout<<"preorder refinement iteration:"<<preorder_refinement_iteration<<std::endl;
			for(Linear_Expression frontier: frontiers)
				std::cout << "frontiers: \n" << frontier << "\n";
			std::cout<<"we are about to refine ordering\n";
//			getchar();
#endif
			execution_info<<"these new partitions added: \n";
			std::set<int> partition_indcs;
			assert(frontiers.size()>0);
			bool some_new_partition_added= false;
			for(Linear_Expression f: frontiers){
#ifndef lazy_refinement
					//in case of greedy refinement, there should not be any repetitive partition
				for(int i=0;i<preorder->get_partitions().size();i++){
					assert(!(frontiers.size()==1 && f.implies(preorder->get_partitions()[i]) &&
							preorder->get_partitions()[i].implies(f)));
				}
#else
				bool repetitive = false;
				int corresponding_index;
				for(int i=0;i<preorder->get_partitions().size();i++){
					if(f.implies(preorder->get_partitions()[i]) && preorder->get_partitions()[i].implies(f)){//old partition==new partition
							repetitive = true;
							corresponding_index = i;
							break;
					}
				}
				if(!repetitive){
					some_new_partition_added= true;
#endif
					preorder->constrainWith(f);
					execution_info<<f<<std::endl;
#ifdef lazy_refinement
					execution_info<<"refinement point: "<<refinement_point<<std::endl;
					partition_indcs.insert(preorder->get_partitions().size()-1);//index of the last added partition is added to the set
				}
				else{
					execution_info<<"a repetitive partition the same as index "<<corresponding_index<<std::endl;
					execution_info<<"refinement point: "<<refinement_point<<std::endl;
					partition_indcs.insert(corresponding_index);
				}
#endif
			}

#ifdef lazy_refinement
			execution_info<<".......lazy refinement......\n";
			update_refinement_information(partition_indcs);
//			getchar();
#endif

			g_preorder = preorder;
			//update all rules
#ifdef lazy_refinement
			if(some_new_partition_added){
#endif
				for(Rule::sptr old_rule:rules){
					old_rule->compute_pre_partitions();
				}

				std::cout<<"new order "<<preorder<<std::endl;
				execution_info<<"The new ordering: \n";
				execution_info<<preorder;
#ifdef lazy_refinement
			}
#endif

#ifndef time_measuring_mode
			getchar();
#endif

#ifdef stop_rule_bwreach_item
			getchar();
#endif
			preorder_refinement_iteration++;
		}//end of BIG while!

	}

private:
	static bool find_all_intersection_witnesses(FW_Constraint::MSet& set_A/*inits*/,
			std::vector<std::pair<Ideal::sptr,std::vector<FW_Constraint::sptr>>>& all_witnesses,
			Ideal::MSet& set_B/*bwreach*/)
	{
		std::cout<<"find_all_intersection_witnesses() "<<std::endl;
		bool found = false;

		Ideal::MSet_it b_it = set_B.get_iterator();
		Ideal::const_iterator b = b_it.get_current_it();

		for(;!b_it.is_end(); b=b_it.next_it()){

			FW_Constraint::MSet_it a_it = set_A.get_iterator();
			FW_Constraint::const_iterator a = a_it.get_current_it();

			std::vector<FW_Constraint::sptr> temp_witnesses_a;
			for(;!a_it.is_end();a= a_it.next_it()){
				FW_Constraint::MSet c= Utility::intersect(*a,*b);//optimize this with checking keys first

				if(!c.empty()){
					temp_witnesses_a.push_back(*a);

					found = true;
#ifdef debug_all_witness
					std::cout<<"intersection with init is not empty "<<std::endl;
					std::cout<<"witness in inits: "<<*a<<std::endl;
					std::cout<<"witness in bwreach: "<<*b<<std::endl;
					getchar();
#endif
				}

			}

			if(!temp_witnesses_a.empty()){
				all_witnesses.push_back(std::make_pair(*b/*witness_in_B*/,temp_witnesses_a));
			}
		}
		return found;
	}



	static bool find_intersection_witnesses(FW_Constraint::MSet& set_A,
			FW_Constraint::sptr& witness_in_A,
			Ideal::MSet& set_B,
			Ideal::sptr& witness_in_B)
	{
		std::cout<<"find_intersection_witnesses() "<<std::endl;
		bool found = false;
		FW_Constraint::MSet_it a_it = set_A.get_iterator();
		FW_Constraint::const_iterator a = a_it.get_current_it();


		for(;!a_it.is_end();a= a_it.next_it()){

			/////******
			Ideal::MSet_it b_it = set_B.get_iterator();
			Ideal::const_iterator b = b_it.get_current_it();
			////********

			for(;!b_it.is_end(); b=b_it.next_it()){
//				FW_Constraint::sptr c= Utility::intersect(*a,*b);
				FW_Constraint::MSet c= Utility::intersect(*a,*b);
				if(!c.empty()){
					witness_in_A= *a;
					witness_in_B= *b;
					found = true;
#ifdef debug_witness
					std::cout<<"intersection is not empty "<<std::endl;
					std::cout<<"witness in inits: "<<witness_in_A<<std::endl;
					std::cout<<"witness in bwreach: "<<witness_in_B<<std::endl;
					getchar();
#endif
					return found;
				}
			}
		}
		return found;
	}

//used by counter-example analyzer when making sequence
	bool find_an_intersection_witness(FW_Constraint::MSet& set_A,
			FW_Constraint::sptr& witness_in_A, const Ideal::sptr& cstr_B)
	{
		FW_Constraint::MSet_it a_it = set_A.get_iterator();
		FW_Constraint::const_iterator a = a_it.get_current_it();

		for(;!a_it.is_end(); a= a_it.next_it())
		{
//			FW_Constraint::sptr c= Utility::intersect(*a,cstr_B);
			FW_Constraint::MSet c= Utility::intersect(*a,cstr_B);

//			if(!c->isEmpty()){
			if(!c.empty()){
				witness_in_A= *a;
#ifdef debug_a_witness
				std::cout<<*a<<std::endl;
#endif
				return true;
			}
		}
		return false;
	}

	void update_refinement_information(std::set<int> partition_indcs){
		Refine_Info::sptr new_refine_obj=Refine_Info::sptr(new Refine_Info(refinement_point));
/*
 we must check previous refinement information. two cases may happen:
 case 1. there are some old refine_objs bigger than the new_refine_obj, so  new_refine_obj must inherit their partitions and add
 	 	 itself to the minset which automatically removes the old ones

 case 2. there are some old refine_objs "smaller" than the new_refine_obj, so this new point must not be added but it must only
 update the partitions of the old ones

 i think always only one of the above two cases may happen , not both. correct?????
 */

		Point::print_point(refinement_point,"new refinement point: ");
		std::list<Refine_Info::sptr> bigger_refine_points =refine_objects.get_impliers_of(new_refine_obj);
		std::list<Refine_Info::sptr> smaller_refine_points =refine_objects.get_implied_by(new_refine_obj);
		std::cout<<"size of impliers:"<<bigger_refine_points.size()<<std::endl;

		assert(smaller_refine_points.empty() || bigger_refine_points.empty());//correct???

		//case 1:
		if(!bigger_refine_points.empty()){

			for(Refine_Info::sptr ref_info : bigger_refine_points){

				std::cout<<"bigger ref_info exists: "<<ref_info<<std::endl;
				std::set<int> indcs = ref_info->get_indcs();//*
				//we don't want repetitive objects
				assert(!(ref_info->get_point()==refinement_point && std::includes(indcs.begin(),indcs.end(),partition_indcs.begin(),partition_indcs.end())));
				partition_indcs.insert(indcs.begin(),indcs.end());//merge partitioning indices if an implier is found
#ifdef debug_reachable
				getchar();
#endif
			}

			new_refine_obj=Refine_Info::sptr(new Refine_Info(refinement_point,partition_indcs));
			refine_objects.insert(new_refine_obj);//this implicitly removes the impliers
		}
		//case 2: check if this refine_obj is implied by any one
		else if (!smaller_refine_points.empty()){
			for(Refine_Info::sptr old_ref_info : smaller_refine_points){

				std::cout<<"smaller ref_info exists: "<<old_ref_info<<std::endl;
				std::set<int> new_indcs = old_ref_info->get_indcs();//*
				assert(!(old_ref_info->get_point()==refinement_point && std::includes(new_indcs.begin(),new_indcs.end(),partition_indcs.begin(),partition_indcs.end())));

				new_indcs.insert(partition_indcs.begin(),partition_indcs.end());//merge partitioning indices if a found implier

				new_refine_obj=Refine_Info::sptr(new Refine_Info(old_ref_info->get_point(),new_indcs));
				refine_objects.insert(new_refine_obj);//this implicitly removes the old ones
#ifdef debug_reachable
				getchar();
#endif
			}
		}
		//non of the above cases, i.e. no impliers, no implieds
		else{
			new_refine_obj=Refine_Info::sptr(new Refine_Info(refinement_point,partition_indcs));
			refine_objects.insert(new_refine_obj);//this implicitly removes the impliers
			std::cout<<"no impliers, no implieds for "<<new_refine_obj<<std::endl;
#ifdef debug_reachable
			getchar();
#endif

		}
	}



};


#endif /* FIXPOINT_HPP_ */
