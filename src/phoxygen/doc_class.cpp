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

ClassesMap g_mapClasses;

ClassComment::ClassComment(const string &strKeyword,
                           const string &strIdentifier,
                           const string &strComment,
                           const string &strInputFile,
                           int linenoFirst,
                           int linenoLast)
    : CommentBase(Type::CLASS,
                  strKeyword,
                  strIdentifier,
                  strComment,
                  strInputFile,
                  linenoFirst,
                  linenoLast),
      _reClass("(^|p>|\\s+)" + strIdentifier + "($|\\s|[.,!;])"),
      _strReplSelf("$1<b>" + strIdentifier + "</b>$2"),
      _strReplOther("$1<a href=\"class_" + strIdentifier + ".html\">" + strIdentifier + "</a>$2")
{
}

/* static */
PClassComment ClassComment::Make(const string &strKeyword,
                                 const string &strIdentifier,
                                 const string &strComment,
                                 const string &strInputFile,
                                 int linenoFirst,
                                 int linenoLast)
{
    class Derived : public ClassComment { public: Derived(const string &strKeyword,
                                 const string &strIdentifier,
                                 const string &strComment,
                                 const string &strInputFile,
                                 int linenoFirst,
                                 int linenoLast) : ClassComment(strKeyword, strIdentifier, strComment, strInputFile, linenoFirst, linenoLast) {}
    };
    auto p = make_shared<Derived>(strKeyword, strIdentifier, strComment, strInputFile, linenoFirst, linenoLast);
    g_mapClasses[strIdentifier] = p;
    return p;
}

void ClassComment::addMember(PFunctionComment pMember)
{
    _vMembers.push_back(pMember);
    _mapMembers[pMember->getIdentifier()] = pMember;
}

/* virtual */
string ClassComment::getTitle(OutputMode mode) /* override */
{
    return "Class " + _identifier;
}

string ClassComment::formatChildrenList()
{
    string htmlBody;

    if (_vChildren.size())
    {
        htmlBody += "<ul>";
        for (auto pChild : _vChildren)
        {
            const string &strIdentifier = pChild->getIdentifier();
            htmlBody += "<li><a href=\"class_" + strIdentifier + ".html\">" + strIdentifier + "</a>";

            htmlBody += pChild->formatChildrenList();

            htmlBody += "</li>\n";
        }
        htmlBody += "</ul>";
    }

    return htmlBody;
}

string ClassComment::makeLink(FormatterBase &fmt,
                              const string &strDisplay,
                              const string *pstrAnchor)
{
    return fmt.makeLink("class_" + _identifier,
                        pstrAnchor,
                        strDisplay);
}

/**
 *  Called from CommentBase::formatComment() for each class to linkify class names.
 */
void ClassComment::linkify(OutputMode mode,
                           string &htmlComment,
                           bool fSelf)
{
    if (mode == OutputMode::HTML)
        _reClass.findReplace(htmlComment,
                            fSelf ? _strReplSelf
                                : _strReplOther,
                            true); // fGlobal
}

/* static */
const ClassesMap& ClassComment::GetAll()
{
    return g_mapClasses;
}

/* static */
PClassComment ClassComment::Find(const string &strClass)
{
    auto it = g_mapClasses.find(strClass);
    if (it != g_mapClasses.end())
        return it->second;

    return NULL;
}
