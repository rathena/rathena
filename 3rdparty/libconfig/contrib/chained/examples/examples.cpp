/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   libconfig chained - Extension for reading the configuration and defining 
                       the configuration specification at once.
   Copyright (C) 2016 Richard Schubert

   This file is part of libconfig contributions.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

using namespace std;
using namespace libconfig;

// This is the main entry point for all examples.
// The examples will be called consecutively. 

void example1(Config& cfg);
void example2(Config& cfg);
void example3(Config& cfg);

int main(int argc, char **argv)
{
  Config cfg;

  // Read the file. If there is an error, report it and exit.
  try
  {
    cfg.readFile("../example.cfg");
  }
  catch(const FileIOException&)
  {
    std::cerr << "I/O error while reading file. " << std::endl;
    return(EXIT_FAILURE);
  }
  catch(const ParseException& pex)
  {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    return(EXIT_FAILURE);
  }

  cout << endl << endl << "<<< Example 1 >>>" << endl << endl;
  example1(cfg);
  cout << endl << endl << "<<< Example 2 >>>" << endl << endl;
  example2(cfg);
  cout << endl << endl << "<<< Example 3 >>>" << endl << endl;
  example3(cfg);

  return(EXIT_SUCCESS);
}
