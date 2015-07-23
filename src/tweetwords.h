#ifndef TWEETWORDS_H
#define TWEETWORDS_H

#include "globals.h"

/*
	Class TweetWords
	This class performs all computations regarding calculating word occurrence
	frequency (feature 1).  
	
	For Windows systems, the implementation is straightforward; the input file is read
	line by line, each tweet is extracted and tokenized using C strtok and the individual
	words are fed into the dictionary (a std::map from strings to frequency counts), which
	is subsequently written to disk.
		For UNIX-based systems, I was able to achieve substantially faster (2x) performance
	by creating multiple threads and creating a dictionary for each thread which only comprises
	a fraction of the ASCII range.  Each thread reads the *entire* file, but only updates its
	dictionary when a word in its alphabetical range is read.  This method increases CPU usage
	and code complexity but avoids synchronization issues as would occur if different threads
	updated a single dictionary but were dedicated to different thread segments.
		On all platforms, the unique word count for each tweet is fed into a std::vector which
	is used by a RunningMedian object to compute the running median.
*/
class TweetWords
{
	public:
		typedef std::map<std::string,int>		Dictionary;

		static const int 	AVG_CHARS_PER_TWEET = 60, //According to the internet
							AVG_WORDS_PER_TWEET = 10; //According to OxfordWords, rounding down
		
		TweetWords(const std::string& input_file, const std::string& ft1, int user_max_threads);
		
		/*
		ReadTweets(): Launches the threads which process the tweets and update the dictionary,
		then it waits until they are all finished.
		Modifies: _countSet
		Calls: ReadTweetsT
		*/
		void ReadTweets();
			
		/*
		InitChunk(): Sets up the threads and associated file locations for parallel
		processing of the input file.
		Modifies: _chunkBegin, _chunkEnd
		*/
		long int InitChunk(long int start_byte);
		
		long int NumBytes() const { return _numBytes; }
		
		/*
		UniqueCts(): Return the vector containing the number of unique words for every tweet processed.
		*/
		const std::vector<uchar>& UniqueCts() const { return _counts; }
		
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
		inline void ReadTweetsT(int tnum);
			
		//_countSet: A vector of vectors for collecting the results from the multi-threaded
		//read-in.
		std::vector<uchar> _counts;
		
		//_ft1, _ft2: Strings containing the input and output filenames respectively.
		const std::string _inputFile, _ft1;
		
		//_numBytes: Number of bytes in the file
		long int _numBytes;
		
		//_numThreads: Number of threads used for reading the file. 
		int _numThreads;
		
		//chunkStart, _chunkEnd: The beginning and end within the file for the current chunk
		long int _chunkStart, _chunkEnd;
	
		//_words: Stores the unique words from all tweets and their frequencies.
		std::vector<Dictionary> _dicts;
};

#endif //TWEETWORDS_H