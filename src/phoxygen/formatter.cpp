/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#include "xwp/basetypes.h"
#include "xwp/stringhelp.h"
#include "xwp/regex.h"

#include "phoxygen/formatter.h"

FormatterPlain g_fmtPlain;
FormatterHTML g_fmtHTML;
FormatterLatex g_fmtLatex;

/***************************************************************************
 *
 *  FormatterBase
 *
 **************************************************************************/

const string FormatterBase::Empty = "";
const string FormatterBase::TwoNewlines = "\n\n";

/* static */
FormatterBase& FormatterBase::Get(OutputMode mode)
{
    switch (mode)
    {
        case OutputMode::PLAINTEXT:
            return g_fmtPlain;

        case OutputMode::HTML:
            return g_fmtHTML;

        case OutputMode::LATEX:
        break;
    }

    // LaTeX
    return g_fmtLatex;
}

/***************************************************************************
 *
 *  FormatterHTML
 *
 **************************************************************************/

const string FormatterHTML::OpenP = "<p>";
const string FormatterHTML::CloseP = "</p>";
const string FormatterHTML::OpenPRE = "\n<pre>\n";
const string FormatterHTML::ClosePRE = "\n</pre>\n";
const string FormatterHTML::OpenUL = "<ul>";
const string FormatterHTML::CloseUL = "</ul>\n\n";
const string FormatterHTML::OpenOL = "<ol>";
const string FormatterHTML::CloseOL = "</ol>\n\n";
const string FormatterHTML::OpenLI = "<li>";
const string FormatterHTML::EndLI = "</li>";
const string FormatterHTML::OpenCODE = "<code>";
const string FormatterHTML::CloseCODE = "</code>";

/* virtual */
string FormatterHTML::format(const string &str) /* override */
{
    return ::toHTML2(str);
}

/* virtual */
void FormatterHTML::convertFormatting(string &str) /* override */
{
    /* format() above converted <TAG> to &lt;TAG&gt; --
       permit the following HTML tags by converting &lt;TAG&gt; back to <TAG>: */
    static const StringVector svTags( { "ol", "ul", "li", "b", "i", "code" } );
    static const Regex reTags("&lt;(/?(?:" + implode("|", svTags) + "))&gt;");
    reTags.findReplace(str, "<$1>", true);

    static const Regex reCode("`([^`]+)`");
    reCode.findReplace(str, "<code>$1</code>", true);

    // Linkify links.
    static const Regex reLink("(https?://\\S+)");
            // "(?:^|[\\W])((ht|f)tp(s?):\\/\\/|www\\.)(([\\w\\-]+\\.){1,}?([\\w\\-.~]+\\/?)*[\\[a-zA-Z0-9].,%_=?&#\\-+()\\[\\]\\*$~@!:/{};']*)");
    reLink.findReplace(str, "<a href=\"$1\">$1</a>", true);
}

/* virtual */
string FormatterHTML::makeLink(const string &strIdentifier,     //!< in: including "table_" etc. prefix
                               const string *pstrAnchor,
                               const string &strTitle) /* override */
{
    string str = "<a href=\"" + strIdentifier + ".html";
    if (pstrAnchor)
        str += "#" + *pstrAnchor;
    return str + "\">" + strTitle + "</a>";
}


/***************************************************************************
 *
 *  FormatterLaTeX
 *
 **************************************************************************/

const string FormatterLatex::BeginVerbatim = "\n\\begin{verbatim}\n";
const string FormatterLatex::EndVerbatim = "\n\\end{verbatim}\n";
const string FormatterLatex::BeginItemize = "\\begin{itemize}\n";
const string FormatterLatex::EndItemize = "\n\\end{itemize}";
const string FormatterLatex::BeginEnumerate = "\\begin{enumerate}\n";
const string FormatterLatex::EndEnumerate = "\n\\end{enumerate}";
const string FormatterLatex::Item = "\\item ";
const string FormatterLatex::OpenTextTT = "\\texttt{";
const string FormatterLatex::CloseCurly = "}";

/* virtual */
string FormatterLatex::format(const string &str) /* override */
{
    return ::toLaTeX2(str);
}

/* virtual */
void FormatterLatex::convertFormatting(string &str) /* override */
{
    static const Regex re2(R"i____(<ul>)i____");
    re2.findReplace(str, BeginItemize, true);
    static const Regex re3(R"i____(<\/ul>)i____");
    re3.findReplace(str, EndItemize, true);
    static const Regex re4(R"i____(<ol>)i____");
    re4.findReplace(str, BeginEnumerate, true);
    static const Regex re5(R"i____(<\/ol>)i____");
    re5.findReplace(str, EndEnumerate, true);
    static const Regex re6(R"i____(<li>)i____");
    re6.findReplace(str, Item, true);
    static const Regex re7(R"i____(<\/li>)i____");
    re7.findReplace(str, Empty, true);

    // Markdown
    static const Regex reCode("`([^`]+)`");
    reCode.findReplace(str, OpenTextTT + "$1" + CloseCurly, true);

    // Linkify links
    static const Regex reLink("(https?://\\S+)");
            // "(?:^|[\\W])((ht|f)tp(s?):\\/\\/|www\\.)(([\\w\\-]+\\.){1,}?([\\w\\-.~]+\\/?)*[\\[a-zA-Z0-9].,%_=?&#\\-+()\\[\\]\\*$~@!:/{};']*)");
    reLink.findReplace(str, "\\url{$1}", true);
}
