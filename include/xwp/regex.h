/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 * 
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef XWP_REGEX_H
#define XWP_REGEX_H

#include "xwp/basetypes.h"

#include <functional>

class Regex;

class RegexMatches
{
    friend class Regex;
    StringVector v;
    const Regex *pRE = NULL;

public:
    const string& get(uint u);
    
    size_t size()
    {
        return v.size() ? v.size() - 1 : 0;
    }
};

class Regex
{
private:
    class Impl;
    Impl    *_pImpl;
    
public:
    Regex(const string &strRE);
    ~Regex();
    
    static int GetMaxCapture(const string &strReplace);
    
    bool matches(const string &strHaystack) const;
    bool matches(const string &strHaystack,
                 RegexMatches &aMatches) const;
    bool matches(const string &strHaystack,
                 RegexMatches &aMatches,
                 size_t &ofs) const;

    size_t findReplace(string &strHaystack,
                       const string &strReplace,
                       bool fGlobal) const;
    size_t findReplace(string &strHaystack,
                       std::function<void (const string &strFound)> fnMatch,
                       bool fGlobal) const;

    void split(const string &strHaystack, StringVector &sv) const;

    const string& toString() const;

    static string FindInFile(const string &strFilename,
                             const string &strRegex);
};

#endif
