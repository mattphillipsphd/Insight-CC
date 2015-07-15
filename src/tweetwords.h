#ifndef TWEETWORDS_H
#define TWEETWORDS_H

#include "globals.h"

/*
	Class TweetWords
	This class performs all computations regarding calculating word occurrence
	frequency (feature 1).  
	
	The implementation of feature 1 is very straightforward.  Words from the
	current tweet are retrieved using Tweet::UniqueWords and inserted into a
	std::map<std::string,int>, _words, which maps each word onto its frequency count.  
	For potential speed increase, we use multiple threads to read
	in and process tweets.  However testing did not reveal a consistent improvement
	so we have left the default to be single-threaded, with multiple threads 
	specifiable as an argument to the main program.
*/
class TweetWords
{
	public:
		static const int 	AVG_CHARS_PER_TWEET = 60, //According to the internet
							AVG_WORDS_PER_TWEET = 10; //According to OxfordWords, rounding down
		
		TweetWords(const std::string& input_file, const std::string& ft1, int max_threads = -1);
		
		/*
		ReadTweets(): Launches the threads which process the tweets and update the dictionary,
		then it waits until they are all finished.
		Modifies: _countSet
		Calls: ReadTweetsT
		*/
		void ReadTweets();
			
		/*
		InitThreads(): Sets up the threads and associated file locations for parallel
		processing of the input file.
		Modifies: _countSet, _numThreads, _thdStarts
		*/
		long int InitThreads(long int bstart);
		
		long int NumBytes() const { return _numBytes; }
		
		/*
		UniqueCts(): Return a vector containing the number of unique words for every tweet processed.
		*/
		std::vector<uchar> UniqueCts() const;
		
		/*
		Write(): Write the data for feature 1 to file.
		*/
		void Write() const;
		
	private:
		
		/*
		ReadTweetsT(): For a given segment of the input file, this function reads
		in and processes the tweets, updating the dictionary and 
		putting the vector of unique word counts into _countSet.
		Modifies: _countSet
		Calls: UpdateWordCount
		*/
		inline void ReadTweetsT(int tnum, std::streampos start, std::streampos end);
		
		/*
		UpdateWordCount(): Updates count of each word.  Mutex-protected.
		Modifies: _words
		*/
		inline void UpdateWordCount(const std::unordered_map<std::string, int>& words);
	
		//_countSet: A vector of vectors for collecting the results from the multi-threaded
		//read-in.
		std::vector< std::vector<uchar> > _countSet;
		
		//_ft1, _ft2: Strings containing the input and output filenames respectively.
		const std::string _inputFile, _ft1;
		
		std::mutex _mutex;
		
		//_numBytes: Number of bytes in the file
		long int _numBytes;
		
		//_numThreads, _thdStarts: Thread management data members 
		int _numThreads;
		std::vector<std::streampos> _thdStarts;
	
		//_words: Stores the unique words from all tweets and their frequencies.
		std::map<std::string, int> _words;
};

#endif //TWEETWORDS_H