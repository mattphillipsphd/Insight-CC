#include "runningmedian.h"

RunningMedian::RunningMedian(const std::string& ft2) 
	: _ft2(ft2), _groupIdx(-1), _medianIdx(-1)
{
	memset(_freqCts, 0, sizeof(int)*MAX_WORD_CT);
	_medianTimes2.reserve(MAX_CHUNK_SIZE / TweetWords::AVG_WORDS_PER_TWEET);
	
	std::ofstream out_ft2(_ft2, std::ofstream::out | std::ofstream::trunc);
	out_ft2.close();
}

void RunningMedian::UpdateMedian(const std::vector<uchar>& word_cts)
{
	const int word_cts_size = word_cts.size();
	for (int i=0; i<word_cts_size; ++i)
	{
		++_freqCts[ word_cts[i] ];
		UpdateMedian( word_cts[i] );
	}
}

void RunningMedian::Write()
{
	std::ofstream out(_ft2, std::ofstream::out | std::ofstream::app);
	const int num_current_tweets = _medianTimes2.size();
	out.precision(2); //As specified in the FAQ
	out << std::fixed;
	for (int i=0; i<num_current_tweets; ++i)
		out << (float)_medianTimes2[i] / 2.0f << std::endl;
	out.close();
	
	_medianTimes2.clear();
}

/*
	The distribution of unique word counts consists of a finite number of groups of 
	integers.  E.g.,
	
	2 2 2 3 3 3 3 4 5 5 6
	          ^
	
	in which case there are 5 groups (corresponding to integers 2-6), and the the median is 3.  
	Each new integer nudges the median to the left or right by *half* of a unit.  If we keep
	track of which group (bin) the median is in, and where in the group the median is located,
	then we can update the median with each new value *without* needing to store the entire
	array of values, re-sorting, or inserting.  For the above example, a length-8 version of 
	_freqCts would equal
	
	{0, 0, 3, 4, 1, 2, 1, 0}
	
	_medianIdx would equal 3, and _groupIdx would correspond to the third position in that bin.

	Then our strategy is, with each new value, to shift _groupIdx up or down depending on
	whether that value is larger than or smaller than _medianIdx.  When _groupIdx hits zero, 
	_medianIdx is decremented and _groupIdx is reset to the 'top' value of the new bin 
	indexed by _medianIdx.  Likewise when _groupIdx 'overflows' its current bin count, 
	_medianIdx is incremented and _groupIdx is set to the 'bottom' value of the new bin.
	
	For the purpose of indexing with _groupIdx, the size of a bin is set to 2 times
	the number of values in that bin--so for bin 3 in our example, its size is 8.  This
	allows us to increment/decrement _groupIdx in integer steps.
	
	Note: What is tracked is the median *index*, not the median itself.  These can be different
	when the input count is even and the median is the average of two bin values.  In this case,
	the median index will point to the lower of these two values, and the algorithm will calculate
	the median as the average.
*/
void RunningMedian::UpdateMedian(int word_ct)
{
	if (_medianIdx == -1)
	{
		_medianIdx = word_ct;
		_medianTimes2.push_back( _medianIdx*2 );
		_groupIdx = 1;
		return;
	}

	if (word_ct > _medianIdx)
	{
		++_groupIdx;
		if (_groupIdx < 2*_freqCts[_medianIdx])
			_medianTimes2.push_back(_medianIdx*2);
		else
			for (int i=_medianIdx+1; i<MAX_WORD_CT; ++i)
				if (_freqCts[i] != 0)
				{
					if (_groupIdx == 2*_freqCts[_medianIdx])
						_medianTimes2.push_back( i+_medianIdx );
					else
					{
						_medianTimes2.push_back( i*2 );
						_groupIdx = 1;
						_medianIdx = i;
					}
					break;
				}
	}
	else if (word_ct < _medianIdx)
	{
		--_groupIdx;
		if (_groupIdx > 0)
			_medianTimes2.push_back(_medianIdx*2);
		else
		{
			for (int i=_medianIdx-1; i>=0; --i)
				if (_freqCts[i] != 0)
				{
					_medianTimes2.push_back( i+_medianIdx );					
					_groupIdx = 2*_freqCts[i];
					_medianIdx = i;					
					break;
				}
		}
	}
	else
	{
		//If the new value is the same as the current median, then we just push
		//_groupIdx closer to the center of that bin.
		++_groupIdx;
		if (_groupIdx % 2 == 0)
			_medianTimes2.push_back( word_ct+_medianIdx );
		else
		{
			_medianTimes2.push_back(word_ct*2);
			if (word_ct != _medianIdx)
				_groupIdx = _freqCts[word_ct];
			_medianIdx = word_ct;
		}
	}
}
