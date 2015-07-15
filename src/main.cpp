#include "runningmedian.h"
#include "tweetwords.h"

/*
	This program differs from the 'naive' implementation of the two features in three ways:
	
	1) It breaks the input file into 2GB chunks and flushes the buffer containing the running
		median to file after the completion of each chunk.  Since the running median is stored
		very economically (one byte/value) this means that the memory consumption should be no more
		than about 50MB at any point, and that the program can handle the largest possible input file.
		The size of the file chunk is controlled with MAX_CHUNK_SIZE in globals.h.
		
	2) The running median is calculated by updating a few indexes (see runningmedian.cpp for details)
		and so has complexity O(N) with input length, as opposed to O(N*log(N)) or worse for a naive 
		implementation.
		
	3) The calculation of words tweeted (feature 1) is in principle implemented in a multi-threaded
		way.  However testing indicated that multiple threads did not improve performance so the
		number of threads has been set to 1 in run.sh.  This a command-line argument which defaults
		to the number of cores.
		
	In addition, this program makes substantial use of C++11 language features and data structures
	(move semantics, unordered_map, et al.) for maximum efficiency.  On my ASUS laptop (c. 2013) it
	processes 1M tweets in less than a minute.
	
	Classes:
	TweetWords: 	Implements feature 1.
	RunningMedian: 	Implements feature 2.   
*/

int main(int argc, char* argv[])
{
	std::string file_name = (argc<2)
		? "tweet_input/tweets.txt"
		: std::string(argv[1]);
	const int max_threads = (argc<3) ? -1 : std::atoi(argv[2]);
	
	const std::string 	ft1 = "tweet_output/ft1.txt",
						ft2 = "tweet_output/ft2.txt";
	
	TweetWords tweet_words(file_name, ft1, max_threads);
	long int num_bytes = tweet_words.NumBytes();
	const int num_chunks = num_bytes / MAX_CHUNK_SIZE + 1;
		//The number of file chunks is determined by file size in bytes and the MAX_CHUNK_SIZE value.

	RunningMedian rmed(ft2);
	
	long int bstart = 0;
	for (long int i=0; i<num_chunks; ++i)
	{
		if (num_chunks>1) std::cout << "Starting chunk " << i+1 << " of " << num_chunks << "..." << std::endl;
		
		//Feature 1
		bstart = tweet_words.InitThreads(bstart);
			//We determine the start and end point within the file for each chunk, and for the
			//threads within the chunk.
		
		tweet_words.ReadTweets(); //This creates the dictionary.
					
		//Feature 2
		std::vector<uchar>&& unique_cts = tweet_words.UniqueCts();
		rmed.UpdateMedian(unique_cts);
			//We get the unique word counts from tweet_words and calculate the running median.
					
		rmed.Write();
			//Write feature 2 to disk.

		if (num_chunks>1) std::cout << "Finished chunk " << i+1 << " of " << num_chunks << ".\n" << std::endl;
	}
	
	tweet_words.Write();
		//Write feature 1 to disk.
	return 0;
}
