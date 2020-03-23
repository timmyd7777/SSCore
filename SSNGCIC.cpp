//
//  SSNGCIC.cpp
//  SSCore
//
//  Created by Tim DeBenedictis on 3/23/20.
//  Copyright © 2020 Southern Stars. All rights reserved.
//

#include "SSNGCIC.hpp"

#include <iostream>
#include <fstream>

void importNGCIC ( const char *filename, SSObjectVec &objects )
{
    // Open file; report error and return empty map on failure.

    ifstream file ( filename );
    if ( ! file )
    {
        cout << "Failure: can't open " << filename << endl;
        return;
    }

    // Read file line-by-line until we reach end-of-file

    string line = "";
    int linecount = 0;

    while ( getline ( file, line ) )
    {
		linecount++;
		
        if ( line.length() < 256 )
            continue;

		
	}
}
