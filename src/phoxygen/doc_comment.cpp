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

string CommentBase::formatContext()
{
    return _file + " (lines " + to_string(_linenoFirst) + "--" + to_string(_linenoLast) + ")";
}

/* virtual */
string CommentBase::formatComment()
{
    string htmlComment;
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

    stringstream ss(_comment);
    string line;

    while(std::getline(ss, line, '\n'))
    {
        static const Regex s_reEmptyLine(R"i____(^\s*$)i____");
        static const Regex s_reOpenPRE(R"i____(^\s*```(?:php|javascript|xml)\s*$)i____");
        static const Regex s_reClosePRE(R"i____(^\s*```\s*$)i____");
        if (s_reOpenPRE.matches(line))
        {
            paraState = PState::VERBATIM;
            htmlComment += "\n<pre>\n";
        }
        else if (paraState == PState::VERBATIM)
        {
            if (s_reClosePRE.matches(line))
            {
                htmlComment += "</pre>\n";
                paraState = PState::INIT;
            }
            else
                htmlComment += ::toHTML(line) + "\n";
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
                {
                    htmlComment += "</p>\n\n";
                }
                else if (    (paraState == PState::UL)
                          || (paraState == PState::OL)
                        )
                {
                    htmlComment += "</li>";
                }

                static const Regex s_reOpenUL(R"i____(^\s+(?:--?|\*)\s+)i____");
                static const Regex s_reOpenOL(R"i____(^\s+\d+[.)]\s+)i____");
                RegexMatches aMatches;
                if (s_reOpenUL.matches(line))
                {
                    if (paraState != PState::UL)
                        htmlComment += "<ul>";
                    else
                        htmlComment += "\n\n";

                    htmlComment += "<li>";
                    paraState = PState::UL;

                    static const Regex s_stripBullet(R"i____(^\s+(?:--?|\*)\s+)i____");
                    s_stripBullet.findReplace(line, " ", false);
                }
                else if (s_reOpenOL.matches(line))
                {
                    if (paraState != PState::OL)
                        htmlComment += "<ol>";
                    else
                        htmlComment += "\n\n";

                    htmlComment += "<li>";
                    paraState = PState::OL;

                    static const Regex s_stripNumber(R"i____(^\s+\d+[.)]\s+)i____");
                    s_stripNumber.findReplace(line, " ", false);
                }
                else
                {
                    if (paraState == PState::UL)
                        htmlComment += "</ul>\n\n";
                    else if (paraState == PState::OL)
                        htmlComment += "</ol>\n\n";

                    htmlComment += "<p>";
                    paraState = PState::OPEN;
                }

                fPrependNewline = false;
                fHadEmptyLine = false;
            } // if (fHadEmptyLine)

            static const Regex s_reClearLeadingWhitespace(R"i____(^\s+)i____");
            s_reClearLeadingWhitespace.findReplace(line, "", false);

            if (fPrependNewline)
                htmlComment += "\n";

            htmlComment += ::toHTML(line);

            fPrependNewline = true;
        }
    } // end while(std::getline(ss, line, '\n'))

    if (paraState == PState::OPEN)
    {
        htmlComment += "</p>";
    }
    else if (paraState == PState::UL)
    {
        htmlComment += "</ul>";
    }
    else if (paraState == PState::OL)
    {
        htmlComment += "</ol>";
    }

    // Permit the following HTML tags by converting &lt;TAG&gt; back to <TAG>.
    static const StringVector svTags( { "ol", "ul", "li", "b", "i", "code" } );
    static const Regex reTags("&lt;(/?(?:" + implode("|", svTags) + "))&gt;");
    reTags.findReplace(htmlComment, "<$1>", true);

    // Some Markdown syntax.
    static const Regex reCode("`([^`]+)`");
    reCode.findReplace(htmlComment, "<code>$1</code>", true);

    // Linkify links.
    static const Regex reLink("(https?://\\S+)");
            // "(?:^|[\\W])((ht|f)tp(s?):\\/\\/|www\\.)(([\\w\\-]+\\.){1,}?([\\w\\-.~]+\\/?)*[\\[a-zA-Z0-9].,%_=?&#\\-+()\\[\\]\\*$~@!:/{};']*)");
    reLink.findReplace(htmlComment, "<a href=\"$1\">$1</a>", true);

    // Linkify all class names.
    for (const auto &it : ClassComment::GetAll())
    {
        const string &strClass = it.first;
        PClassComment pClass = it.second;

        //  print ::Dumper($self);
        if (    (this->getType() == Type::CLASS)
             && (this->getIdentifier() == strClass)
           )
            pClass->linkify(htmlComment, true);
        else
            pClass->linkify(htmlComment, false);
    }

    // Resolve \refs to functions.
    // htmlComment =~ s/\\ref\s+([a-zA-Z_0-9]+::[a-zA-Z_0-9]+\(\))/resolveFunctionRef($1)/eg;

    // Resolve \refs to page IDs.
    static const Regex s_reResolveRefs(R"i____(\\ref\s+([-a-zA-Z_0-9:\(]+))i____");
    s_reResolveRefs.findReplace(htmlComment,
                                [this](const StringVector &vMatches, string &strReplace)
                                {
                                    strReplace = this->resolveExplicitRef(vMatches[1]);
                                },
                                true);

    // Linkify REST API references.
    static const Regex s_reRESTAPI(R"i____((GET|POST|PUT|DELETE)\s+\/([-a-zA-Z]+)\s+REST)i____");
    s_reRESTAPI.findReplace(htmlComment,
                            [](const StringVector &vMatches, string &strReplace)
                            {
                                const string &strMethod = vMatches[1];
                                const string &strName = vMatches[2];
                                string strIdentifier = RESTComment::MakeIdentifier(strMethod, strName);
                                PRESTComment pREST;
                                if ((pREST = RESTComment::Find(strIdentifier)))
                                    strReplace = pREST->makeLink();
                                else
                                {
                                    Debug::Warning("Invalid REST API reference " + strIdentifier);
                                    strReplace = "?!?!? " + strIdentifier;
                                }
                            },
                            true);

    return htmlComment;
}

/**
 *  Called from a closure in formatComment() whenever a \ref is encountered. strMatch has the
 *  contents of what follows \ref, see the regexp in the code above.
 */
string CommentBase::resolveExplicitRef(const string &strMatch)
{
    PTableComment pTable;
    PPageComment pPage;
    RegexMatches aMatches2;
    if ((pTable = TableComment::Find(strMatch)))
        return pTable->makeLink();

    if ((pPage = PageComment::Find(strMatch)))
        return pPage->makeLink();

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
            return pClass->makeLink(strMatch, &strFunction);
    }

    Debug::Warning("Invalid \\ref " + strMatch);
    return "?!?!?!?";
}
