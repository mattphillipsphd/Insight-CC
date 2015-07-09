#ifndef TWEETWORDS_H
#define TWEETWORDS_H

#include "globals.h"
#include "tweet.h"

/*
	Class TweetWords
	This class performs all computations regarding calculating word occurrence
	frequency (feature 1).  Summary::AddTweet
	processes each tweet in turn.
	
	The implementation of feature 1 is very straightforward.  Words from the
	current tweet are retrieved using Tweet::UniqueWords and inserted into a
	std::multiset, _words.  The implementation of std::multiset keeps track
	of the number of repeats so it is an ideal data structure for representing frequency.
	For speed given a resonably modern machine, we use multiple threads to read
	in and process tweets.  Measured against an earlier single-threaded version 
	of this program, multi-threading led to a substantial (4x) speedup on an 8-core
	Windows laptop.  However urprisingly little measurable speedup on a 12-core Ubuntu,
	desktop was seen, although there was still some gain.  Time was measured
	for the execution of the entire program.
*/
class TweetWords
{
	public:
		static const int AVG_WORDS_PER_TWEET = 11; //According to OxfordWords
		
		TweetWords(const std::string& input_file, const std::string& ft1);
		
		/*
			AddTweet(): Processes a tweet.  It calls Tweet::UniqueWords 
			and updates the count for each word, as well as the running 
			median, and appends the running median data to file 
			if ARRAY_MAX has been reached.
			Modifies: 	_freqCts, _tweetCount
			Calls:		UpdateWordCount
		*/
		int AddTweet(const Tweet& tweet);
		
		/*
			ReadTweets(): Launches as many threads as recommended by the OS, then waits until
			they are all finished.
			Modifies: _countSet, _threadsLeft
			Calls: ReadTweetsT
		*/
		void ReadTweets();
		
		ULL NumBytes() const { return _numBytes; }
		
		/*
			Return a vector containing the number of unique words for every tweet processed.
		*/
		std::vector<uchar> UniqueCts() const;
		
		/*
			Write the data for feature 1 to file.
		*/
		void Write() const;
		
	private:
		static const int MS_THREAD_WAIT = 500;
			
		/*
			InitThreads: Sets up the threads and associated file locations for parallel
			processing of the input file.
			Modifies: _countSet, _numThreads, _threadsLeft, _thdStarts
		*/
		inline void InitThreads();
		
		/*
			ReadTweetsT: For a given segment of the input file, this function reads
			in and processes the tweets, putting the vector of unique word counts into _countSet.
			Modifies: _countSet
			Calls: _DecThreadCt
		*/
		inline void ReadTweetsT(int tnum, std::streampos start, std::streampos end);
		
		/*
			UpdateWordCount(): Updates count of each word using the word list
			from Tweet::UniqueWords.  Mutex-protected.
			Modifies: _words
		*/
		inline void UpdateWordCount(const std::multiset<std::string>& words);
	
		//_countSet: A vector of vectors for collecting the results from the multi-threaded
		//read-in.
		std::vector< std::vector<uchar> > _countSet;
		
		//_ft1, _ft2: Strings containing the input and output filenames respectively.
		const std::string _inputFile, _ft1;
		
		std::mutex _mutex;
		
		//_numBytes: Number of bytes in the file
		ULL _numBytes;
		
		//_numThreads, _threadsLeft, _thdStarts: Thread management data members 
		int _numThreads;
		std::atomic<int> _threadsLeft;
		std::vector<std::streampos> _thdStarts;
	
		//_words: Stores the unique words from all tweets and their frequencies,
		//the latter of which are accessed with std::multiset::count.
		std::multiset<std::string> _words;
};

#endif //TWEETWORDS_H