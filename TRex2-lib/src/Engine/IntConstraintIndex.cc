//
// This file is part of T-Rex, a Complex Event Processing Middleware.
// See http://home.dei.polimi.it/margara
//
// Authors: Alessandro Margara
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "IntConstraintIndex.h"
#include "../Common/trace-framework.hpp"

using namespace std;

IntConstraintIndex::IntConstraintIndex() {}

IntConstraintIndex::~IntConstraintIndex() {
  for (auto itc : usedConstraints)
    delete itc;
}

void IntConstraintIndex::installConstraint(Constraint& constraints,
                                           TablePred* predicate) {
  // Looks if the same constraint is already installed in the table
  IntTableConstraint* itc = getConstraint(constraints);
  if (itc == NULL) {
    // If the constraint is not found, it creates a new one ...
    itc = createConstraint(constraints);
    // ... and installs it
    installConstraint(itc);
  }
  // In both cases connects the predicate with the constraint
  itc->connectedPredicates.insert(predicate);
}

void IntConstraintIndex::processMessage(PubPkt* pkt, MatchingHandler& mh,
                                        map<TablePred*, int>& predCount) {
  for (int i = 0; i < pkt->getAttributesNum(); i++) {
    if (pkt->getAttribute(i).type != INT)
      continue;

    traceEvent(502);
    string name = pkt->getAttribute(i).name;
    int val = pkt->getIntAttributeVal(i);
    if (indexes.find(name) == indexes.end())
      continue;
    // Equality constraints
    auto it = indexes[name].eq.find(val);
    if (it != indexes[name].eq.end()) {
      IntTableConstraint* itc = it->second;
      processConstraint(itc, mh, predCount);
    }
    // Less than constraints (iterating in descending order)
    for (auto rit = indexes[name].lt.rbegin(); rit != indexes[name].lt.rend(); ++rit) {
      if (rit->first <= val)
        break;

      //traceEvent(31);
      IntTableConstraint* itc = rit->second;
      processConstraint(itc, mh, predCount);
    }
    // Less than or equal to constraints (iterating in descending order)
    for (auto rit = indexes[name].le.rbegin(); rit != indexes[name].le.rend(); ++rit) {
      if (rit->first < val)
        break;

      //traceEvent(32);
      IntTableConstraint* itc = rit->second;
      processConstraint(itc, mh, predCount);
    }
    // Greater than constraints (iterating in ascending order)
    for (it = indexes[name].gt.begin(); it != indexes[name].gt.end(); ++it) {
      if (it->first >= val)
        break;

      //traceEvent(33);
      IntTableConstraint* itc = it->second;
      processConstraint(itc, mh, predCount);
    }
    // Greater than or equal to constraints (iterating in ascending order)
    for (it = indexes[name].ge.begin(); it != indexes[name].ge.end(); ++it) {
      if (it->first > val)
        break;

      //traceEvent(34);
      IntTableConstraint* itc = it->second;
      processConstraint(itc, mh, predCount);
    }
    // Different from constraints (iterating in ascending order)
    for (it = indexes[name].ne.begin(); it != indexes[name].ne.end(); ++it) {
      if (it->first == val)
        continue;

      //traceEvent(35);
      IntTableConstraint* itc = it->second;
      processConstraint(itc, mh, predCount);
    }
  }
}

IntTableConstraint* IntConstraintIndex::getConstraint(Constraint& c) {
  for (auto itc : usedConstraints) {
    if (itc->op != c.op)
      continue;
    if (itc->val != c.intVal)
      continue;
    if (strcmp(itc->name, c.name) != 0)
      continue;
    return (itc);
  }
  return NULL;
}

IntTableConstraint* IntConstraintIndex::createConstraint(Constraint& c) {
  auto itc = new IntTableConstraint;
  strcpy(itc->name, c.name);
  itc->op = c.op;
  itc->val = c.intVal;
  return itc;
}

inline void IntConstraintIndex::installConstraint(IntTableConstraint* c) {
  usedConstraints.insert(c);
  string s = c->name;
  if (indexes.find(s) == indexes.end()) {
    IntOperatorsTable emptyTable;
    indexes.insert(make_pair(s, emptyTable));
  }
  if (c->op == EQ)
    indexes[s].eq.insert(make_pair(c->val, c));
  else if (c->op == LT)
    indexes[s].lt.insert(make_pair(c->val, c));
  else if (c->op == GT)
    indexes[s].gt.insert(make_pair(c->val, c));
  else if (c->op == LE)
    indexes[s].le.insert(make_pair(c->val, c));
  else if (c->op == GE)
    indexes[s].ge.insert(make_pair(c->val, c));
  else
    indexes[s].ne.insert(make_pair(c->val, c));
}

inline void IntConstraintIndex::processConstraint(
    IntTableConstraint* c, MatchingHandler& mh,
    map<TablePred*, int>& predCount) {
  for (auto it : c->connectedPredicates) {
    traceEvent(35);
    // If satisfied for the first time, sets count to 1
    if (predCount.find(it) == predCount.end())
      predCount.insert(make_pair(it, 1));
    // Otherwise increases count by one
    else
      ++predCount[it];
    if (predCount[it] == it->constraintsNum)
      addToMatchingHandler(mh, it);
  }
}
