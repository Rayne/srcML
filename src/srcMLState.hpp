/*
  srcMLState.hpp

  Copyright (C) 2003-2010  SDML (www.sdml.info)

  This file is part of the srcML translator.

  The srcML translator is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  The srcML translator is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the srcML translator; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef SRCMLSTATE_H
#define SRCMLSTATE_H

#include "State.hpp"

class srcMLState : public State {
 public:

  srcMLState(State::MODE_TYPE mode, State::MODE_TYPE transmode)
    : State(mode, transmode), parencount(0), typecount(0)
    {}

  srcMLState()
    : State(), parencount(0), typecount(0)
    {}

  // parentheses count
  int getParen() const {
    return parencount;
  }

  // increment the parentheses count
  void incParen() {
    ++parencount;
  }

  // decrement the parentheses count
  void decParen() {
    --parencount;
  }

  // type count
  int getTypeCount() const {
    return typecount;
  }

  // set type count
  void setTypeCount(int n) {
    typecount = n;
  }

  // increment the type count
  void incTypeCount() {
    ++typecount;
  }

  // decrement the type count
  void decTypeCount() {
    --typecount;
  }

  ~srcMLState() {
  }

 private:
  int parencount;
  int typecount;
};

#endif
