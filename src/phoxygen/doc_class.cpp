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
string ClassComment::getTitle(bool fHTML) /* override */
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

string ClassComment::makeLink(const string &strDisplay,
                              const string *pstrAnchor)
{
    string str = "<li><a href=\"class_" + _identifier + ".html";
    if (pstrAnchor)
        str += "#" + *pstrAnchor;
    return str + "\">" + strDisplay + "</a>";
}

/**
 *  Called from CommentBase::formatComment() for each class to linkify class names.
 */
void ClassComment::linkify(string &htmlComment,
                           bool fSelf)
{
    _reClass.findReplace(htmlComment,
                         fSelf ? _strReplSelf
                               : _strReplOther,
                         true); // fGlobal
    // s/(^|p>|\s+)$class($|\s|[.,!;])/$1<b>$class<\/b>$2/g;
    // $htmlComment =~ s/(^|p>|\s+)$class($|\s|[.,!;])/$1<a href=\"class_$class.html\">$class<\/a>$2/g;
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
