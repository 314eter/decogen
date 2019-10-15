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

#include <stdlib.h>
#include <stdio.h>
#include "decogen.h"

void compute_planar_code(PreDeco *pd, unsigned char *code, int number[]) {
  register Edge *run;
  register int vertex;
  Edge *temp, *startedge[pd->size];
  int i, last_number, actual_number;

  for (i = 0; i < pd->size; i++) number[i] = 0;

  *code = pd->size; code++;

  if (pd->size < 2) {
    *code = 0; code++;
    return;
  }

  temp = pd->v1;
  number[temp->start] = 1;
  number[temp->end] = 2;
  last_number = 2;
  startedge[1] = temp->inverse;

  actual_number = 1;

  while (actual_number <= pd->size) {
    *code = number[temp->end]; code++;
    for (run = temp->next; run != temp; run = run->next) {
      vertex = run->end;
      if (!number[vertex]) {
        startedge[last_number++] = run->inverse;
        number[vertex] = last_number;
      }
      *code = number[vertex]; code++;
    }
    *code = 0; code++;
    temp = startedge[actual_number++];
  }
}

void write_planar_header() {
  unsigned char header[15] = ">>planar_code<<";
  fwrite(header, sizeof(unsigned char), 15, OUTFILE);
}

void write_planar_code(PreDeco *pd) {
  int size = pd->size + pd->nedges + 1;
  int number[pd->size];
  unsigned char code[size];

  compute_planar_code(pd, code, number);
  fwrite(code, sizeof(unsigned char), size, OUTFILE);
}
