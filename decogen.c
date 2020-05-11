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
#include <getopt.h>
#include "construct.h"
#include "deco_code.h"

unsigned int predecoration_used = 0;
unsigned long long precount = 0;
unsigned long long count = 0;
unsigned long long wrongfactor = 0;
unsigned long long cutvertices = 0;
unsigned long long wrongconnectivity = 0;

int OUTPUT = 0;
int ALL = 0;
int CONNECTIVITY = 3;
int RES = 0;
int MOD = 1;
int SPLIT_LEVEL = 3;
int level_index = 0;
FILE *OUTFILE;

static void testcanon_init(PreDeco *pd, Edge *edge, int representation[]) {
  register Edge *run;
  register int vertex;
  Edge *startedge[pd->size];
  int number[pd->size], i;
  int last_number, actual_number;

  for (i = 0; i < pd->size; i++) number[i] = 0;

  number[edge->start] = 1;
  number[edge->end] = 2;
  last_number = 2;
  startedge[1] = edge->inverse;

  actual_number = 1;
  while (actual_number <= pd->size) {
    for (run = edge->next; run != edge; run = run->next) {
      vertex = run->end;
      if (!number[vertex]) {
        startedge[last_number++] = run->inverse;
        number[vertex] = last_number;
        vertex = pd->deg[vertex] + pd->size;
      } else {
        vertex = number[vertex];
      }
      *representation = vertex;
      representation++;
    }
    *representation = 0;
    representation++;
    edge = startedge[actual_number++];
  }
}

static int testcanon(PreDeco *pd, Edge *edge, int representation[]) {
  register Edge *run;
  register int vertex;
  Edge *startedge[pd->size];
  int number[pd->size], i;
  int last_number, actual_number;

  for (i = 0; i < pd->size; i++) number[i] = 0;

  number[edge->start] = 1;
  number[edge->end] = 2;
  last_number = 2;
  startedge[1] = edge->inverse;

  actual_number = 1;
  while (actual_number <= pd->size) {
    for (run = edge->next; run != edge; run = run->next) {
      vertex = run->end;
      if (!number[vertex]) {
        startedge[last_number++] = run->inverse;
        number[vertex] = last_number;
        vertex = pd->deg[vertex] + pd->size;
      } else {
        vertex = number[vertex];
      }
      if (vertex > *representation) return -1;
      if (vertex < *representation) return 1;
      representation++;
    }
    if (0 > *representation) return -1;
    if (0 < *representation) return 1;
    representation++;
    edge = startedge[actual_number++];
  }

  return 0;
}

static int edge_cmp(PreDeco *pd, Edge *a, Edge *b) {
  if (pd->deg[a->start] < pd->deg[b->start]) return -1;
  if (pd->deg[a->start] > pd->deg[b->start]) return 1;
  if (pd->deg[a->end] < pd->deg[b->end]) return -1;
  if (pd->deg[a->end] > pd->deg[b->end]) return 1;
  return 0;
}

static int add_to_list(PreDeco *pd, Edge **list, Edge *edge, Edge *candidate) {
  switch (edge_cmp(pd, candidate, edge)) {
    case -1:
      return -1;
    case 0:
      *list = candidate;
      return 1;
    default:
      return 0;
  }
}

int canon(PreDeco *pd, int construction, Edge *edge) {
  Edge *edgelist[4 * pd->size - pd->nedges - 4]; // TODO boundary length?
  Edge *run = edge;
  int listlength = 0, added, i, symmetries = 0;
  int representation[pd->nedges];

  // edgelist[listlength++] = edge;

  /* Find construction 1 */
  for (i = 0; i < pd->n0; i++) {
    if (construction != 1) return 0;
    run = pd->edge0[i]->prev;
    if (run == edge) continue;
    listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
    if (added == -1) return 0;
  }
  for (i = 0; i < pd->n1; i++) {
    if (construction != 1) return 0;
    run = pd->edge1[i]->prev;
    if (run == edge) continue;
    listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
    if (added == -1) return 0;
  }
  run = edge;

  /* Find construction 2 */
  if (construction > 1) {
    while ((run = run->next->inverse) != edge) {
      if (run->next->label == 1) {
        if (construction != 2) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  /* Find construction 3 */
  if (construction > 2) {
    while ((run = run->next->inverse) != edge) {
      if (run->next->label == 0 &&
          pd->deg[run->next->end] == 2 &&
          pd->deg[run->next->next->end] == 2 &&
          pd->deg[run->start] > 2 &&
          pd->deg[run->next->inverse->next->end] > 2) {
        if (construction != 3) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  /* Find construction 4 */
  if (construction > 3) {
    while ((run = run->next->inverse) != edge) {
      if (run->next->label == 0 &&
          pd->deg[run->start] > 2 &&
          pd->deg[run->next->end] == 2 &&
          pd->deg[run->next->inverse->next->end] == 2 &&
          pd->deg[run->next->next->end] > 2 &&
          run->next->next->label == 1) {
        if (construction != 4) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  /* Find construction 5 */
  if (construction > 4) {
    for (i = 0; i < pd->n2; i++) {
      if (construction != 5) return 0;
      run = pd->edge2[i]->prev->inverse->prev;
      if (run == edge) continue;
      listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
      if (added == -1) return 0;
    }
    run = edge;
  }

  /* Find construction 6 */
  if (construction > 5) {
    while ((run = run->next->inverse) != edge) {
      if (run->next->label == 0 &&
          run->next->next->label == 0 &&
          pd->deg[run->start] > 3 &&
          pd->deg[run->next->end] == 2 &&
          pd->deg[run->next->inverse->next->end] == 2 &&
          pd->deg[run->next->next->end] == 3 &&
          pd->deg[run->next->next->next->end] == 2 &&
          pd->deg[run->next->next->next->inverse->next->end] == 2) {
        if (construction != 6) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  /* Find construction 7 */
  if (construction > 6) {
    while ((run = run->next->inverse) != edge) {
      if (run->next->label == 0 &&
          pd->deg[run->start] > 2 &&
          run->next->inverse->prev->inverse->label == 0 &&
          pd->deg[run->next->end] == 3 &&
          pd->deg[run->next->inverse->next->end] == 2 &&
          pd->deg[run->next->inverse->next->inverse->next->end] == 2 &&
          pd->deg[run->next->inverse->prev->end] == 3 &&
          pd->deg[run->next->next->end] == 2) {
        if (construction != 7) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  /* Find construction 7b */
  if (construction > 7) {
    while ((run = run->next->inverse) != edge) {
      if (run->next->label == 0 &&
          pd->deg[run->start] > 2 &&
          run->next->next->inverse->next->label == 0 &&
          pd->deg[run->next->end] == 2 &&
          pd->deg[run->next->inverse->next->end] == 3 &&
          pd->deg[run->next->inverse->next->inverse->next->end] == 2 &&
          pd->deg[run->next->next->end] == 3 &&
          pd->deg[run->next->next->inverse->prev->end] == 2) {
        if (construction != 8) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  /* Find construction 8 */
  if (construction > 8) {
    while ((run = run->next->inverse) != edge) {
      if (pd->deg[run->start] > 2 &&
          (pd->out[run->start] == 1 || pd->out[run->next->next->end] == 1) &&
          pd->deg[run->next->end] == 2 &&
          pd->deg[run->next->inverse->next->end] == 2 &&
          pd->deg[run->next->next->end] > 2) {
        if (construction != 9) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  /* Find construction 9 */
  if (construction > 9) {
    while ((run = run->next->inverse) != edge) {
      if (pd->deg[run->start] > 2 &&
          pd->out[run->start] == 1 &&
          pd->deg[run->next->end] == 2 &&
          pd->deg[run->next->inverse->next->end] > 2 &&
          pd->out[run->next->inverse->next->end] == 1) {
        if (construction != 10) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  /* Find construction 10 */
  if (construction > 10) {
    while ((run = run->next->inverse) != edge) {
      if (pd->deg[run->start] > 2 &&
          pd->out[run->start] == 1 &&
          run->next->next->inverse->next->label == 0 &&
          pd->deg[run->next->end] > 2 &&
          pd->out[run->next->end] == 1) {
        if (construction != 11) return 0;
        listlength += added = add_to_list(pd, edgelist + listlength, edge, run);
        if (added == -1) return 0;
      }
    }
  }

  testcanon_init(pd, edge, representation);

  do {
    run->canonical = 0;
  } while ((run = run->next->inverse) != edge);

  pd->orbits = 0;

  for (i = 0; i < listlength; i++) {
    switch(testcanon(pd, edgelist[i], representation)) {
      case 1:
        return 0;
      case 0:
        edgelist[symmetries++] = edgelist[i];
    }
  }

  while (run->canonical == 0) {
    pd->canonical[pd->orbits++] = run;
    run->canonical = 1;
    for (i = 0; i < symmetries; i++) {
      edgelist[i]->canonical = -1;
      edgelist[i] = edgelist[i]->next->inverse;
    }
    run = run->next->inverse;
  }

  return 1;
}

static int FACTOR;

int check2connected(PreDeco *pd) {
  int label[pd->size];
  Edge *edge, *temp, *stop;

  for (int i = 0; i < pd->size; i++) label[i] = -1;

  stop = pd->v2->next->inverse;
  if (pd->v1->label == 2 && pd->deg[pd->v1->start] > 1) label[pd->v1->end] = 0;
  for (edge = pd->v1; edge != stop; edge = edge->next->inverse) {
    if (label[edge->start] == 0) return 0;
    if (edge->label == 3) {
      label[edge->start] = 0;
    } else {
      for (temp = edge->prev; temp != edge->next; temp = temp->prev) {
        label[temp->end] = 0;
      }
    }
  }

  if (pd->deg[pd->v0->start] > 1) label[pd->v0->end] = 2;
  stop = (pd->v1->label == 4) ? pd->v1 : pd->v1->next->inverse;
  for (edge = pd->v0; edge != stop; edge = edge->next->inverse) {
    if (label[edge->start] == 2) return 0;
    if (edge->label == 3) {
      label[edge->start] = 2;
    } else {
      for (temp = edge->prev; temp != edge->next; temp = temp->prev) {
        label[temp->end] = 2;
      }
    }
  }

  stop = pd->v0->next->inverse;
  if (pd->deg[pd->v2->start] > 1) label[pd->v2->end] = 1;
  if (pd->deg[pd->v0->start] > 1) {
    if (label[pd->v0->next->end] == 1) return 0;
    label[pd->v0->next->end] = 1;
  }
  edge = pd->v2;
  do {
    if (label[edge->start] == 1) return 0;
    if (edge->label == 3) {
      label[edge->start] = 1;
    } else {
      for (temp = edge->prev; temp != edge->next; temp = temp->prev) {
        label[temp->end] = 1;
      }
    }
    edge = edge->next->inverse;
  } while (edge != stop);

  return 1;
}

int check3connected(PreDeco *pd) {
  int label[pd->size];
  Edge *edge, *temp, *stop;

  for (int i = 0; i < pd->size; i++) label[i] = -1;

  stop = pd->v2->next->inverse;
  if (pd->v1->label == 2 && pd->deg[pd->v1->start] > 1) label[pd->v1->end] = 0;
  for (edge = pd->v1; edge != stop; edge = edge->next->inverse) {
    if (label[edge->start] == 0) return 0;
    if (edge->label == 3) {
      label[edge->start] = 0;
    } else {
      for (temp = edge->prev; temp != edge->next; temp = temp->prev) {
        if (label[temp->end] == 0) return 0;
        label[temp->end] = 0;
      }
    }
  }
  if (pd->deg[edge->end] > 1) {
    if (label[edge->start] == 0) return 0;
    label[edge->start] = 0;
  }

  if (pd->deg[pd->v0->start] > 1) label[pd->v0->end] = 2;
  stop = (pd->v1->label == 4) ? pd->v1 : pd->v1->next->inverse;
  for (edge = pd->v0; edge != stop; edge = edge->next->inverse) {
    if (edge->label == 3) {
      if (label[edge->start] == 2) return 0;
      label[edge->start] = 2;
    } else {
      if (label[edge->start] >= 0) return 0;
      for (temp = edge->prev; temp != edge->next; temp = temp->prev) {
        if (label[temp->end] == 2) return 0;
        label[temp->end] = 2;
      }
    }
  }
  if (edge->label == 3 && label[edge->start] == 2 && pd->deg[edge->end] > 1) return 0;

  stop = pd->v0->next->inverse;
  if (pd->deg[pd->v2->start] > 1) label[pd->v2->end] = 1;
  if (pd->deg[pd->v0->start] > 1) {
    if (label[pd->v0->next->end] == 1) return 0;
    label[pd->v0->next->end] = 1;
  }
  edge = pd->v2;
  do {
    if (label[edge->start] == 1) return 0;
    if (edge->label == 3) {
      label[edge->start] = 1;
    } else {
      for (temp = edge->prev; temp != edge->next; temp = temp->prev) {
        if (label[temp->end] == 1) return 0;
        label[temp->end] = 1;
      }
    }
    edge = edge->next->inverse;
  } while (edge != stop);

  return 1;
}

void completed(PreDeco *pd) {
  int i;
  int factor = 2 * pd->nedges - 4 * pd->size + 4 + 2 * pd->extensions + (pd->v1->label == 4);

  if (factor > FACTOR || (!ALL && factor != FACTOR)) {
    wrongfactor++;
    return;
  }
  for (i = 0; i < pd->size; i++) if (pd->out[i] > 1) {
    cutvertices++;
    return;
  }
  if (CONNECTIVITY == 3 && !check3connected(pd)) {
    wrongconnectivity++;
    return;
  }
  if (CONNECTIVITY == 2 && !check2connected(pd)) {
    wrongconnectivity++;
    return;
  }

  if (!predecoration_used) {
    predecoration_used = 1;
    precount++;
  }
  count++;
  if (OUTPUT) write_deco_code(pd);
}

void complete(PreDeco *pd, Edge *edge) {
  Edge *next = edge->next->inverse, *prev = edge->inverse->prev;;

  if (edge == pd->v1) {
    if (edge->label == 4) {
      completed(pd);
    } else {
      if (pd->deg[edge->start] > 1 || edge->label == 2) {
        if (prev == pd->v0) {
          if (pd->deg[prev->start] > 1 || prev->label == 2) completed(pd);
        } else if (pd->deg[prev->start] > 2 || prev->label == 2) {
          completed(pd);
        }
      }
    }
    return;
  }

  if (edge->label == 1 && next->label == 1 && (pd->deg[edge->start] > 2 || pd->out[edge->start] > 1) && edge != pd->v0 && edge != pd->v1 && edge != pd->v2) {
    // complete
    edge->label = 3;
    next->label = 2;
    pd->out[edge->start]--;
    pd->extensions++;
    if (next != pd->v1) {
      complete(pd, next->next->inverse);
    } else {
      completed(pd);
    }
    edge->label = 1;
    next->label = 1;
    pd->out[edge->start]++;
    pd->extensions--;
  }

  if (prev == pd->v1 || prev == pd->v0 || prev == pd->v2) {
    if (pd->deg[prev->start] > 1 || prev->label == 2 || prev->label == 4) complete(pd, next);
  } else if (pd->deg[prev->start] > 2 || pd->out[prev->start] > 1 || prev->label == 2) {
    complete(pd, next);
  }
}

void select0(PreDeco *pd, int n0, int n1, int n2) {
  Edge *edge = pd->v2;

  while ((edge = pd->v0 = edge->next->inverse) != pd->v1) {
    if (pd->deg[edge->start] == 1) {
      if (pd->deg[edge->end] == 2) {
        // edge0
        complete(pd, pd->v1->next->inverse);
      } else if (n0 < 1) {
        // edge1
        complete(pd, pd->v1->next->inverse);
      }
    } else if (edge->next->inverse->label != 4 && !edge->next->label && pd->deg[edge->start] == 2 && pd->deg[edge->end]  == 2 && pd->deg[edge->next->end] == 2) {
      if (n0 < 1) {
        // edge2
        complete(pd, pd->v1->next->inverse);
      }
    } else if (n0 + n1 + n2 < 1) {
      complete(pd, pd->v1->next->inverse);
    }
  }
}

void select2(PreDeco *pd, int n0, int n1, int n2) {
  Edge *edge = pd->v1;

  if (edge->label == 4) edge = edge->inverse->prev;

  while ((edge = pd->v2 = edge->next->inverse)->next->inverse != pd->v1) {
    if (pd->deg[edge->start] == 1) {
      if (pd->deg[edge->end] == 2) {
        // edge0
        select0(pd, n0 - 1, n1, n2);
      } else if (n0 < 2) {
        // edge1
        select0(pd, n0, n1 - 1, n2);
      }
    } else if (edge->label != 4 && !edge->next->label && pd->deg[edge->start] == 2 && pd->deg[edge->end]  == 2 && pd->deg[edge->next->end] == 2) {
      if (n0 < 2) {
        // edge2
        select0(pd, n0, n1, n2 - 1);
      }
    } else if (n0 + n1 + n2 < 2) {
      select0(pd, n0, n1, n2);
    }
  }
}

void select1(PreDeco *pd) {
  int n0 = pd->n0, n1 = pd->n1, n2 = pd->n2, i;
  Edge *edge;

  predecoration_used = 0;

  for (i = 0; i < pd->orbits; i++) {
    edge = pd->v1 = pd->canonical[i];
    if (pd->deg[edge->start] == 1) {
      if (pd->deg[edge->end] != 2 && (ALL || FACTOR % 2 == 0)) {
        // edge1
        select2(pd, n0, n1 - 1, n2);
      }
      if (n0 + n1 + n2 < 3 && (ALL || FACTOR % 2)) {
        edge->label = 4;
        select2(pd, n0, n1, n2);
      }
    } else if (!edge->next->label && pd->deg[edge->start] == 2 && pd->deg[edge->end] == 2 && pd->deg[edge->next->end] == 2) {
      // edge2
      if (ALL || FACTOR % 2 == 0) {
        select2(pd, n0, n1, n2 - 1);
      }
      if (ALL || FACTOR % 2) {
        edge->label = 4;
        select2(pd, n0, n1, n2 - 1);
      }
    } else if (!edge->next->label && pd->deg[edge->start] == 2 && pd->deg[edge->end] == 2 && pd->deg[edge->inverse->next->end] == 2) {
      // edge2
      if (n0 + n1 + n2 < 3 && (ALL || FACTOR % 2 == 0)) {
        select2(pd, n0, n1, n2);
      }
      if (ALL || FACTOR % 2) {
        edge->label = 4;
        select2(pd, n0, n1, n2 - 1);
      }
    } else if (n0 + n1 + n2 < 3) {
      if (ALL || FACTOR % 2 == 0) {
        select2(pd, n0, n1, n2);
      }
      if (ALL || FACTOR % 2) {
        edge->label = 4;
        select2(pd, n0, n1, n2);
      }
    }
    edge->label = 1;
  }
}

void grow(PreDeco*, int);

int edge_exists(PreDeco *pd, int vertex1, int vertex2) {
  Edge *edge = pd->firstedge[vertex1];
  for (int i = 0; i < pd->deg[vertex1]; edge = edge->next, i++) {
    if (edge->end == vertex2) return 1;
  }
  return 0;
}

void try_construct(PreDeco *pd, void (*construct)(PreDeco*, Edge*), void (*destruct)(PreDeco*, Edge*), int construction, int level) {
  PreDeco copy;
  Edge *edge;
  int i;

  for (i = 0; i < pd->orbits; i++) {
    edge = pd->canonical[i];
    if (construction < 9 ||
        (construction == 9 &&
          (pd->out[edge->start] == 1 ||
          pd->out[edge->next->end] == 1)) ||
        (construction == 10 &&
          pd->out[edge->start] == 1 &&
          pd->out[edge->next->end] == 1 &&
          pd->deg[edge->next->end] > 2 &&
          pd->out[edge->next->inverse->next->end] == 1) ||
        (construction == 11 &&
          pd->out[edge->start] == 1 &&
          pd->out[edge->next->end] == 1 &&
          pd->deg[edge->next->end] > 2 &&
          pd->out[edge->next->inverse->next->end] == 1 &&
          pd->deg[edge->next->inverse->next->end] > 2 &&
          pd->out[edge->next->inverse->next->inverse->next->end] == 1 &&
          !edge_exists(pd, edge->start, edge->next->inverse->next->inverse->next->end))) {
      copy = *pd;
      construct(&copy, edge);
      if (copy.n0 <= 2 && copy.n0 + copy.n1 + copy.n2 <= 3) {
        copy.canonical = malloc(copy.nedges * sizeof(Edge*));
        if (canon(&copy, construction, edge)) {
          grow(&copy, level + 1);
        }
        free(copy.canonical);
      }
      destruct(&copy, edge);
    }
  }
}

void try_cutconstruct(PreDeco *pd, void (*construct)(PreDeco*, Edge*, Edge*), void (*destruct)(PreDeco*, Edge*, Edge*), int construction, int level) {
  PreDeco copy;
  Edge *edge, *other;
  int i;

  for (i = 0; i < pd->orbits; i++) {
    edge = pd->canonical[i];
    if (pd->out[edge->start] > 1) {
      for (other = edge->next; other != edge; other = other->next) if (other->label) {
        copy = *pd;
        construct(&copy, edge, other);
        if (copy.n0 <= 2 && copy.n0 + copy.n1 + copy.n2 <= 3) {
          copy.canonical = malloc(copy.nedges * sizeof(Edge*));
          if (canon(&copy, construction, edge)) grow(&copy, level + 1);
          free(copy.canonical);
        }
        destruct(&copy, edge, other);
      }
    }
  }
}

unsigned long int levelcount = 0;

void grow(PreDeco *pd, int level) {
  if (level == SPLIT_LEVEL) {
    levelcount++;
    if (level_index == MOD) level_index = 0;
    if (level_index++ != RES) return;
  }
  if (4 - 4 * pd->size + 2 * pd->nedges + 2 * pd->cuts > FACTOR) return;
  if ((level >= SPLIT_LEVEL || RES == 0) && (ALL || pd->nedges >= FACTOR)) {
    select1(pd);
  }
  try_construct(pd, construct1, destruct1, 1, level);
  if (pd->n0 || pd->n1) return;
  try_cutconstruct(pd, construct2, destruct2, 2, level);
  if (pd->bridges) return;
  try_cutconstruct(pd, construct3, destruct3, 3, level);
  try_cutconstruct(pd, construct4, destruct4, 4, level);
  try_construct(pd, construct5, destruct5, 5, level);
  try_construct(pd, construct6, destruct6, 6, level);
  try_construct(pd, construct7, destruct7, 7, level);
  try_construct(pd, construct7b, destruct7b, 8, level);
  try_construct(pd, construct8, destruct8, 9, level);
  try_construct(pd, construct9, destruct9, 10, level);
  if (CONNECTIVITY > 1 && 4 * pd->size - pd->nedges - 4 <= 4) return;
  if (CONNECTIVITY > 2 && 4 * pd->size - pd->nedges - 4 <= 6) return;
  try_construct(pd, construct10, destruct10, 11, level);
}

void start_construction(PreDeco *pd) {
  pd->size = 2;
  pd->nedges = 0;

  Edge *edge = &pd->edges[pd->nedges++];
  Edge *inverse = &pd->edges[pd->nedges++];
  edge->start = inverse->end = 0;
  edge->end = inverse->start = 1;
  edge->label = inverse->label = 1;
  edge->prev = edge->next = inverse->inverse = edge;
  inverse->prev = inverse->next = edge->inverse = inverse;

  pd->n0 = 0;
  pd->n1 = 2;
  pd->edge1[0] = edge;
  pd->edge1[1] = inverse;
  pd->n2 = 0;
  pd->bridges = 1;
  pd->cuts = 0;

  pd->firstedge[0] = edge; pd->firstedge[1] = inverse;
  pd->deg[0] = pd->deg[1] = 1;
  pd->out[0] = pd->out[1] = 1;

  pd->orbits = 1;
  pd->canonical = malloc(6 * sizeof(Edge*));
  pd->canonical[0] = inverse;

  pd->extensions = 0;

  grow(pd, 0);

  pd->size = 4;

  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];
  Edge *newedge3 = &pd->edges[pd->nedges++];
  Edge *inverse3 = &pd->edges[pd->nedges++];

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;
  newedge3->inverse = inverse3; inverse3->inverse = newedge3;

  newedge1->start = inverse1->end = 1;
  newedge1->end = inverse1->start = newedge2->start = inverse2->end = 2;
  newedge2->end = inverse2->start = newedge3->start = inverse3->end = 3;
  newedge3->end = inverse3->start = 0;

  edge->label = newedge1->label = newedge2->label = newedge3->label = 0;
  inverse1->label = inverse2->label = inverse3->label = 1;

  inverse->next = inverse->prev = newedge1; newedge1->next = newedge1->prev = inverse;
  inverse1->next = inverse1->prev = newedge2; newedge2->next = newedge2->prev = inverse1;
  inverse2->next = inverse2->prev = newedge3; newedge3->next = newedge3->prev = inverse2;
  inverse3->next = inverse3->prev = edge; edge->next = edge->prev = inverse3;

  pd->n1 = 0;
  pd->n2 = 4;
  pd->edge2[0] = edge;
  pd->edge2[1] = newedge1;
  pd->edge2[2] = newedge2;
  pd->edge2[3] = newedge3;
  pd->bridges = 0;

  pd->firstedge[2] = inverse1;
  pd->firstedge[3] = inverse2;

  pd->deg[0] = pd->deg[1] = pd->deg[2] = pd->deg[3] = 2;
  pd->out[2] = pd->out[3] = 1;

  if ((FACTOR == 5 || (ALL && FACTOR > 5)) && RES == 0) select1(pd);

  pd->size = 5;

  Edge *newedge4 = &pd->edges[pd->nedges++];
  Edge *inverse4 = &pd->edges[pd->nedges++];

  newedge4->inverse = inverse4; inverse4->inverse = newedge4;

  newedge4->start = inverse4->end = 0;
  newedge4->end = inverse4->start = 4;

  newedge4->label = inverse4->label = 1;

  edge->prev = newedge4; newedge4->next = edge;
  inverse3->next = newedge4; newedge4->prev = inverse3;
  inverse4->next = inverse4->prev = inverse4;

  pd->n1 = 1;
  pd->edge1[0] = newedge4;
  pd->n2 = 1;
  pd->edge2[0] = newedge1;
  pd->bridges = 1;
  pd->cuts = 1;

  pd->firstedge[4] = inverse4;

  pd->deg[0] = 3;
  pd->deg[4] = 1;
  pd->out[0] = 2;
  pd->out[4] = 1;

  pd->orbits = 6;
  pd->canonical[1] = inverse1;
  pd->canonical[2] = inverse2;
  pd->canonical[3] = inverse3;
  pd->canonical[4] = inverse4;
  pd->canonical[5] = newedge4;

  grow(pd, 0);

  pd->size = 7;

  Edge *newedge5 = &pd->edges[pd->nedges++];
  Edge *inverse5 = &pd->edges[pd->nedges++];
  Edge *newedge6 = &pd->edges[pd->nedges++];
  Edge *inverse6 = &pd->edges[pd->nedges++];
  Edge *newedge7 = &pd->edges[pd->nedges++];
  Edge *inverse7 = &pd->edges[pd->nedges++];

  newedge5->inverse = inverse5; inverse5->inverse = newedge5;
  newedge6->inverse = inverse6; inverse6->inverse = newedge6;
  newedge7->inverse = inverse7; inverse7->inverse = newedge7;

  newedge5->start = inverse5->end = 4; newedge7->end = inverse7->start = 0;
  newedge5->end = inverse5->start = newedge6->start = inverse6->end = 5;
  newedge7->start = inverse7->end = newedge6->end = inverse6->start = 6;

  inverse4->label = inverse5->label = inverse6->label = inverse7->label = 0;
  newedge5->label = newedge6->label = newedge7->label = 1;

  inverse3->next = inverse7; inverse7->prev = inverse3;
  inverse7->next = newedge4; newedge4->prev = inverse7;
  inverse4->next = inverse4->prev = newedge5;
  newedge5->next = newedge5->prev = inverse4;
  newedge7->next = newedge7->prev = inverse6;
  inverse6->next = inverse6->prev = newedge7;
  inverse5->next = inverse5->prev = newedge6;
  newedge6->next = newedge6->prev = inverse5;

  pd->n1 = 0;
  pd->n2 = 2;
  pd->edge2[1] = inverse6;
  pd->bridges = 0;
  pd->cuts = 1;

  pd->firstedge[5] = inverse5;
  pd->firstedge[6] = inverse6;

  pd->deg[0] = 4;
  pd->deg[4] = pd->deg[5] = pd->deg[6] = 2;
  pd->out[5] = pd->out[6] = 1;

  pd->orbits = 4;

  grow(pd, 0);

  pd->size = 6;
  pd->nedges -= 2;

  newedge6->end = inverse6->start = 3;

  inverse3->label = 0;

  inverse3->next = newedge4; newedge4->prev = inverse3;
  inverse6->prev = inverse2; inverse2->next = inverse6;
  inverse6->next = newedge3; newedge3->prev = inverse6;

  pd->n2 = 0;
  pd->cuts = 0;

  pd->deg[0] = pd->deg[3] = 3;
  pd->out[0] = 1;

  pd->orbits = 3;

  grow(pd, 0);

  free(pd->canonical);
}

static void write_help() {
  fprintf(stdout, "Usage: decogen [-d] [-a] [-c 1|2|3] [-o OUTFILE] FACTOR\n\n");
  fprintf(stdout, " -d,--decocode      write decocode to stdout or outfile\n");
  fprintf(stdout, " -a,--all           generate decorations with smaller factors\n");
  fprintf(stdout, " -c,--connectivity  generate decorations with connectivity 1/2/3\n");
  fprintf(stdout, " -o,--output        write to OUTFILE instead of stdout\n");
  fprintf(stdout, " FACTOR             generate decorations with factor FACTOR (or smaller with -a)\n");
}

int main(int argc, char *argv[]) {
  int c, option_index;

  OUTFILE = stdout;

  static struct option long_options[] = {
    {"decocode"  ,  no_argument,       0, 'd'},
    {"all",         no_argument,       0, 'a'},
    {"c",           required_argument, 0, 'c'},
    {"output",      required_argument, 0, 'o'},
    {"help",        no_argument,       0, 'h'},
    {"mod",         required_argument, 0, 'm'},
    {"res",         required_argument, 0, 'r'},
    {"split",       required_argument, 0, 's'},
  };

  while (1) {
    c = getopt_long(argc, argv, "dac:o:hm:r:s:", long_options, &option_index);
    if (c == -1) break;
    switch (c) {
      case 'd':
        OUTPUT = 1;
        break;
      case 'a':
        ALL = 1;
        break;
      case 'c':
        CONNECTIVITY = strtol(optarg, NULL, 10);
        if (CONNECTIVITY < 1 || CONNECTIVITY > 3) {
          fprintf(stderr, "Connectivity has to be 1, 2 or 3.\n");
          return 1;
        }
        break;
      case 'o':
        OUTFILE = fopen(optarg, "w");
        break;
      case 'h':
        write_help();
        return 0;
      case 'm':
        MOD = strtoul(optarg, NULL, 10);
        break;
      case 'r':
        RES = strtoul(optarg, NULL, 10);
        break;
      case 's':
        SPLIT_LEVEL = strtoul(optarg, NULL, 10);
        break;
      default:
        write_help();
        return 1;
    }
  }

  if (optind == argc) {
    write_help();
    return 1;
  }

  FACTOR = strtol(argv[optind], NULL, 10);
  if (FACTOR < 1) {
    fprintf(stderr, "\"%s\" is no positive numeric value.\n", argv[optind]);
    return 1;
  }

  PreDeco *pd = malloc(sizeof(PreDeco));
  int maxsize = FACTOR + 7;
  int maxedges = 2 * maxsize;

  pd->edges = malloc(maxedges * sizeof(Edge));
  pd->firstedge = malloc(maxsize * sizeof(Edge*));
  pd->deg = malloc(maxsize * sizeof(int));
  pd->out = malloc(maxsize * sizeof(int));

  if (OUTPUT) write_deco_header();

  start_construction(pd);

  fprintf(stderr, "%lld decorations (%lld predecorations, %lld wrong factor, %lld cutvertices, %lld wrongconnectivity)\n", 2 * count, precount, wrongfactor, cutvertices, wrongconnectivity);

  free(pd->edges);
  free(pd->firstedge);
  free(pd->deg);
  free(pd->out);
  free(pd);

  return 0;
}
