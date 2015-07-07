#include "tweetwords.h"

TweetWords::TweetWords(const std::string& input_file, const std::string& ft1)
	: _inputFile(input_file), _ft1(ft1)
{
	std::ifstream ifs(_inputFile, std::ios::binary | std::ios::ate);
	if (!ifs.good())
	{
		std::cerr << "Bad file name, \"" << _inputFile << "\"" << std::endl;
		exit(-1);
	}
	_numBytes = (ULL)ifs.tellg();
	ifs.close();
		
	InitThreads();
}

int TweetWords::AddTweet(const Tweet& tweet)
{
	int unique_ct;
	std::multiset<std::string>&& words = tweet.Words(unique_ct);
	UpdateWordCount(words);
	return unique_ct;
}

void TweetWords::ReadTweets()
{
	_threadsLeft = _numThreads;
	_countSet = std::vector< std::vector<uchar> >(_numThreads);

	for (int i=0; i<_numThreads; ++i)
	{
		std::thread t(&TweetWords::ReadTweetsT, 
									this, i, _thdStarts.at(i), _thdStarts.at(i+1) - std::streampos(1));
		t.detach();
	}
	
	std::chrono::duration<double, std::milli> wait(MS_THREAD_WAIT);
	while (_threadsLeft > 0)
		std::this_thread::sleep_for(wait);
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
		out << *it << std::setw(num_spaces) << _words.count(*it) << std::endl;
	}
	out.close();
}

void TweetWords::DecThreadCt()
{
	std::lock_guard<std::mutex> lock(_mutex);
	--_threadsLeft;
}

//We divide the file length into as many contiguous pieces as there are threads,
//and then prepare each thread to read just from its segment.  The results
//are collected in _countSet
void TweetWords::InitThreads()
{
	_numThreads = (_numBytes > 4096 && std::thread::hardware_concurrency()>0)
					? std::thread::hardware_concurrency()
					: 1;
	
	_thdStarts = std::vector<std::streampos>(_numThreads+1);
	_thdStarts[0] = 0;
	_thdStarts[_numThreads] = _numBytes;
	
	const int bytes_per_thread = _numBytes / _numThreads;
	std::ifstream in(_inputFile.c_str(), std::ifstream::in);
	for (int i=1; i<_numThreads; ++i)
	{
		std::streampos offset = i*bytes_per_thread;
		in.seekg(offset, std::ios_base::beg);
		char c;
		do 
			in.get(c);
		while ( c != '\n' ) ;
		_thdStarts[i] = in.tellg();
	}
	in.close();
}

void TweetWords::ReadTweetsT(int tnum, std::streampos start, std::streampos end)
{
	std::ifstream in(_inputFile.c_str(), std::ifstream::in);
	in.seekg(start, std::ios_base::beg);
	std::vector<uchar> cts;
	cts.reserve( (int)(end - start) / AVG_WORDS_PER_TWEET );
	std::string line;
	while (in.tellg() < end - std::streampos(1))
	{
		std::getline(in, line);
		int ct = AddTweet( Tweet( std::move(line) ) );
		cts.push_back(ct);
	}
	in.close();
	_countSet[tnum] = cts;

	DecThreadCt();
}

void TweetWords::UpdateWordCount(const std::multiset<std::string>& tweet_words)
{
	std::lock_guard<std::mutex> lock(_mutex);
	for (const auto& it : tweet_words)
		_words.insert(it);
}

