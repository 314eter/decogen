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
#include "planar_code.h"

static void compute_deco_code(PreDeco *pd, unsigned char *code, int dual) {
  int number[pd->size];

  compute_planar_code(pd, code, number);

  code += 1 + pd->size + pd->nedges;

  Edge *edge = pd->v1;
  *code = dual + 2 * (edge->label != 4);
  code++;

  *code = number[edge->start]; code++;
  *code = number[edge->end]; code++;

  do {
    if (edge == pd->v2 || edge == pd->v0) { *code = 0; code++; }
    edge = edge->next->inverse;
    if (edge->label == 1) {
      *code = 1; code++;
    } else if (edge->label == 3) {
      *code = 2; code++;
      edge = edge->next->inverse;
    }
  } while (edge != pd->v1);
  *code = 0; code++;
}

void write_deco_header() {
  unsigned char header[13] = ">>deco_code<<";
  fwrite(header, sizeof(unsigned char), 13, OUTFILE);
}

void write_deco_code(PreDeco *pd) {
  int size = 5 * pd->size + 3 - pd->extensions - (pd->v1->label == 4);

  unsigned char code[size];

  compute_deco_code(pd, code, 0);
  fwrite(code, sizeof(unsigned char), size, OUTFILE);

  compute_deco_code(pd, code, 1);
  fwrite(code, sizeof(unsigned char), size, OUTFILE);
}
