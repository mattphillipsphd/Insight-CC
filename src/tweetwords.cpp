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
			c = in.get();
		while ( c != '\n' && !in.eof()) ;
		_thdStarts[i] = in.eof() ? std::streampos(_numBytes) : in.tellg();
	}
	in.close();
		
	//We return the end point of the last file segment, which will be the start of the 
	//next file chunk if there are multiple chunks.
	return (long int)_thdStarts.at(_numThreads);
}

void TweetWords::ReadTweets()
{
	//Initialize the variable that will hold the vector of counts for each thread
	_countSet = std::vector< std::vector<uchar> >(_numThreads);

	//Launch each thread, which reads from a particular segment of the input file
	std::vector<std::thread> tvec;
	for (int i=0; i<_numThreads; ++i)
		tvec.emplace_back( 
							&TweetWords::ReadTweetsT, 
							this, i, _thdStarts.at(i), _thdStarts.at(i+1) - std::streampos(1)
						);
	
	//Wait for all threads to complete
	for (auto& it : tvec)
		it.join();
}

std::vector<uchar> TweetWords::UniqueCts() const
{
	int num_tweets = 0;
	for (const auto& it : _countSet) num_tweets += it.size();
	std::vector<uchar> unique_cts;
	unique_cts.reserve(num_tweets);
	
	//The vectors of counts for each thread are concatenated and returned
	//in a single vector.
	for (const auto& it : _countSet)
		unique_cts.insert(unique_cts.end(), it.cbegin(), it.cend());
	
	return unique_cts;
}

void TweetWords::Write() const
{
	std::ofstream out(_ft1, std::ofstream::out);
	if (!out.good())
	{
		std::cerr << "Bad file name, \"" << _ft1 << "\"" << std::endl;
		exit(-1);
	}
	
	auto words_cend = _words.cend();
	for (auto it = _words.cbegin(); it != words_cend; ++it)
		out << it->first << '\t' << it->second << '\n';
		//Printing the formatting (so that the column of numbers is aligned) is actually
		//pretty time-consuming so we simply tab-separate the data.
	out.close();
}

//We move to the start of the file segment assigned to this thread, and read
//tweets until we reach the end.  Words are added to the dictionary, _words, and the unique
//count for each tweet is added to _countSet
void TweetWords::ReadTweetsT(int tnum, std::streampos start, std::streampos end)
{
	//We use C file-handling here, as tokenization proved to be fastest using strtok,
	//as compared to C++ tokenization (find_first_of) or boost::tokenizer.
	FILE* fp = fopen(_inputFile.c_str(), "r");
	fseek(fp, (long int)start, SEEK_SET);
	
	std::vector<uchar> cts;
	cts.reserve( (int)(end - start) / AVG_CHARS_PER_TWEET );
		//cts will hold all the unique counts for each file segment
	
	const char whitespace[] = " \t\v\r\f\n"; 
		//See http://en.cppreference.com/w/cpp/string/byte/isspace.  \v and \f are
		//probably overkill but I don't know the twitter API guarantees.

	std::unordered_map<std::string, int> tweet_words;
		//We use an unordered_map to hold the words for each tweet because 
		//a) it has fast insertion, b) we don't care about order, and c) the number
		//of unique elements is held by the size.
	
	char line[MAX_TWEET_LEN];
	while (ftell(fp) < (long int)end && !feof(fp))
	{
		//Get the next line of text, clear the unordered_set
		fgets(line, MAX_TWEET_LEN, fp);
		tweet_words.clear();
	
		// We simply take each word from the tweet and insert it into the set.
		char* tok = strtok(line, whitespace);
		while (tok)
		{
			if (strlen(tok)>0) ++tweet_words[tok];
				//If the word isn't present in the dictionary, its count is initialized with
				//int() = 0; then its count is incremented.
			tok = strtok(NULL, whitespace);		
		}
		
		//Update the dictionary with all the words from this tweet
		UpdateWordCount(tweet_words);
	
		//Insert the count of unique words
		cts.push_back( tweet_words.size() ); 
	}

	fclose(fp);
	_countSet[tnum] = cts;
}

void TweetWords::UpdateWordCount(const std::unordered_map<std::string, int>& tweet_words)
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto words_cend = tweet_words.cend();
	for (auto it = tweet_words.cbegin(); it != words_cend; ++it)
		_words[it->first] += it->second;
			//If the word isn't present in the dictionary, its count is initialized with
			//int() = 0; then its count is increased.
}

