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

#ifndef DECOGEN_H_
#define DECOGEN_H_

#include <stdio.h>

typedef struct _Edge {
  int start;
  int end;

  struct _Edge *prev;
  struct _Edge *next;
  struct _Edge *inverse;

  int label;
  int canonical;
  int face;
} Edge;

typedef struct {
  int size;
  int nedges;

  Edge *edges;
  Edge **firstedge;

  int orbits;
  Edge **canonical;

  int n0, n1, n2, bridges, cuts;
  Edge *edge0[3], *edge1[4], *edge2[4];

  int *deg;
  int *out;

  Edge *v0, *v1, *v2;
  int extensions;
} PreDeco;

FILE *OUTFILE;

#endif
