Insight Coding challenge
Matthew Phillips
mattphillipsphd@yahoo.com

Requires g++, 4.7 or higher.  If this is run from the Windows command prompt, run.sh must be modified
by removing the '-pthread' linker option.

	This program differs from the 'naive' implementation of the two features in three ways:
	
	1) It breaks the input file into 2GB chunks and flushes the buffer containing the running
		median to file after the completion of each chunk.  Since the running median is stored
		very economically (one byte/value) this means that the memory consumption should be no more
		than about 50MB at any point, and that the program can handle the largest possible input file.
		The size of the file chunk is controlled with MAX_CHUNK_SIZE in globals.h.
		
	2) The running median is calculated by updating a few indexes (see runningmedian.cpp for details)
		and so has complexity O(N) with input length as opposed to O(N*log(N)) or worse for a naive 
		implementation.
		
	3) The calculation of words tweeted (feature 1) is implemented in a multithreaded way on
		Linux-based systems.  With even 4 cores used this cut processing time by over 50% 
		relative to a single-threaded implementation.
		
	In addition, this program makes substantial use of C++11 language features and data structures
	(std::thread, unordered_map, et al.) for maximum efficiency.  On my ASUS laptop (c. 2013) it
	processes 1M tweets in less than a minute; on a Linux machine using 4 or more cores it process 1M
	tweets in about 15sec.

	Tradeoffs: I had to make some judgment calls which felt ultimately subjective in nature so I will
	flag them here.
	
	1) Multi-threading.  Due to lack of support and performance differences on Windows platforms 
		I could not write timely, 'pure' (no #ifdefs) multi-threaded cross-platform code.
		However the performance gain on Linux (see #3 above) was substantial, so I decided to
		trade a bit of simplicity for the performance increase.  This also comes at cost of increased
		CPU usage but I thought it was worth it and it is in any case under user control (command-line argument).
		
	2) Calculating the median.  It was possible to refine the algorithm I used to achieve even
		greater efficiency (see runningmedian.cpp for details); but since calculating the median
		is such a tiny fraction of the overall runtime, in this case I kept the code simple and
		took the miniscule performance hit.
		
	3) File chunking.  The capacity of a vector<char> is large so I could have avoided this
		and just taken my chances that the test files would be small enough relative to available
		free memory, but scalability is critical here so I decided file chunking was necessary.