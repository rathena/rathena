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
#include "../libconfig_chained.h"

using namespace std;
using namespace libconfig;

// This example reads basic information from config file 
// and reacts on missing mandatory values.

void example1(Config& cfg)
{
	ChainedSetting cs(cfg.getRoot());

	string name = cs["name"].defaultValue("<name>").isMandatory();
	string abstract = cs["abstract"].defaultValue("<unknown>");
	double longitude = cs["longitude"].min(-180.0).max(180.0).isMandatory();
	double latitude = cs["latitude"].min(-90.0).max(90.0).isMandatory();

	if (cs.isAnyMandatorySettingMissing())
	{
		cerr << "Cannot proceed until all mandatory settings are set." << endl;
		return;
	}

	// from here on all read config values are valid
}
