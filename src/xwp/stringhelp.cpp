/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#include "xwp/stringhelp.h"
#include "xwp/regex.h"

#include <functional>
#include <algorithm>

namespace XWP
{

// trim from start (in place)
void ltrim(string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
void rtrim(string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
void trim(string &s)
{
    ltrim(s);
    rtrim(s);
}

string trimmed(const string &s)
{
    string s2(s);
    trim(s2);
    return s2;
}


void ForEachSubstring(const string &str,
                      const string &strDelimiter,
                      std::function<void (const string&)> fnParticle)
{
    size_t p1 = 0;
    size_t p2;
    while ((p2 = str.find(strDelimiter, p1)) != string::npos)
    {
        int len = p2 - p1;
        if (len > 0)
            fnParticle(str.substr(p1, len));
        p1 = p2 + 1;
    }

    fnParticle(str.substr(p1));
}

StringSet explodeSet(const string &str,
                     const string &strDelimiter,
                     bool fTrim, /* = false */
                     bool fKeepEmpty /* = false */)
{
    set<string> s;

    ForEachSubstring(str,
                     strDelimiter,
                     [&s, fTrim, fKeepEmpty](const string &strParticle)
                     {
                         if (fKeepEmpty || (!strParticle.empty()))
                         {
                             if (fTrim)
                                 s.insert(trimmed(strParticle));
                             else
                                 s.insert(strParticle);
                         }
                     });

    return s;
}

StringVector explodeVector(const string &str,
                           const string &strDelimiter,
                           bool fTrim, /* = false */
                           bool fKeepEmpty /* = false */)
{
    vector<string> v;

    ForEachSubstring(str,
                     strDelimiter,
                     [&v, fTrim, fKeepEmpty](const string &strParticle)
                     {
                         if (fKeepEmpty || (!strParticle.empty()))
                         {
                             if (fTrim)
                                 v.push_back(trimmed(strParticle));
                             else
                                 v.push_back(strParticle);
                         }
                     });

    return v;
}

string implode(const string &strGlue, const StringSet &sset)
{
    string str;
    for (const auto &s : sset)
        if (!s.empty())
        {
            if (!str.empty())
                str += strGlue + s;
            else
                str += s;
        }

    return str;
}

string implode(const string &strGlue, const StringVector &v)
{
    string str;
    for (const auto &s : v)
        if (!s.empty())
        {
            if (!str.empty())
                str += strGlue + s;
            else
                str += s;
        }

    return str;
}

string implode(const string &strGlue, const StringRefVector &v)
{
    string str;
    for (const auto &s : v)
        if (!s.get().empty())
        {
            if (!str.empty())
                str += strGlue + s.get();
            else
                str += s;
        }

    return str;
}

string toHTML(const string &str)
{
    string strCopy = str;
    stringReplace(strCopy, "&", "&amp;");
    stringReplace(strCopy, "<", "&lt;");
    stringReplace(strCopy, ">", "&gt;");

    // Permit <B> and <I> HTML tags.
    static const Regex re("&lt;(/?[bi])&gt;");
    re.findReplace(strCopy, "<$1>", true);

    return strCopy;
}

void stringReplace(string &subject,
                   const string &search,
                   const string &replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != string::npos)
    {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

string strToLower(const string &str0)
{
    string str = str0;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

string strToUpper(const string &str0)
{
    string str = str0;
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

bool startsWith(const string &strHaystack,
                const string &strStart)
{
    if (strHaystack.length() >= strStart.length())
        return (0 == strHaystack.compare(0,
                                         strStart.length(),
                                         strStart));

    return false;
}

bool endsWith(const string &strHaystack,
              const string &strEnd)
{
    if (strHaystack.length() >= strEnd.length())
        return (0 == strHaystack.compare(strHaystack.length() - strEnd.length(),
                                         strEnd.length(),
                                         strEnd));

    return false;
}

string makePath(const string &str1,
                const string &str2)
{
    string s = str1;
    if (    (!s.empty())
         && (!endsWith(s, "/"))
       )
        s += "/";
    return s + str2;
}

} // namespace XWP
