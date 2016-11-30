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
string ClassComment::makeTarget(FormatterBase &fmt) /* override */
{
    return fmt.makeTarget("class", _identifier);
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

string ClassComment::formatHierarchy(FormatterBase &fmt)
{
    string strFormatted;

    auto pllParents = getParents();
    auto pllChildren = getChildren();
    if (pllParents.size())
    {
        for (const string &strParent : pllParents)
        {
            auto pParent = ClassComment::Find(strParent);
            if (pParent)
                strFormatted += fmt.makeLink(pParent->makeTarget(fmt),
                                             NULL,
                                             strParent) + fmt.mdash();
        }
    }

    if (    (pllParents.size())
         || (pllChildren.size())
       )
    {
        strFormatted += fmt.makeBold(_identifier);
    }
    else
    {
        strFormatted += fmt.openPara() + "This class stands alone and has neither parents nor children." + fmt.closePara();
    }

    if (pllChildren.size())
    {
        strFormatted += fmt.mdash();
        size_t c = 0;
        for (auto pChild : pllChildren)
        {
            if (c)
                strFormatted += ", ";
            const string &strChild = pChild->getIdentifier();
            strFormatted += fmt.makeLink(pChild->makeTarget(fmt), NULL, strChild);
            ++c;
        }
    }

    return strFormatted;
}

string ClassComment::formatMembers(FormatterBase &fmt)
{
    string htmlBody, htmlThis;

    auto pllMembers = getMembers();
    size_t cMembers = pllMembers.size();
    Debug::Log(MAIN, "Class " + _identifier + " has " + to_string(cMembers) + " members");
    if (cMembers)
    {
        htmlBody += "\n\n" + fmt.openPara() + fmt.makeBold(to_string(cMembers) + " members") + fmt.closePara();

        htmlThis = fmt.openUL();
        for (auto pMemberFunction : pllMembers)
        {
            const string &strIdentifier = pMemberFunction->getIdentifier();
            htmlThis += fmt.openLI() + fmt.makeLink(this->makeTarget(fmt),
                                                    &strIdentifier,
                                                    pMemberFunction->formatFunction(fmt, false));
            htmlThis += fmt.closeLI();
        }
        htmlThis += fmt.closeUL();

        htmlBody += htmlThis;

        if (fmt.getMode() == OutputMode::HTML)
        {
            htmlBody += fmt.makeHeading(2, "Details");;
            htmlThis = "";

            for (auto pMemberFunction : pllMembers)
            {
                htmlThis += "<dl><dt id=\"" + pMemberFunction->getIdentifier() + "\">";
                htmlThis += pMemberFunction->formatFunction(fmt, true) + "</dt>\n";
                htmlThis += "<dd>" + pMemberFunction->formatComment(fmt.getMode()) + "</dd></dl>";
                htmlThis += "\n";
            }

            htmlBody += htmlThis;
        }
        else
        {
            htmlBody += "\n\\begin{description}\n";

            for (auto pMemberFunction : pllMembers)
                htmlBody += "\\item[" + pMemberFunction->formatFunction(fmt, false) + "] --- " + pMemberFunction->formatComment(fmt.getMode());

            htmlBody += "\n\\end{description}\n";
        }


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
