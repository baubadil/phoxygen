/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 * 
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#include "xwp/debug.h"

#include <list>
#include <iostream>

namespace XWP {

int g_flDebugSet = 0;

struct FuncItem
{
    DebugFlag fl;
    string strFuncName;
};

list<FuncItem> g_llFuncs;
uint g_iIndent = 0;
bool g_fNeedsNewline = false;

/* static */
void Debug::Enter(DebugFlag fl,
                  const string &strFuncName,
                  const string &strExtra /* = "" */ )
{
    if ( (fl == DEBUG_ALWAYS) || (g_flDebugSet & (uint)fl) )
    {
        string str2("Entering " + strFuncName);
        if (strExtra.length())
            str2.append(": " + strExtra);
        Debug::Log(fl, str2);
        ++g_iIndent;
    }
    g_llFuncs.push_back({fl, strFuncName});
}

/* static */
void Debug::Leave(const string &strExtra /* = "" */)
{
    if (!g_llFuncs.empty())
    {
        FuncItem f = g_llFuncs.back();      // Make a copy.
        g_llFuncs.pop_back();

        if ( (f.fl == DEBUG_ALWAYS) || (g_flDebugSet & (uint)f.fl) )
        {
            --g_iIndent;
            string s = "Leaving " + f.strFuncName;
            if (!strExtra.empty())
                s += " (" + strExtra + ")";
            Debug::Log(f.fl, s);
        }
    }
}

/* static */
void Debug::Log(DebugFlag fl,
                const string &str,
                uint8_t flMessage /* = 0 */)
{
    bool fAlways;
    if (    (fAlways = (fl == DEBUG_ALWAYS))
         || (g_flDebugSet & (uint)fl)
       )
    {
        if (g_fNeedsNewline)
        {
            if (0 == (flMessage & CONTINUE_FROM_PREVIOUS))
                cout << "\n";
            g_fNeedsNewline = false;
        }

        string strIndent;
        if (fAlways && (g_iIndent > 0))
            cout << MakeColor(AnsiColor::BRIGHT_WHITE, ">") << string(g_iIndent * 2 - 1, ' ');
        else
            cout << string(g_iIndent * 2, ' ');
        cout << str;
        if ( (!fAlways) || (0 == (flMessage & NO_ECHO_NEWLINE)) )
            cout << "\n";
        else
        {
            cout.flush();
            g_fNeedsNewline = true;     // for next message
        }
    }
}

/* static */
void Debug::Message(const string &str,
                    uint8_t flMessage /* = 0 */ )
{
    Debug::Log(DEBUG_ALWAYS,
               str,
               flMessage);
}

// https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_RED_BRIGHT   "\x1b[31;1m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[33m"
#define ANSI_COLOR_BLUE         "\x1b[34m"
#define ANSI_COLOR_MAGENTA      "\x1b[35m"
#define ANSI_COLOR_CYAN         "\x1b[36m"
#define ANSI_COLOR_RESET        "\x1b[0m"
#define ANSI_COLOR_WHITE_BRIGHT "\x1b[37;1m"

string Debug::MakeColor(AnsiColor c, string str)
{
    switch (c)
    {
        case AnsiColor::BRIGHT_WHITE:
            return ANSI_COLOR_WHITE_BRIGHT + str + ANSI_COLOR_RESET;
        break;

        case AnsiColor::RED:
            return ANSI_COLOR_RED + str + ANSI_COLOR_RESET;
        break;

        case AnsiColor::BRIGHT_RED:
            return ANSI_COLOR_RED_BRIGHT + str + ANSI_COLOR_RESET;
        break;

        case AnsiColor::GREEN:
            return ANSI_COLOR_GREEN + str + ANSI_COLOR_RESET;
        break;

        case AnsiColor::YELLOW:
            return ANSI_COLOR_YELLOW + str + ANSI_COLOR_RESET;
        break;

        case AnsiColor::BLUE:
            return ANSI_COLOR_BLUE + str + ANSI_COLOR_RESET;
        break;

        case AnsiColor::MAGENTA:
            return ANSI_COLOR_MAGENTA + str + ANSI_COLOR_RESET;
        break;

        case AnsiColor::CYAN:
            return ANSI_COLOR_CYAN + str + ANSI_COLOR_RESET;
        break;
    }

    return str;
}

void Debug::Warning(const string& str)
{
    Log(DEBUG_ALWAYS, MakeColor(AnsiColor::YELLOW, str));
}

} // namespace
