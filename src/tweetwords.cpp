#include "tweetwords.h"

TweetWords::TweetWords(const std::string& input_file, const std::string& ft1, int user_max_threads)
	: _inputFile(input_file), _ft1(ft1)
{
	FILE* fp = fopen(_inputFile.c_str(), "r");
	if (!fp)
	{
		std::cerr << "Bad file name, \"" << _inputFile << "\"" << std::endl;
		exit(-1);
	}
	fseek(fp, 0, SEEK_END);
	_numBytes = ftell(fp);
	fclose(fp);
	
#ifdef USE_THREADS
	//Get best number of threads from OS
	_numThreads = (_numBytes > 4096 && std::thread::hardware_concurrency()>0)
					? std::thread::hardware_concurrency()
					: 1;
	
	//We restrict the number of threads to both programmatically (globals.h) and user-supplied
	//maxima, as well as the maximum number of open file pointers
	_numThreads = std::min(FOPEN_MAX, 
					std::min(user_max_threads, 
						std::min(_numThreads, MAX_NUM_THREADS)));
	
	//We round down to the nearest power of 2
	if (_numThreads>2)
		while ( floor(log2(_numThreads)) != log2(_numThreads) ) --_numThreads;

	std::cout << "Using " << _numThreads << " thread(s) to create dictionary." << std::endl;
#else
	_numThreads = 1;
#endif

	//One dictionary is constructed per thread, covering a unique range of the ASCII
	//table.
	_dicts = std::vector<Dictionary>(_numThreads);
}

//We divide the current file chunk into as many contiguous pieces as there are threads,
//and then prepare each thread to read just from its segment.  The results
//are fed into the dictionary, _words, using the mutex-protected function UpdateWordCount.
//The unique word counts are collected in _countSet and merged when UniqueCts is called.
long int TweetWords::InitChunk(long int start_byte)
{
	_chunkStart = start_byte;
	
	//Figure out where to start looking for the last byte
	const long int offset = std::min(_chunkStart+MAX_CHUNK_SIZE, _numBytes) - 1;
	FILE* fp = fopen(_inputFile.c_str(), "r");
	fseek(fp, offset, SEEK_SET);
	
	//We cannot break the file mid-tweet.  So we take offset as a 'hint' and search
	//for the first newline.  This will be the end of this piece and will determine
	//the start of the next one.
	char c;
	do 
		c = fgetc(fp);
	while ( c != '\n' && !feof(fp)) ;
	_chunkEnd = feof(fp) ? _numBytes : ftell(fp);
	
	fclose(fp);
		
	//The count of unique words is cleared for each count
	_counts.clear();
		
	//We return the end point of the last file segment, which will be the start of the 
	//next file chunk if there are multiple chunks.
	return _chunkEnd;  
}

//We launch the threads which read the input file and wait for them to complete, or, 
//if USE_THREADS is not defined we call ReadTweetsT within the main thread.
void TweetWords::ReadTweets()
{
	//Initialize the variable that will hold the vector of counts for each thread
	_counts.reserve( std::min(_numBytes,MAX_CHUNK_SIZE) / AVG_CHARS_PER_TWEET );

#ifdef USE_THREADS
	//Launch each thread, which reads the input file but only writes to a single
	//dictionary.
	std::vector<std::thread> tvec;
	for (int i=0; i<_numThreads; ++i)
		tvec.emplace_back( &TweetWords::ReadTweetsT, this, i);
	
	//Wait for all threads to complete
	for (auto& it : tvec)
		it.join();
#else
	TweetWords::ReadTweetsT(0);
#endif
}

void TweetWords::Write() const
{
	FILE* fp = fopen(_ft1.c_str(), "w");
	if (!fp)
	{
		std::cerr << "Bad file name, \"" << _ft1 << "\"" << std::endl;
		exit(-1);
	}
	
	int ct = 0;
	for (auto itd : _dicts)
	{
		if (itd.empty()) continue;
		auto itd_cend = itd.cend();
		for (auto it = itd.cbegin(); it != itd_cend; ++it)
			fprintf(fp, "%s\t%d\n", it->first.c_str(), it->second);
			//Printing the formatting (so that the column of numbers is aligned) is actually
			//pretty time-consuming so we simply tab-separate the data.

	}
	fclose(fp);
}

//We move to the start of the file segment assigned to this thread, and read
//tweets until we reach the end.  Words are added to the dictionaries, _dicts, and the unique
//count for each tweet is added to _counts
void TweetWords::ReadTweetsT(int tnum)
{
	//We use C file-handling here, as tokenization proved to be fastest using strtok,
	//as compared to C++ tokenization (find_first_of) or boost::tokenizer.
	FILE* fp = fopen(_inputFile.c_str(), "r");
	fseek(fp, _chunkStart, SEEK_SET);
	
	const char whitespace[] = " \t\v\r\f\n"; 
		//See http://en.cppreference.com/w/cpp/string/byte/isspace.  \v and \f are
		//probably overkill but I don't know the twitter API guarantees.

#ifdef USE_THREADS
	static char* save_ptrs[MAX_NUM_THREADS];
	char** save_ptr = &save_ptrs[tnum];
	
	//There are a maximum of 16 'slots' a tweet word can be placed into, based on its
	//first letter.  If there are fewer than 16 threads, slots are collapsed accordingly
	//using tscale
	const int num_slots = 16;
	const int tscale = num_slots / _numThreads;
#endif

	//We avoid potentially millions of member access operations by taking a reference
	Dictionary& dict = _dicts[tnum];  
		
	std::unordered_set<std::string> tweet_words;
	char line[MAX_TWEET_LEN];
	while ( ftell(fp) < _chunkEnd && !feof(fp)) 
	{
		//Get the next line of text, clear the unordered_set
		fgets(line, MAX_TWEET_LEN, fp);
		tweet_words.clear();

#ifdef USE_THREADS
		int slot, c;
		char* tok = strtok_r(line, whitespace, save_ptr);
		while (tok)
		{
			if (strlen(tok)>0)
			{		
				//Each token which is read in is assigned, on the basis of its first letter,
				//to an alphabetic 'slot'.  This is used to unambiguously determine into which
				//dictionary the token is inserted.
				c = tok[0];
				if (c<47) slot = 0;
				else if (c < 58) slot = 1;
				else if (c < 65) slot = 2;
				else if (c < 97) slot = 3;
				else if (c > 118) slot = 15;
				else slot = (c - 97) / 2 + 4;

				if (slot/tscale == tnum) ++( dict[tok] ); 
					//If the word isn't present in the dictionary, its count is initialized with
					//int() = 0; then its count is incremented.

				//For uniqueness, we only count each word once, in thread 0
				if (tnum==0) tweet_words.insert(tok);
			}
			tok = strtok_r(NULL, whitespace, save_ptr);		
		}
#else
		char* tok = strtok(line, whitespace);
		while (tok)
		{
			if (strlen(tok)>0)
			{
				++( dict[tok] );
					//If the word isn't present in the dictionary, its count is initialized with
					//int() = 0; then its count is incremented.
				tweet_words.insert(tok);
			}
			tok = strtok(NULL, whitespace);		
		}
#endif
	
		//Insert the count of unique words
		if (tnum==0) _counts.push_back( tweet_words.size() ); 
	}

	fclose(fp);
}
