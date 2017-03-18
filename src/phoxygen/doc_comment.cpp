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
#include "xwp/regex.h"
#include "xwp/except.h"

#include <sstream>

const string CommentBase::s_strUnknown = "Unknown";

CommentBase::CommentBase(Type theType,
                         const string &strKeyword,
                         const string &strIdentifier,
                         const string &strComment,
                         const string &strFile,
                         int linenoFirst,
                         int linenoLast,
                         const string &strTargetBase)
    : _type(theType),
        _keyword(strKeyword),
        _identifier(strIdentifier),
        _comment(strComment),
        _file(strFile),
        _linenoFirst(linenoFirst),
        _linenoLast(linenoLast)
{
    if (!strTargetBase.empty())
        makeTargets(strTargetBase);
};

void CommentBase::makeTargets(const string &strTargetBase)
{
    _strTargetHTML = strTargetBase + ".html";
    _strTargetLaTeX = strTargetBase;
    stringReplace(_strTargetLaTeX, "_", "@");
}

string CommentBase::formatContext()
{
    return _file + " (lines " + to_string(_linenoFirst) + "--" + to_string(_linenoLast) + ")";
}

/* virtual */
string CommentBase::formatComment(OutputMode mode)
{
    string strOutput;
    bool fHadEmptyLine = true;
    bool fPrependNewline = true;

    enum class PState
    {
        INIT,
        OPEN,
        UL,
        OL,
        VERBATIM
    };
    PState paraState = PState::INIT;

    // Replace all \ref with @ref. This allows for using both syntaxes and also avoids problems with
    // escaping \\ in LaTeX.
    string strComment2 = _comment;
    stringReplace(strComment2, "\\ref", "@ref");

    stringstream ss(strComment2);
    string line;

    FormatterBase &fmt = FormatterBase::Get(mode);

    while(std::getline(ss, line, '\n'))
    {
        static const Regex s_reEmptyLine(R"i____(^\s*$)i____");
        static const Regex s_reOpenPRE(R"i____(^\s*```(?:php|javascript|xml|sql)\s*$)i____");
        static const Regex s_reClosePRE(R"i____(^\s*```\s*$)i____");
        if (s_reOpenPRE.matches(line))
        {
            paraState = PState::VERBATIM;
            strOutput += fmt.openPRE();
        }
        else if (paraState == PState::VERBATIM)
        {
            if (s_reClosePRE.matches(line))
            {
                strOutput += fmt.closePRE();
                paraState = PState::INIT;
            }
            else
                strOutput += fmt.format(line, true) + "\n";
        }
        else if (s_reEmptyLine.matches(line))
        {
            Debug::Log(FORMAT, "Had empty line");
            // Line consists entirely of whitespace, or is empty:
            fHadEmptyLine = true;
        }
        else
        {
            Debug::Log(FORMAT, "Formatting line: \"" + line + "\"");
            if (fHadEmptyLine)
            {
                if (paraState == PState::OPEN)
                    strOutput += fmt.closePara();
                else if (    (paraState == PState::UL)
                          || (paraState == PState::OL)
                        )
                    strOutput += fmt.closeLI();

                static const Regex s_reOpenUL(R"i____(^\s+(?:--?|\*)\s+)i____");
                static const Regex s_reOpenOL(R"i____(^\s+(?:\d+[.)]|[a-z]\))\s+)i____");
                RegexMatches aMatches;
                if (s_reOpenUL.matches(line))
                {
                    if (paraState != PState::UL)
                        strOutput += fmt.openUL();
                    else
                        strOutput += "\n\n";

                    strOutput += fmt.openLI();
                    paraState = PState::UL;

                    static const Regex s_stripBullet(R"i____(^\s+(?:--?|\*)\s+)i____");
                    s_stripBullet.findReplace(line, " ", false);
                }
                else if (s_reOpenOL.matches(line))
                {
                    if (paraState != PState::OL)
                        strOutput += fmt.openOL();
                    else
                        strOutput += "\n\n";

                    strOutput += fmt.openLI();
                    paraState = PState::OL;

                    static const Regex s_stripNumber(R"i____(^\s+[a-z0-9]+[.)]\s+)i____");
                    s_stripNumber.findReplace(line, " ", false);
                }
                else
                {
                    if (paraState == PState::UL)
                        strOutput += fmt.closeUL();
                    else if (paraState == PState::OL)
                        strOutput += fmt.closeOL();

                    strOutput += fmt.openPara();
                    paraState = PState::OPEN;
                }

                fPrependNewline = false;
                fHadEmptyLine = false;
            } // if (fHadEmptyLine)

            static const Regex s_reClearLeadingWhitespace(R"i____(^\s+)i____");
            s_reClearLeadingWhitespace.findReplace(line, "", false);

            if (fPrependNewline)
                strOutput += "\n";

            strOutput += fmt.format(line, false);

            fPrependNewline = true;
        }
    } // end while(std::getline(ss, line, '\n'))

    if (paraState == PState::OPEN)
    {
        strOutput += fmt.closePara();
    }
    else if (paraState == PState::UL)
    {
        strOutput += fmt.closeUL();
    }
    else if (paraState == PState::OL)
    {
        strOutput += fmt.closeOL();
    }

    // Allow <b>, <i>, <ol> etc.
    fmt.convertFormatting(strOutput);

    // Linkify all class names.
    string strSelf;
    const string *pstrSelf = NULL;
    if (this->getType() == Type::CLASS)
    {
        strSelf = this->getIdentifier();
        pstrSelf = &strSelf;
    }
    ClassComment::LinkifyClasses(fmt, strOutput, pstrSelf);

    // Resolve \refs to functions.
    // htmlComment =~ s/\\ref\s+([a-zA-Z_0-9]+::[a-zA-Z_0-9]+\(\))/resolveFunctionRef($1)/eg;

    // Resolve \refs to page IDs.
    static const Regex s_reResolveRefs(R"i____(@ref\s+([-a-zA-Z0-9:\(\\_]+))i____");
    s_reResolveRefs.findReplace(strOutput,
                                [&fmt, this](const StringVector &vMatches, string &strReplace)
                                {
                                    strReplace = this->resolveExplicitRef(fmt, vMatches[1]);
                                },
                                true);

    // Linkify REST API references.
    static const Regex s_reRESTAPI(R"i____((GET|POST|PUT|DELETE)\s+\/([-a-zA-Z]+)\s+REST)i____");
    s_reRESTAPI.findReplace(strOutput,
                            [&fmt](const StringVector &vMatches, string &strReplace)
                            {
                                const string &strMethod = vMatches[1];
                                const string &strName = vMatches[2];
                                string strIdentifier = RESTComment::MakeIdentifier(strMethod, strName);
                                PRESTComment pREST;
                                if ((pREST = RESTComment::Find(strIdentifier)))
                                    strReplace = pREST->makeLink(fmt);
                                else
                                {
                                    Debug::Warning("Invalid REST API reference " + strIdentifier);
                                    strReplace = "?!?!? " + strIdentifier;
                                }
                            },
                            true);

    return strOutput;
}

/**
 *  Called from a closure in formatComment() whenever a \ref is encountered. strMatch has the
 *  contents of what follows \ref, see the regexp in the code above.
 */
string CommentBase::resolveExplicitRef(FormatterBase &fmt,
                                       const string &strMatch0)
{
    // In LaTeX mode, the ref has underscores escaped; then we need to unescape them.
    string strMatch = strMatch0;
    if (fmt.getMode() == OutputMode::LATEX)
        stringReplace(strMatch, "\\_", "_");

    PTableComment pTable;
    PPageComment pPage;
    RegexMatches aMatches2;
    if ((pTable = TableComment::Find(strMatch)))
        return pTable->makeLink(fmt);

    if ((pPage = PageComment::Find(strMatch)))
        return pPage->makeLink(fmt);

    static const Regex s_reClassAndFunction(R"i____((?:([a-zA-Z_0-9]+)::)?([a-zA-Z_0-9]+)\()i____");
    if (s_reClassAndFunction.matches(strMatch, aMatches2))
    {
        PClassComment p2;
        const string &strClass = aMatches2.get(1);
        const string &strFunction = aMatches2.get(2);
        ClassComment *pClass;
        if (strClass.empty())
            pClass = this->getClassForFunctionRef();
        else
        {
            if (!(p2 = ClassComment::Find(strClass)))
                Debug::Warning("Invalid class \"" + strClass + "\" in \\ref to function \"" + strMatch + "\"");
            pClass = p2.get();
        }
        if (pClass)
            return pClass->makeLink(fmt, strMatch, &strFunction);
    }

    Debug::Warning("Invalid \\ref " + strMatch);
    return "?!?!?!?";
}
