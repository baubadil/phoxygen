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
#include "xwp/except.h"

void FunctionComment::setClass(PClassComment pClass)
{
    _pClass = pClass;
}

void FunctionComment::parseParam(string &oneParam,
                                 const string &descr)
{
    RegexMatches aMatches2;
    // First check if there is a default argument.
    static const Regex s_reDefaultArg(R"i____((.+)(=.*))i____");
    string strDefaultArg;
    if (s_reDefaultArg.matches(oneParam, aMatches2))
    {
        oneParam = aMatches2.get(1);
        strDefaultArg = trimmed(aMatches2.get(2));
    }

    static const Regex s_reType(R"i____(\s*([A-Za-z0-9_]+\s+)?(&?\$[A-Za-z0-9_]+)\s*)i____");
    if (!s_reType.matches(oneParam, aMatches2))
        throw FSException("cannot figure out type from param \"" + oneParam + "\" in function \"" + _identifier + "\"");

    string type;
    string strTypeFormattedHTML, strTypeFormattedLaTeX;
    bool fType = aMatches2.size();
    if (fType)
    {
        type = aMatches2.get(1);

        if (type.length())
        {
            auto fmt1 = FormatterBase::Get(OutputMode::HTML);
            strTypeFormattedHTML = type;
            ClassComment::LinkifyClasses(fmt1,
                                        strTypeFormattedHTML,
                                        &_identifier);

            auto fmt2 = FormatterBase::Get(OutputMode::LATEX);
            strTypeFormattedLaTeX = type;
            ClassComment::LinkifyClasses(fmt2,
                                        strTypeFormattedLaTeX,
                                        &_identifier);
        }
    }
    const string &argname = aMatches2.get( (fType) ? 2 : 1);

    _vParams.push_back(Param(type,
                             argname,
                             strDefaultArg,
                             descr,
                             strTypeFormattedHTML,
                             strTypeFormattedLaTeX));
}


/**
 *  Called from main.cpp for every line after a function comment that has
 *  arguments, until the closing bracket.
 *
 *  For every line, if the line was recognized as a function comment,
 *  this function must set state to State::IN_FUNCTION_HEADER; if the
 *  closing bracket was encountered, state must be set back to State::INIT.
 */
void FunctionComment::parseArguments(const string &strLine,
                                     State &state)
{
    /*
     *   1.  When called for first time, strLine has everything after "(". This
     *       could be
     *
     *          --  "Type $var[,)]"
     *
     *          --  "$var1 = "default", $var2[,)]"
     *
     *          --  "$var1, Type $var2[,)]"
     *
     *          --  "$var1[,)] //!< comment"
     *
     *   2.  When called fur subsequent lines, the input should be exactly the same.
     *
     */

    RegexMatches aMatches;
    static const Regex s_reFuncOpen(R"i____(^\s*([&\$A-Za-z0-9_ ='":!-]+)\s*([,;)]+)\s*(?:\/\/!<\s*(.*))?$)i____");
    static const Regex s_reFuncEnd(R"i____((.*)\)(.*))i____");
    if (s_reFuncOpen.matches(strLine, aMatches))
    {
        const string &params = aMatches.get(1);
        const string &sep = aMatches.get(2);
        const string &descr = (aMatches.size() > 2) ? aMatches.get(3) : "";

        // See if param is really several params.
        StringVector vParams = explodeVector(params, ",");
        for (auto oneParam : vParams)
            parseParam(oneParam, descr);

        state = (sep == ",")
            ? State::IN_FUNCTION_HEADER
            : State::INIT;
    }
    else if (s_reFuncEnd.matches(strLine, aMatches))
    {
        const string &args = aMatches.get(1);
        const string &descr = aMatches.get(2);
        StringVector vParams = explodeVector(args, ",");
        for (auto oneParam : vParams)
            parseParam(oneParam, descr);

        state = State::INIT;
    }
    else
    {
        Debug::Warning("don't know how to handle function header in " + formatContext());
    }
}

string FunctionComment::formatFunction(FormatterBase &fmt,
                                       bool fLong)
{
    //    Build a table like this:
    //    +----------------------+------------------------+---------------------+
    //    |   funcname(          | arg1,                  | comment1            |
    //    |                      | arg2,                  | comment2            |
    //    |                      | arg3)                  | comment3            |
    //    +----------------------+------------------------+---------------------+

    return fmt.makeFunctionHeader(_keyword,
                                  _identifier,
                                  _vParams,
                                  fLong);
}

