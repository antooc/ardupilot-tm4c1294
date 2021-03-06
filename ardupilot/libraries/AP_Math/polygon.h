/*
 * polygon.h
 * Copyright (C) Andrew Tridgell 2011
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __AP_Math_polygon_h__
#define __AP_Math_polygon_h__


#include "vector2.h"

template <typename T>
bool        Polygon_outside(const Vector2<T> &P, const Vector2<T> *V, unsigned n);
template <typename T>
bool        Polygon_complete(const Vector2<T> *V, unsigned n);


#endif /* __AP_Math_polygon_h__ */