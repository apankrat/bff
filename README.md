    bff 1.0.7
    =========

    	Moderately-optimizing brainfuck interpreter

    	http://swapped.cc/bff

    =
    	To build with GNU tools run
    		make

    	To install in /usr/bin (not sure why you'd be wanting this though)
    		make install

    	To clean up the build files
    		make clean

    =
    	1.0.7 - Aug 09, 2018
    		fixed an issue in preprocessing stage that caused it 
    		to ingest more bf code than there was if the bf code
    		started with <<... or >>... sequence. This lead to a
    		heap corruption with variable consequences. kudos to
    		Alex Stefanov for reporting this.

    	1.0.6 - Apr 27, 2015
    		added special handling for [-] and [+]

    	1.0.5 - Dec 10, 2012
    		fixed grow() as per Mitch Schwartz note

    	1.0.4 - Aug 01, 2011
    		fixed xalloc() to zero newly allocated blocks

    	1.0.3.1 - Aug 02, 2004
    		fixed a bug where programs starting with < or > command
    		were not handled properly

    	1.0.3 - Jul 29, 2004
    		further optimized the main loop, now runs twice as fast

    	1.0.2 - Apr 27, 2004
    		made it compilable under MSVC
    		added -Wall to Makefile
    		added missing string.h include

    	1.0.1 - Apr 27, 2004
    		the cell array is now dynamically grown when exhausted;
    		the input may be read from the file

    	1.0.0 - Apr 26, 2004
    		an initial public release

    ==========================================================================
    Copyright (c) 2004-12, Alex Pankratov (ap@swapped.cc). All rights reserved.

