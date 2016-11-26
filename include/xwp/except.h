/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 * 
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef XWP_EXCEPT_H
#define XWP_EXCEPT_H

#include <exception>
#include <string>

using namespace std;

namespace XWP
{

/***************************************************************************
 *
 *  FSException
 *
 **************************************************************************/

class FSException : public exception
{
    string _str;
public:
    FSException(const string &str);
    virtual const char* what() const throw();
};

} // namespace XWP

#endif // XWP_EXCEPT_H
