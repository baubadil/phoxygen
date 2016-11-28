/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef PHOXYGEN_H
#define PHOXYGEN_H

#include "xwp/basetypes.h"
#include "xwp/stringhelp.h"
#include "xwp/debug.h"
#include "xwp/regex.h"

#include <memory>
#include <map>

using namespace XWP;

const DebugFlag MAIN        = (1 <<  1);
const DebugFlag FORMAT      = (1 <<  2);

enum class State
{
    INIT,
    IN_DOCCOMMENT,
    IN_DOCCOMMENT_PAGE,
    IN_DOCCOMMENT_MAINPAGE,
    EXAMINE_NEXT_AFTER_DOCCOMMENT,
    IN_FUNCTION_HEADER,
    IN_CREATE_TABLE,
};


class CommentBase;
typedef shared_ptr<CommentBase> PCommentBase;

class ClassComment;
typedef shared_ptr<ClassComment> PClassComment;
typedef vector<PClassComment> ClassesVector;
typedef map<string, PClassComment> ClassesMap;

class FunctionComment;
typedef shared_ptr<FunctionComment> PFunctionComment;
typedef vector<PFunctionComment> FunctionsVector;
typedef map<string, PFunctionComment> FunctionsMap;

class TableComment;
typedef shared_ptr<TableComment> PTableComment;
typedef map<string, PTableComment> TablesMap;

class PageComment;
typedef shared_ptr<PageComment> PPageComment;
typedef map<string, PPageComment> PagesMap;

class MainPageComment;
typedef shared_ptr<MainPageComment> PMainPageComment;


/***************************************************************************
 *
 *  DocComment
 *
 **************************************************************************/

class CommentBase
{
    static const string s_strUnknown;

protected:
    enum class Type
    {
        PAGE,
        MAINPAGE,
        CLASS,
        FUNCTION,
        TABLE,
        REST
    };

    Type _type;
    string _keyword;
    string _identifier;
    string _comment;
    string _file;
    int _linenoFirst;
    int _linenoLast;

    CommentBase(Type theType,
                const string &strKeyword,
                const string &strIdentifier,
                const string &strComment,
                const string &strFile,
                int linenoFirst,
                int linenoLast)
        : _type(theType),
          _keyword(strKeyword),
          _identifier(strIdentifier),
          _comment(strComment),
          _file(strFile),
          _linenoFirst(linenoFirst),
          _linenoLast(linenoLast)
    { };

public:
    Type getType() const
    {
        return _type;
    }

    const string& getIdentifier() const
    {
        return _identifier;
    }

    virtual string getTitle(bool fHTML)
    {
        return s_strUnknown;
    }

    void append(string strLine)
    {
        _comment += strLine;
    }

    string formatContext();

    virtual string formatComment();
};


/***************************************************************************
 *
 *  PageComment
 *
 **************************************************************************/

class PageComment : public CommentBase
{
    string      _title;
    string      _strTitleHTML;

protected:
    PageComment(const string &strPageID,
                const string &strTitle,
                const string &strInputFile,
                int linenoFirst,
                int linenoLast);

public:
    static PPageComment Make(const string &strPageID,
                             const string &strTitle,
                             const string &strInputFile,
                             int linenoFirst,
                             int linenoLast);

    virtual string getTitle(bool fHTML) override;

    string makeLink()
    {
        return "<a href=\"page_" + _identifier + ".html\">" + getTitle(true) + "</a>";
    }

    static const PagesMap& GetAll();

    static PPageComment Find(const string &strPage);
};


/***************************************************************************
 *
 *  MainPageComment
 *
 **************************************************************************/

class MainPageComment : public PageComment
{
public:
    MainPageComment(const string &strTitle,
                    const string &strInputFile,
                    int linenoFirst,
                    int linenoLast)
        : PageComment("",
                      strTitle,
                      strInputFile,
                      linenoFirst,
                      linenoLast)
    {
        _type = Type::MAINPAGE;
    }
};


/***************************************************************************
 *
 *  RESTComment
 *
 **************************************************************************/

class RESTComment;
typedef shared_ptr<RESTComment> PRESTComment;
typedef map<string, PRESTComment> RESTMap;

class RESTComment : public CommentBase
{
    string      _strMethod;
    string      _strName;
    string      _strArgs;

    RESTComment(const string &strMethod,
                const string &strName,
                const string &strArgs,
                const string &strComment,
                const string &strInputFile,
                int linenoFirst,
                int linenoLast);
public:
    static PRESTComment Make(const string &strMethod,
                             const string &strName,
                             const string &strArgs,
                             const string &strComment,
                             const string &strInputFile,
                             int linenoFirst,
                             int linenoLast);

    virtual string getTitle(bool fHTML) override
    {
        return (fHTML) ? "<code>" + _strMethod + " /api/" + _strName + "</code> REST API"
                       : _strMethod + " /api/" + _strName + " REST API";
    }

    string makeHREF()
    {
        return "rest_" + getIdentifier() + ".html";
    }

    string makeLink()
    {
        return "<a href=\"" + makeHREF() + "\">" + _strMethod + " /api/" + _strName + "</a>";
    }

    bool operator<(const RESTComment &p) const
    {
        if (_strName < p._strName)
            return true;
        if (_strName == p._strName)
            return _strMethod < p._strMethod;
        return false;
    }

    static string MakeIdentifier(const string &name, const string method)
    {
        return name + "_" + strToLower(method);
    }

    static const RESTMap& GetAll();

    static PRESTComment Find(const string &strIdentifier);
};


/***************************************************************************
 *
 *  TableComment
 *
 **************************************************************************/

class TableComment : public CommentBase
{
    StringVector _vDefinitionLines;

    TableComment(const string &strIdentifier,
                 const string &strComment,
                 const string &strInputFile,
                 int linenoFirst,
                 int linenoLast)
        : CommentBase(Type::TABLE,
                      "",
                      strIdentifier,
                      strComment,
                      strInputFile,
                      linenoFirst,
                      linenoLast)
    { }

public:
    static PTableComment Make(const string &strIdentifier,
                              const string &strComment,
                              const string &strInputFile,
                              int linenoFirst,
                              int linenoLast);

    void processInputLine(const string &strLine, State &state);

    virtual string getTitle(bool fHTML) override;

    virtual string formatComment() override;

    string makeLink()
    {
        return "<a href=\"table_" + _identifier + ".html\">" + _identifier + "</a>";
    }

    static const TablesMap& GetAll();

    static PTableComment Find(const string &strTable);
};


/***************************************************************************
 *
 *  ClassComment
 *
 **************************************************************************/

class ClassComment : public CommentBase
{
    FunctionsVector     _vMembers;
    FunctionsMap        _mapMembers;

    StringVector        _vParents;
    ClassesVector       _vChildren;

    StringVector        _vImplements;
    StringVector        _vExtends;

    // Regex for class linkification. Instantiate them in the class data so we don't have to recompile them.
    Regex               _reClass;

    string              _strReplSelf;
    string              _strReplOther;

    ClassComment(const string &strKeyword,
                 const string &strIdentifier,
                 const string &strComment,
                 const string &strInputFile,
                 int linenoFirst,
                 int linenoLast);

public:
    static PClassComment Make(const string &strKeyword,
                              const string &strIdentifier,
                              const string &strComment,
                              const string &strInputFile,
                              int linenoFirst,
                              int linenoLast);

    void addImplements(const string &str)
    {
        _vImplements.push_back(str);
    }

    void addExtends(const string &str)
    {
        _vExtends.push_back(str);
    }

    void addParent(const string &str)
    {
        _vParents.push_back(str);
    }

    void addChild(PClassComment pChild)
    {
        _vChildren.push_back(pChild);
    }

    void addMember(PFunctionComment pMember);

    const StringVector& getParents()
    {
        return _vParents;
    }

    const ClassesVector& getChildren()
    {
        return _vChildren;
    }

    const FunctionsVector& getMembers()
    {
        return _vMembers;
    }

    virtual string getTitle(bool fHTML) override;

    string formatChildrenList();

    string makeLink(const string &strDisplay,
                    const string *pstrAnchor);

    void linkify(string &htmlComment,
                 bool fSelf);

    static const ClassesMap& GetAll();

    static PClassComment Find(const string &strClass);

};


/***************************************************************************
 *
 *  FunctionComment
 *
 **************************************************************************/

struct Param
{
    string param;
    string description;
};
typedef vector<Param> ParamsVector;

class FunctionComment : public CommentBase
{
    PClassComment   _pClass;
    ParamsVector    _vParams;

public:
    FunctionComment(const string &strKeyword,
                    const string &strIdentifier,
                    const string &strComment,
                    const string &strInputFile,
                    int linenoFirst,
                    int linenoLast)
        : CommentBase(Type::FUNCTION,
                      strKeyword,
                      strIdentifier,
                      strComment,
                      strInputFile,
                      linenoFirst,
                      linenoLast)
    { }

    void setClass(PClassComment pClass);

    void processInputLine(const string &strLine, State &state);

    string formatFunction(bool fLong);
};

#endif // PHOXYGEN_H
