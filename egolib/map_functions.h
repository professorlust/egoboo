#pragma once

//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file map_functions.h
/// @brief Definitions for mpd functionality ported from cartman and EgoMap
///
/// @details

#include "typedef.h"

#if defined(__cplusplus)
extern "C"
{
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

    struct s_map;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#   define CARTMAN_FIXNUM            4.125f ///< 4.150f        ///< Magic number
#   define CARTMAN_SLOPE             50                        ///< increments for terrain slope

//--------------------------------------------------------------------------------------------
// generic functions
//--------------------------------------------------------------------------------------------

    bool_t twist_to_normal( Uint8 twist, float v[], float slide );

    struct s_map * map_generate_tile_twist_data( struct s_map * pmesh );
    struct s_map * map_generate_fan_type_data( struct s_map * pmesh );
    struct s_map * map_generate_vertex_data( struct s_map * pmesh );

//--------------------------------------------------------------------------------------------
// Cartman functions
//--------------------------------------------------------------------------------------------

    Uint8  cartman_calc_twist( int dx, int dy );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#define _map_functions_h
