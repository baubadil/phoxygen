/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 * 
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#include "phoxygen/phoxygen.h"

#include "xwp/debug.h"
#include "xwp/regex.h"

void FunctionComment::setClass(PClassComment pClass)
{
    _pClass = pClass;
}

void FunctionComment::processInputLine(const string &strLine,
                                       State &state)
{
    // ::myLog("function line: $line");
    RegexMatches aMatches;
    static const Regex s_reFuncOpen(R"i____(^\s*([&\$A-Za-z0-9_ ='"-]+)\s*([,;)])\s*(?:\/\/!<\s*(.*))?$)i____");
    static const Regex s_reFuncEnd(R"i____((.*)\)(.*))i____");
    if (s_reFuncOpen.matches(strLine, aMatches))
    {
        const string &param = aMatches.get(1);
        const string &sep = aMatches.get(2);
        const string &descr = (aMatches.size() > 2) ? aMatches.get(3) : "";
    // if ( (my $param, my $sep, my $descr) = $line =~ /^\s*([&\$A-Za-z0-9_ ='"-]+)\s*([,;)])\s*(?:\/\/!<\s*(.*))?$/)
//         auto oldstate = state;
        state = (sep == ",")
                        ? State::IN_FUNCTION_HEADER
                        : State::INIT;
        // ::myLog("  after function: param=$param, sep='$sep', descr=$descr, state=$oldstate->".$$pState);

        _vParams.push_back(Param( { param, descr } ));
    }
    else if (s_reFuncEnd.matches(strLine, aMatches))
    {
        const string &args = aMatches.get(1);
        StringVector vArgs = explodeVector(args, ",");
        for (auto arg : vArgs)
        {
            trim(arg);
            _vParams.push_back(Param( { arg, "" } ));
        }
        // ::myLog("after function 1+ args: \"".join('\", \"', @llArgs).'"');
        state = State::INIT;
    }
    else
    {
        Debug::Warning("don't know how to handle function header in " + formatContext());
    }
}

string FunctionComment::formatFunction(bool fLong)
{
    string htmlBody;

    auto &pllArgs = _vParams;
    size_t cTotal = pllArgs.size();

    //    Build a table like this:
    //    +----------------------+------------------------+---------------------+
    //    |   funcname(          | arg1,                  | comment1            |
    //    |                      | arg2,                  | comment2            |
    //    |                      | arg3)                  | comment3            |
    //    +----------------------+------------------------+---------------------+

    if (!cTotal)
        htmlBody += _keyword + " <b>" + _identifier + "</b>()";
    else
    {
        if (fLong)
            htmlBody += "<table class=\"functable\"><tr><td>";
        htmlBody += _keyword + " <b>" + _identifier + "</b>(";
        if (fLong)
            htmlBody += "</td>";
            
        size_t c = 0;
        for (const auto &param : pllArgs)
        {
            ++c;
            if (fLong)
            {
                if (c > 1)
                    htmlBody += "<tr><td>&nbsp;</td>";                    
                htmlBody += "<td>";
            }
            htmlBody += param.param;
            htmlBody += (c == cTotal) ? ") " : ", ";
            if (fLong)
            {
                htmlBody += "</td>\n<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<i>" + toHTML(param.description);
                htmlBody += "</i></td></tr>";
            }
        }
        if (fLong)
            htmlBody += "</table>";
    }

    return htmlBody;
}
