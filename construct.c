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
#include "construct.h"

void construct1(PreDeco *pd, Edge *edge) {
  int i;
  Edge *tempedge = 0;
  Edge *newedge = &pd->edges[pd->nedges++];
  Edge *inverse = &pd->edges[pd->nedges++];

  newedge->label = 1;
  inverse->label = 1;

  newedge->start = inverse->end = edge->start;
  newedge->end = inverse->start = pd->size++;
  inverse->inverse = newedge; newedge->inverse = inverse;
  newedge->next = edge->next; newedge->next->prev = newedge;
  newedge->prev = edge; edge->next = newedge;
  inverse->prev = inverse->next = inverse;

  pd->firstedge[inverse->start] = inverse;

  pd->deg[edge->start]++;
  pd->deg[newedge->end] = 1;

  pd->out[edge->start]++;
  pd->out[newedge->end] = 1;

  pd->cuts++;

  if (pd->deg[edge->start] == 2) {
    if (pd->deg[edge->end] == 1) {
      // one edge1 -> edge0
      pd->n0 = 2;
      pd->edge0[0] = edge;
      pd->edge0[1] = newedge;
      pd->n1 = 0;
    } else if (pd->deg[edge->end] == 2) {
      // edge0 -> edge0
      for (i = 0; i < pd->n0; i++) if (pd->edge0[i] == edge->inverse) {
        pd->edge0[i] = newedge;
        break;
      }
    } else {
      // edge1 -> edge0
      for (i = 0; i < pd->n1; i++) if (pd->edge1[i] == edge->inverse) {
        pd->n1--;
        for (; i < pd->n1; i++) pd->edge1[i] = pd->edge1[i + 1];
        pd->edge0[pd->n0++] = newedge;
        break;
      }
    }
  } else if (pd->deg[edge->start] == 3) {
    if (!edge->inverse->label) {
      if (pd->deg[edge->prev->end] == 2 && pd->deg[edge->inverse->next->end] == 2) {
        tempedge = edge->prev;
      } else if (pd->deg[edge->end] == 2 && pd->deg[edge->prev->end] == 2) {
        tempedge = edge->inverse;
      } else if (pd->deg[edge->end] == 2 && pd->deg[edge->inverse->next->end] == 2) {
        tempedge = edge->inverse->next->inverse;
      }
      if (tempedge) {
        // edge2 -> edge1
        for (i = 0; i < pd->n2; i++) if (pd->edge2[i] == tempedge) {
          pd->n2--;
          for (; i < pd->n2; i++) pd->edge2[i] = pd->edge2[i + 1];
          break;
        }
      }
      pd->edge1[pd->n1++] = newedge;
    } else {
      if (pd->deg[edge->end] == 1) {
        // edge0 -> edge1 + edge1
        for (i = 0; i < pd->n0; i++) if (pd->edge0[i] == edge) {
          pd->n0--;
          for (; i < pd->n0; i++) pd->edge0[i] = pd->edge0[i + 1];
          break;
        }
        pd->edge1[pd->n1++] = edge;
      }
      if (pd->deg[edge->prev->end] == 1) {
        // edge0 -> edge1 + edge1
        for (i = 0; i < pd->n0; i++) if (pd->edge0[i] == edge->prev) {
          pd->n0--;
          for (; i < pd->n0; i++) pd->edge0[i] = pd->edge0[i + 1];
          break;
        }
        pd->edge1[pd->n1++] = edge->prev;
      }
      pd->edge1[pd->n1++] = newedge;
    }
  } else {
    // -> edge1
    pd->edge1[pd->n1++] = newedge;
  }
}

void destruct1(PreDeco *pd, Edge *edge) {
  edge->next = edge->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start]--;
  pd->out[edge->start]--;
  pd->firstedge[edge->start] = edge;
}

void construct2(PreDeco *pd, Edge *leftedge, Edge *rightedge) {
  Edge *tempedge = leftedge;
  pd->deg[pd->size] = 1;
  pd->out[pd->size] = 1;
  do {
    tempedge = tempedge->next;
    tempedge->start = tempedge->inverse->end = pd->size;
    pd->deg[pd->size]++;
    pd->deg[leftedge->start]--;
    if (tempedge->label) {
      pd->out[pd->size]++;
      pd->out[leftedge->start]--;
    }
  } while (tempedge != rightedge);
  pd->size++;

  Edge *newedge = &pd->edges[pd->nedges++];
  Edge *inverse = &pd->edges[pd->nedges++];

  newedge->label = 1;
  inverse->label = 1;

  newedge->start = inverse->end = leftedge->start;
  newedge->end = inverse->start = rightedge->start;

  inverse->inverse = newedge; newedge->inverse = inverse;

  inverse->next = leftedge->next; inverse->next->prev = inverse;
  newedge->prev = leftedge; leftedge->next = newedge;
  newedge->next = rightedge->next; newedge->next->prev = newedge;
  inverse->prev = rightedge; rightedge->next = inverse;

  pd->firstedge[leftedge->start] = leftedge;
  pd->firstedge[rightedge->start] = rightedge;

  pd->deg[leftedge->start]++;
  pd->out[leftedge->start]++;

  pd->bridges++;
  pd->cuts++;
}

void destruct2(PreDeco *pd, Edge *leftedge, Edge *rightedge) {
  Edge *tempedge = leftedge->next->inverse;
  do {
    tempedge = tempedge->next;
    tempedge->start = tempedge->inverse->end = leftedge->start;
    pd->deg[leftedge->start]++;
    if (tempedge->label) {
      pd->out[leftedge->start]++;
    }
  } while (tempedge != rightedge);

  leftedge->next = leftedge->next->inverse->next;
  leftedge->next->prev = leftedge;
  rightedge->next = rightedge->next->inverse->next;
  rightedge->next->prev = rightedge;

  pd->deg[leftedge->start]--;
  pd->out[leftedge->start]--;

  pd->firstedge[leftedge->start] = leftedge;
}

void construct3(PreDeco *pd, Edge *leftedge, Edge *rightedge) {
  Edge *tempedge = leftedge;
  pd->deg[pd->size] = 2;
  pd->out[pd->size] = 1;
  do {
    tempedge = tempedge->next;
    tempedge->start = tempedge->inverse->end = pd->size;
    pd->deg[pd->size]++;
    pd->deg[leftedge->start]--;
    if (tempedge->label) {
      pd->out[pd->size]++;
      pd->out[leftedge->start]--;
    }
  } while(tempedge != rightedge);
  pd->size++;

  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];
  Edge *newedge3 = &pd->edges[pd->nedges++];
  Edge *inverse3 = &pd->edges[pd->nedges++];
  Edge *newedge4 = &pd->edges[pd->nedges++];
  Edge *inverse4 = &pd->edges[pd->nedges++];

  newedge1->label = 0;
  inverse1->label = 1;
  newedge2->label = 1;
  inverse2->label = 0;
  newedge3->label = 1;
  inverse3->label = 0;
  newedge4->label = 0;
  inverse4->label = 1;

  newedge1->start = newedge2->start = inverse1->end = inverse2->end = leftedge->start;
  newedge3->start = newedge4->start = inverse3->end = inverse4->end = rightedge->start;
  newedge1->end = newedge3->end = inverse1->start = inverse3->start = pd->size++;
  newedge2->end = newedge4->end = inverse2->start = inverse4->start = pd->size++;

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;
  newedge3->inverse = inverse3; inverse3->inverse = newedge3;
  newedge4->inverse = inverse4; inverse4->inverse = newedge4;

  newedge3->next = leftedge->next; newedge3->next->prev = newedge3;
  newedge1->prev = leftedge; leftedge->next = newedge1;
  newedge2->next = rightedge->next; newedge2->next->prev = newedge2;
  newedge4->prev = rightedge; rightedge->next = newedge4;

  newedge1->next = newedge2; newedge2->prev = newedge1;
  newedge4->next = newedge3; newedge3->prev = newedge4;
  inverse1->next = inverse1->prev = inverse3;
  inverse3->next = inverse3->prev = inverse1;
  inverse2->next = inverse2->prev = inverse4;
  inverse4->next = inverse4->prev = inverse2;

  pd->firstedge[leftedge->start] = leftedge;
  pd->firstedge[rightedge->start] = rightedge;
  pd->firstedge[inverse1->start] = inverse1;
  pd->firstedge[inverse4->start] = inverse4;

  pd->deg[leftedge->start] += 2;
  pd->deg[newedge1->end] = pd->deg[newedge2->end] = 2;
  pd->out[leftedge->start]++;
  pd->out[newedge1->end] = pd->out[newedge2->end] = 1;

  pd->cuts++;
}

void destruct3(PreDeco *pd, Edge *leftedge, Edge *rightedge) {
  Edge *tempedge = leftedge->next->inverse->next->inverse;
  do {
    tempedge = tempedge->next;
    tempedge->start = tempedge->inverse->end = leftedge->start;
    pd->deg[leftedge->start]++;
    if (tempedge->label) {
      pd->out[leftedge->start]++;
    }
  } while (tempedge != rightedge);

  leftedge->next = leftedge->next->inverse->next->inverse->next;
  leftedge->next->prev = leftedge;
  rightedge->next = rightedge->next->inverse->next->inverse->next;
  rightedge->next->prev = rightedge;

  pd->deg[leftedge->start] -= 2;
  pd->out[leftedge->start]--;

  pd->firstedge[leftedge->start] = leftedge;
}

void construct4(PreDeco *pd, Edge *leftedge, Edge *rightedge) {
  Edge *tempedge = leftedge;
  pd->deg[pd->size] = 2;
  pd->out[pd->size] = 1;
  do {
    tempedge = tempedge->next;
    tempedge->start = tempedge->inverse->end = pd->size;
    pd->deg[pd->size]++;
    pd->deg[leftedge->start]--;
    if (tempedge->label) {
      pd->out[pd->size]++;
      pd->out[leftedge->start]--;
    }
  } while(tempedge != rightedge);
  pd->size++;

  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];
  Edge *newedge3 = &pd->edges[pd->nedges++];
  Edge *inverse3 = &pd->edges[pd->nedges++];
  Edge *newedge4 = &pd->edges[pd->nedges++];
  Edge *inverse4 = &pd->edges[pd->nedges++];

  newedge1->label = 1;
  inverse1->label = 0;
  newedge2->label = 0;
  inverse2->label = 1;
  newedge3->label = 0;
  inverse3->label = 1;
  newedge4->label = 1;
  inverse4->label = 0;

  newedge1->start = newedge2->start = inverse1->end = inverse2->end = leftedge->start;
  newedge1->end = newedge4->start = inverse1->start = inverse4->end = rightedge->start;
  newedge2->end = newedge3->start = inverse2->start = inverse3->end = pd->size++;
  newedge3->end = newedge4->end = inverse3->start = inverse4->start = pd->size++;

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;
  newedge3->inverse = inverse3; inverse3->inverse = newedge3;
  newedge4->inverse = inverse4; inverse4->inverse = newedge4;

  newedge4->next = leftedge->next; newedge4->next->prev = newedge4;
  newedge2->prev = leftedge; leftedge->next = newedge2;
  newedge1->next = rightedge->next; newedge1->next->prev = newedge1;
  inverse1->prev = rightedge; rightedge->next = inverse1;

  newedge1->prev = newedge2; newedge2->next = newedge1;
  newedge4->prev = inverse1; inverse1->next = newedge4;
  inverse2->next = inverse2->prev = newedge3;
  newedge3->next = newedge3->prev = inverse2;
  inverse3->next = inverse3->prev = inverse4;
  inverse4->next = inverse4->prev = inverse3;

  pd->firstedge[leftedge->start] = leftedge;
  pd->firstedge[rightedge->start] = rightedge;
  pd->firstedge[inverse2->start] = inverse2;
  pd->firstedge[inverse3->start] = inverse3;

  pd->deg[leftedge->start] += 2;
  pd->deg[newedge2->end] = pd->deg[newedge3->end] = 2;
  pd->out[leftedge->start]++;
  pd->out[newedge2->end] = pd->out[newedge3->end] = 1;

  pd->cuts++;
}

void destruct4(PreDeco *pd, Edge *leftedge, Edge *rightedge) {
  Edge *tempedge = leftedge->next->next->inverse->next;
  do {
    tempedge = tempedge->next;
    tempedge->start = tempedge->inverse->end = leftedge->start;
    pd->deg[leftedge->start]++;
    if (tempedge->label) {
      pd->out[leftedge->start]++;
    }
  } while (tempedge != rightedge);

  leftedge->next = leftedge->next->next->inverse->next->next;
  leftedge->next->prev = leftedge;
  rightedge->next = rightedge->next->inverse->next;
  rightedge->next->prev = rightedge;

  pd->deg[leftedge->start] -= 2;
  pd->out[leftedge->start]--;

  pd->firstedge[leftedge->start] = leftedge;
}

void construct5(PreDeco *pd, Edge *edge) {
  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];
  Edge *newedge3 = &pd->edges[pd->nedges++];
  Edge *inverse3 = &pd->edges[pd->nedges++];
  Edge *newedge4 = &pd->edges[pd->nedges++];
  Edge *inverse4 = &pd->edges[pd->nedges++];

  newedge1->label = 0;
  inverse1->label = 1;
  newedge2->label = 1;
  inverse2->label = 0;
  newedge3->label = 0;
  inverse3->label = 1;
  newedge4->label = 1;
  inverse4->label = 0;

  newedge1->start = newedge2->start = inverse1->end = inverse2->end = edge->start;
  newedge1->end = newedge3->start = inverse1->start = inverse3->end = pd->size++;
  newedge2->end = newedge4->start = inverse2->start = inverse4->end = pd->size++;
  newedge3->end = newedge4->end = inverse3->start = inverse4->start = pd->size++;

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;
  newedge3->inverse = inverse3; inverse3->inverse = newedge3;
  newedge4->inverse = inverse4; inverse4->inverse = newedge4;

  newedge2->next = edge->next; newedge2->next->prev = newedge2;
  newedge1->prev = edge; edge->next = newedge1;

  newedge1->next = newedge2; newedge2->prev = newedge1;
  inverse1->next = inverse1->prev = newedge3;
  newedge3->next = newedge3->prev = inverse1;
  inverse2->next = inverse2->prev = newedge4;
  newedge4->next = newedge4->prev = inverse2;
  inverse3->next = inverse3->prev = inverse4;
  inverse4->next = inverse4->prev = inverse3;

  pd->firstedge[inverse1->start] = inverse1;
  pd->firstedge[inverse2->start] = inverse2;
  pd->firstedge[inverse3->start] = inverse3;

  pd->deg[edge->start] += 2;
  pd->deg[newedge1->end] = pd->deg[newedge2->end] = pd->deg[newedge3->end] = 2;
  pd->out[edge->start]++;
  pd->out[newedge1->end] = pd->out[newedge2->end] = pd->out[newedge3->end] = 1;

  pd->edge2[pd->n2++] = newedge3;

  pd->cuts++;
}

void destruct5(PreDeco *pd, Edge *edge) {
  edge->next = edge->next->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start] -= 2;
  pd->out[edge->start]--;
  pd->firstedge[edge->start] = edge;
}

void construct6(PreDeco *pd, Edge *edge) {
  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];
  Edge *newedge3 = &pd->edges[pd->nedges++];
  Edge *inverse3 = &pd->edges[pd->nedges++];
  Edge *newedge4 = &pd->edges[pd->nedges++];
  Edge *inverse4 = &pd->edges[pd->nedges++];
  Edge *newedge5 = &pd->edges[pd->nedges++];
  Edge *inverse5 = &pd->edges[pd->nedges++];
  Edge *newedge6 = &pd->edges[pd->nedges++];
  Edge *inverse6 = &pd->edges[pd->nedges++];
  Edge *newedge7 = &pd->edges[pd->nedges++];
  Edge *inverse7 = &pd->edges[pd->nedges++];

  newedge1->label = 0;
  inverse1->label = 1;
  newedge2->label = 0;
  inverse2->label = 0;
  newedge3->label = 1;
  inverse3->label = 0;
  newedge4->label = 0;
  inverse4->label = 1;
  newedge5->label = 0;
  inverse5->label = 1;
  newedge6->label = 1;
  inverse6->label = 0;
  newedge7->label = 1;
  inverse7->label = 0;

  newedge1->start = newedge2->start = newedge3->start = inverse1->end = inverse2->end = inverse3->end = edge->start;
  newedge1->end = newedge4->start = inverse1->start = inverse4->end = pd->size++;
  newedge3->end = newedge6->start = inverse3->start = inverse6->end = pd->size++;
  newedge4->end = newedge5->start = inverse4->start = inverse5->end = pd->size++;
  newedge6->end = newedge7->start = inverse6->start = inverse7->end = pd->size++;
  newedge2->end = newedge5->end = newedge7->end = inverse2->start = inverse5->start = inverse7->start = pd->size++;

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;
  newedge3->inverse = inverse3; inverse3->inverse = newedge3;
  newedge4->inverse = inverse4; inverse4->inverse = newedge4;
  newedge5->inverse = inverse5; inverse5->inverse = newedge5;
  newedge6->inverse = inverse6; inverse6->inverse = newedge6;
  newedge7->inverse = inverse7; inverse7->inverse = newedge7;

  newedge3->next = edge->next; newedge3->next->prev = newedge3;
  newedge1->prev = edge; edge->next = newedge1;

  newedge1->next = newedge2; newedge2->prev = newedge1;
  newedge2->next = newedge3; newedge3->prev = newedge2;
  inverse2->next = inverse5; inverse5->prev = inverse2;
  inverse5->next = inverse7; inverse7->prev = inverse5;
  inverse7->next = inverse2; inverse2->prev = inverse7;
  inverse1->next = inverse1->prev = newedge4;
  newedge4->next = newedge4->prev = inverse1;
  inverse3->next = inverse3->prev = newedge6;
  newedge6->next = newedge6->prev = inverse3;
  inverse4->next = inverse4->prev = newedge5;
  newedge5->next = newedge5->prev = inverse4;
  inverse6->next = inverse6->prev = newedge7;
  newedge7->next = newedge7->prev = inverse6;

  pd->firstedge[inverse1->start] = inverse1;
  pd->firstedge[inverse4->start] = inverse4;
  pd->firstedge[inverse5->start] = inverse5;
  pd->firstedge[newedge6->start] = newedge6;
  pd->firstedge[newedge7->start] = newedge7;

  pd->deg[edge->start] += 3;
  pd->deg[newedge1->end] = pd->deg[newedge4->end] = 2;
  pd->deg[newedge3->end] = pd->deg[newedge6->end] = 2;
  pd->deg[newedge2->end] = 3;

  pd->out[edge->start]++;
  pd->out[newedge1->end] = pd->out[newedge4->end] = 1;
  pd->out[newedge3->end] = pd->out[newedge6->end] = 1;
  pd->out[newedge2->end] = 1;

  pd->cuts++;
}

void destruct6(PreDeco *pd, Edge *edge) {
  edge->next = edge->next->next->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start] -= 3;
  pd->out[edge->start]--;
  pd->firstedge[edge->start] = edge;
}

void construct7(PreDeco *pd, Edge *edge) {
  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];
  Edge *newedge3 = &pd->edges[pd->nedges++];
  Edge *inverse3 = &pd->edges[pd->nedges++];
  Edge *newedge4 = &pd->edges[pd->nedges++];
  Edge *inverse4 = &pd->edges[pd->nedges++];
  Edge *newedge5 = &pd->edges[pd->nedges++];
  Edge *inverse5 = &pd->edges[pd->nedges++];
  Edge *newedge6 = &pd->edges[pd->nedges++];
  Edge *inverse6 = &pd->edges[pd->nedges++];
  Edge *newedge7 = &pd->edges[pd->nedges++];
  Edge *inverse7 = &pd->edges[pd->nedges++];

  newedge1->label = 0;
  inverse1->label = 1;
  newedge2->label = 1;
  inverse2->label = 0;
  newedge3->label = 1;
  inverse3->label = 0;
  newedge4->label = 0;
  inverse4->label = 0;
  newedge5->label = 0;
  inverse5->label = 1;
  newedge6->label = 0;
  inverse6->label = 1;
  newedge7->label = 1;
  inverse7->label = 0;

  newedge1->start = newedge2->start = inverse1->end = inverse2->end = edge->start;
  newedge1->end = newedge4->start = newedge5->start = inverse1->start = inverse4->end = inverse5->end = pd->size++;
  newedge2->end = newedge3->start = inverse2->start = inverse3->end = pd->size++;
  newedge3->end = newedge4->end = newedge7->start = inverse3->start = inverse4->start = inverse7->end = pd->size++;
  newedge5->end = newedge6->start = inverse5->start = inverse6->end = pd->size++;
  newedge6->end = newedge7->end = inverse6->start = inverse7->start = pd->size++;

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;
  newedge3->inverse = inverse3; inverse3->inverse = newedge3;
  newedge4->inverse = inverse4; inverse4->inverse = newedge4;
  newedge5->inverse = inverse5; inverse5->inverse = newedge5;
  newedge6->inverse = inverse6; inverse6->inverse = newedge6;
  newedge7->inverse = inverse7; inverse7->inverse = newedge7;

  newedge2->next = edge->next; newedge2->next->prev = newedge2;
  newedge1->prev = edge; edge->next = newedge1;

  newedge1->next = newedge2; newedge2->prev = newedge1;
  inverse1->next = newedge5; newedge5->prev = inverse1;
  newedge5->next = newedge4; newedge4->prev = newedge5;
  newedge4->next = inverse1; inverse1->prev = newedge4;
  inverse2->next = inverse2->prev = newedge3;
  newedge3->next = newedge3->prev = inverse2;
  inverse3->next = inverse4; inverse4->prev = inverse3;
  inverse4->next = newedge7; newedge7->prev = inverse4;
  newedge7->next = inverse3; inverse3->prev = newedge7;
  inverse5->next = inverse5->prev = newedge6;
  newedge6->next = newedge6->prev = inverse5;
  inverse6->next = inverse6->prev = inverse7;
  inverse7->next = inverse7->prev = inverse6;

  pd->firstedge[inverse1->start] = inverse1;
  pd->firstedge[newedge3->start] = newedge3;
  pd->firstedge[inverse5->start] = inverse5;
  pd->firstedge[inverse6->start] = inverse6;
  pd->firstedge[newedge7->start] = newedge7;

  pd->deg[edge->start] += 2;
  pd->deg[newedge1->end] = pd->deg[newedge3->end] = 3;
  pd->deg[newedge5->end] = pd->deg[newedge6->end] = 2;
  pd->deg[newedge2->end] = 2;

  pd->out[edge->start]++;
  pd->out[newedge1->end] = pd->out[newedge3->end] = 1;
  pd->out[newedge5->end] = pd->out[newedge6->end] = 1;
  pd->out[newedge2->end] = 1;

  pd->cuts++;
}

void destruct7(PreDeco *pd, Edge *edge) {
  edge->next = edge->next->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start] -= 2;
  pd->out[edge->start]--;
  pd->firstedge[edge->start] = edge;
}

void construct7b(PreDeco *pd, Edge *edge) {
  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];
  Edge *newedge3 = &pd->edges[pd->nedges++];
  Edge *inverse3 = &pd->edges[pd->nedges++];
  Edge *newedge4 = &pd->edges[pd->nedges++];
  Edge *inverse4 = &pd->edges[pd->nedges++];
  Edge *newedge5 = &pd->edges[pd->nedges++];
  Edge *inverse5 = &pd->edges[pd->nedges++];
  Edge *newedge6 = &pd->edges[pd->nedges++];
  Edge *inverse6 = &pd->edges[pd->nedges++];
  Edge *newedge7 = &pd->edges[pd->nedges++];
  Edge *inverse7 = &pd->edges[pd->nedges++];

  newedge1->label = 1;
  inverse1->label = 0;
  newedge2->label = 0;
  inverse2->label = 1;
  newedge3->label = 0;
  inverse3->label = 1;
  newedge4->label = 0;
  inverse4->label = 0;
  newedge5->label = 1;
  inverse5->label = 0;
  newedge6->label = 1;
  inverse6->label = 0;
  newedge7->label = 0;
  inverse7->label = 1;

  newedge1->start = newedge2->start = inverse1->end = inverse2->end = edge->start;
  newedge1->end = newedge4->start = newedge5->start = inverse1->start = inverse4->end = inverse5->end = pd->size++;
  newedge2->end = newedge3->start = inverse2->start = inverse3->end = pd->size++;
  newedge3->end = newedge4->end = newedge7->start = inverse3->start = inverse4->start = inverse7->end = pd->size++;
  newedge5->end = newedge6->start = inverse5->start = inverse6->end = pd->size++;
  newedge6->end = newedge7->end = inverse6->start = inverse7->start = pd->size++;

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;
  newedge3->inverse = inverse3; inverse3->inverse = newedge3;
  newedge4->inverse = inverse4; inverse4->inverse = newedge4;
  newedge5->inverse = inverse5; inverse5->inverse = newedge5;
  newedge6->inverse = inverse6; inverse6->inverse = newedge6;
  newedge7->inverse = inverse7; inverse7->inverse = newedge7;

  newedge1->next = edge->next; newedge1->next->prev = newedge1;
  newedge2->prev = edge; edge->next = newedge2;

  newedge1->prev = newedge2; newedge2->next = newedge1;
  inverse1->prev = newedge5; newedge5->next = inverse1;
  newedge5->prev = newedge4; newedge4->next = newedge5;
  newedge4->prev = inverse1; inverse1->next = newedge4;
  inverse2->prev = inverse2->next = newedge3;
  newedge3->prev = newedge3->next = inverse2;
  inverse3->prev = inverse4; inverse4->next = inverse3;
  inverse4->prev = newedge7; newedge7->next = inverse4;
  newedge7->prev = inverse3; inverse3->next = newedge7;
  inverse5->prev = inverse5->next = newedge6;
  newedge6->prev = newedge6->next = inverse5;
  inverse6->prev = inverse6->next = inverse7;
  inverse7->prev = inverse7->next = inverse6;

  pd->firstedge[inverse2->start] = inverse2;
  pd->firstedge[inverse3->start] = inverse3;
  pd->firstedge[newedge5->start] = newedge5;
  pd->firstedge[newedge6->start] = newedge6;
  pd->firstedge[inverse7->start] = inverse7;

  pd->deg[edge->start] += 2;
  pd->deg[newedge1->end] = pd->deg[newedge3->end] = 3;
  pd->deg[newedge5->end] = pd->deg[newedge6->end] = 2;
  pd->deg[newedge2->end] = 2;

  pd->out[edge->start]++;
  pd->out[newedge1->end] = pd->out[newedge3->end] = 1;
  pd->out[newedge5->end] = pd->out[newedge6->end] = 1;
  pd->out[newedge2->end] = 1;

  pd->cuts++;
}

void destruct7b(PreDeco *pd, Edge *edge) {
  edge->next = edge->next->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start] -= 2;
  pd->out[edge->start]--;
  pd->firstedge[edge->start] = edge;
}

void construct8(PreDeco *pd, Edge *edge) {
  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];
  Edge *newedge3 = &pd->edges[pd->nedges++];
  Edge *inverse3 = &pd->edges[pd->nedges++];

  newedge1->label = 0;
  inverse1->label = 1;
  newedge2->label = 1;
  inverse2->label = 0;
  newedge3->label = 0;
  inverse3->label = 1;

  newedge1->start = inverse1->end = edge->start;
  newedge2->start = inverse2->end = edge->next->end;
  newedge1->end = newedge3->start = inverse1->start = inverse3->end = pd->size++;
  newedge2->end = newedge3->end = inverse2->start = inverse3->start = pd->size++;

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;
  newedge3->inverse = inverse3; inverse3->inverse = newedge3;

  newedge2->next = edge->next->inverse->next; newedge2->next->prev = newedge2;
  newedge2->prev = edge->next->inverse; newedge2->prev->next = newedge2;
  newedge1->next = edge->next; newedge1->next->prev = newedge1;
  newedge1->prev = edge; edge->next = newedge1;

  inverse1->next = inverse1->prev = newedge3;
  newedge3->next = newedge3->prev = inverse1;
  inverse3->next = inverse3->prev = inverse2;
  inverse2->next = inverse2->prev = inverse3;

  pd->firstedge[inverse1->start] = inverse1;
  pd->firstedge[inverse3->start] = inverse3;

  pd->deg[edge->start]++;
  pd->deg[newedge2->start]++;
  pd->deg[newedge1->end] = pd->deg[newedge2->end] = 2;

  pd->out[newedge1->end] = pd->out[newedge2->end] = 1;

  newedge2->prev->label = 0;
}

void destruct8(PreDeco *pd, Edge *edge) {
  edge->next = edge->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start]--;
  pd->firstedge[edge->start] = edge;
  edge = edge->next->inverse;
  edge->next = edge->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start]--;
  edge->label = 1;
  pd->firstedge[edge->start] = edge;
}

void construct9(PreDeco *pd, Edge *edge) {
  Edge *newedge1 = &pd->edges[pd->nedges++];
  Edge *inverse1 = &pd->edges[pd->nedges++];
  Edge *newedge2 = &pd->edges[pd->nedges++];
  Edge *inverse2 = &pd->edges[pd->nedges++];

  newedge1->label = 0;
  inverse1->label = 1;
  newedge2->label = 1;
  inverse2->label = 0;

  newedge1->start = inverse1->end = edge->start;
  newedge2->start = inverse2->end = edge->next->inverse->next->end;
  newedge1->end = newedge2->end = inverse1->start = inverse2->start = pd->size++;

  newedge1->inverse = inverse1; inverse1->inverse = newedge1;
  newedge2->inverse = inverse2; inverse2->inverse = newedge2;

  newedge2->next = edge->next->inverse->next->inverse->next; newedge2->next->prev = newedge2;
  newedge2->prev = edge->next->inverse->next->inverse; newedge2->prev->next = newedge2;
  newedge1->next = edge->next; newedge1->next->prev = newedge1;
  newedge1->prev = edge; edge->next = newedge1;

  inverse1->next = inverse1->prev = inverse2;
  inverse2->next = inverse2->prev = inverse1;

  pd->firstedge[inverse1->start] = inverse1;

  pd->deg[edge->start]++;
  pd->deg[newedge2->start]++;
  pd->deg[newedge1->end] = 2;

  pd->out[newedge1->end] = 1;
  pd->out[newedge1->next->end]--;

  newedge1->next->inverse->label = 0;
  newedge2->prev->label = 0;
}

void destruct9(PreDeco *pd, Edge *edge) {
  edge->next = edge->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start]--;
  edge->next->inverse->label = 1;
  pd->firstedge[edge->start] = edge;
  edge = edge->next->inverse->next->inverse;
  edge->next = edge->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start]--;
  pd->out[edge->end]++;
  edge->label = 1;
  pd->firstedge[edge->start] = edge;
}

void construct10(PreDeco *pd, Edge *edge) {
  Edge *newedge = &pd->edges[pd->nedges++];
  Edge *inverse = &pd->edges[pd->nedges++];

  newedge->label = 0;
  inverse->label = 1;

  newedge->start = inverse->end = edge->start;
  newedge->end = inverse->start = edge->next->inverse->next->inverse->next->end;

  newedge->inverse = inverse; inverse->inverse = newedge;

  inverse->next = edge->next->inverse->next->inverse->next->inverse->next; inverse->next->prev = inverse;
  inverse->prev = edge->next->inverse->next->inverse->next->inverse; inverse->prev->next = inverse;
  newedge->next = edge->next; newedge->next->prev = newedge;
  newedge->prev = edge; edge->next = newedge;

  pd->deg[edge->start]++;
  pd->deg[newedge->end]++;

  pd->out[newedge->next->end]--;
  pd->out[newedge->next->inverse->next->end]--;

  newedge->next->inverse->label = 0;
  newedge->next->inverse->next->inverse->label = 0;
  inverse->prev->label = 0;
}

void destruct10(PreDeco *pd, Edge *edge) {
  edge->next = edge->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start]--;
  pd->out[edge->next->end]++;
  edge->next->inverse->label = 1;
  edge->next->inverse->next->inverse->label = 1;
  pd->firstedge[edge->start] = edge;
  edge = edge->next->inverse->next->inverse->next->inverse;
  edge->next = edge->next->next;
  edge->next->prev = edge;
  pd->deg[edge->start]--;
  pd->out[edge->end]++;
  edge->label = 1;
  pd->firstedge[edge->start] = edge;
}
