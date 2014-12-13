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

/// @file egolib_typedef.h
/// @details some basic types that are used throughout the game code.

#include <assert.h>
#include <ctype.h>

#include <SDL.h>

// this include must be the absolute last include
#include "egolib_config.h"

// this include must be the absolute last include
#include "../egolib/mem.h"

#if defined(__cplusplus)
extern "C"
{
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

    struct s_irect;
    typedef struct s_irect irect_t;

    struct s_frect;
    typedef struct s_frect frect_t;

    struct s_ego_irect;
    typedef struct s_ego_irect ego_irect_t;

    struct s_ego_frect;
    typedef struct s_ego_frect ego_frect_t;

    struct s_pair;
    typedef struct s_pair IPair;

    struct s_range;
    typedef struct s_range FRange;

//--------------------------------------------------------------------------------------------
// place the definition of the lambda operator in a macro
#define LAMBDA(AA,BB,CC) ((AA) ? (BB) : (CC))

//--------------------------------------------------------------------------------------------
// portable definition of assert. the c++ version can be activated below.
// make assert into a warning if _DEBUG is not defined

    void non_fatal_assert( int val, const char * format, ... );

#if defined(_DEBUG)
#   define C_EGOBOO_ASSERT(X) assert(X)
#else
#   define C_EGOBOO_ASSERT(X) non_fatal_assert( X, "%s - failed an assert \"%s\"\n", __FUNCTION__, #X )
#endif

//--------------------------------------------------------------------------------------------
// a replacement for memset()
#    if !defined(BLANK_STRUCT)
#       define BLANK_STRUCT(XX)  memset( &(XX), 0, sizeof(XX) );
#    endif

#    if !defined(BLANK_STRUCT_PTR)
#       define BLANK_STRUCT_PTR(XX)  memset( XX, 0, sizeof( *(XX) ) );
#    endif

#    if !defined(BLANK_ARY)
#       define BLANK_ARY(XX)  memset( XX, 0, sizeof( XX ) );
#    endif

//--------------------------------------------------------------------------------------------
// BOOLEAN

#   define C_TRUE SDL_TRUE
#   define C_FALSE SDL_FALSE
#   define C_BOOLEAN SDL_bool

#   if defined(__cplusplus)
#       define ego_true  true
#       define ego_false false
#       define ego_bool  bool
#   else
#       define ego_true  C_TRUE
#       define ego_false C_FALSE
#       define ego_bool  C_BOOLEAN
#endif

    // this typedef must be after the enum definition or gcc has a fit

#   if !defined(TO_EGO_BOOL)
#       if defined(__cplusplus)
#           define TO_EGO_BOOL(VAL) LAMBDA(VAL, ego_true, ego_false)
#           define TO_C_BOOL(VAL)   LAMBDA(VAL, C_TRUE, C_FALSE)
#       else
#           define TO_EGO_BOOL(VAL) (VAL)
#           define TO_C_BOOL(VAL)   (VAL)
#       endif
#   endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

/// special return values
    enum e_egolib_rv
    {
        rv_error   = -1,
        rv_fail    = C_FALSE,
        rv_success = C_TRUE
    };

    // this typedef must be after the enum definition or gcc has a fit
    typedef enum e_egolib_rv egolib_rv;

//--------------------------------------------------------------------------------------------
// 24.8 fixed point types

    typedef Uint32 UFP8_T;
    typedef Sint32 SFP8_T;

    /// fast version of V1 / 256
#   define UFP8_TO_UINT(V1)   ( ((unsigned)(V1)) >> 8 )
    /// signed version of V1 / 256
#   define SFP8_TO_SINT(V1)   LAMBDA( (V1) < 0, -((signed)UFP8_TO_UINT(-V1)), (signed)UFP8_TO_UINT(V1) )

    /// fast version of V1 / 256
#   define UINT_TO_UFP8(V1)   ( ((unsigned)(V1)) << 8 )
    /// signed version of V1 / 256
#   define SINT_TO_SFP8(V1)   LAMBDA( (V1) < 0, -((signed)UINT_TO_UFP8(-V1)), (signed)UINT_TO_UFP8(V1) )

    /// version of V1 / 256.0f
#   define FP8_TO_FLOAT(V1)   ( (float)(V1) * INV_0100 )
    /// version of V1 * 256.0f
#   define FLOAT_TO_FP8(V1)   ( (Uint32)((V1) * (float)(0x0100) ) )

#   define FP8_MUL(V1, V2)    ( ((V1)*(V2)) >> 8 )               ///< this may overflow if V1 or V2 have non-zero bits in their upper 8 bits
#   define FP8_DIV(V1, V2)    ( ((V1)<<8) / (V2) )               ///< this  will fail if V1 has bits in the upper 8 bits

//--------------------------------------------------------------------------------------------
    /// the type for the 16-bit value used to store angles
    typedef Uint16   FACING_T;

    /// the type for the 14-bit value used to store angles
    typedef FACING_T TURN_T;

#   define TO_FACING(X) ((FACING_T)(X))
#   define TO_TURN(X)   ((TURN_T)((TO_FACING(X)>>2) & TRIG_TABLE_MASK))

//--------------------------------------------------------------------------------------------
// 16.16 fixed point types

    typedef Uint32 UFP16_T;
    typedef Sint32 SFP16_T;

#   define FLOAT_TO_FP16( V1 )  ( (Uint32)((V1) * 0x00010000) )
#   define FP16_TO_FLOAT( V1 )  ( (float )((V1) * 0.0000152587890625f ) )

//--------------------------------------------------------------------------------------------
// BIT FIELDS
    typedef Uint32 BIT_FIELD;                              ///< A big string supporting 32 bits

#   define FULL_BIT_FIELD      0x7FFFFFFF                  ///< A bit string where all bits are flagged as 1
#   define EMPTY_BIT_FIELD     0                           ///< A bit string where all bits are flagged as 0
#   define FILL_BIT_FIELD(XX)  (XX) = FULL_BIT_FIELD       ///< Fills up all bits in a bit pattern
#   define RESET_BIT_FIELD(XX) (XX) = EMPTY_BIT_FIELD      ///< Resets all bits in a BIT_FIELD to 0

#    if !defined(SET_BIT)
#       define SET_BIT(XX, YY) (XX) |= (YY)
#    endif

#    if !defined(UNSET_BIT)
#       define UNSET_BIT(XX, YY) (XX) &= ~(YY)
#    endif

#    if !defined(BOOL_TO_BIT)
#       define BOOL_TO_BIT(XX)       LAMBDA(XX, 1, 0 )
#    endif

#    if !defined(BIT_TO_BOOL)
#       define BIT_TO_BOOL(XX)       (1 == (XX))
#    endif

#    if !defined(HAS_SOME_BITS)
#       define HAS_SOME_BITS(XX,YY) (0 != ((XX)&(YY)))
#    endif

#    if !defined(HAS_ALL_BITS)
#       define HAS_ALL_BITS(XX,YY)  ((YY) == ((XX)&(YY)))
#    endif

#    if !defined(HAS_NO_BITS)
#       define HAS_NO_BITS(XX,YY)   (0 == ((XX)&(YY)))
#    endif

#    if !defined(MISSING_BITS)
#       define MISSING_BITS(XX,YY)  (HAS_SOME_BITS(XX,YY) && !HAS_ALL_BITS(XX,YY))
#    endif

#   define CLIP_TO_08BITS( V1 )  ( (V1) & 0xFF       )
#   define CLIP_TO_16BITS( V1 )  ( (V1) & 0xFFFF     )
#   define CLIP_TO_24BITS( V1 )  ( (V1) & 0xFFFFFF   )
#   define CLIP_TO_32BITS( V1 )  ( (V1) & 0xFFFFFFFF )

//--------------------------------------------------------------------------------------------
// RECTANGLE
    struct s_irect
    {
        int left;
        int right;
        int top;
        int bottom;
    };

    C_BOOLEAN irect_point_inside( irect_t * prect, int   ix, int   iy );

    struct s_frect
    {
        float left;
        float right;
        float top;
        float bottom;
    };

    C_BOOLEAN frect_point_inside( frect_t * prect, float fx, float fy );

    struct s_ego_irect
    {
        int xmin, ymin;
        int xmax, ymax;
    };

    struct s_ego_frect
    {
        float xmin, ymin;
        float xmax, ymax;
    };

//--------------------------------------------------------------------------------------------
// PAIR AND RANGE

    /// Specifies a value between "base" and "base + rand"
    struct s_pair
    {
        int base, rand;
    };

    /// Specifies a value from "from" to "to"
    struct s_range
    {
        float from, to;
    };

    void pair_to_range( IPair pair, FRange * prange );
    void range_to_pair( FRange range, IPair * ppair );

    void ints_to_range( int base, int rand, FRange * prange );
    void floats_to_pair( float vmin, float vmax, IPair * ppair );

//--------------------------------------------------------------------------------------------
// IDSZ
    typedef Uint32 IDSZ;

#    if !defined(MAKE_IDSZ)
#       define MAKE_IDSZ(C0,C1,C2,C3) \
    ((IDSZ)( \
             ((((C0)-'A')&0x1F) << 15) |       \
             ((((C1)-'A')&0x1F) << 10) |       \
             ((((C2)-'A')&0x1F) <<  5) |       \
             ((((C3)-'A')&0x1F) <<  0)         \
           ))
#    endif

#   define IDSZ_NONE            MAKE_IDSZ('N','O','N','E')       ///< [NONE]

    const char * undo_idsz( IDSZ idsz );

//--------------------------------------------------------------------------------------------
// STRING
    typedef char STRING[256];

//--------------------------------------------------------------------------------------------
// ego_message_t

    /// the maximum length egoboo messages
#   define EGO_MESSAGE_SIZE      90

    typedef char ego_message_t[EGO_MESSAGE_SIZE];

//--------------------------------------------------------------------------------------------

/// the "base class" of Egoboo profiles
#   define  EGO_PROFILE_STUFF \
    C_BOOLEAN loaded;                  /** Was the data read in? */ \
    STRING name;                    /** Usually the source filename */ \
    int    request_count;           /** the number of attempted spawnx */ \
    int    create_count;            /** the number of successful spawns */

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/// The latch used by the input system
    struct s_latch
    {
        float          x;         ///< the x input
        float          y;         ///< the y input
        BIT_FIELD      b;         ///< the button bits
    };
    typedef struct s_latch latch_t;

    void latch_init( latch_t * platch );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// References

    /// base reference type
    typedef Uint16 REF_T;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// definitions for the compiler environment

#   define EGOBOO_ASSERT(X) C_EGOBOO_ASSERT(X)
#   define _EGOBOO_ASSERT(X) C_EGOBOO_ASSERT(X)

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#define _egolib_typedef_h
