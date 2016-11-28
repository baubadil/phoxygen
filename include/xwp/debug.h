/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 * 
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef XWP_DEBUG_H
#define XWP_DEBUG_H

#include "xwp/basetypes.h"

namespace XWP
{

extern int g_flDebugSet;

enum class AnsiColor
{
    BRIGHT_WHITE,
    RED,
    BRIGHT_RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN
};


/***************************************************************************
 *
 *  Debug
 *
 **************************************************************************/

typedef uint DebugFlag;
const DebugFlag DEBUG_ALWAYS          = 0;

const uint8_t NO_ECHO_NEWLINE           = 0x01;
const uint8_t CONTINUE_FROM_PREVIOUS    = 0x02;

class Debug
{
public:
    static void Enter(DebugFlag fl,
                      const string &strFuncName,
                      const string &strExtra = "");
    static void Leave(const string &strExtra = "");
    static void Log(DebugFlag fl,
                    const string &str,
                    uint8_t flMessage = 0);
    static void Message(const string &str,
                        uint8_t flMessage = 0);
    static void Warning(const string &str);

    static string MakeColor(AnsiColor c, string str);
};

} // namespace XWP

#endif // XWP_STRINGHELP_H