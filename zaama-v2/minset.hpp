/*
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

  Contact information: Rezine Ahmed <rezine.ahmed@gmail.com>
 */

#ifndef _MINSET_H
#define _MINSET_H

#include <cstdlib>
#include <list>
#include "printable.hpp"
#include <set>
#include <map>
#include <memory>


//#define MIN_DEBUG


#ifdef MIN_DEBUG
#include <iostream>
#endif

///////////
template<class T , typename K=int,  class Kle = std::less<K> >
class Minset_Iterator {

public:
	//		const_iterator c_current_key_iterator;
	typedef std::list<T> ContainerT;
	typedef std::map<K, ContainerT> OrganizedContainerT;
	typedef typename ContainerT::const_iterator const_iterator;
	typedef typename ContainerT::iterator iterator;


	typedef typename OrganizedContainerT::const_iterator const_setIterator;
	typedef typename OrganizedContainerT::iterator iterator2;
private:
	int index;
	K current_key;
	const_iterator row_iterator;
	const_setIterator map_iterator;
	OrganizedContainerT store;
	bool reached_end;
public:

	Minset_Iterator(OrganizedContainerT _store):store(_store){
		if(!store.empty()){
			current_key = store.begin()->first;
			map_iterator = store.begin();
			row_iterator = map_iterator->second.begin();
			reached_end = false;
			index=0;
		}
		else
		{
			map_iterator = store.end();
			reached_end = true;
		}
	}

	void copy(Minset_Iterator<T,K,Kle > other){
		index=other.index;
		current_key=other.current_key;
		reached_end=other.reached_end;

		if(!other.reached_end){
			map_iterator=store.find(current_key);
			row_iterator=map_iterator->second.begin();
			std::advance(row_iterator, index);
		}
		else{
			map_iterator=store.end();
		}
	}

	bool is_end(){
		return reached_end;
	}

	const_iterator get_current_it ()const{
		return row_iterator;
	}
	int get_index(){
		return index;
	}
	void decrease_index(){
		if(index>0)
			index--;
	}

	const_iterator next_it (){
		index++;
		row_iterator++;
		if(row_iterator==map_iterator->second.end()){
			map_iterator++;
			if(map_iterator == store.end()){
				reached_end = true;
			}
			else{
				index=0;
				row_iterator = map_iterator->second.begin();
				current_key = map_iterator->first;
			}
		}

		return row_iterator;
	}

	K get_key(){
		return current_key;
	}

};

//! Set of minimal elements
template<class T, class le/*less/equal ?*/ = std::less_equal<T> ,typename K=int,  class Kle = std::less<K>>
		class MinSet: public Printable
		{
	friend class Minset_Iterator<T>;

public:
	typedef std::list<T> ContainerT;
	typedef std::map<K, ContainerT> OrganizedContainerT;

	class InsertStrategy
	{
	protected:
		void real_insert(ContainerT& c, const T& t) {
#if 0
			if (random() > RAND_MAX/2)
				c.push_back(t);
			else
				c.push_front(t);
#else
			c.push_back(t);
#endif
		}
	public:
		virtual bool insert(ContainerT& c, const T& t, bool blindly=false) = 0;
		virtual ~InsertStrategy() {}
	};

		public:
	class BlindInsertStrategy : public InsertStrategy
	{
	public:
		bool insert(ContainerT& c, const T& t, bool blindly=false) { this->real_insert(c, t); return true; }
	};

	class CheckCoverInsertStrategy : public InsertStrategy
	{
	public:

		bool insert(ContainerT& c, const T& new_elem, bool blindly=false) {
			if(blindly)
			{
				this->real_insert(c,new_elem);
				return true;
			}
			clean(c, new_elem);
			typename ContainerT::iterator existing;
			for (existing = c.begin(); existing != c.end() && (*existing)->get_weight() <= new_elem->get_weight(); ++existing){


				if (le()(*existing,new_elem)) {

#ifdef MIN_DEBUG
					std::cerr<<new_elem<<"was not inserted because of ("<<(*existing)<<")"<<std::endl;

#endif

					return false;
				}
			}
#ifdef MIN_DEBUG
			std::cout<<new_elem<<" will be inserted"<<std::endl;
#endif
			c.insert(existing,new_elem);//inserts t before P (which is either c.end() or the first element having weight>t.weight )
			//			this->real_insert(c,t);

			return true;

		}

		void clean(ContainerT& c, const T& new_elem) {

			for (typename ContainerT::iterator existing = c.begin(); existing != c.end() ;) {

				typename ContainerT::iterator next = existing; ++next;
				if(next!=c.end())
					assert((*existing)->get_weight()<=(*next)->get_weight());
				if (le()(new_elem,*existing) && !le()(*existing,new_elem) ) {
#ifdef MIN_DEBUG
					std::cerr<<"cleaned "<<(*existing)<<" because of \n"<<new_elem<<std::endl;
//					getchar();
#endif
					c.erase(existing);
				}
				existing = next;
			}
		}
	};

	public:
	typedef typename ContainerT::const_iterator const_iterator;
	typedef typename ContainerT::iterator iterator;
	typedef typename ContainerT::const_reverse_iterator const_reverse_iterator;
	typedef typename ContainerT::reverse_iterator reverse_iterator;


	typedef typename OrganizedContainerT::const_iterator const_iterator2;
	typedef typename OrganizedContainerT::iterator iterator2;
	typedef typename OrganizedContainerT::const_reverse_iterator const_reverse_iterator2;
	typedef typename OrganizedContainerT::reverse_iterator reverse_iterator2;

	public:
	//FIXME: insert strategies leak. => Zeinab: what does it mean??
	MinSet():
		insert_strategy(std::shared_ptr<InsertStrategy>(new CheckCoverInsertStrategy))
	{}
	MinSet(std::shared_ptr<InsertStrategy> insert_strategy):
		insert_strategy(insert_strategy)
	{};


	ContainerT get(K key){
		return store[key];
	}

	std::set<K> get_existing_keys(){
		std::set<K> keys;
		for(const_iterator2 it = store.begin();it!=store.end();it++)
			keys.insert( it->first);

		return keys;
	}
	//! try to insert t
	/*! \return true iff t could be inserted */
	bool insert(const T& t, bool blindly=false) {

		K key = t->get_key();
		if(key==-1){//we are not going to add this cstr which is unsat
#ifdef MIN_DEBUG
			std::cout<<"won't be inserted because key=-1\n";
#endif
			return false;
		}

#ifdef MIN_DEBUG
		std::cout<<"size before: "<<store[key].size()<<std::endl;
#endif
		bool result = insert_strategy->insert(store[key], t, blindly);
#ifdef MIN_DEBUG
		std::cout<<"size after insert: "<<store[key].size()<<std::endl;
#endif
		return result;

	}


	//! Insert a range
	void insert(MinSet<T,le,K,Kle> new_set, bool blindly=false)
	{
//				std::cout<<"function insert(minset)\n";
		Minset_Iterator<T,K,Kle> cur = new_set.get_iterator();
		const_iterator p = cur.get_current_it();


#if 0
		for (; !cur.is_end(); p=cur.next_it())
			insert(*p, blindly);
#else
		if(blindly)
			for (; !cur.is_end(); p=cur.next_it())
				insert(*p, true);
		else
		{
			ContainerT binserted;
			int i=0;
			for (; !cur.is_end(); p=cur.next_it())
			{
				if(!includes(*p)){
					binserted.push_back(*p);
					i++;
				}
			}
#ifdef MIN_DEBUG
			std::cout<<binserted.size()<<" elements are to be inserted among "<<size()<<std::endl;

#endif

			if(binserted.size()==0) return;

			int round=1;
			Minset_Iterator<T,K,Kle> ite =get_iterator();
			for(const_iterator mine=ite.get_current_it();
					!ite.is_end();)
			{
				//				std::cout<<"round "<<round<<std::endl;
				round++;
				Minset_Iterator<T,K,Kle> next_ite (store);
				next_ite.copy(ite);
				next_ite.next_it();

				for (typename ContainerT::const_iterator his=binserted.begin(); his != binserted.end(); ++his)
				{

					if(le()(*his, *mine))
					{
						//his < mine => erase mine
#ifdef MIN_DEBUG
						std::cout<<"mine: "<<*mine<<"\n must be removed because of his: "<<*his<<std::endl;
#endif
						next_ite.decrease_index();
						erase(ite);
						break;
					}

				}
				ite.copy(next_ite);
				mine=ite.get_current_it();
				//				getchar();
			}

			for (const_iterator his = binserted.begin();  his != binserted.end(); ++his)
			{
				insert(*his);
			}
		}
#endif
	}

	//check if T& t is already included in the old elements of the min_set
	bool includes(const T& t) const {
		Minset_Iterator<T,K,Kle> cur = get_iterator();

		for (const_iterator p = cur.get_current_it();
				!cur.is_end();
				p=cur.next_it()) {
			if (le()(*p, t)){
#ifdef MIN_DEBUG
				std::cout<<"this \n"<<*p<<"\n is included in \n"<<t<<std::endl;
//				getchar();
#endif
				return true;//correct
			}
		}
		return false;
	}

	ContainerT get_implied_by(const T& t) const {
		ContainerT result;

		Minset_Iterator<T,K,Kle> cur = get_iterator();
		for (const_iterator p = cur.get_current_it();  !cur.is_end();  p=cur.next_it()) {
			if (le()(*p, t)){
#ifdef MIN_DEBUG
				std::cout<<"this \n"<<*p<<"\n includes \n"<<t<<std::endl;
//				getchar();
#endif
				result.push_back(*p);
			}
		}
#ifdef MIN_DEBUG
		std::cout<<"container size: "<<result.size()<<std::endl;
#endif
		return result;
	}


	ContainerT get_impliers_of(const T& t) const {
		ContainerT result;

		Minset_Iterator<T,K,Kle> cur = get_iterator();
		for (const_iterator p = cur.get_current_it();  !cur.is_end();  p=cur.next_it()) {
			if (le()(t,*p)){
#ifdef MIN_DEBUG
				std::cout<<"this \n"<<*p<<"\n is included in \n"<<t<<std::endl;
//				getchar();
#endif
				result.push_back(*p);
			}
		}
#ifdef MIN_DEBUG
		std::cout<<"container size: "<<result.size()<<std::endl;
#endif
		return result;
	}

	int size() const {
		int size = 0;
		for(const_iterator2 it = store.begin();it!=store.end();it++)
			size+= it->second.size();
//		std::cout<<"minset size: "<<size<<std::endl;
		return size;
	}
	bool empty() const { return store.empty(); }


	void clear() { store.clear(); }

	void erase(Minset_Iterator<T,K,Kle> it) {
#ifdef MIN_DEBUG
		std::cout<<"erase this "<<*(it.get_current_it())<<std::endl;
		std::cout<<"before erase, size is "<<store[it.get_key()].size()<<std::endl;
#endif
		iterator it2 = store[it.get_key()].begin();
		std::advance(it2, it.get_index());
		store[it.get_key()].erase(it2);

#ifdef MIN_DEBUG
		std::cout<<"after erase, size is "<<store[it.get_key()].size()<<std::endl;
//		getchar();
#endif
	}


	Minset_Iterator<T,K,Kle > get_iterator()const{
		return Minset_Iterator<T,K,Kle>(store);
	}

	void print(std::ostream& out)const{

		Minset_Iterator<T,K,Kle> p =get_iterator();
		while (!p.is_end()) {
			out<<*(p.get_current_it());
			out<<std::endl<<std::endl;
			p.next_it();
		}
		out<<"}";
	}


	private:
		std::shared_ptr<InsertStrategy> insert_strategy;
		OrganizedContainerT store;
		std::string name;



};


#endif
