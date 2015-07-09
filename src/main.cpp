#include "runningmedian.h"
#include "tweetwords.h"

int main(int argc, char* argv[])
{
	std::string file_name = (argc<2)
		? "tweet_input/tweets.txt"
		: std::string(argv[1]);
	const int max_threads = (argc<3) ? -1 : std::atoi(argv[2]);
	
	auto start = std::chrono::system_clock::now();
	TweetWords tweet_words(file_name, "tweet_output/ft1.txt", max_threads);
	long int num_bytes = tweet_words.NumBytes();
	const int num_chunks = num_bytes / MAX_CHUNK_SIZE + 1;

	RunningMedian rmed("tweet_output/ft2.txt");
	
	long int bstart = 0;
	for (long int i=0; i<num_chunks; ++i)
	{
		if (num_chunks>1) std::cout << "Starting chunk " << i+1 << " of " << num_chunks << "..." << std::endl;
		
		//Feature 1
		auto t0 = std::chrono::system_clock::now();
		bstart = tweet_words.InitThreads(bstart, i);
		
		tweet_words.ReadTweets();
		auto t1 = std::chrono::system_clock::now();
		std::cout 	<< "Creating dictionary: "
					<<std::chrono::duration_cast<TimeT>(t1-t0).count() << "ms." << std::endl;
					
		//Feature 2
		std::vector<uchar>&& unique_cts = tweet_words.UniqueCts();
		rmed.UpdateMedian(unique_cts);
		auto t2 = std::chrono::system_clock::now();
		std::cout 	<< "Calculating median: "
					<< std::chrono::duration_cast<TimeT>(t2-t1).count() << "ms." << std::endl;
					
		rmed.Write();
		auto t3 = std::chrono::system_clock::now();
		std::cout 	<< "Writing ft2.txt: "
					<< std::chrono::duration_cast<TimeT>(t3-t2).count() << "ms." << std::endl;

		if (num_chunks>1) std::cout << "Finished chunk " << i+1 << " of " << num_chunks << ".\n" << std::endl;
	}
	
	auto t4 = std::chrono::system_clock::now();
	tweet_words.Write();
	auto t5 = std::chrono::system_clock::now();
	std::cout 	<< "Writing ft1.txt: " 
				<< std::chrono::duration_cast<TimeT>(t5-t4).count() << "ms." << std::endl;

	std::cout 	<< "Total time: "
				<< std::chrono::duration_cast<TimeT>(t5-start).count() << "ms." << std::endl;
				
	return 0;
}
