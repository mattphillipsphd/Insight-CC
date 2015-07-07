#include "summary.h"

Summary::Summary(const std::string& ft1, const std::string& ft2) 
	: _ft1(ft1), _ft2(ft2), _groupIdx(-1), _medianIdx(-1), _tweetCount(0)
{
	memset(_freqCts, 0, sizeof(int)*MAX_WORD_CT);
	_runningMedian = new float[ARRAY_MAX];

	std::ofstream out_ft2(_ft2, std::ofstream::out | std::ofstream::trunc);
	out_ft2.close();
}
Summary::~Summary()
{
	delete[] _runningMedian;
}

int Summary::AddTweet(const Tweet& tweet)
{
	//Update data members with information from the new tweet
	int unique_ct;
	std::multiset<std::string>&& words = tweet.Words(unique_ct);
	UpdateWordCount(words);
	return unique_ct;
}

void Summary::UpdateMedian(const std::vector<uchar>& word_vec)
{
	const int vec_size = word_vec.size();
	for (int i=0; i<vec_size; ++i)
	{
		++_tweetCount;
		++_freqCts[ word_vec.at(i) ];
		UpdateMedian( word_vec.at(i) );
	}
}

void Summary::Write() const
{
//	WriteFeature1();
	if (_tweetCount % ARRAY_MAX != 0) WriteFeature2();
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
*/
void Summary::UpdateMedian(int num_words)
{
	const int current_tweet_idx = (_tweetCount - 1) % ARRAY_MAX;
	if (_tweetCount==1)
	{
		_runningMedian[current_tweet_idx] = _medianIdx = num_words;
		_groupIdx = 1;
		return;
	}

	if (num_words > _medianIdx)
	{
		++_groupIdx;
		if (_groupIdx < 2*_freqCts[_medianIdx])
			_runningMedian[current_tweet_idx] = _medianIdx;
		else
			for (int i=_medianIdx+1; i<MAX_WORD_CT; ++i)
				if (_freqCts[i] != 0)
				{
					if (_groupIdx == 2*_freqCts[_medianIdx])
						_runningMedian[current_tweet_idx] = (float)(i+_medianIdx)/2.0;
					else
					{
						_runningMedian[current_tweet_idx] = (float)(i);
						_groupIdx = 1;
						_medianIdx = i;
					}
					break;
				}
	}
	else if (num_words < _medianIdx)
	{
		--_groupIdx;
		if (_groupIdx > 0)
			_runningMedian[current_tweet_idx] = _medianIdx;
		else
		{
			for (int i=_medianIdx-1; i>=0; --i)
				if (_freqCts[i] != 0)
				{
					_runningMedian[current_tweet_idx] = (float)(i+_medianIdx)/2.0;					
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
		if (_tweetCount % 2 == 0)
			_runningMedian[current_tweet_idx] = (float)(num_words+_medianIdx)/2.0;
		else
		{
			_runningMedian[current_tweet_idx] = num_words;
			if (num_words != _medianIdx)
				_groupIdx = _freqCts[num_words];
			_medianIdx = num_words;
		}
	}
}

void Summary::UpdateWordCount(const std::multiset<std::string>& tweet_words)
{
	std::lock_guard<std::mutex> lock(_mutex);
	for (const auto& it : tweet_words)
		_words.insert(it);
}

void Summary::WriteFeature1() const
{
	std::ofstream out(_ft1, std::ofstream::out);
	const int padding = 64;
	auto words_cend = _words.cend();
	for (auto it=_words.cbegin(); it!=words_cend; it=_words.upper_bound(*it))
	{
		const int num_spaces = padding - it->length();
		out << *it << std::setw(num_spaces) << _words.count(*it) << std::endl;
	}
	out.close();
}

void Summary::WriteFeature2() const
{
	std::ofstream out(_ft2, std::ofstream::out | std::ofstream::app);
	const int num_current_tweets = (_tweetCount-1) % ARRAY_MAX + 1;
	for (int i=0; i<num_current_tweets; ++i)
		out << _runningMedian[i] << std::endl;
	out.close();
}
