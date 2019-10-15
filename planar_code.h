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

#ifndef PLANAR_CODE_H_
#define PLANAR_CODE_H_

#include "decogen.h"

void compute_planar_code(PreDeco*, unsigned char*, int[]);
void write_planar_header();
void write_planar_code(PreDeco*);
void write_complete_code(PreDeco*);

#endif
