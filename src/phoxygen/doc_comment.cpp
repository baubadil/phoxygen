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

#include <sstream>

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
        OL
    };
    PState paraState = PState::INIT;

    stringstream ss(_comment);
    string line;

    while(std::getline(ss, line, '\n'))
    {
        static const Regex s_reEmptyLine(R"i____(^\s*$)i____");
        if (s_reEmptyLine.matches(line))
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
            }

            static const Regex s_reClearLeadingWhitespace(R"i____(^\s+)i____");
            s_reClearLeadingWhitespace.findReplace(line, "", false);

            if (fPrependNewline)
                htmlComment += "\n";

            htmlComment += ::toHTML(line);

            fPrependNewline = true;
        }
    }

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

    static const StringVector svTags( { "ol", "/ol", "ul", "/ul", "li", "/li" } );
    for (const auto &tag : svTags)
    {
        stringReplace(htmlComment,
                      "&lt;" + tag + "&gt;",
                      "<" + tag + ">");
    }

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
    // htmlComment =~ s/\\ref\s+([a-zA-Z_0-9]+::[a-zA-Z_0-9]+\(\))/resolveFunctionRef($1)/eg; TODO

    // Resolve \refs to page IDs.
    static const Regex s_reResolveRefs(R"i____(\\ref\s+([-a-zA-Z_0-9]+))i____");
    s_reResolveRefs.findReplace(htmlComment, 
                                [](const string &strFound)
                                {
                                },
                                true);
    // $htmlComment =~ s/\\ref\s+([-a-zA-Z_0-9]+)/resolvePlainRef($1)/eg; TODO

    // Linkify REST API references.
    // $htmlComment =~ s/(GET|POST|PUT|DELETE)\s+\/([-a-zA-Z]+)\s+REST/resolveRESTRef($1, $2)/eg; TODO

    return htmlComment;
}

const Regex g_reFuncRef(R"i____(([a-zA-Z_0-9]+)::([a-zA-Z_0-9]+)\(\))i____");

string resolveFunctionRef(const string &id)
{
    // my ($class, $function) = $id =~ /([a-zA-Z_0-9]+)::([a-zA-Z_0-9]+)\(\)/;

    RegexMatches aMatches;
    if (g_reFuncRef.matches(id, aMatches))
    {
        const string &strClass = aMatches.get(1);
        // const string &strFunction = aMatches.get(2);

        PClassComment pClass = ClassComment::Find(strClass);
        if (pClass)
        {
//             my $paMembers = $pClass->paMembers; TODO
//             if (my $pFunction = $paMembers->{$function})
//             {
//                 return "<a href=\"class_$class.html#$function\">$id</a>";
//             }
//             else
//             {
//                 ::myWarning("Invalid function name $function in reference $id");
//             }
        }
        else
        {
//             ::myWarning("Invalid class name $class in reference $id");
        }

        // return $class.$function;
    }

        return "";

        //     sub resolvePlainRef
//     {
//         my ($id) = @_;
//         if (my $p = $g_aPages{$id})
//         {
//             my $title = ::toHTML($p->title);
//             return "<a href=\"page_$id.html\">$title</a>";
//         }
//         else if ($p = $g_aTables{$id})
//         {
//             return "<a href=\"table_$id.html\">$id</a>";
//         }
//         else if ($p = $g_aRESTComments{$id})
//         {
//             return "<a href=\"rest_$id.html\">$id</a>";
//         }
//         ::myWarning("Invalid reference: page ID $id not found");
//         return $id;
//     }
//
//     sub resolveRESTRef
//     {
//         my ($method, $name) = @_;
//         my $id = $name.'_'.lc($method);
//         if (my $p = $g_aRESTComments{$id})
//         {
//             return "<a href=\"rest_$id.html\">$method /$name REST</a>";
//         }
//         ::myWarning("Invalid REST API reference: page ID $id not found");
//         return $id;
//     }

};
