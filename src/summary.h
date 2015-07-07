#ifndef SUMMARY_H
#define SUMMARY_H

#include "globals.h"
#include "tweet.h"

/*
	Class Summary
	This class performs all computations regarding calculating word occurrence
	frequency (feature 1) and the running median (feature 2).  Summary::AddTweet
	processes each tweet in turn.
	
	The implementation of feature 1 is very straightforward.  Words from the
	current tweet are retrieved using Tweet::UniqueWords and inserted into a
	std::multiset, _words.  The implementation of std::multiset keeps track
	of the number of repeats so it is an ideal data structure for representing frequency.
	
	For feature 2, we exploit the fact that the number of possible unique words 
	in a tweet is sharply bounded (70), and is always a whole number.  This allows
	us to keep track of frequency counts with a small integer array (_freqCts) and
	use the current median index (_medianIdx), along with a number representing how 
	close the distribution is to the 'tipping point' from one median value to the next 
	(_groupIdx), to	update the median in constant time.
*/
class Summary
{
	public: 
		/* 	
			ARRAY_MAX, MAX_TWEET_LEN, MAX_WORD_CT: Constants for array initialization.  
			ARRAY_MAX is the number of tweets processed before the running median results 
			are appended to file, and flushed. Setting it to a value considerably 
			smaller than SIZE_MAX/sizeof(float) ensures that we maintain a 
			reasonably small memory footprint, while having a negligible
			effect on performance.  MAX_TWEET_LEN is self-explanatory, MAX_WORD_CT
			is the maximum possible words in a tweet (+1 for indexing reasons).
		*/
		static const int 	ARRAY_MAX = 64 * 1024 *1024,
							MAX_TWEET_LEN = 140,
							MAX_WORD_CT = MAX_TWEET_LEN/2 + 1;
	
		/*
			Summary(), ~Summary(): Perform basic initialization, resource allocation
			and freeing.
		*/
		Summary(const std::string& ft1, const std::string& ft2);
		~Summary();
		
		/*
			AddTweet(): Processes a tweet.  It calls Tweet::UniqueWords 
			and updates the count for each word, as well as the running 
			median, and appends the running median data to file 
			if ARRAY_MAX has been reached.
			Modifies: 	_freqCts, _tweetCount
			Calls:		UpdateWordCount, UpdateMedian, WriteFeature2
		*/
		int AddTweet(const Tweet& tweet);
		
		void UpdateMedian(const std::vector<uchar>& word_vec);
		
		/*
			Write(): Calls WriteFeature1() and WriteFeature2(), which write
			the data for ft1 and ft2 to file, respectively.
		*/
		void Write() const;
		
	private:
		/*
			UpdateMedian(): Updates the running median with the new value from
			Tweet::UniqueWords.
			Modifies: _runningMedian, _groupIdx, _medianIdx
		*/
		inline void UpdateMedian(int num_words);
		
		/*
			UpdateWordCount(): Updates count of each word using the word list
			from Tweet::UniqueWords.
			Modifies: _words
		*/
		inline void UpdateWordCount(const std::multiset<std::string>& words);
		
		/*
			WriteFeature1(): Self-explanatory.
		*/
		inline void WriteFeature1() const;
		
		/*
			WriteFeature2(): Appends median data to file, up to ARRAY_MAX elements. 
		*/
		inline void WriteFeature2() const;

		//_freqCts: The frequency of word counts.  Since word counts are sharply
		//bounded by the Twitter API and, crucially, always take an
		//integer value, we can represent them in an integer array without
		//loss of information.
		int _freqCts[MAX_WORD_CT];
		
		//_ft1, _ft2: Strings containing the output filenames.
		const std::string _ft1, _ft2;
			
		//_groupIdx, _medianIdx: Indexes for keeping track of the state of the running median
		//_medianIdx indicates the last bin (group) whence the median was calculated, taking the
		//smaller if a median is the average of two values.  _groupIdx specifies the location of the 
		//median within the bin.  The height of a bin is set to 2 times the # of occurrences of that
		//value, so that integer calculations can be used.
		int _groupIdx, _medianIdx;
		
		std::mutex _mutex;
		
		//_runningMedian: An array storing up to ARRAY_MAX values.  We could cut
		//memory consumption further by storing median*2 in a unsigned char array
		//but that would make the code less transparent.
		float* _runningMedian; 
			
		//_tweetCount: The number of tweets that have been processed
		size_t _tweetCount;
			
		//_words: Stores the unique words from all tweets and their frequencies,
		//the latter of which are accessed with std::multiset::count.
		std::multiset<std::string> _words;
};

#endif //SUMMARY_H