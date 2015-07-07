#ifndef RUNNING_MEDIAN_H
#define RUNNING_MEDIAN_H

#include "globals.h"
#include "tweetwords.h"

/*
	Class RunningMedian
	This class performs all computations regarding calculating the running median (feature 2).  
	
	For feature 2, we exploit the fact that the number of possible unique words 
	in a tweet is sharply bounded (70), and is always a whole number.  This allows
	us to keep track of frequency counts with a small integer array (_freqCts) and
	use the current median index (_medianIdx), along with a number representing how 
	close the distribution is to the 'tipping point' from one median value to the next 
	(_groupIdx), to	update the median in constant time.
*/
class RunningMedian
{
	public:
		/* 	
			MAX_TWEET_LEN, MAX_WORD_CT: Constants for array initialization.  
			MAX_TWEET_LEN is self-explanatory, MAX_WORD_CT
			is the maximum possible words in a tweet (+1 for indexing reasons).
		*/
		static const int 	MAX_TWEET_LEN = 140,
							MAX_WORD_CT = MAX_TWEET_LEN/2 + 1;
							
		RunningMedian(const std::string& ft2, int num_bytes);
		
		/*
			UpdateMedian(): Calls UpdateMedian(...) for each element in word_cts.
			Modifies: _freqCts
		*/
		void UpdateMedian(const std::vector<uchar>& word_cts);
		
		/*
			Write(): Writes the data for feature 2 to file.
		*/
		void Write() const;
		
	private:
		/*
			UpdateMedian(): Updates the running median with the new value from
			Tweet::UniqueWords.
			Modifies: _medianTimes2, _groupIdx, _medianIdx
		*/
		inline void UpdateMedian(int word_ct);

		//_freqCts: The frequency of word counts.  Since word counts are sharply
		//bounded by the Twitter API and, crucially, always take an
		//integer value, we can represent them in an integer array without
		//loss of information.
		int _freqCts[MAX_WORD_CT];
		
		//_ft2: Contains the output filenames.
		const std::string _ft2;
			
		//_groupIdx, _medianIdx: Indexes for keeping track of the state of the running median
		//_medianIdx indicates the last bin (group) whence the median was calculated, taking the
		//smaller if a median is the average of two values.  _groupIdx specifies the location of the 
		//median within the bin.  The height of a bin is set to 2 times the # of occurrences of that
		//value, so that integer calculations can be used.
		int _groupIdx, _medianIdx;
		
		//_medianTimes2: A vector storing running median values, multiplied by
		//2 so we can store in a single unsigned char (1 byte) instead of a float (4 bytes).
		//Benchmarking indicates no effect on performance by doing this.  We divide by 2
		//before we write the medians to file. 
		std::vector<uchar> _medianTimes2; 
};

#endif //RUNNING_MEDIAN_H