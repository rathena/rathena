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

// This example reads complex types from config file using method chains.

void example2(Config& cfg)
{
	ChainedSetting cs(cfg.getRoot());

	auto books = cs["inventory"]["books"];
	if (!books.exists())
	{
		cerr << "No book section available." << endl;
		return;
	}

	cout << setw(30) << left << "TITLE" << "  "
			<< setw(30) << left << "AUTHOR" << "   "
			<< setw(6) << left << "PRICE" << "  "
			<< "QTY"
			<< endl;

	const int count = books.getLength();
	for(int i = 0; i < count; ++i)
	{
		auto book = books[i];

		string title = book["title"];
		string author = book["author"];
		double price = book["price"].min(0.0);
		int qty = book["qty"].min(0);

		// Only output the record if all of the expected fields are present.
		if(book.isAnySettingMissing()) continue;

		cout << setw(30) << left << title << "  "
			<< setw(30) << left << author << "  "
			<< '$' << setw(6) << right << price << "  "
			<< qty
			<< endl;
	}
}
