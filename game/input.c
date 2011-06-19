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

/// @file input.c
/// @brief Keyboard, mouse, and joystick handling code.
/// @details

#include "input.h"

#if defined(USE_LUA_CONSOLE)
#    include "lua_console.h"
#else
#    include "egoboo_console.h"
#endif

#include "ui.h"
#include "log.h"
#include "network.h"
#include "menu.h"
#include "graphic.h"
#include "camera_system.h"

#include "egoboo_setup.h"
#include "egoboo_fileutil.h"
#include "egoboo_strutil.h"
#include "egoboo_math.h"
#include "egoboo.h"

#include "file_formats/controls_file.h"
#include "extensions/SDL_extensions.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

// generic device list
device_list_t     InputDevices;

//Raw input devices
mouse_data_t           mous = MOUSE_INIT;
keyboard_data_t        keyb = KEYBOARD_INIT;
joystick_data_t JoyList[MAX_JOYSTICK];
input_cursor_t          input_cursor = {0, 0, bfalse, bfalse, bfalse, bfalse};

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

static void input_system_init_keyboard( void );
static void input_system_init_mouse( void );
static void input_system_init_joysticks( void );
static void input_system_init_devices( void );

static void input_read_mouse( void );
static void input_read_keyboard( void );
static void input_read_joysticks( void );
static void input_read_joystick( int which );

static bool_t input_handle_SDL_Event( SDL_Event * pevt );
static bool_t input_handle_SDL_KEYDOWN( SDL_Event * pevt );
static bool_t input_handle_chat( SDL_Event * pevt );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void input_system_init_keyboard()
{
    // set up the keyboard
    BLANK_STRUCT( keyb )

    init_scancodes();

    keyb.on         = btrue;
    keyb.state_size = 0;
    keyb.state_ptr  = NULL;
}

//--------------------------------------------------------------------------------------------
void input_system_init_mouse()
{
    /// @details BB@> set up the mouse
    BLANK_STRUCT( mous )
    mous.on      = btrue;
    mous.sense   = 12; //24;
}

//--------------------------------------------------------------------------------------------
void input_system_init_joysticks()
{
    /// @details BB@> init the joysticks

    int i;

    for ( i = 0; i < MAX_JOYSTICK; i++ )
    {
        BLANK_STRUCT( JoyList[i] );

        if ( i < SDL_NumJoysticks() )
        {
            JoyList[i].sdl_ptr = SDL_JoystickOpen( i );
            JoyList[i].on      = ( NULL != JoyList[i].sdl_ptr );
        }
    }
}

//--------------------------------------------------------------------------------------------
void input_system_init_devices()
{
    int cnt;

    for ( cnt = 0; cnt < MAX_LOCAL_PLAYERS; cnt++ )
    {
        input_device_ctor( InputDevices.lst + cnt );
    }
    InputDevices.count = 0;
}

//--------------------------------------------------------------------------------------------
void input_system_init()
{
    /// @details BB@> initialize the inputs

    log_info( "Intializing SDL Joystick... " );
    if ( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) < 0 )
    {
        log_message( "Failed!\n" );
    }
    else
    {
        log_message( "Success!\n" );
    }

    input_system_init_keyboard();
    input_system_init_mouse();
    input_system_init_joysticks();
    input_system_init_devices();
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void input_read_mouse()
{
    int x, y, b;

    if ( process_running( PROC_PBASE( MProc ) ) )
    {
        b = SDL_GetMouseState( &x, &y );
    }
    else
    {
        b = SDL_GetRelativeMouseState( &x, &y );

        //Move mouse to the center of the screen since SDL does not detect motion outside the window
        if ( !cfg.dev_mode ) SDL_WarpMouse( GFX_WIDTH >> 1, GFX_HEIGHT >> 1 );
    }

    mous.x = -x; // mous.x and mous.y are the wrong type to use in above call
    mous.y = -y;
    mous.button[0] = ( b & SDL_BUTTON( 1 ) ) ? 1 : 0;
    mous.button[1] = ( b & SDL_BUTTON( 3 ) ) ? 1 : 0;
    mous.button[2] = ( b & SDL_BUTTON( 2 ) ) ? 1 : 0; // Middle is 2 on SDL
    mous.button[3] = ( b & SDL_BUTTON( 4 ) ) ? 1 : 0;

    // Mouse mask
    mous.b = ( mous.button[3] << 3 ) | ( mous.button[2] << 2 ) | ( mous.button[1] << 1 ) | ( mous.button[0] << 0 );
}

//--------------------------------------------------------------------------------------------
void input_read_keyboard()
{
    keyb.state_ptr = SDL_GetKeyState( &( keyb.state_size ) );
}

//--------------------------------------------------------------------------------------------
void input_read_joystick( int which )
{
    int dead_zone = 0x8000 / 10;
    int i, button_count, x, y;
    joystick_data_t * pjoy;

    if ( !JoyList[which].on ) return;

    pjoy = JoyList + which;

    // get the raw values
    x = SDL_JoystickGetAxis( pjoy->sdl_ptr, 0 );
    y = SDL_JoystickGetAxis( pjoy->sdl_ptr, 1 );

    // make a dead zone
    if ( x > dead_zone ) x -= dead_zone;
    else if ( x < -dead_zone ) x += dead_zone;
    else x = 0;

    if ( y > dead_zone ) y -= dead_zone;
    else if ( y < -dead_zone ) y += dead_zone;
    else y = 0;

    // store the values
    pjoy->x = x / ( float )( 0x8000 - dead_zone );
    pjoy->y = y / ( float )( 0x8000 - dead_zone );

    // get buttons
    button_count = SDL_JoystickNumButtons( pjoy->sdl_ptr );
    button_count = MIN( JOYBUTTON, button_count );
    for ( i = 0; i < button_count; i++ )
    {
        pjoy->button[i] = SDL_JoystickGetButton( pjoy->sdl_ptr, i );
    }

    // buttonmask mask
    pjoy->b = 0;
    for ( i = 0; i < button_count; i++ )
    {
        SET_BIT( pjoy->b, pjoy->button[i] << i );
    }

    return;
}

//--------------------------------------------------------------------------------------------
void input_read_joysticks()
{
    int cnt;

    SDL_JoystickUpdate();
    for ( cnt = 0; cnt < MAX_JOYSTICK; cnt++ )
    {
        input_read_joystick( cnt );
    }
}

//--------------------------------------------------------------------------------------------
bool_t input_handle_chat( SDL_Event * pevt )
{
    Uint32 kmod;
    bool_t is_alt, is_shift;

    if ( NULL == pevt || SDL_KEYDOWN != pevt->type ) return bfalse;

    kmod = SDL_GetModState();

    is_alt   = HAS_SOME_BITS( kmod, KMOD_ALT | KMOD_CTRL );
    is_shift = HAS_SOME_BITS( kmod, KMOD_SHIFT );

    if ( is_alt ) return bfalse;

    if ( SDLK_RETURN == pevt->key.keysym.sym || SDLK_KP_ENTER == pevt->key.keysym.sym )
    {
        net_chat.buffer[net_chat.buffer_count] = CSTR_END;
        keyb.chat_mode = bfalse;
        keyb.chat_done = btrue;
        SDL_EnableKeyRepeat( 0, SDL_DEFAULT_REPEAT_DELAY );
    }
    else if ( SDLK_ESCAPE == pevt->key.keysym.sym )
    {
        // reset the keyboard buffer
        keyb.chat_mode = bfalse;
        keyb.chat_done = bfalse;
        net_chat.buffer_count = 0;
        net_chat.buffer[0] = CSTR_END;
        SDL_EnableKeyRepeat( 0, SDL_DEFAULT_REPEAT_DELAY );
    }
    else if ( SDLK_BACKSPACE == pevt->key.keysym.sym )
    {
        if ( net_chat.buffer_count > 0 )
        {
            net_chat.buffer_count--;
        }
        net_chat.buffer[net_chat.buffer_count] = CSTR_END;
    }
    else if ( net_chat.buffer_count < CHAT_BUFFER_SIZE )
    {
        if ( is_shift )
        {
            net_chat.buffer[net_chat.buffer_count++] = scancode_to_ascii_shift[pevt->key.keysym.sym];
        }
        else
        {
            net_chat.buffer[net_chat.buffer_count++] = scancode_to_ascii[pevt->key.keysym.sym];
        }
        net_chat.buffer[net_chat.buffer_count] = CSTR_END;
    }

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t input_handle_SDL_KEYDOWN( SDL_Event * pevt )
{
    bool_t handled = bfalse;

    if ( NULL == pevt || SDL_KEYDOWN != pevt->type ) return bfalse;

    if ( keyb.chat_mode )
    {
        handled = input_handle_chat( pevt );
    }
    else
    {
        if ( SDLK_ESCAPE == pevt->key.keysym.sym )
        {
            // tell the main process about the escape request
            EProc->escape_requested = btrue;
            handled = btrue;
        }
    }

    return handled;
}

//--------------------------------------------------------------------------------------------
bool_t input_handle_SDL_Event( SDL_Event * pevt )
{
    bool_t handled = bfalse;

    if ( NULL == pevt ) return bfalse;

    handled = btrue;
    switch ( pevt->type )
    {
        case SDL_ACTIVEEVENT:
            // the application has gained or lost some form of focus
            if ( SDL_APPACTIVE == pevt->active.type && 1 == pevt->active.gain )
            {
                // the application has recovered from being minimized
                // the textures need to be reloaded into OpenGL memory

                gfx_reload_all_textures();
            }
            else if ( SDL_APPMOUSEFOCUS == pevt->active.type )
            {
                if ( 1 == pevt->active.gain )
                {
                    // gained mouse focus
                    mous.on = btrue;
                }
                else
                {
                    // lost mouse focus
                    mous.on = bfalse;
                }
            }
            else if ( SDL_APPINPUTFOCUS == pevt->active.type )
            {
                if ( 1 == pevt->active.gain )
                {
                    // gained mouse focus
                    keyb.on = btrue;
                }
                else
                {
                    // lost mouse focus
                    keyb.on = bfalse;
                }
            }
            break;

        case SDL_VIDEORESIZE:
            if ( SDL_VIDEORESIZE == pevt->resize.type )
            {
                // The video has been resized.
                // If the game is active, some camera info mught need to be recalculated
                // and possibly the auto-formatting for the menu system and the ui system
                // The ui will handle its own issues.

                // grab all the new SDL screen info
                SDLX_Get_Screen_Info( &sdl_scr, SDL_FALSE );
            }
            break;

        case SDL_VIDEOEXPOSE:
            // something has been done to the screen and it needs to be re-drawn.
            // For instance, a window above the app window was moved. This has no
            // effect on the game at the moment.
            break;

        case SDL_MOUSEBUTTONDOWN:
            if ( pevt->button.button == SDL_BUTTON_WHEELUP )
            {
                input_cursor.z++;
                input_cursor.wheel_event = btrue;
            }
            else if ( pevt->button.button == SDL_BUTTON_WHEELDOWN )
            {
                input_cursor.z--;
                input_cursor.wheel_event = btrue;
            }
            else
            {
                input_cursor.pending_click = btrue;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            input_cursor.pending_click = bfalse;
            break;

        case SDL_MOUSEMOTION:
            input_cursor.x = pevt->motion.x;
            input_cursor.y = pevt->motion.y;
            break;

        case SDL_QUIT:
            //Someone pressed the little X in the corner while running windowed mode
            exit( 0 );
            break;

            // use this loop to grab any console-mode entry from the keyboard
        case SDL_KEYDOWN:
            input_handle_SDL_KEYDOWN( pevt );
            break;

        default:
            handled = btrue;
            break;
    }

    return handled;
}

//--------------------------------------------------------------------------------------------
void input_read_all_devices()
{
    /// @details ZZ@> This function gets all the current player input states

    SDL_Event evt;

    if ( EMPTY_BIT_FIELD == SDL_WasInit( SDL_INIT_EVERYTHING ) ) return;

    // Run through SDL's event loop to get info in the way that we want
    // it for the Gui code
    while ( SDL_PollEvent( &evt ) )
    {
        if ( cfg.dev_mode )
        {
            if ( NULL == egoboo_console_handle_events( &evt ) )
            {
                continue;
            }
        }

        ui_handle_SDL_Event( &evt );

        input_handle_SDL_Event( &evt );
    }

    // Get immediate mode state for the rest of the game
    input_read_keyboard();
    input_read_mouse();
    input_read_joysticks();
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void input_cursor_reset()
{
    input_cursor.pressed       = bfalse;
    input_cursor.clicked       = bfalse;
    input_cursor.pending_click = bfalse;
    input_cursor.wheel_event   = bfalse;
    input_cursor.z             = 0;
}

//--------------------------------------------------------------------------------------------
void input_cursor_finish_wheel_event()
{
    input_cursor.wheel_event   = bfalse;
    input_cursor.z             = 0;
}

//--------------------------------------------------------------------------------------------
bool_t input_cursor_wheel_event_pending()
{
    if ( input_cursor.wheel_event && 0 == input_cursor.z )
    {
        input_cursor.wheel_event = bfalse;
    }

    return input_cursor.wheel_event;
}

//--------------------------------------------------------------------------------------------
int translate_string_to_input_type( const char *string )
{
    /// @details ZF@> This function turns a string into a input type (mouse, keyboard, joystick, etc.)

    int retval = INPUT_DEVICE_UNKNOWN;

    if ( INVALID_CSTR( string ) ) return INPUT_DEVICE_UNKNOWN;

    if ( 0 == strcmp( string, "KEYBOARD" ) )
    {
        retval = INPUT_DEVICE_KEYBOARD;
    }
    else if ( 0 == strcmp( string, "MOUSE" ) )
    {
        retval = INPUT_DEVICE_MOUSE;
    }
    else if ( 0 == strncmp( string, "JOYSTICK", 8 ) && CSTR_END != string[9] )
    {
        int ijoy = ( int )string[9] - ( int )'A';

        if ( ijoy >= 0 && ijoy < MAX_JOYSTICK )
        {
            retval = INPUT_DEVICE_JOY + ijoy;
        }
    }

    // No matches
    if ( INPUT_DEVICE_UNKNOWN == retval )
    {
        retval = INPUT_DEVICE_KEYBOARD;
        log_warning( "Unknown device controller parsed (%s) - defaulted to Keyboard\n", string );
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
const char* translate_input_type_to_string( const int type )
{
    /// @details ZF@> This function turns a input type into a string

    static STRING retval;

    if ( type == INPUT_DEVICE_KEYBOARD )
    {
        strncpy( retval, "KEYBOARD", SDL_arraysize( retval ) );
    }
    else if ( type == INPUT_DEVICE_MOUSE )
    {
        strncpy( retval, "MOUSE", SDL_arraysize( retval ) );
    }
    else if ( IS_VALID_JOYSTICK( type ) )
    {
        snprintf( retval, SDL_arraysize( retval ), "JOYSTICK_%c", ( char )( 'A' + ( type - INPUT_DEVICE_JOY ) ) );
    }
    else
    {
        // No matches
        strncpy( retval, "UNKNOWN", SDL_arraysize( retval ) );
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
// input_device_t
//--------------------------------------------------------------------------------------------
input_device_t * input_device_ctor( input_device_t * pdevice )
{
    if ( NULL == pdevice ) return NULL;

    // clear out all the data, including all
    // control data
    BLANK_STRUCT_PTR( pdevice )

    pdevice->device_type = INPUT_DEVICE_UNKNOWN;

    return pdevice;
}

//--------------------------------------------------------------------------------------------
void input_device_init( input_device_t * pdevice, int req_type )
{
    int type;

    if ( NULL == pdevice ) return;

    // save the old type
    if ( INPUT_DEVICE_UNKNOWN == req_type )
    {
        type = pdevice->device_type;
    }
    else
    {
        type = req_type;
    }

    // clear out all the data
    BLANK_STRUCT_PTR( pdevice )

    // set everything that is not 0, bfalse, 0.0f, etc.
    pdevice->sustain     = 0.58f;
    pdevice->cover       = 1.0f - pdevice->sustain;
    pdevice->device_type = type;
}

//--------------------------------------------------------------------------------------------
BIT_FIELD input_device_get_buttonmask( input_device_t *pdevice )
{
    // assume the worst
    BIT_FIELD buttonmask = EMPTY_BIT_FIELD;

    // make sure the idevice is valid
    if ( NULL == pdevice ) return EMPTY_BIT_FIELD;

    // scan for devices that use buttons
    if ( INPUT_DEVICE_MOUSE == pdevice->device_type )
    {
        buttonmask = mous.b;
    }
    else if ( IS_VALID_JOYSTICK( pdevice->device_type ) )
    {
        int ijoy = pdevice->device_type - INPUT_DEVICE_JOY;

        buttonmask = JoyList[ijoy].b;
    }

    return buttonmask;
}

//--------------------------------------------------------------------------------------------
bool_t input_device_is_enabled( input_device_t *pdevice )
{
    //@details ZF@> This determines if the specified input device is enabled or not

    // assume the worst
    bool_t retval = bfalse;

    // make sure the idevice is valid
    if ( NULL == pdevice ) return bfalse;

    if ( INPUT_DEVICE_KEYBOARD == pdevice->device_type )
    {
        retval = keyb.on;
    }
    else if ( INPUT_DEVICE_MOUSE == pdevice->device_type )
    {
        retval = mous.on;
    }
    else if ( IS_VALID_JOYSTICK( pdevice->device_type ) )
    {
        int ijoy = pdevice->device_type - INPUT_DEVICE_JOY;

        retval = JoyList[ijoy].on;
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
bool_t input_device_control_active( input_device_t *pdevice, CONTROL_BUTTON icontrol )
{
    /// @details ZZ@> This function returns btrue if the given icontrol is pressed...

    bool_t      retval = bfalse;
    control_t   * pcontrol;

    // make sure the idevice is valid
    if ( NULL == pdevice ) return bfalse;
    pcontrol = pdevice->control + icontrol;

    // if no control information was loaded, it can't be pressed
    if ( !pcontrol->loaded ) return bfalse;

    if ( INPUT_DEVICE_KEYBOARD == pdevice->device_type || pcontrol->is_key )
    {
        retval = SDLKEYDOWN( pcontrol->tag );
    }
    else
    {
        retval = ( input_device_get_buttonmask( pdevice ) == pcontrol->tag );
    }

    return retval;
}

//--------------------------------------------------------------------------------------------

void input_device_add_latch( input_device_t * pdevice, float newx, float newy )
{
    // Sustain old movements to ease mouse/keyboard play

    float dist;

    if ( NULL == pdevice ) return;

    pdevice->latch_old = pdevice->latch;

    pdevice->latch.x = pdevice->latch.x * pdevice->sustain + newx * pdevice->cover;
    pdevice->latch.y = pdevice->latch.y * pdevice->sustain + newy * pdevice->cover;

    // make sure that the latch never overflows
    dist = pdevice->latch.x * pdevice->latch.x + pdevice->latch.y * pdevice->latch.y;
    if ( dist > 1.0f )
    {
        float scale = 1.0f / SQRT( dist );

        pdevice->latch.x *= scale;
        pdevice->latch.y *= scale;
    }
}
