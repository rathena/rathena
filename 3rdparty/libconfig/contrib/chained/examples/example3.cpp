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
// and prints out the expected configuration specification.

void example3(Config& cfg)
{
	ChainedSetting cs(cfg.getRoot());

	Config tmpCfgSpec;
	tmpCfgSpec.setOptions(Config::OptionOpenBraceOnSeparateLine | Config::OptionSemicolonSeparators);
	tmpCfgSpec.setTabWidth(3);
	cs.captureExpectedSpecification(&tmpCfgSpec);

	string name = cs["name"].defaultValue("<name>").isMandatory();
	string abstract = cs["abstract"].defaultValue("<unknown>");
	double longitude = cs["longitude"].min(-180.0).max(180.0).isMandatory();
	double latitude = cs["latitude"].min(-90.0).max(90.0).isMandatory();

	ChainedSetting movie0 = cs["inventory"]["movies"][0];
	string book0tile = cs["inventory"]["books"][0]["title"].defaultValue("bookXYZ");

	if (cs.isAnyMandatorySettingMissing())
	{
		cerr << "Cannot proceed until all mandatory settings are set." << endl << endl;

		auto cfgSpec = cs.getCapturedSpecification("tmpCfgSpec.cfg");
		cerr << "Expected Config Layout: " << endl << endl;
		cerr << "// -- begin --" << endl;
		cerr << cfgSpec;
		cerr << "// --- end ---" << endl << endl;
		return;
	}

	// from here on all read config values are valid
}
