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

/**@file trace.hpp
 * Whatever is needed for tracing
 *
 *@author Ahmed Rezine
 */

#ifndef _ZAAMA_TRACE_HPP_
#define _ZAAMA_TRACE_HPP_

#include <iostream>
#include <map>

#include "ideal.hpp"
#include "rule.hpp"


class childRuleParent
{
 public:

  childRuleParent(){}

  childRuleParent(const Ideal::sptr& _child, 
		  const Rule::sptr& _rule, 
		  const Ideal::sptr& _parent)
   :child(_child),rule(_rule),parent(_parent){}

  friend std::ostream& operator<< (std::ostream& out, 
				   const childRuleParent& crp);

  friend class Edges;

 private:
  Ideal::sptr child, parent;
  Rule::sptr rule;
};


class Edges : public std::map<unsigned, childRuleParent>
{
 public:

  Edges(){}

  bool add(const Ideal::sptr& child, 
	   const Rule::sptr& rule, 
	   const Ideal::sptr& parent){
//    if(find(child->get_identifier())!=end()){
//    	std::cout<<"if\n";
//      return false;
//    }
	assert(find(child->get_identifier())==end());

    operator[](child->get_identifier())=childRuleParent(child,rule,parent);

    return true;
  }

  Ideal::sptr parentOf(const Ideal::sptr& child) const{
    std::map<unsigned, childRuleParent>::const_iterator
      it=find(child->get_identifier());
    assert(it!=end());
    //    assert(it->second.parent);
    return it->second.parent;
  }
  Rule::sptr generatingRuleOf(const Ideal::sptr& child) const{
    std::map<unsigned, childRuleParent>::const_iterator
      it=find(child->get_identifier());
    assert(it!=end());
    //    assert(it->second.rule);
    return it->second.rule;
  }

};

inline std::ostream& operator<<
(std::ostream& o, const childRuleParent& crp)
{
  o<< crp.child << std::endl 
   << "via" << std::endl 
   << crp.rule << std::endl 
   << "gives" << std::endl 
   << crp.parent << std::endl ;

  return o;
};

#endif /* _ZAAMA_TRACE_HPP_ */

