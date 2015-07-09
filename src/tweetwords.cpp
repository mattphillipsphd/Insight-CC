#include "tweetwords.h"

TweetWords::TweetWords(const std::string& input_file, const std::string& ft1, int max_threads)
	: _inputFile(input_file), _ft1(ft1)
{
	std::ifstream ifs(_inputFile, std::ios::binary | std::ios::ate);
	if (!ifs.good())
	{
		std::cerr << "Bad file name, \"" << _inputFile << "\"" << std::endl;
		exit(-1);
	}
	_numBytes = ifs.tellg();
	ifs.close();
	
	_numThreads = (_numBytes > 4096 && std::thread::hardware_concurrency()>0)
					? std::thread::hardware_concurrency()
					: 1;
	if (max_threads>0) _numThreads = std::min(_numThreads, max_threads);
	std::cout << "Using " << _numThreads << " thread(s) to create dictionary." << std::endl;
}

int TweetWords::AddTweet(const Tweet& tweet)
{
	int unique_ct;
	std::multiset<std::string>&& words = tweet.Words(unique_ct);
	UpdateWordCount(words);
	return unique_ct;
}

//We divide the current file chunk into as many contiguous pieces as there are threads,
//and then prepare each thread to read just from its segment.  The results
//are fed into the dictionary, _words, using the mutex-protected function UpdateWordCount.
//The unique word counts are collected in _countSet and merged when UniqueCts is called.
long int TweetWords::InitThreads(long int bstart)
{
	_thdStarts = std::vector<std::streampos>(_numThreads+1);
	_thdStarts[0] = bstart;
	
	const long int last_byte = std::min(bstart+MAX_CHUNK_SIZE, _numBytes) - 1, 
					bytes_per_thread = (last_byte - bstart) / (long int)_numThreads;
	std::ifstream in(_inputFile.c_str(), std::ifstream::in);
	for (long int i=1; i<_numThreads+1; ++i)
	{
		std::streampos offset = bstart + i*bytes_per_thread;
		in.seekg(offset, std::ios_base::beg);
		
		//We cannot break the file mid-tweet.  So we take offset as a 'hint' and search
		//for the first newline.  This will be the end of this piece and will determine
		//the start of the next one.
		char c;
		do 
			in.get(c);
		while ( c != '\n' && c != EOF) ;
		_thdStarts[i] = in.tellg();
	}
	in.close();
		
	//We return the end point of the last file segment, which will be the start of the 
	//next file chunk if there are multiple chunks.
	return (long int)_thdStarts.at(_numThreads);
}

void TweetWords::ReadTweets()
{
	_countSet = std::vector< std::vector<uchar> >(_numThreads);

	std::vector<std::thread> tvec;
	for (int i=0; i<_numThreads; ++i)
		tvec.emplace_back( 
							&TweetWords::ReadTweetsT, 
							this, i, _thdStarts.at(i), _thdStarts.at(i+1) - std::streampos(1)
						);
	
	for (auto& it : tvec)
		it.join();
}

std::vector<uchar> TweetWords::UniqueCts() const
{
	int num_tweets = 0;
	for (const auto& it : _countSet) num_tweets += it.size();
	std::vector<uchar> unique_cts;
	unique_cts.reserve(num_tweets);
	
	for (const auto& it : _countSet)
		unique_cts.insert(unique_cts.end(), it.cbegin(), it.cend());
	
	return unique_cts;
}

void TweetWords::Write() const
{
	std::ofstream out(_ft1, std::ofstream::out);
	
	const int padding = 64;
	auto words_cend = _words.cend();
	for (auto it=_words.cbegin(); it!=words_cend; it=_words.upper_bound(*it))
	{
		const int num_spaces = padding - it->length();
		out << *it << " " << std::setw(num_spaces) << _words.count(*it) << std::endl;
	}
	out.close();
}

//We move to the start of the file segment assigned to this thread, and read
//tweets until we reach the end.  Word counts are deposited in _words and _countSet.
void TweetWords::ReadTweetsT(int tnum, std::streampos start, std::streampos end)
{
	std::ifstream in(_inputFile.c_str(), std::ifstream::in);
	in.seekg(start, std::ios_base::beg);
	std::vector<uchar> cts;
	cts.reserve( (int)(end - start) / AVG_WORDS_PER_TWEET );
	std::string line;
	while (in.tellg() < end && !in.eof())
	{
		std::getline(in, line);
		int ct = AddTweet( Tweet( std::move(line) ) );
		cts.push_back(ct);
	}
	in.close();
	_countSet[tnum] = cts;
}

void TweetWords::UpdateWordCount(const std::multiset<std::string>& tweet_words)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_words.insert(tweet_words.cbegin(), tweet_words.cend());
}

