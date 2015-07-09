#include "runningmedian.h"
#include "tweetwords.h"

int main(int argc, char* argv[])
{
	std::string file_name = (argc<2)
		? "tweet_input/tweets.txt"
		: std::string(argv[1]);
	const int max_threads = (argc<3) ? -1 : std::atoi(argv[2]);
	
	//Feature 1
	auto start = std::chrono::system_clock::now();
	TweetWords tweet_words(file_name, "tweet_output/ft1.txt", max_threads);
	tweet_words.ReadTweets();
	auto t1 = std::chrono::system_clock::now();
	std::cout 	<< "Creating dictionary: "
				<<std::chrono::duration_cast<TimeT>(t1-start).count() << "ms." << std::endl;
				
	tweet_words.Write();
	auto t2 = std::chrono::system_clock::now();
	std::cout 	<< "Writing ft1.txt: " 
				<< std::chrono::duration_cast<TimeT>(t2-t1).count() << "ms." << std::endl;

	//Feature 2
	std::vector<uchar>&& unique_cts = tweet_words.UniqueCts();
	RunningMedian rmed("tweet_output/ft2.txt", tweet_words.NumBytes());
	rmed.UpdateMedian(unique_cts);
	auto t3 = std::chrono::system_clock::now();
	std::cout 	<< "Calculating median: "
				<< std::chrono::duration_cast<TimeT>(t3-t2).count() << "ms." << std::endl;
				
	rmed.Write();
	auto t4 = std::chrono::system_clock::now();
	std::cout 	<< "Writing ft2.txt: "
				<< std::chrono::duration_cast<TimeT>(t4-t3).count() << "ms." << std::endl;

	return 0;
}
