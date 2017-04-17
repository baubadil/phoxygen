/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef XWP_EXEC_H
#define XWP_EXEC_H

#include <string>

namespace XWP
{

std::string exec(const std::string &str);

std::string getExecutableFileName(const char *argv0);

}

#endif // XWP_EXEC_H
