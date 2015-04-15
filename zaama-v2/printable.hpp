/*
    Copyright (C) 2006 Ben Henda Noomene and Rezine Ahmed

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

    Contact information: Ben Henda Noomene <Noomene.BenHenda@it.uu.se> and
                         Rezine Ahmed <rezine.ahmed@gmail.com>  
*/

/**@file printable.h
 *ADT for printing classes.
 *This class is used later to overload the operator "<<". Thus any derived class
 *can be printed.
 *
 *@author Noomene Ben-Henda
 */
#ifndef PRINTABLE_H
#define PRINTABLE_H
#include<iostream>

/**
 * This class implements a general way of printing different derived classes.
 */
class Printable
{
 public:
  virtual void print(std::ostream& out) const=0; //!< print function.

  virtual ~Printable(){} //!< destructor.
};

/**
 *Operator "<<".
 *Prints any derived class from Printable
 *@param an output stream
 *@param a derived class from Printable
 *@return the output stream modified
 */
inline std::ostream& operator << (std::ostream& out , const Printable& p)
{
  p.print(out);
  return out;
}

#endif
