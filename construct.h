// Copyright (C) 2019 Pieter Goetschalckx
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef CONSTRUCT_H_
#define CONSTRUCT_H_

#include "decogen.h"

void construct1(PreDeco *pd, Edge *edge);
void construct2(PreDeco *pd, Edge *leftedge, Edge *rightedge);
void construct3(PreDeco *pd, Edge *leftedge, Edge *rightedge);
void construct4(PreDeco *pd, Edge *leftedge, Edge *rightedge);
void construct5(PreDeco *pd, Edge *edge);
void construct6(PreDeco *pd, Edge *edge);
void construct7(PreDeco *pd, Edge *edge);
void construct7b(PreDeco *pd, Edge *edge);
void construct8(PreDeco *pd, Edge *edge);
void construct9(PreDeco *pd, Edge *edge);
void construct10(PreDeco *pd, Edge *edge);

void destruct1(PreDeco *pd, Edge *edge);
void destruct2(PreDeco *pd, Edge *leftedge, Edge *rightedge);
void destruct3(PreDeco *pd, Edge *leftedge, Edge *rightedge);
void destruct4(PreDeco *pd, Edge *leftedge, Edge *rightedge);
void destruct5(PreDeco *pd, Edge *edge);
void destruct6(PreDeco *pd, Edge *edge);
void destruct7(PreDeco *pd, Edge *edge);
void destruct7b(PreDeco *pd, Edge *edge);
void destruct8(PreDeco *pd, Edge *edge);
void destruct9(PreDeco *pd, Edge *edge);
void destruct10(PreDeco *pd, Edge *edge);

#endif
