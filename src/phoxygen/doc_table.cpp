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

TablesMap g_mapTables;

/* static */
PTableComment TableComment::Make(const string &strIdentifier,
                                 const string &strComment,
                                 const string &strInputFile,
                                 int linenoFirst,
                                 int linenoLast)
{
    class Derived : public TableComment { public: Derived(const string &strIdentifier,
                                 const string &strComment,
                                 const string &strInputFile,
                                 int linenoFirst,
                                 int linenoLast) : TableComment(strIdentifier, strComment, strInputFile, linenoFirst, linenoLast) {}
    };
    auto p = make_shared<Derived>(strIdentifier, strComment, strInputFile, linenoFirst, linenoLast);
    g_mapTables[strIdentifier] = p;
    return p;

}

void TableComment::processInputLine(const string &strLine,
                                    State &state)
{
    _vDefinitionLines.push_back(trimmed(strLine));

    static const Regex s_reEOD(R"i____(^\s*(?:EOD|SQL|\)))i____");

    if (s_reEOD.matches(strLine))
        state = State::INIT;
}

/* virtual */
string TableComment::getTitle(OutputMode mode) /* override */
{
    switch (mode)
    {
        case OutputMode::PLAINTEXT:
            return "SQL table " + _identifier;

        case OutputMode::HTML:
            return "SQL table <code>" + _identifier + "</code>";

        case OutputMode::LATEX:
        break;
    }

    // LaTeX
    return "SQL table \\texttt{" + _identifier + "}";
}

/* virtual */
string TableComment::formatComment(OutputMode mode) /* override */
{
    string str = CommentBase::formatComment(mode);

    FormatterBase &fmt = FormatterBase::Get(mode);

    size_t cLines = _vDefinitionLines.size();
    if (cLines)
    {
        size_t c = 0;

        str += fmt.openPRE();
        for (const string &line : _vDefinitionLines)
        {
            ++c;
            str += "    ";
            if ( (c > 1) && (c < cLines) )
                str += "    ";

            string htmlLine = fmt.format(line);

            for (auto it : g_mapTables)
            {
                const string &strTable = it.first;
                Regex reTableRef("REFERENCES\\s+" + strTable + "\\(");
                reTableRef.findReplace(htmlLine,
                                       "REFERENCES <a href=\"table_" + strTable + ".html\">" + strTable + "</a>(",
                                       true);   // fGlobal
            }

            str += htmlLine + "\n";
        }
        str += fmt.closePRE();
    }

    return str;
}

/* static */
const TablesMap& TableComment::GetAll()
{
    return g_mapTables;
}

/* static */
PTableComment TableComment::Find(const string &strTable)
{
    auto it = g_mapTables.find(strTable);
    if (it != g_mapTables.end())
        return it->second;

    return NULL;
}
