/* Egoboo - game.c
 */

/*
    This file is part of Egoboo.

    Egoboo is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Egoboo is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Egoboo.  If not, see <http:// www.gnu.org/licenses/>.
*/

//#define MENU_DEMO    // Uncomment this to build just the menu demo
#define DECLARE_GLOBALS
#include "egoboo.h"
#include "Clock.h"
#include "Link.h"
#include "Ui.h"
#include "Font.h"
#include "Log.h"
#include <SDL_endian.h>

#include "char.h"
#include "camera.h"
#include "id_md2.h"

float sinlut[MAXLIGHTROTATION];
float coslut[MAXLIGHTROTATION];

//---------------------------------------------------------------------------------------------
int what_action( char cTmp )
{
  // ZZ> This function changes a letter into an action code
  int action;
  action = ACTIONDA;
  if ( cTmp=='U' || cTmp=='u' )  action = ACTIONUA;
  if ( cTmp=='T' || cTmp=='t' )  action = ACTIONTA;
  if ( cTmp=='S' || cTmp=='s' )  action = ACTIONSA;
  if ( cTmp=='C' || cTmp=='c' )  action = ACTIONCA;
  if ( cTmp=='B' || cTmp=='b' )  action = ACTIONBA;
  if ( cTmp=='L' || cTmp=='l' )  action = ACTIONLA;
  if ( cTmp=='X' || cTmp=='x' )  action = ACTIONXA;
  if ( cTmp=='F' || cTmp=='f' )  action = ACTIONFA;
  if ( cTmp=='P' || cTmp=='p' )  action = ACTIONPA;
  if ( cTmp=='Z' || cTmp=='z' )  action = ACTIONZA;
  return action;
}


//------------------------------------------------------------------------------
// Random Things-----------------------------------------------------------------
//------------------------------------------------------------------------------
void make_newloadname( char *modname, char *appendname, char *newloadname )
{
  // ZZ> This function takes some names and puts 'em together
  int cnt, tnc;
  char ctmp;

  cnt = 0;
  ctmp = modname[cnt];
  while ( ctmp != 0 )
  {
    newloadname[cnt] = ctmp;
    cnt++;
    ctmp = modname[cnt];
  }
  tnc = 0;
  ctmp = appendname[tnc];
  while ( ctmp != 0 )
  {
    newloadname[cnt] = ctmp;
    cnt++;
    tnc++;
    ctmp = appendname[tnc];
  }
  newloadname[cnt] = 0;
}


////--------------------------------------------------------------------------------------------
void export_one_character( int character, int owner, int number, bool_t is_local )
{
  // ZZ> This function exports a character
  int tnc, profile;
  char letter;
  char fromdir[128];
  char todir[128];
  char fromfile[128];
  char tofile[128];
  char todirname[16];
  char todirfullname[64];

  // Don't export enchants
  disenchant_character( character );

  profile = chrmodel[character];
  if ( ( capcancarrytonextmodule[profile] || capisitem[profile]==bfalse ) && exportvalid )
  {
    // TWINK_BO.OBJ
    sprintf( todirname, "badname.obj" );
    tnc = 0;
    letter = chrname[owner][tnc];
    while ( tnc < 8 && letter != 0 )
    {
      letter = chrname[owner][tnc];
      if ( letter >= 'A' && letter <= 'Z' )  letter -= 'A' - 'a';
      if ( letter != 0 )
      {
        if ( letter < 'a' || letter > 'z' )  letter = '_';
        todirname[tnc] = letter;
        tnc++;
      }
    }
    todirname[tnc] = '.'; tnc++;
    todirname[tnc] = 'o'; tnc++;
    todirname[tnc] = 'b'; tnc++;
    todirname[tnc] = 'j'; tnc++;
    todirname[tnc] = 0;

    // Is it a character or an item?
    if ( owner != character )
    {
      // Item is a subdirectory of the owner directory...
      sprintf( todirfullname, "%s" SLASH_STR "%d.obj", todirname, number );
    }
    else
    {
      // Character directory
      sprintf( todirfullname, "%s", todirname );
    }


    // players/twink.obj or players/twink.obj/sword.obj
    if ( is_local )
    {
      sprintf( todir, "players" SLASH_STR "%s", todirfullname );
    }
    else
    {
      sprintf( todir, "remote" SLASH_STR "%s", todirfullname );
    }

    // modules/advent.mod/objects/advent.obj
    sprintf( fromdir, "%s", madname[profile] );

    // Delete all the old items
    if ( owner == character )
    {
      tnc = 0;
      while ( tnc < 8 )
      {
        sprintf( tofile, "%s" SLASH_STR "%d.obj", todir, tnc );  /*.OBJ*/
        fs_removeDirectoryAndContents( tofile );
        tnc++;
      }
    }

    // Make the directory
    fs_createDirectory( todir );

    // Build the DATA.TXT file
    sprintf( tofile, "%s" SLASH_STR "data.txt", todir );  /*DATA.TXT*/
    export_one_character_profile( tofile, character );

    // Build the SKIN.TXT file
    sprintf( tofile, "%s" SLASH_STR "skin.txt", todir );  /*SKIN.TXT*/
    export_one_character_skin( tofile, character );

    // Build the NAMING.TXT file
    sprintf( tofile, "%s" SLASH_STR "naming.txt", todir );  /*NAMING.TXT*/
    export_one_character_name( tofile, character );

    // Copy all of the misc. data files
    sprintf( fromfile, "%s" SLASH_STR "message.txt", fromdir );  /*MESSAGE.TXT*/
    sprintf( tofile, "%s" SLASH_STR "message.txt", todir );  /*MESSAGE.TXT*/
    fs_copyFile( fromfile, tofile );
    sprintf( fromfile, "%s" SLASH_STR "tris.md2", fromdir );  /*TRIS.MD2*/
    sprintf( tofile,   "%s" SLASH_STR "tris.md2", todir );  /*TRIS.MD2*/
    fs_copyFile( fromfile, tofile );
    sprintf( fromfile, "%s" SLASH_STR "copy.txt", fromdir );  /*COPY.TXT*/
    sprintf( tofile,   "%s" SLASH_STR "copy.txt", todir );  /*COPY.TXT*/
    fs_copyFile( fromfile, tofile );
    sprintf( fromfile, "%s" SLASH_STR "script.txt", fromdir );
    sprintf( tofile,   "%s" SLASH_STR "script.txt", todir );
    fs_copyFile( fromfile, tofile );
    sprintf( fromfile, "%s" SLASH_STR "enchant.txt", fromdir );
    sprintf( tofile,   "%s" SLASH_STR "enchant.txt", todir );
    fs_copyFile( fromfile, tofile );
    sprintf( fromfile, "%s" SLASH_STR "credits.txt", fromdir );
    sprintf( tofile,   "%s" SLASH_STR "credits.txt", todir );
    fs_copyFile( fromfile, tofile );

    // Copy all of the particle files
    tnc = 0;
    while ( tnc < MAXPRTPIPPEROBJECT )
    {
      sprintf( fromfile, "%s" SLASH_STR "part%d.txt", fromdir, tnc );
      sprintf( tofile,   "%s" SLASH_STR "part%d.txt", todir,   tnc );
      fs_copyFile( fromfile, tofile );
      tnc++;
    }

    // Copy all of the sound files
    tnc = 0;
    while ( tnc < MAXWAVE )
    {
      sprintf( fromfile, "%s" SLASH_STR "sound%d.wav", fromdir, tnc );
      sprintf( tofile,   "%s" SLASH_STR "sound%d.wav", todir,   tnc );
      fs_copyFile( fromfile, tofile );
      tnc++;
    }

    // Copy all of the image files
    tnc = 0;
    while ( tnc < 4 )
    {
      sprintf( fromfile, "%s" SLASH_STR "tris%d.bmp", fromdir, tnc );
      sprintf( tofile,   "%s" SLASH_STR "tris%d.bmp", todir,   tnc );
      fs_copyFile( fromfile, tofile );
      sprintf( fromfile, "%s" SLASH_STR "icon%d.bmp", fromdir, tnc );
      sprintf( tofile,   "%s" SLASH_STR "icon%d.bmp", todir,   tnc );
      fs_copyFile( fromfile, tofile );
      tnc++;
    }
  }
}

//---------------------------------------------------------------------------------------------
void export_all_players( bool_t require_local )
{
  // ZZ> This function saves all the local players in the
  //     PLAYERS directory
  bool_t is_local;
  int cnt, character, item, number;

  // Check each player
  for ( cnt = 0; cnt < MAXPLAYER; cnt++ )
  {
    is_local = ( 0 != pladevice[cnt] );
    if ( require_local && !is_local ) continue;
    if ( !plavalid[cnt] ) continue;

    // Is it alive?
    character = plaindex[cnt];
    if ( !chron[character] || !chralive[character] ) continue;

    // Export the character
    number = 0;
    export_one_character( character, character, number++, is_local );

    // Export the left hand item
    item = chrholdingwhich[character][0];
    if ( item != MAXCHR && chrisitem[item] )  export_one_character( item, character, number++, is_local );

    // Export the right hand item
    item = chrholdingwhich[character][1];
    if ( item != MAXCHR && chrisitem[item] )  export_one_character( item, character, number++, is_local );

    // Export the inventory
    item = chrnextinpack[character];
    while ( item != MAXCHR )
    {
      if ( chrisitem[item] )
      {
        export_one_character( item, character, number++, is_local );
      }

      item = chrnextinpack[item];
    }
  }

}

//---------------------------------------------------------------------------------------------
void export_all_local_players( void )
{
  // ZZ> This function saves all the local players in the
  //     PLAYERS directory

  // Check each player
  if ( exportvalid )
  {
    export_all_players( btrue );
  }
}

//---------------------------------------------------------------------------------------------
void quit_module( void )
{
  // ZZ> This function forces a return to the menu
  moduleactive = bfalse;
  hostactive = bfalse;
  export_all_local_players();
  gamepaused = bfalse;
  if ( soundvalid ) Mix_FadeOutChannel( -1, 500 );     // Stop all sounds that are playing
}

//--------------------------------------------------------------------------------------------
void quit_game( void )
{
  log_info( "Exiting Egoboo %s the good way...\n", VERSION );
  // ZZ> This function exits the game entirely
  if ( gameactive )
  {
    gameactive = bfalse;
  }
  if ( moduleactive )
  {
    quit_module();
  }
  if ( floatmemory != NULL )
  {
    free( floatmemory );
    floatmemory = NULL;
  }
}

//--------------------------------------------------------------------------------------------
void goto_colon( FILE* fileread )
{
  // ZZ> This function moves a file read pointer to the next colon
//    char cTmp;
  Uint32 ch = fgetc( fileread );

//    fscanf(fileread, "%c", &cTmp);
  while ( ch != ':' )
  {
    if ( ch == EOF )
    {
      // not enough colons in file!
      log_error( "There are not enough colons in file! (%s)\n", globalname );
    }

    ch = fgetc( fileread );
  }
}

//--------------------------------------------------------------------------------------------
Uint8 goto_colon_yesno( FILE* fileread )
{
  // ZZ> This function moves a file read pointer to the next colon, or it returns
  //     bfalse if there are no more
  char cTmp;

  do
  {
    if ( fscanf( fileread, "%c", &cTmp )==EOF )
    {
      return bfalse;
    }
  }
  while ( cTmp != ':' );

  return btrue;
}

//--------------------------------------------------------------------------------------------
char get_first_letter( FILE* fileread )
{
  // ZZ> This function returns the next non-whitespace character
  char cTmp;
  fscanf( fileread, "%c", &cTmp );
  while ( isspace( cTmp ) )
  {
    fscanf( fileread, "%c", &cTmp );
  }
  return cTmp;
}

//--------------------------------------------------------------------------------------------
// Tag Reading---------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void reset_tags()
{
  // ZZ> This function resets the tags
  numscantag = 0;
}

//--------------------------------------------------------------------------------------------
int read_tag( FILE *fileread )
{
  // ZZ> This function finds the next tag, returning btrue if it found one
  if ( goto_colon_yesno( fileread ) )
  {
    if ( numscantag < MAXTAG )
    {
      fscanf( fileread, "%s%d", tagname[numscantag], &tagvalue[numscantag] );
      numscantag++;
      return btrue;
    }
  }
  return bfalse;
}

//--------------------------------------------------------------------------------------------
void read_all_tags( char *szFilename )
{
  // ZZ> This function reads the scancode.txt file
  FILE* fileread;


  reset_tags();
  fileread = fopen( szFilename, "r" );
  if ( fileread )
  {
    while ( read_tag( fileread ) );
    fclose( fileread );
  }
}

//--------------------------------------------------------------------------------------------
int tag_value( char *string )
{
  // ZZ> This function matches the string with its tag, and returns the value...
  //     It will return 255 if there are no matches.
  int cnt;

  cnt = 0;
  while ( cnt < numscantag )
  {
    if ( strcmp( string, tagname[cnt] )==0 )
    {
      // They match
      return tagvalue[cnt];
    }
    cnt++;
  }
  // No matches
  return 255;
}

//--------------------------------------------------------------------------------------------
void read_controls( char *szFilename )
{
  // ZZ> This function reads the controls.txt file
  FILE* fileread;
  char currenttag[TAGSIZE];
  int cnt;


  fileread = fopen( szFilename, "r" );
  if ( fileread )
  {
    cnt = 0;
    while ( goto_colon_yesno( fileread ) && cnt < MAXCONTROL )
    {
      fscanf( fileread, "%s", currenttag );
      controlvalue[cnt] = tag_value( currenttag );
      // printf("CTRL: %i, %s\n", controlvalue[cnt], currenttag);
      controliskey[cnt] = ( currenttag[0] == 'K' );
      cnt++;
    }
    fclose( fileread );
  }
}

//--------------------------------------------------------------------------------------------
Uint8 control_key_is_pressed( Uint8 control )
{
  // ZZ> This function returns btrue if the given control is pressed...
  if ( netmessagemode )  return bfalse;

  if ( sdlkeybuffer )
    return ( sdlkeybuffer[controlvalue[control]]!=0 );
  else
    return bfalse;
}

//--------------------------------------------------------------------------------------------
Uint8 control_mouse_is_pressed( Uint8 control )
{
  // ZZ> This function returns btrue if the given control is pressed...
  if ( controliskey[control] )
  {
    if ( netmessagemode )  return bfalse;

    if ( sdlkeybuffer )
      return ( sdlkeybuffer[controlvalue[control]]!=0 );
    else
      return bfalse;
  }
  else
  {
    return ( msb==controlvalue[control] );
  }
  return bfalse;
}

//--------------------------------------------------------------------------------------------
Uint8 control_joya_is_pressed( Uint8 control )
{
  // ZZ> This function returns btrue if the given control is pressed...
  if ( controliskey[control] )
  {
    if ( netmessagemode )  return bfalse;

    if ( sdlkeybuffer )
      return ( sdlkeybuffer[controlvalue[control]]!=0 );
    else
      return bfalse;
  }
  else
  {
    return ( jab==controlvalue[control] );
  }
  return bfalse;
}

//--------------------------------------------------------------------------------------------
Uint8 control_joyb_is_pressed( Uint8 control )
{
  // ZZ> This function returns btrue if the given control is pressed...
  if ( controliskey[control] )
  {
    if ( netmessagemode )  return bfalse;

    if ( sdlkeybuffer )
      return ( sdlkeybuffer[controlvalue[control]]!=0 );
    else
      return bfalse;
  }
  else
  {
    return ( jbb==controlvalue[control] );
  }
  return bfalse;
}

//--------------------------------------------------------------------------------------------
void undo_idsz( int idsz )
{
  // ZZ> This function takes an integer and makes an text IDSZ out of it.
  //     It will set valueidsz to "NONE" if the idsz is 0
  if ( idsz == IDSZNONE )
  {
    sprintf( valueidsz, "NONE" );
  }
  else
  {
    valueidsz[0] = ( ( idsz>>15 )&31 ) + 'A';
    valueidsz[1] = ( ( idsz>>10 )&31 ) + 'A';
    valueidsz[2] = ( ( idsz>>5 )&31 ) + 'A';
    valueidsz[3] = ( ( idsz )&31 ) + 'A';
    valueidsz[4] = 0;
  }
  return;
}

//--------------------------------------------------------------------------------------------
int get_idsz( FILE* fileread )
{
  // ZZ> This function reads and returns an IDSZ tag, or IDSZNONE if there wasn't one
  int test;

  int idsz = IDSZNONE;
  char cTmp = get_first_letter( fileread );
  if ( cTmp == '[' )
  {
    fscanf( fileread, "%c", &cTmp );  cTmp = cTmp-'A';  idsz=idsz|( cTmp<<15 );
    fscanf( fileread, "%c", &cTmp );  cTmp = cTmp-'A';  idsz=idsz|( cTmp<<10 );
    fscanf( fileread, "%c", &cTmp );  cTmp = cTmp-'A';  idsz=idsz|( cTmp<<5 );
    fscanf( fileread, "%c", &cTmp );  cTmp = cTmp-'A';  idsz=idsz|( cTmp );
  }

  test = Make_IDSZ( 'N','O','N','E' );  // [NONE]

  if ( idsz == test )
    idsz = IDSZNONE;

  return idsz;
}

//--------------------------------------------------------------------------------------------
int get_free_message( void )
{
  // This function finds the best message to use
  // Pick the first one
  int tnc = msgstart;
  msgstart++;
  msgstart = msgstart % maxmessage;
  return tnc;
}


//--------------------------------------------------------------------------------------------
void getadd( int min, int value, int max, int* valuetoadd )
{
  // ZZ> This function figures out what value to add should be in order
  //     to not overflow the min and max bounds
  int newvalue;

  newvalue = value+( *valuetoadd );
  if ( newvalue < min )
  {
    // Increase valuetoadd to fit
    *valuetoadd = min-value;
    if ( *valuetoadd > 0 )  *valuetoadd=0;
    return;
  }


  if ( newvalue > max )
  {
    // Decrease valuetoadd to fit
    *valuetoadd = max-value;
    if ( *valuetoadd < 0 )  *valuetoadd=0;
  }
}

//--------------------------------------------------------------------------------------------
void fgetadd( float min, float value, float max, float* valuetoadd )
{
  // ZZ> This function figures out what value to add should be in order
  //     to not overflow the min and max bounds
  float newvalue;


  newvalue = value+( *valuetoadd );
  if ( newvalue < min )
  {
    // Increase valuetoadd to fit
    *valuetoadd = min-value;
    if ( *valuetoadd > 0 )  *valuetoadd=0;
    return;
  }


  if ( newvalue > max )
  {
    // Decrease valuetoadd to fit
    *valuetoadd = max-value;
    if ( *valuetoadd < 0 )  *valuetoadd=0;
  }
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void load_action_names( char* loadname )
{
  // ZZ> This function loads all of the 2 letter action names
  FILE* fileread;
  int cnt;
  char first, second;

  fileread = fopen( loadname, "r" );
  if ( fileread )
  {
    cnt = 0;
    while ( cnt < MAXACTION )
    {
      goto_colon( fileread );
      fscanf( fileread, "%c%c", &first, &second );
      cActionName[cnt][0] = first;
      cActionName[cnt][1] = second;
      cnt++;
    }
    fclose( fileread );
  }
}

//--------------------------------------------------------------------------------------------
void get_name( FILE* fileread, char *szName )
{
  // ZZ> This function loads a string of up to MAXCAPNAMESIZE characters, parsing
  //     it for underscores.  The szName argument is rewritten with the null terminated
  //     string
  int cnt;
  char cTmp;
  char szTmp[256];


  fscanf( fileread, "%s", szTmp );
  cnt = 0;
  while ( cnt < MAXCAPNAMESIZE-1 )
  {
    cTmp = szTmp[cnt];
    if ( cTmp=='_' )  cTmp=' ';
    szName[cnt] = cTmp;
    cnt++;
  }
  szName[cnt] = 0;
}

//--------------------------------------------------------------------------------------------
void read_setup( char* filename )
{
  // ZZ> This function loads the setup file

  ConfigFilePtr lConfigSetup;
  char lCurSectionName[64];
  bool_t lTempBool;
  Sint32 lTmpInt;
  char lTmpStr[24];


  lConfigSetup = OpenConfigFile( filename );
  if ( lConfigSetup == NULL )
  {
    // Major Error
    log_error( "Could not find Setup.txt\n" );
  }
  else
  {
    globalname = filename; // heu!?

    /*********************************************

    GRAPHIC Section

    *********************************************/

    strcpy( lCurSectionName, "GRAPHIC" );

    // Draw z reflection?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "Z_REFLECTION", &zreflect ) == 0 )
    {
      zreflect = bfalse; // default
    }

    // Max number of vertrices (Should always be 100!)
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "MAX_NUMBER_VERTICES", &lTmpInt ) == 0 )
    {
      lTmpInt = 100; // default
    }
    maxtotalmeshvertices = lTmpInt * 1024;

    // Do fullscreen?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "FULLSCREEN", &fullscreen ) == 0 )
    {
      fullscreen = bfalse; // default
    }

    // Screen Size
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "SCREENSIZE_X", &lTmpInt ) == 0 )
    {
      lTmpInt = 640; // default
    }
    scrx = lTmpInt;
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "SCREENSIZE_Y", &lTmpInt ) == 0 )
    {
      lTmpInt = 480; // default
    }
    scry = lTmpInt;

    // Color depth
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "COLOR_DEPTH", &lTmpInt ) == 0 )
    {
      lTmpInt = 16; // default
    }
    scrd = lTmpInt;

    // The z depth
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "Z_DEPTH", &lTmpInt ) == 0 )
    {
      lTmpInt = 16; // default
    }
    scrz = lTmpInt;

    // Max number of messages displayed
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "MAX_TEXT_MESSAGE", &lTmpInt ) == 0 )
    {
      lTmpInt = 1; // default
    }
    messageon = btrue;
    maxmessage = lTmpInt;
    if ( maxmessage < 1 )  { maxmessage = 1;  messageon = bfalse; }
    if ( maxmessage > MAXMESSAGE )  { maxmessage = MAXMESSAGE; }

    // Show status bars? (Life, mana, character icons, etc.)
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "STATUS_BAR", &staton ) == 0 )
    {
      staton = btrue; // default
    }
    wraptolerance = 32;
    if ( staton )
    {
      wraptolerance = 90;
    }

    // Perspective correction
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "PERSPECTIVE_CORRECT", &perspective ) == 0 )
    {
      perspective = bfalse; // default
    }

    // Enable dithering? (Reduces quality but increases preformance)
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "DITHERING", &dither ) == 0 )
    {
      dither = bfalse; // default
    }

    // Reflection fadeout
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "FLOOR_REFLECTION_FADEOUT", &lTempBool ) == 0 )
    {
      lTempBool = bfalse; // default
    }
    if ( lTempBool )
    {
      reffadeor = 0;
    }
    else
    {
      reffadeor = 255;
    }

    // Draw Reflection?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "REFLECTION", &refon ) == 0 )
    {
      refon = bfalse; // default
    }

    // Draw shadows?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "SHADOWS", &shaon ) == 0 )
    {
      shaon = bfalse; // default
    }

    // Draw good shadows (BAD! Not working yet)
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "SHADOW_AS_SPRITE", &shasprite ) == 0 )
    {
      shasprite = btrue; // default
    }

    // Draw phong mapping?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "PHONG", &phongon ) == 0 )
    {
      phongon = btrue; // default
    }

    // Draw water with more layers?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "MULTI_LAYER_WATER", &twolayerwateron ) == 0 )
    {
      twolayerwateron = bfalse; // default
    }

    // TODO: This is not implemented
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "OVERLAY", &overlayvalid ) == 0 )
    {
      overlayvalid = bfalse; // default
    }

    // Allow backgrounds?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "BACKGROUND", &backgroundvalid ) == 0 )
    {
      backgroundvalid = bfalse; // default
    }

    // Enable fog?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "FOG", &fogallowed ) == 0 )
    {
      fogallowed = bfalse; // default
    }

    // Do gourad shading?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "GOURAUD_SHADING", &lTempBool ) == 0 )
    {
      shading = GL_SMOOTH; // default
    }
    else
    {
      shading = lTempBool ? GL_SMOOTH : GL_FLAT;
    }

    // Enable antialiasing?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "ANTIALIASING", &antialiasing ) == 0 )
    {
      antialiasing = bfalse; // default
    }

    // Do we do texture filtering?
    if ( GetConfigValue( lConfigSetup, lCurSectionName, "TEXTURE_FILTERING", lTmpStr, 24 ) == 0 )
    {
      strcpy( lTmpStr, "LINEAR" ); // default
    }

    if ( lTmpStr[0] == 'L' || lTmpStr[0] == 'l' )  texturefilter = 1;
    if ( lTmpStr[0] == 'B' || lTmpStr[0] == 'b' )  texturefilter = 2;
    if ( lTmpStr[0] == 'T' || lTmpStr[0] == 't' )  texturefilter = 3;
    if ( lTmpStr[0] == 'A' || lTmpStr[0] == 'a' )  texturefilter = 4;


    /*********************************************

    SOUND Section

    *********************************************/

    strcpy( lCurSectionName, "SOUND" );

    // Enable sound
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "SOUND", &soundvalid ) == 0 )
    {
      soundvalid = bfalse; // default
    }

    // Enable music
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "MUSIC", &musicvalid ) == 0 )
    {
      musicvalid = bfalse; // default
    }

    // Music volume
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "MUSIC_VOLUME", &musicvolume ) == 0 )
    {
      musicvolume = 50; // default
    }

    // Sound volume
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "SOUND_VOLUME", &soundvolume ) == 0 )
    {
      soundvolume = 75; // default
    }

    // Max number of sound channels playing at the same time
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "MAX_SOUND_CHANNEL", &maxsoundchannel ) == 0 )
    {
      maxsoundchannel = 16; // default
    }
    if ( maxsoundchannel < 8 ) maxsoundchannel = 8;
    if ( maxsoundchannel > 32 ) maxsoundchannel = 32;

    // The output buffer size
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "OUPUT_BUFFER_SIZE", &buffersize ) == 0 )
    {
      buffersize = 2048; // default
    }
    if ( buffersize < 512 ) buffersize = 512;
    if ( buffersize > 8196 ) buffersize = 8196;


    /*********************************************

    CONTROL Section

    *********************************************/

    strcpy( lCurSectionName, "CONTROL" );

    // Camera control mode
    if ( GetConfigValue( lConfigSetup, lCurSectionName, "AUTOTURN_CAMERA", lTmpStr, 24 ) == 0 )
    {
      strcpy( lTmpStr, "GOOD" ); // default
    }

    if ( lTmpStr[0] == 'G' || lTmpStr[0] == 'g' )  autoturncamera = 255;
    if ( lTmpStr[0] == 'T' || lTmpStr[0] == 't' )  autoturncamera = btrue;
    if ( lTmpStr[0] == 'F' || lTmpStr[0] == 'f' )  autoturncamera = bfalse;

    //[claforte] Force autoturncamera to bfalse, or else it doesn't move right.
    // autoturncamera = bfalse;



    /*********************************************

    NETWORK Section

    *********************************************/

    strcpy( lCurSectionName, "NETWORK" );

    // Enable networking systems?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "NETWORK_ON", &networkon ) == 0 )
    {
      networkon = bfalse; // default
    }

    // Max lag
    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "LAG_TOLERANCE", &lTmpInt ) == 0 )
    {
      lTmpInt = 2; // default
    }
    lag = lTmpInt;

    /*
    goto_colon(fileread); fscanf(fileread, "%d", &orderlag);

    if ( GetConfigIntValue( lConfigSetup, lCurSectionName, "RTS_LAG_TOLERANCE", &lTmpInt ) == 0 )
      {
      lTmpInt = 25; // default
      }
    orderlag = lTmpInt;
    */

    // Name or IP of the host or the target to join
    if ( GetConfigValue( lConfigSetup, lCurSectionName, "HOST_NAME", nethostname, 64 ) == 0 )
    {
      strcpy( nethostname, "no host" ); // default
    }

    // Multiplayer name
    if ( GetConfigValue( lConfigSetup, lCurSectionName, "MULTIPLAYER_NAME", netmessagename, 64 ) == 0 )
    {
      strcpy( netmessagename, "little Raoul" ); // default
    }


    /*********************************************

    DEBUG Section

    *********************************************/

    strcpy( lCurSectionName, "DEBUG" );

    // Show the FPS counter?
    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "DISPLAY_FPS", &lTempBool ) == 0 )
    {
      lTempBool = btrue; // default
    }
    fpson = lTempBool;

    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "HIDE_MOUSE", &gHideMouse ) == 0 )
    {
      gHideMouse = btrue; // default
    }

    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "GRAB_MOUSE", &gGrabMouse ) == 0 )
    {
      gGrabMouse = btrue; // default
    }

    if ( GetConfigBooleanValue( lConfigSetup, lCurSectionName, "DEV_MODE", &gDevMode ) == 0 )
    {
      gDevMode = btrue; // default
    }

    CloseConfigFile( lConfigSetup );

  }
}
//--------------------------------------------------------------------------------------------
void log_madused( char *savename )
{
  // ZZ> This is a debug function for checking model loads
  FILE* hFileWrite;
  int cnt;

  hFileWrite = fopen( savename, "w" );
  if ( hFileWrite )
  {
    fprintf( hFileWrite, "Slot usage for objects in last module loaded...\n" );
    fprintf( hFileWrite, "%d of %d frames used...\n", madloadframe, MAXFRAME );
    cnt = 0;
    while ( cnt < MAXMODEL )
    {
      fprintf( hFileWrite, "%3d %32s %s\n", cnt, capclassname[cnt], madname[cnt] );
      cnt++;
    }
    fclose( hFileWrite );
  }
}

//---------------------------------------------------------------------------------------------
float light_for_normal( int rotation, int normal, float lx, float ly, float lz, float ambi )
{
  // ZZ> This function helps make_lighttable
  float fTmp;
  float nx, ny, nz;
  float sinrot, cosrot;

  nx = kMd2Normals[normal][0];
  ny = kMd2Normals[normal][1];
  nz = kMd2Normals[normal][2];
  sinrot = sinlut[rotation];
  cosrot = coslut[rotation];
  fTmp = cosrot*nx + sinrot*ny;
  ny = cosrot*ny - sinrot*nx;
  nx = fTmp;
  fTmp = nx*lx + ny*ly + nz*lz + ambi;
  if ( fTmp < ambi ) fTmp = ambi;
  return fTmp;
}


//---------------------------------------------------------------------------------------------
void make_lighttable( float lx, float ly, float lz, float ambi )
{
  // ZZ> This function makes a light table to fake directional lighting
  int lev, cnt, tnc;
  int itmp, itmptwo;

  // Build a lookup table for sin/cos
  for ( cnt = 0; cnt < MAXLIGHTROTATION; cnt++ )
  {
    sinlut[cnt] = SIN( TWO_PI*cnt/MAXLIGHTROTATION );
    coslut[cnt] = COS( TWO_PI*cnt/MAXLIGHTROTATION );
  }

  for ( cnt = 0; cnt < MD2LIGHTINDICES-1; cnt++ )  // Spikey mace
  {
    for ( tnc = 0; tnc < MAXLIGHTROTATION; tnc++ )
    {
      lev = MAXLIGHTLEVEL-1;
      itmp = ( 255*light_for_normal( tnc,
                                     cnt,
                                     lx*lev/MAXLIGHTLEVEL,
                                     ly*lev/MAXLIGHTLEVEL,
                                     lz*lev/MAXLIGHTLEVEL,
                                     ambi ) );
      // This creates the light value for each level entry
      while ( lev >= 0 )
      {
        itmptwo = ( ( ( lev*itmp/( MAXLIGHTLEVEL-1 ) ) ) );
        if ( itmptwo > 255 )  itmptwo = 255;
        lighttable[lev][tnc][cnt] = ( Uint8 ) itmptwo;
        lev--;
      }
    }
  }
  // Fill in index number 162 for the spike mace
  for ( tnc = 0; tnc < MAXLIGHTROTATION; tnc++ )
  {
    lev = MAXLIGHTLEVEL-1;
    itmp = 255;
    // This creates the light value for each level entry
    while ( lev >= 0 )
    {
      itmptwo = ( ( ( lev*itmp/( MAXLIGHTLEVEL-1 ) ) ) );
      if ( itmptwo > 255 )  itmptwo = 255;
      lighttable[lev][tnc][cnt] = ( Uint8 ) itmptwo;
      lev--;
    }
  }
}

//---------------------------------------------------------------------------------------------
int vertexconnected( int modelindex, int vertex )
{
  // ZZ> This function returns 1 if the model vertex is connected, 0 otherwise
  int cnt, tnc, entry;

  entry = 0;
  for ( cnt = 0; cnt < madcommands[modelindex]; cnt++ )
  {
    for ( tnc = 0; tnc < madcommandsize[modelindex][cnt]; tnc++ )
    {
      if ( madcommandvrt[modelindex][entry] == vertex )
      {
        // The vertex is used
        return 1;
      }
      entry++;
    }
  }

  // The vertex is not used
  return 0;
}

//---------------------------------------------------------------------------------------------
void get_madtransvertices( int modelindex )
{
  // ZZ> This function gets the number of vertices to transform for a model...
  //     That means every one except the grip ( unconnected ) vertices
  int cnt, trans = 0;

  for ( cnt = 0; cnt < madvertices[modelindex]; cnt++ )
    trans += vertexconnected( modelindex, cnt );

  madtransvertices[modelindex] = trans;
}

//---------------------------------------------------------------------------------------------
int rip_md2_header( void )
{
  // ZZ> This function makes sure an md2 is really an md2
  int iTmp;
  int* ipIntPointer;

  // Check the file type
  ipIntPointer = ( int* ) cLoadBuffer;
#ifdef SDL_LIL_ENDIAN
  iTmp = ipIntPointer[0];
#else
  iTmp = SDL_Swap32( ipIntPointer[0] );
#endif
  if ( iTmp != MD2START ) return bfalse;

  return btrue;
}

//---------------------------------------------------------------------------------------------
void fix_md2_normals( Uint16 modelindex )
{
  // ZZ> This function helps light not flicker so much
  int cnt, tnc;
  Uint8 indexofcurrent, indexofnext, indexofnextnext, indexofnextnextnext;
  Uint8 indexofnextnextnextnext;
  Uint32 frame;

  frame = madframestart[modelindex];
  cnt = 0;
  while ( cnt < madvertices[modelindex] )
  {
    tnc = 0;
    while ( tnc < madframes[modelindex] )
    {
      indexofcurrent = madvrta[frame][cnt];
      indexofnext = madvrta[frame+1][cnt];
      indexofnextnext = madvrta[frame+2][cnt];
      indexofnextnextnext = madvrta[frame+3][cnt];
      indexofnextnextnextnext = madvrta[frame+4][cnt];
      if ( indexofcurrent == indexofnextnext && indexofnext != indexofcurrent )
      {
        madvrta[frame+1][cnt] = indexofcurrent;
      }
      if ( indexofcurrent == indexofnextnextnext )
      {
        if ( indexofnext != indexofcurrent )
        {
          madvrta[frame+1][cnt] = indexofcurrent;
        }
        if ( indexofnextnext != indexofcurrent )
        {
          madvrta[frame+2][cnt] = indexofcurrent;
        }
      }
      if ( indexofcurrent == indexofnextnextnextnext )
      {
        if ( indexofnext != indexofcurrent )
        {
          madvrta[frame+1][cnt] = indexofcurrent;
        }
        if ( indexofnextnext != indexofcurrent )
        {
          madvrta[frame+2][cnt] = indexofcurrent;
        }
        if ( indexofnextnextnext != indexofcurrent )
        {
          madvrta[frame+3][cnt] = indexofcurrent;
        }
      }
      tnc++;
    }
    cnt++;
  }
}

//---------------------------------------------------------------------------------------------
void rip_md2_commands( Uint16 modelindex )
{
  // ZZ> This function converts an md2's GL commands into our little command list thing
  int iTmp;
  float fTmpu, fTmpv;
  int iNumVertices;
  int tnc;

  // char* cpCharPointer = (char*) cLoadBuffer;
  int* ipIntPointer = ( int* ) cLoadBuffer;
  float* fpFloatPointer = ( float* ) cLoadBuffer;

  // Number of GL commands in the MD2
#ifdef SDL_LIL_ENDIAN
  int iNumCommands = ipIntPointer[9];
#else
  int iNumCommands = SDL_Swap32( ipIntPointer[9] );
#endif

  // Offset (in DWORDS) from the start of the file to the gl command list.
#ifdef SDL_LIL_ENDIAN
  int iCommandOffset = ipIntPointer[15]>>2;
#else
  int iCommandOffset = SDL_Swap32( ipIntPointer[15] )>>2;
#endif

  // Read in each command
  // iNumCommands isn't the number of commands, rather the number of dwords in
  // the command list...  Use iCommandCount to figure out how many we use
  int iCommandCount = 0;
  int entry = 0;

  int cnt = 0;
  while ( cnt < iNumCommands )
  {
#ifdef SDL_LIL_ENDIAN
    iNumVertices = ipIntPointer[iCommandOffset]; iCommandOffset++; cnt++;
#else
    iNumVertices = SDL_Swap32( ipIntPointer[iCommandOffset] );  iCommandOffset++;  cnt++;
#endif
    if ( iNumVertices != 0 )
    {
      if ( iNumVertices < 0 )
      {
        // Fans start with a negative
        iNumVertices = -iNumVertices;
        madcommandtype[modelindex][iCommandCount] = GL_TRIANGLE_FAN;
        madcommandsize[modelindex][iCommandCount] = ( Uint8 ) iNumVertices;
      }
      else
      {
        // Strips start with a positive
        madcommandtype[modelindex][iCommandCount] = GL_TRIANGLE_STRIP;
        madcommandsize[modelindex][iCommandCount] = ( Uint8 ) iNumVertices;
      }

      // Read in vertices for each command
      tnc = 0;
      while ( tnc < iNumVertices )
      {
#ifdef SDL_LIL_ENDIAN
        fTmpu = fpFloatPointer[iCommandOffset];  iCommandOffset++;  cnt++;
        fTmpv = fpFloatPointer[iCommandOffset];  iCommandOffset++;  cnt++;
        iTmp = ipIntPointer[iCommandOffset];  iCommandOffset++;  cnt++;
#else
        fTmpu = LoadFloatByteswapped( &fpFloatPointer[iCommandOffset] );  iCommandOffset++;  cnt++;
        fTmpv = LoadFloatByteswapped( &fpFloatPointer[iCommandOffset] );  iCommandOffset++;  cnt++;
        iTmp = SDL_Swap32( ipIntPointer[iCommandOffset] );  iCommandOffset++;  cnt++;
#endif
        madcommandu[modelindex][entry] = fTmpu-( 0.5f/64 ); // GL doesn't align correctly
        madcommandv[modelindex][entry] = fTmpv-( 0.5f/64 ); // with D3D
        madcommandvrt[modelindex][entry] = ( Uint16 ) iTmp;
        entry++;
        tnc++;
      }
      iCommandCount++;
    }
  }
  madcommands[modelindex] = iCommandCount;
}

//---------------------------------------------------------------------------------------------
int rip_md2_frame_name( int frame )
{
  // ZZ> This function gets frame names from the load buffer, it returns
  //     btrue if the name in cFrameName[] is valid
  int iFrameOffset;
  int iNumVertices;
  int iNumFrames;
  int cnt;
  int* ipNamePointer;
  int* ipIntPointer;
  int foundname;

  // Jump to the Frames section of the md2 data
  ipNamePointer = ( int* ) cFrameName;
  ipIntPointer = ( int* ) cLoadBuffer;

#ifdef SDL_LIL_ENDIAN
  iNumVertices = ipIntPointer[6];
  iNumFrames = ipIntPointer[10];
  iFrameOffset = ipIntPointer[14]>>2;
#else
  iNumVertices = SDL_Swap32( ipIntPointer[6] );
  iNumFrames = SDL_Swap32( ipIntPointer[10] );
  iFrameOffset = SDL_Swap32( ipIntPointer[14] )>>2;
#endif


  // Chug through each frame
  foundname = bfalse;
  cnt = 0;
  while ( cnt < iNumFrames && !foundname )
  {
    iFrameOffset+=6;
    if ( cnt == frame )
    {
      ipNamePointer[0] = ipIntPointer[iFrameOffset]; iFrameOffset++;
      ipNamePointer[1] = ipIntPointer[iFrameOffset]; iFrameOffset++;
      ipNamePointer[2] = ipIntPointer[iFrameOffset]; iFrameOffset++;
      ipNamePointer[3] = ipIntPointer[iFrameOffset]; iFrameOffset++;
      foundname = btrue;
    }
    else
    {
      iFrameOffset+=4;
    }
    iFrameOffset+=iNumVertices;
    cnt++;
  }
  cFrameName[15] = 0;  // Make sure it's null terminated
  return foundname;
}

//---------------------------------------------------------------------------------------------
void rip_md2_frames( Uint16 modelindex )
{
  // ZZ> This function gets frames from the load buffer and adds them to
  //     the indexed model
  Uint8 cTmpx, cTmpy, cTmpz;
  Uint8 cTmpNormalIndex;
  float fRealx, fRealy, fRealz;
  float fScalex, fScaley, fScalez;
  float fTranslatex, fTranslatey, fTranslatez;
  int iFrameOffset;
  int iNumVertices;
  int iNumFrames;
  int cnt, tnc;
  char* cpCharPointer;
  int* ipIntPointer;
  float* fpFloatPointer;


  // Jump to the Frames section of the md2 data
  cpCharPointer = ( char* ) cLoadBuffer;
  ipIntPointer = ( int* ) cLoadBuffer;
  fpFloatPointer = ( float* ) cLoadBuffer;

#ifdef SDL_LIL_ENDIAN
  iNumVertices = ipIntPointer[6];
  iNumFrames = ipIntPointer[10];
  iFrameOffset = ipIntPointer[14]>>2;
#else
  iNumVertices = SDL_Swap32( ipIntPointer[6] );
  iNumFrames = SDL_Swap32( ipIntPointer[10] );
  iFrameOffset = SDL_Swap32( ipIntPointer[14] )>>2;
#endif


  // Read in each frame
  madframestart[modelindex] = madloadframe;
  madframes[modelindex] = iNumFrames;
  madvertices[modelindex] = iNumVertices;
  madscale[modelindex] = ( float ) ( 1.0f/320.0f );  // Scale each vertex float to fit it in a short
  cnt = 0;
  while ( cnt < iNumFrames && madloadframe < MAXFRAME )
  {
#ifdef SDL_LIL_ENDIAN
    fScalex = fpFloatPointer[iFrameOffset]; iFrameOffset++;
    fScaley = fpFloatPointer[iFrameOffset]; iFrameOffset++;
    fScalez = fpFloatPointer[iFrameOffset]; iFrameOffset++;
    fTranslatex = fpFloatPointer[iFrameOffset]; iFrameOffset++;
    fTranslatey = fpFloatPointer[iFrameOffset]; iFrameOffset++;
    fTranslatez = fpFloatPointer[iFrameOffset]; iFrameOffset++;
#else
    fScalex = LoadFloatByteswapped( &fpFloatPointer[iFrameOffset] ); iFrameOffset++;
    fScaley = LoadFloatByteswapped( &fpFloatPointer[iFrameOffset] ); iFrameOffset++;
    fScalez = LoadFloatByteswapped( &fpFloatPointer[iFrameOffset] ); iFrameOffset++;
    fTranslatex = LoadFloatByteswapped( &fpFloatPointer[iFrameOffset] ); iFrameOffset++;
    fTranslatey = LoadFloatByteswapped( &fpFloatPointer[iFrameOffset] ); iFrameOffset++;
    fTranslatez = LoadFloatByteswapped( &fpFloatPointer[iFrameOffset] ); iFrameOffset++;
#endif


    iFrameOffset+=4;
    tnc = 0;
    while ( tnc < iNumVertices )
    {
      // This should work because it's reading a single character
      cTmpx = cpCharPointer[( iFrameOffset<<2 )];
      cTmpy = cpCharPointer[( iFrameOffset<<2 )+1];
      cTmpz = cpCharPointer[( iFrameOffset<<2 )+2];
      cTmpNormalIndex = cpCharPointer[( iFrameOffset<<2 )+3];
      fRealx = ( cTmpx*fScalex )+fTranslatex;
      fRealy = ( cTmpy*fScaley )+fTranslatey;
      fRealz = ( cTmpz*fScalez )+fTranslatez;
//            fRealx = (cTmpx*fScalex);
//            fRealy = (cTmpy*fScaley);
//            fRealz = (cTmpz*fScalez);
//            madvrtx[madloadframe][tnc] = (signed short) (fRealx*256); // HUK
      madvrtx[madloadframe][tnc] = ( signed short ) ( -fRealx*256 );
      madvrty[madloadframe][tnc] = ( signed short ) ( fRealy*256 );
      madvrtz[madloadframe][tnc] = ( signed short ) ( fRealz*256 );
      madvrta[madloadframe][tnc] = cTmpNormalIndex;
      iFrameOffset++;
      tnc++;
    }
    madloadframe++;
    cnt++;
  }
}

//---------------------------------------------------------------------------------------------
int load_one_md2( char* szLoadname, Uint16 modelindex )
{
  // ZZ> This function loads an id md2 file, storing the converted data in the indexed model
//    int iFileHandleRead;
  size_t iBytesRead = 0;
  int iReturnValue;

  // Read the input file
  FILE *file = fopen( szLoadname,"rb" );
  if ( !file )
  {
    log_warning( "Cannot load file! (%s)\n", szLoadname );
    return bfalse;
  }

  // Read up to MD2MAXLOADSIZE bytes from the file into the cLoadBuffer array.
  iBytesRead = fread( cLoadBuffer, 1, MD2MAXLOADSIZE, file );
  if ( iBytesRead == 0 )
    return bfalse;

  // Check the header
  // TODO: Verify that the header's filesize correspond to iBytesRead.
  iReturnValue = rip_md2_header();
  if ( iReturnValue == bfalse )
    return bfalse;

  // Get the frame vertices
  rip_md2_frames( modelindex );
  // Get the commands
  rip_md2_commands( modelindex );
  // Fix them normals
  fix_md2_normals( modelindex );
  // Figure out how many vertices to transform
  get_madtransvertices( modelindex );

  fclose( file );

  return btrue;
}

//--------------------------------------------------------------------------------------------
void make_enviro( void )
{
  // ZZ> This function sets up the environment mapping table
  int cnt;
  float z;
  float x, y;

  // Find the environment map positions
  for ( cnt = 0; cnt < MD2LIGHTINDICES; cnt++ )
  {
    x = kMd2Normals[cnt][0];
    y = kMd2Normals[cnt][1];
    x = ( ATAN2( y, x )+PI )/( PI );
    x--;

    if ( x < 0 )
      x--;

    indextoenvirox[cnt] = x;
  }

  for ( cnt = 0; cnt < 256; cnt++ )
  {
    z = cnt / 256.0f;  // Z is between 0 and 1
    lighttoenviroy[cnt] = z;
  }
}



//--------------------------------------------------------------------------------------------
void add_stat( Uint16 character )
{
  // ZZ> This function adds a status display to the do list
  if ( numstat < MAXSTAT )
  {
    statlist[numstat] = character;
    chrstaton[character] = btrue;
    numstat++;
  }
}

//--------------------------------------------------------------------------------------------
void move_to_top( Uint16 character )
{
  // ZZ> This function puts the character on top of the statlist
  int cnt, oldloc;


  // Find where it is
  oldloc = numstat;

  for ( cnt = 0; cnt < numstat; cnt++ )
    if ( statlist[cnt] == character )
    {
      oldloc = cnt;
      cnt = numstat;
    }

  // Change position
  if ( oldloc < numstat )
  {
    // Move all the lower ones up
    while ( oldloc > 0 )
    {
      oldloc--;
      statlist[oldloc+1] = statlist[oldloc];
    }
    // Put the character in the top slot
    statlist[0] = character;
  }
}

//--------------------------------------------------------------------------------------------
void sort_stat()
{
  // ZZ> This function puts all of the local players on top of the statlist
  int cnt;

  for ( cnt = 0; cnt < numpla; cnt++ )
    if ( plavalid[cnt] && pladevice[cnt] != INPUTNONE )
    {
      move_to_top( plaindex[cnt] );
    }
}
//--------------------------------------------------------------------------------------------
void play_action( Uint16 character, Uint16 action, Uint8 actionready )
{
  // ZZ> This function starts a generic action for a character
  if ( madactionvalid[chrmodel[character]][action] )
  {
    chrnextaction[character] = ACTIONDA;
    chraction[character] = action;
    chrlip[character] = 0;
    chrlastframe[character] = chrframe[character];
    chrframe[character] = madactionstart[chrmodel[character]][chraction[character]];
    chractionready[character] = actionready;
  }
}

//--------------------------------------------------------------------------------------------
void set_frame( Uint16 character, Uint16 frame, Uint8 lip )
{
  // ZZ> This function sets the frame for a character explicitly...  This is used to
  //     rotate Tank turrets
  chrnextaction[character] = ACTIONDA;
  chraction[character] = ACTIONDA;
  chrlip[character] = ( lip<<6 );
  chrlastframe[character] = madactionstart[chrmodel[character]][ACTIONDA] + frame;
  chrframe[character] = madactionstart[chrmodel[character]][ACTIONDA] + frame + 1;
  chractionready[character] = btrue;
}


//--------------------------------------------------------------------------------------------
int generate_number( int numbase, int numrand )
{
  // ZZ> This function generates a random number
  int tmp = 0;

  tmp = numbase;
  if ( numrand > 0 )
  {
    tmp+=( rand()%numrand );
  }
  else
  {
    log_warning( "One of the data pairs is wrong! (%i and %i) Cannot be 0 or less.\n", numbase, numrand );
    numrand = numbase;
  }

  return tmp;
}



//--------------------------------------------------------------------------------------------
void setup_alliances( char *modname )
{
  // ZZ> This function reads the alliance file
  char newloadname[256];
  char szTemp[256];
  Uint8 teama, teamb;
  FILE *fileread;


  // Load the file
  make_newloadname( modname, "gamedat" SLASH_STR "alliance.txt", newloadname );
  fileread = fopen( newloadname, "r" );
  if ( fileread )
  {
    while ( goto_colon_yesno( fileread ) )
    {
      fscanf( fileread, "%s", szTemp );
      teama = ( szTemp[0]-'A' )%MAXTEAM;
      fscanf( fileread, "%s", szTemp );
      teamb = ( szTemp[0]-'A' )%MAXTEAM;
      teamhatesteam[teama][teamb] = bfalse;
    }
    fclose( fileread );
  }
}

//--------------------------------------------------------------------------------------------
void make_twist()
{
  // ZZ> This function precomputes surface normals and steep hill acceleration for
  //     the mesh
  int cnt;
  int x, y;
  float xslide, yslide;

  cnt = 0;
  while ( cnt < 256 )
  {
    y = cnt>>4;
    x = cnt&15;
    y = y-7;  // -7 to 8
    x = x-7;  // -7 to 8
    mapudtwist[cnt] = 32768+y*SLOPE;
    maplrtwist[cnt] = 32768+x*SLOPE;
    if ( ABS( y ) >=7 ) y=y<<1;
    if ( ABS( x ) >=7 ) x=x<<1;
    xslide = x*SLIDE;
    yslide = y*SLIDE;
    if ( xslide < 0 )
    {
      xslide+=SLIDEFIX;
      if ( xslide > 0 )
        xslide=0;
    }
    else
    {
      xslide-=SLIDEFIX;
      if ( xslide < 0 )
        xslide=0;
    }
    if ( yslide < 0 )
    {
      yslide+=SLIDEFIX;
      if ( yslide > 0 )
        yslide=0;
    }
    else
    {
      yslide-=SLIDEFIX;
      if ( yslide < 0 )
        yslide=0;
    }
    veludtwist[cnt] = -yslide*hillslide;
    vellrtwist[cnt] = xslide*hillslide;
    flattwist[cnt] = bfalse;
    if ( ABS( veludtwist[cnt] ) + ABS( vellrtwist[cnt] ) < SLIDEFIX*4 )
    {
      flattwist[cnt] = btrue;
    }
    cnt++;
  }
}

//--------------------------------------------------------------------------------------------
int load_mesh( char *modname )
{
  // ZZ> This function loads the level.mpd file
  FILE* fileread;
  char newloadname[256];
  int itmp, cnt;
  float ftmp;
  int fan;
  int numvert, numfan;
  int x, y, vert;

  make_newloadname( modname, "gamedat" SLASH_STR "level.mpd", newloadname );
  fileread = fopen( newloadname, "rb" );
  if ( fileread )
  {
#ifdef SDL_LIL_ENDIAN
    fread( &itmp, 4, 1, fileread );  if ( itmp != MAPID ) return bfalse;
    fread( &itmp, 4, 1, fileread );  numvert = itmp;
    fread( &itmp, 4, 1, fileread );  meshsizex = itmp;
    fread( &itmp, 4, 1, fileread );  meshsizey = itmp;
#else
    fread( &itmp, 4, 1, fileread );  if ( ( int )SDL_Swap32( itmp ) != MAPID ) return bfalse;
    fread( &itmp, 4, 1, fileread );  numvert = ( int )SDL_Swap32( itmp );
    fread( &itmp, 4, 1, fileread );  meshsizex = ( int )SDL_Swap32( itmp );
    fread( &itmp, 4, 1, fileread );  meshsizey = ( int )SDL_Swap32( itmp );
#endif

    numfan = meshsizex*meshsizey;
    meshedgex = meshsizex*128;
    meshedgey = meshsizey*128;
    numfanblock = ( ( meshsizex>>2 ) )*( ( meshsizey>>2 ) );  // MESHSIZEX MUST BE MULTIPLE OF 4
    watershift = 3;
    if ( meshsizex > 16 )  watershift++;
    if ( meshsizex > 32 )  watershift++;
    if ( meshsizex > 64 )  watershift++;
    if ( meshsizex > 128 )  watershift++;
    if ( meshsizex > 256 )  watershift++;


    // Load fan data
    fan = 0;
    while ( fan < numfan )
    {
      fread( &itmp, 4, 1, fileread );

#ifdef SDL_LIL_ENDIAN
      meshtype[fan] = itmp>>24;
      meshfx[fan] = itmp>>16;
      meshtile[fan] = itmp;
#else
      meshtype[fan] = SDL_Swap32( itmp )>>24;
      meshfx[fan] = SDL_Swap32( itmp )>>16;
      meshtile[fan] = SDL_Swap32( itmp );
#endif

      fan++;
    }
    // Load fan data
    fan = 0;
    while ( fan < numfan )
    {
      fread( &itmp, 1, 1, fileread );

#ifdef SDL_LIL_ENDIAN
      meshtwist[fan] = itmp;
#else
      meshtwist[fan] = SDL_Swap32( itmp );
#endif

      fan++;
    }


    // Load vertex x data
    cnt = 0;
    while ( cnt < numvert )
    {
      fread( &ftmp, 4, 1, fileread );

#ifdef SDL_LIL_ENDIAN
      meshvrtx[cnt] = ftmp;
#else
      meshvrtx[cnt] = LoadFloatByteswapped( &ftmp );
#endif

      cnt++;
    }
    // Load vertex y data
    cnt = 0;
    while ( cnt < numvert )
    {
      fread( &ftmp, 4, 1, fileread );

#ifdef SDL_LIL_ENDIAN
      meshvrty[cnt] = ftmp;
#else
      meshvrty[cnt] = LoadFloatByteswapped( &ftmp );
#endif

      cnt++;
    }
    // Load vertex z data
    cnt = 0;
    while ( cnt < numvert )
    {
      fread( &ftmp, 4, 1, fileread );

#ifdef SDL_LIL_ENDIAN
      meshvrtz[cnt] = ftmp/16.0f;  // Cartman uses 4 bit fixed point for Z
#else
      meshvrtz[cnt] = ( LoadFloatByteswapped( &ftmp ) )/16.0f;  // Cartman uses 4 bit fixed point for Z
#endif

      cnt++;
    }

    if ( shading == GL_FLAT /* && rtscontrol==bfalse */ )
    {
      // Assume fullbright
      cnt = 0;
      while ( cnt < numvert )
      {
        meshvrta[cnt] = 255;
        meshvrtl[cnt] = 255;
        cnt++;
      }
    }
    else
    {
      // Load vertex a data
      cnt = 0;
      while ( cnt < numvert )
      {
        fread( &itmp, 1, 1, fileread );

#ifdef SDL_LIL_ENDIAN
        meshvrta[cnt] = itmp;
#else
        meshvrta[cnt] = SDL_Swap32( itmp );
#endif
        meshvrtl[cnt] = 0;

        cnt++;
      }
    }
    fclose( fileread );


    make_fanstart();


    vert = 0;
    y = 0;
    while ( y < meshsizey )
    {
      x = 0;
      while ( x < meshsizex )
      {
        fan = meshfanstart[y]+x;
        meshvrtstart[fan] = vert;
        vert+=meshcommandnumvertices[meshtype[fan]];
        x++;
      }
      y++;
    }

    return btrue;
  }
  else log_warning( "Cannot find level.mpd!!\n" );
  return bfalse;
}

//--------------------------------------------------------------------------------------------
void update_game()
{
  // ZZ> This function does several iterations of character movements and such
  //     to keep the game in sync.
  int cnt, numdead;

  // Check for all local players being dead
  alllocalpladead = bfalse;
  localseeinvisible = bfalse;
  localseekurse = bfalse;
  cnt = 0;
  numdead = 0;
  while ( cnt < MAXPLAYER )
  {
    if ( plavalid[cnt] && pladevice[cnt] != INPUTNONE )
    {
      if ( chralive[plaindex[cnt]] == bfalse )
      {
        numdead++;
        if ( SDLKEYDOWN( SDLK_SPACE && respawnvalid == btrue ) )
        {
          respawn_character( plaindex[cnt] );
          chrexperience[cnt] = ( chrexperience[cnt] )*EXPKEEP;  // Apply xp Penality
        }
      }
      else
      {
        if ( chrcanseeinvisible[plaindex[cnt]] )
        {
          localseeinvisible = btrue;
        }
        if ( chrcanseekurse[plaindex[cnt]] )
        {
          localseekurse = btrue;
        }
      }
    }
    cnt++;
  }
  if ( numdead >= numlocalpla )
  {
    alllocalpladead = btrue;
  }

  // This is the main game loop
  msgtimechange = 0;

  // [claforte Jan 6th 2001]
  // TODO: Put that back in place once networking is functional.
  while ( wldclock < allclock && ( numplatimes > 0 || rtscontrol ) )
  {
    // Important stuff to keep in sync
    srand( randsave );
    sv_talkToRemotes();
    resize_characters();
    keep_weapons_with_holders();
    let_ai_think();
    do_weather_spawn();
    do_enchant_spawn();
    unbuffer_player_latches();
    move_characters();
    move_particles();
    make_character_matrices();
    attach_particles();
    make_onwhichfan();
    bump_characters();
    stat_return();
    pit_kill();

    // Generate the new seed
    randsave += *( ( Uint32* ) &kMd2Normals[wldframe&127][0] );
    randsave += *( ( Uint32* ) &kMd2Normals[randsave&127][1] );

    // Stuff for which sync doesn't matter
    animate_tiles();
    move_water();

    // Timers
    wldclock+=FRAMESKIP;
    wldframe++;
    msgtimechange++;
    if ( statdelay > 0 )  statdelay--;
    statclock+=FRAMESKIP;
  }
  if ( rtscontrol == bfalse )
  {
    if ( numplatimes == 0 )
    {
      // The remote ran out of messages, and is now twiddling its thumbs...
      // Make it go slower so it doesn't happen again
      wldclock+=25;
    }
    if ( numplatimes > 3 && hostactive == bfalse )
    {
      // The host has too many messages, and is probably experiencing control
      // lag...  Speed it up so it gets closer to sync
      wldclock-=5;
    }
  }
}

//--------------------------------------------------------------------------------------------
void update_timers()
{
  // ZZ> This function updates the game timers
  lstclock = allclock;
  allclock = SDL_GetTicks()-sttclock;
  fpsclock+=allclock-lstclock;
  if ( fpsclock >= TICKS_PER_SEC )
  {
    create_szfpstext( fpsframe );
    fpsclock = 0;
    fpsframe = 0;
  }
}

//--------------------------------------------------------------------------------------------
void read_pair( FILE* fileread )
{
  // ZZ> This function reads a damage/stat pair ( eg. 5-9 )
  char cTmp;
  float  fBase, fRand;

  fscanf( fileread, "%f", &fBase );  // The first number
  pairbase = fBase*256;
  cTmp = get_first_letter( fileread );  // The hyphen
  if ( cTmp!='-' )
  {
    // Not in correct format, so fail
    pairrand = 1;
    return;
  }
  fscanf( fileread, "%f", &fRand );  // The second number
  pairrand = fRand*256;
  pairrand = pairrand-pairbase;
  if ( pairrand<1 )
    pairrand = 1;
}

//--------------------------------------------------------------------------------------------
void undo_pair( int base, int rand )
{
  // ZZ> This function generates a damage/stat pair ( eg. 3-6.5f )
  //     from the base and random values.  It set pairfrom and
  //     pairto
  pairfrom = base/256.0f;
  pairto = rand/256.0f;
  if ( pairfrom < 0.0f )
  {
    pairfrom = 0.0f;
    log_warning( "We got a randomization error again! (Base is less than 0)\n" );
  }
  if ( pairto < 0.0f )
  {
    pairto = 0.0f;
    log_warning( "We got a randomization error again! (Max is less than 0)\n" );
  }
  pairto += pairfrom;
}

//--------------------------------------------------------------------------------------------
void ftruthf( FILE* filewrite, char* text, Uint8 truth )
{
  // ZZ> This function kinda mimics fprintf for the output of
  //     btrue bfalse statements

  fprintf( filewrite, text );
  if ( truth )
  {
    fprintf( filewrite, "TRUE\n" );
  }
  else
  {
    fprintf( filewrite, "FALSE\n" );
  }
}

//--------------------------------------------------------------------------------------------
void fdamagf( FILE* filewrite, char* text, Uint8 damagetype )
{
  // ZZ> This function kinda mimics fprintf for the output of
  //     SLASH CRUSH POKE HOLY EVIL FIRE ICE ZAP statements
  fprintf( filewrite, text );
  if ( damagetype == DAMAGESLASH )
    fprintf( filewrite, "SLASH\n" );
  if ( damagetype == DAMAGECRUSH )
    fprintf( filewrite, "CRUSH\n" );
  if ( damagetype == DAMAGEPOKE )
    fprintf( filewrite, "POKE\n" );
  if ( damagetype == DAMAGEHOLY )
    fprintf( filewrite, "HOLY\n" );
  if ( damagetype == DAMAGEEVIL )
    fprintf( filewrite, "EVIL\n" );
  if ( damagetype == DAMAGEFIRE )
    fprintf( filewrite, "FIRE\n" );
  if ( damagetype == DAMAGEICE )
    fprintf( filewrite, "ICE\n" );
  if ( damagetype == DAMAGEZAP )
    fprintf( filewrite, "ZAP\n" );
  if ( damagetype == DAMAGENULL )
    fprintf( filewrite, "NONE\n" );
}

//--------------------------------------------------------------------------------------------
void factiof( FILE* filewrite, char* text, Uint8 action )
{
  // ZZ> This function kinda mimics fprintf for the output of
  //     SLASH CRUSH POKE HOLY EVIL FIRE ICE ZAP statements
  fprintf( filewrite, text );
  if ( action == ACTIONDA )
    fprintf( filewrite, "WALK\n" );
  if ( action == ACTIONUA )
    fprintf( filewrite, "UNARMED\n" );
  if ( action == ACTIONTA )
    fprintf( filewrite, "THRUST\n" );
  if ( action == ACTIONSA )
    fprintf( filewrite, "SLASH\n" );
  if ( action == ACTIONCA )
    fprintf( filewrite, "CHOP\n" );
  if ( action == ACTIONBA )
    fprintf( filewrite, "BASH\n" );
  if ( action == ACTIONLA )
    fprintf( filewrite, "LONGBOW\n" );
  if ( action == ACTIONXA )
    fprintf( filewrite, "XBOW\n" );
  if ( action == ACTIONFA )
    fprintf( filewrite, "FLING\n" );
  if ( action == ACTIONPA )
    fprintf( filewrite, "PARRY\n" );
  if ( action == ACTIONZA )
    fprintf( filewrite, "ZAP\n" );
}

//--------------------------------------------------------------------------------------------
void fgendef( FILE* filewrite, char* text, Uint8 gender )
{
  // ZZ> This function kinda mimics fprintf for the output of
  //     MALE FEMALE OTHER statements

  fprintf( filewrite, text );
  if ( gender == GENMALE )
    fprintf( filewrite, "MALE\n" );
  if ( gender == GENFEMALE )
    fprintf( filewrite, "FEMALE\n" );
  if ( gender == GENOTHER )
    fprintf( filewrite, "OTHER\n" );
}

//--------------------------------------------------------------------------------------------
void fpairof( FILE* filewrite, char* text, int base, int rand )
{
  // ZZ> This function mimics fprintf in spitting out
  //     damage/stat pairs
  undo_pair( base, rand );
  fprintf( filewrite, text );
  fprintf( filewrite, "%4.2f-%4.2f\n", pairfrom, pairto );
}

//--------------------------------------------------------------------------------------------
void funderf( FILE* filewrite, char* text, char* usename )
{
  // ZZ> This function mimics fprintf in spitting out
  //     a name with underscore spaces
  char cTmp;
  int cnt;


  fprintf( filewrite, text );
  cnt = 0;
  cTmp = usename[0];
  cnt++;
  while ( cTmp != 0 )
  {
    if ( cTmp == ' ' )
    {
      fprintf( filewrite, "_" );
    }
    else
    {
      fprintf( filewrite, "%c", cTmp );
    }
    cTmp = usename[cnt];
    cnt++;
  }
  fprintf( filewrite, "\n" );
}

//--------------------------------------------------------------------------------------------
void get_message( FILE* fileread )
{
  // ZZ> This function loads a string into the message buffer, making sure it
  //     is null terminated.
  int cnt;
  char cTmp;
  char szTmp[256];


  if ( msgtotal<MAXTOTALMESSAGE )
  {
    if ( msgtotalindex>=MESSAGEBUFFERSIZE )
    {
      msgtotalindex = MESSAGEBUFFERSIZE-1;
    }
    msgindex[msgtotal]=msgtotalindex;
    fscanf( fileread, "%s", szTmp );
    szTmp[255] = 0;
    cTmp = szTmp[0];
    cnt = 1;
    while ( cTmp!=0 && msgtotalindex<MESSAGEBUFFERSIZE-1 )
    {
      if ( cTmp=='_' )  cTmp=' ';
      msgtext[msgtotalindex] = cTmp;
      msgtotalindex++;
      cTmp = szTmp[cnt];
      cnt++;
    }
    msgtext[msgtotalindex]=0;  msgtotalindex++;
    msgtotal++;
  }
}

//--------------------------------------------------------------------------------------------
void load_all_messages( char *loadname, int object )
{
  // ZZ> This function loads all of an objects messages
  FILE *fileread;


  madmsgstart[object] = 0;
  fileread = fopen( loadname, "r" );
  if ( fileread )
  {
    madmsgstart[object] = msgtotal;
    while ( goto_colon_yesno( fileread ) )
    {
      get_message( fileread );
    }
    fclose( fileread );
  }
}


//--------------------------------------------------------------------------------------------
void reset_teams()
{
  // ZZ> This function makes everyone hate everyone else
  int teama, teamb;


  teama = 0;
  while ( teama < MAXTEAM )
  {
    // Make the team hate everyone
    teamb = 0;
    while ( teamb < MAXTEAM )
    {
      teamhatesteam[teama][teamb] = btrue;
      teamb++;
    }
    // Make the team like itself
    teamhatesteam[teama][teama] = bfalse;
    // Set defaults
    teamleader[teama] = NOLEADER;
    teamsissy[teama] = 0;
    teammorale[teama] = 0;
    teama++;
  }


  // Keep the null team neutral
  teama = 0;
  while ( teama < MAXTEAM )
  {
    teamhatesteam[teama][NULLTEAM] = bfalse;
    teamhatesteam[NULLTEAM][teama] = bfalse;
    teama++;
  }
}

//--------------------------------------------------------------------------------------------
void reset_messages()
{
  // ZZ> This makes messages safe to use
  int cnt;

  msgtotal=0;
  msgtotalindex=0;
  msgtimechange=0;
  msgstart=0;
  cnt = 0;
  while ( cnt < MAXMESSAGE )
  {
    msgtime[cnt] = 0;
    cnt++;
  }
  cnt = 0;
  while ( cnt < MAXTOTALMESSAGE )
  {
    msgindex[cnt] = 0;
    cnt++;
  }
  msgtext[0] = 0;
}

//--------------------------------------------------------------------------------------------
void make_randie()
{
  // ZZ> This function makes the random number table
  int tnc, cnt;


  // Fill in the basic values
  cnt = 0;
  while ( cnt < MAXRAND )
  {
    randie[cnt] = rand()<<1;
    cnt++;
  }


  // Keep adjusting those values
  tnc = 0;
  while ( tnc < 20 )
  {
    cnt = 0;
    while ( cnt < MAXRAND )
    {
      randie[cnt] += rand();
      cnt++;
    }
    tnc++;
  }

  // All done
  randindex = 0;
}

//--------------------------------------------------------------------------------------------
void reset_timers()
{
  // ZZ> This function resets the timers...
  sttclock = SDL_GetTicks();
  allclock = 0;
  lstclock = 0;
  wldclock = 0;
  statclock = 0;
  pitclock = 0;  pitskill = bfalse;
  wldframe = 0;
  allframe = 0;
  fpsframe = 0;
  outofsync = bfalse;
}

void menu_loadInitialMenu();

extern int doMenu( float deltaTime );
extern int initMenus();

//--------------------------------------------------------------------------------------------
int SDL_main( int argc, char **argv )
{
  // ZZ> This is where the program starts and all the high level stuff happens
  glVector t1={0,0,0};
  glVector t2={0,0,-1};
  glVector t3={0,1,0};
  double frameDuration;
  int menuActive = 1;
  int menuResult;

  // Initialize logging first, so that we can use it everywhere.
  log_init();
  log_setLoggingLevel( 2 );

  // start initializing the various subsystems
  log_message( "Starting Egoboo %s...\n", VERSION );

  sys_initialize();
  clock_init();
  fs_init();

  read_setup( "setup.txt" );
  read_all_tags( "basicdat" SLASH_STR "scancode.txt" );
  read_controls( "controls.txt" );
  reset_ai_script();
  load_ai_codes( "basicdat" SLASH_STR "aicodes.txt" );
  load_action_names( "basicdat" SLASH_STR "actions.txt" );

  sdlinit( argc,argv );
  glinit( argc,argv );
  net_initialize();
  ui_initialize( "basicdat" SLASH_STR "Negatori.ttf", 24 );
  sdlmixer_initialize();

  // Linking system
  log_info( "Initializing module linking... " );
  if ( link_build( "basicdat" SLASH_STR "link.txt", LinkList ) ) log_message( "Success!\n" );
  else log_message( "Failed!\n" );

  if ( !get_mesh_memory() )
  {
    log_error( "Reduce the maximum number of vertices!!!  See SETUP.TXT\n" );
    return bfalse;
  }

  // Matrix init stuff (from remove.c)
  rotmeshtopside = ( ( float )scrx/scry )*ROTMESHTOPSIDE/( 1.33333f );
  rotmeshbottomside = ( ( float )scrx/scry )*ROTMESHBOTTOMSIDE/( 1.33333f );
  rotmeshup = ( ( float )scrx/scry )*ROTMESHUP/( 1.33333f );
  rotmeshdown = ( ( float )scrx/scry )*ROTMESHDOWN/( 1.33333f );
  mWorld = IdentityMatrix();
  mViewSave = ViewMatrix( t1,t2,t3,0 );
  mProjection = ProjectionMatrix( .001f, 2000.0f, ( float )( FOV*PI/180 ) ); // 60 degree FOV
  mProjection = MatrixMult( Translate( 0, 0, -0.999996f ), mProjection ); // Fix Z value...
  mProjection = MatrixMult( ScaleXYZ( -1, -1, 100000 ), mProjection );  // HUK // ...'cause it needs it

  //[claforte] Fudge the values.
  mProjection.v[10] /= 2.0f;
  mProjection.v[11] /= 2.0f;

  // Load stuff into memory
  prime_icons();
  prime_titleimage();
  make_textureoffset();  // THIS SHOULD WORK
  make_lightdirectionlookup(); // THIS SHOULD WORK
  make_turntosin();  // THIS SHOULD WORK
  make_enviro(); // THIS SHOULD WORK
  load_mesh_fans(); // THIS SHOULD WORK
  load_blip_bitmap();
  load_all_menu_images();
  load_all_music_sounds();
  initMenus();        // Start the game menu

  // Let the normal OS mouse cursor work
  SDL_WM_GrabInput( SDL_GRAB_OFF );
  SDL_ShowCursor( 1 );

  // Network's temporarily disabled

  clock_frameStep();
  frameDuration = clock_getFrameDuration();
  gameactive = btrue;

  while ( gameactive || menuActive )
  {
    // Clock updates each frame
    clock_frameStep();
    frameDuration = clock_getFrameDuration();

    // Either run through the menus, or jump into the game
    if ( menuActive )
    {
      // Play the menu music
      play_music( 0, 500, -1 );

      // do menus
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      read_input();

      // Pressed panic button
      if ( SDLKEYDOWN( SDLK_q ) && SDLKEYDOWN( SDLK_LCTRL ) )
      {
        menuActive = 0;
        gameactive = bfalse;
      }

      ui_beginFrame( frameDuration );

      menuResult = doMenu( ( float )frameDuration );
      switch ( menuResult )
      {
        case 1:
          // Go ahead and start the game
          menuActive = 0;
          gameactive = btrue;
          networkon = bfalse;
          hostactive = btrue;

          if ( gGrabMouse )
          {
            SDL_WM_GrabInput ( SDL_GRAB_ON );
          }

          if ( gHideMouse )
          {
            SDL_ShowCursor( 0 );  // Hide the mouse cursor
          }
          break;

        case -1:
          // The user selected "Quit"
          menuActive = 0;
          gameactive = bfalse;
          break;
      }

      ui_endFrame();

      SDL_GL_SwapBuffers();
    } else
    {
      // Do the game
      // Did we get through all the menus?
      if ( gameactive )
      {
        // printf("MENU: game is now active\n");
        // Start a new module
        seed = time( NULL );
        srand( seed );

        load_module( pickedmodule );  // :TODO: Seems to be the next part to fix


        pressed = bfalse;
        make_onwhichfan();
        reset_camera();
        reset_timers();
        figure_out_what_to_draw();
        make_character_matrices();
        attach_particles();

        if ( networkon )
        {
          log_info( "SDL_main: Loading module %s...\n", pickedmodule );
          netmessagemode = bfalse;
          netmessagedelay = 20;
          net_sayHello();
        }

        // Let the game go
        moduleactive = btrue;
        randsave = 0;
        srand( 0 );
        // printf("moduleactive: %d\n", moduleactive);
        while ( moduleactive )
        {
          // This is the control loop
          read_input();
          // input_net_message();

          if ( !SDLKEYDOWN( SDLK_F11 ) ) screenshotkeyready = btrue;
          if ( SDLKEYDOWN( SDLK_F11 ) && keyon && screenshotkeyready )
          {
            if ( !dump_screenshot() )                // Take the shot, returns bfalse if failed
            {
              debug_message( "Error writing screenshot!" );
              log_warning( "Error writing screenshot\n" );    // Log the error in log.txt
            }
            screenshotkeyready = bfalse;
          }


          if ( !SDLKEYDOWN( SDLK_F8 ) ) pausekeyready = btrue;

          // Todo zefz: where to put this?
          // Check for pause key    // TODO: What to do in network games?
          if ( SDLKEYDOWN( SDLK_F8 ) && keyon && pausekeyready )
          {
            pausekeyready = bfalse;
            if ( gamepaused ) gamepaused = bfalse;
            else gamepaused = btrue;
          }

          // Do important things
          if ( !gamepaused || networkon )
          {
            check_stats();
            set_local_latches();
            update_timers();
            check_passage_music();

            // NETWORK PORT
            listen_for_packets();
            if ( waitingforplayers == bfalse )
            {
              cl_talkToHost();
              update_game();
            }
            else
            {

              wldclock = allclock;
            }
          }
          else
          {
            update_timers();
            wldclock = allclock;
          }

          // Do the display stuff
          move_camera();
          figure_out_what_to_draw();
          // printf("DIAG: doing draw_main\n");
          draw_main();

          // Check for quitters
          // :TODO: nolocalplayers is not set correctly
          if ( SDLKEYDOWN( SDLK_ESCAPE ) /*|| nolocalplayers*/ )
          {
            quit_module();
            gameactive = bfalse;
            menuActive = 1;

            // Let the normal OS mouse cursor work
            SDL_WM_GrabInput( SDL_GRAB_OFF );
            SDL_ShowCursor( 1 );
          }
        }
        release_module();
        close_session();
      }
    }
  }

  quit_game();
  Mix_CloseAudio();
  release_grfx();
  ui_shutdown();
  net_shutDown();
  clock_shutdown();
  sys_shutdown();

  return btrue;
}
