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

PagesMap g_mapPages;

PageComment::PageComment(const string &strPageID,
                         const string &strTitle,
                         const string &strInputFile,
                         int linenoFirst,
                         int linenoLast)
        : CommentBase(Type::PAGE,
                      "",
                      strPageID,
                      "",
                      strInputFile,
                      linenoFirst,
                      linenoLast)
{
    _title = strTitle;
}

PPageComment PageComment::Make(const string &strPageID,
                               const string &strTitle,
                               const string &strInputFile,
                               int linenoFirst,
                               int linenoLast)
{
    class Derived : public PageComment { public: Derived(const string &strPageID,
                               const string &strTitle,
                               const string &strInputFile,
                               int linenoFirst,
                               int linenoLast) : PageComment(strPageID, strTitle, strInputFile, linenoFirst, linenoLast) {}
    };
    auto p = make_shared<Derived>(strPageID, strTitle, strInputFile, linenoFirst, linenoLast);
    g_mapPages[strPageID] = p;
    return p;
}

/* virtual */
string PageComment::makeTarget(FormatterBase &fmt) /* override */
{
    return fmt.makeTarget("page", _identifier);
}

/* virtual */
string PageComment::getTitle(OutputMode mode) /* override */
{
    switch (mode)
    {
        case OutputMode::PLAINTEXT:
            return _title;

        case OutputMode::HTML:
            return toHTML2(_title);

        case OutputMode::LATEX:
        break;
    }

    // LaTeX
    return toLaTeX2(_title, false);
}

/* static */
const PagesMap& PageComment::GetAll()
{
    return g_mapPages;
}

/* static */
PPageComment PageComment::Find(const string &strPage)
{
    auto it = g_mapPages.find(strPage);
    if (it != g_mapPages.end())
        return it->second;

    return NULL;
}
