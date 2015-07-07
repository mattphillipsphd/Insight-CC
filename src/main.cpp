#include "runningmedian.h"
#include "tweetwords.h"

int main(int argc, char* argv[])
{
	std::string file_name = (argc<2)
		? "tweet_input/tweets.txt"
		: std::string(argv[1]);
	
	TweetWords tweet_words(file_name, "tweet_output/ft1.txt");
	tweet_words.ReadTweets();
	tweet_words.Write();
	std::cout << "Data written to ft1.txt" << std::endl;

	std::vector<uchar>&& unique_cts = tweet_words.UniqueCts();
	RunningMedian rmed("tweet_output/ft2.txt", tweet_words.NumBytes());
	rmed.UpdateMedian(unique_cts);
	rmed.Write();
	std::cout << "Data written to ft2.txt" << std::endl;

	return 0;
}
