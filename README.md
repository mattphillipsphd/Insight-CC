Insight Coding challenge
Matthew Phillips
mattphillipsphd@yahoo.com

	This program differs from the 'naive' implementation of the two features in three ways:
	
	1) It breaks the input file into 2GB chunks and flushes the buffer containing the running
		median to file after the completion of each chunk.  Since the running median is stored
		very economically (one byte/value) this means that the memory consumption should be no more
		than about 50MB at any point, and that the program can handle the largest possible input file.
		The size of the file chunk is controlled with MAX_CHUNK_SIZE in globals.h.
		
	2) The running median is calculated by updating a few indexes (see runningmedian.cpp for details)
		and so has complexity O(N) with input length as opposed to O(N*log(N)) or worse for a naive 
		implementation.
		
	3) The calculation of words tweeted (feature 1) is in principle implemented in a multi-threaded
		way.  However testing indicated that multiple threads did not improve performance so the
		number of threads has been set to 1 in run.sh.  This a command-line argument which defaults
		to the number of cores.
		
	In addition, this program makes substantial use of C++11 language features and data structures
	(move semantics, multisets, et al.) for maximum efficiency.  The most time is spent writing 
	the results to disk.  I experimented with file buffer size but could not improve on C++ defaults.

	tweet_input/tweets.txt contains an example of the sort of input file I used to test the program.