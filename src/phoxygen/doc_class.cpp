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
                  linenoLast,
                  "class_" + strIdentifier),
      _reClassHTML("(^|p>|\\s)" + strIdentifier + "($|\\s|[.,!;])"),
      _reClassLaTeX("(^|\\s)" + strIdentifier + "($|\\s|[.,!;])"),
      _strReplSelfHTML("$1<b>" + strIdentifier + "</b>$2"),
      _strReplOtherHTML("$1<a href=\"" + _strTargetHTML + "\">" + strIdentifier + "</a>$2")
{
    auto fmt = FormatterBase::Get(OutputMode::LATEX);
    _strReplOtherLaTeX = "$1" + FormatterLatex::MakeLink(_strTargetLaTeX,
                                                         _identifier) + "$2";
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

string ClassComment::formatChildrenList(FormatterBase &fmt)
{
    string htmlBody;

    if (_vChildren.size())
    {
        htmlBody += "<ul>";
        for (auto pChild : _vChildren)
        {
            htmlBody += "<li>" + pChild->makeLink(fmt, pChild->getIdentifier(), NULL);

            // Recurse!
            htmlBody += pChild->formatChildrenList(fmt);

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
                strFormatted += fmt.makeLink(pParent->getTarget(fmt),
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
            strFormatted += fmt.makeLink(pChild->getTarget(fmt), NULL, strChild);
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
            htmlThis += fmt.openLI() + fmt.makeLink(this->getTarget(fmt),
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
//             htmlBody += fmt.openUL();
            int c = 0;
            for (auto pMemberFunction : pllMembers)
            {
                if (c++ > 0)
                    htmlBody += "\\vspace{4mm}\n\n\\noindent{} ";
                htmlBody += pMemberFunction->formatFunction(fmt, true) + "\n\n\\vspace{1mm}\\noindent{}" + pMemberFunction->formatComment(fmt.getMode());
            }
//             htmlBody += fmt.closeUL();
        }


    }

    return htmlBody;
}

string ClassComment::makeLink(FormatterBase &fmt,
                              const string &strDisplay,
                              const string *pstrAnchor)
{
    return fmt.makeLink(getTarget(fmt),
                        pstrAnchor,
                        strDisplay);
}

/* static */
void ClassComment::LinkifyClasses(FormatterBase &fmt,
                                  string &str,
                                  const string *pstrSelf)
{
    for (const auto &it : GetAll())
    {
        const string &strClass = it.first;
        PClassComment pClass = it.second;

        if (pstrSelf && (strClass == *pstrSelf))
            pClass->linkify(fmt, str, true);
        else
            pClass->linkify(fmt, str, false);
    }
}

/**
 *  Called from CommentBase::formatComment() for each class to linkify class names.
 */
void ClassComment::linkify(FormatterBase &fmt,
                           string &str,
                           bool fSelf)
{
    if (fmt.getMode() == OutputMode::HTML)
        _reClassHTML.findReplace(str,
                             fSelf ? _strReplSelfHTML
                                   : _strReplOtherHTML,
                             true); // fGlobal
    else
        _reClassLaTeX.findReplace(str,
                                  _strReplOtherLaTeX,
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
