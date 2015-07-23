#include "runningmedian.h"
#include "tweetwords.h"

/*
	See README.md for program overview.
	
	Classes:
	TweetWords: 	Implements feature 1.
	RunningMedian: 	Implements feature 2.   
*/

int main(int argc, char* argv[])
{
	const int user_max_threads = (argc<2)
		? MAX_NUM_THREADS
		: atoi(argv[1]);
	
	const std::string input_file = "tweet_input/tweets.txt";
	const std::string 	ft1 = "tweet_output/ft1.txt",
						ft2 = "tweet_output/ft2.txt";
	
	TweetWords tweet_words(input_file, ft1, user_max_threads);
	long int num_bytes = tweet_words.NumBytes();
	const int num_chunks = num_bytes / MAX_CHUNK_SIZE + 1;
		//The number of file chunks is determined by file size in bytes and the MAX_CHUNK_SIZE value.

	RunningMedian rmed(ft2);
	
	long int start_byte = 0;
	for (long int i=0; i<num_chunks; ++i)
	{
		if (num_chunks>1) std::cout << "Starting chunk " << i+1 << " of " << num_chunks << "..." << std::endl;
		
		//Feature 1
		start_byte = tweet_words.InitChunk(start_byte);
			//We determine the start and end point within the file for each chunk.
		
		tweet_words.ReadTweets(); //This creates the dictionary.
					
		//Feature 2
		rmed.UpdateMedian( tweet_words.UniqueCts() );
			//We get the unique word counts from tweet_words and calculate the running median.
					
		rmed.Write();
			//Write feature 2 to disk.

		if (num_chunks>1) std::cout << "Finished chunk " << i+1 << " of " << num_chunks << ".\n" << std::endl;
	}
	
	tweet_words.Write();
		//Write feature 1 to disk.
		
	std::cout << "Program completed.  Output written to " << ft1 << " and " << ft2 << std::endl;
	
	return 0;
}
