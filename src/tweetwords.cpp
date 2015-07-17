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
	_thdStarts = std::vector<std::streampos>(2);
	_thdStarts[0] = bstart;
	
	const long int last_byte = std::min(bstart+MAX_CHUNK_SIZE, _numBytes) - 1, 
					bytes_per_thread = last_byte - bstart;
	std::ifstream in(_inputFile.c_str(), std::ifstream::in);
	std::streampos offset = bstart + bytes_per_thread;
	in.seekg(offset, std::ios_base::beg);
	
	//We cannot break the file mid-tweet.  So we take offset as a 'hint' and search
	//for the first newline.  This will be the end of this piece and will determine
	//the start of the next one.
	char c;
	do 
		c = in.get();
	while ( c != '\n' && !in.eof()) ;
	_thdStarts[1] = in.eof() ? std::streampos(_numBytes) : in.tellg();
	
	in.close();
		
	//We return the end point of the last file segment, which will be the start of the 
	//next file chunk if there are multiple chunks.
	return (long int)_thdStarts.at(1);  
}

void TweetWords::ReadTweets()
{
	//Initialize the variable that will hold the vector of counts for each thread
	_counts.reserve( std::min(_numBytes,MAX_CHUNK_SIZE) / AVG_CHARS_PER_TWEET );

	_dicts = std::vector<Dictionary>(_numThreads);
	
	//Launch each thread, which reads from a particular segment of the input file
	std::vector<std::thread> tvec;
	for (int i=0; i<_numThreads; ++i)
		tvec.emplace_back( &TweetWords::ReadTweetsT, this, i);
	
	//Wait for all threads to complete
	for (auto& it : tvec)
		it.join();
}

std::vector<uchar> TweetWords::UniqueCts() const
{
	return _counts;
}

void TweetWords::Write() const
{
	std::ofstream out(_ft1, std::ofstream::out);
	if (!out.good())
	{
		std::cerr << "Bad file name, \"" << _ft1 << "\"" << std::endl;
		exit(-1);
	}
	
	int ct = 0;
	for (auto itd : _dicts)
	{
		std::cerr << "Thread: " << ct++ << ": " << itd.size() << std::endl;
		if (itd.empty()) continue;
		auto itd_cend = itd.cend();
		for (auto it = itd.cbegin(); it != itd_cend; ++it)
			out << it->first << '\t' << it->second << '\n';
			//Printing the formatting (so that the column of numbers is aligned) is actually
			//pretty time-consuming so we simply tab-separate the data.
	}
	out.close();
}

//We move to the start of the file segment assigned to this thread, and read
//tweets until we reach the end.  Words are added to the dictionary, _words, and the unique
//count for each tweet is added to _countSet
void TweetWords::ReadTweetsT(int tnum)
{
	//We use C file-handling here, as tokenization proved to be fastest using strtok,
	//as compared to C++ tokenization (find_first_of) or boost::tokenizer.
	FILE* fp = fopen(_inputFile.c_str(), "r");
	std::cerr << "Starting thread " << tnum << std::endl;
	
	const char whitespace[] = " \t\v\r\f\n"; 
		//See http://en.cppreference.com/w/cpp/string/byte/isspace.  \v and \f are
		//probably overkill but I don't know the twitter API guarantees.

	std::unordered_set<std::string> tweet_words;
		
	char line[MAX_TWEET_LEN];
	long int end = _thdStarts[1];
	const int tscale = 16 / _numThreads;
	int ct = 0;
	while ( ftell(fp) < end && !feof(fp))
	{
		//Get the next line of text, clear the unordered_set
		fgets(line, MAX_TWEET_LEN, fp);
		tweet_words.clear();
	
		// We simply take each word from the tweet and insert it into the set.
		char* tok = strtok(line, whitespace);
		int slot, c;
		while (tok)
		{
			if (strlen(tok)>0)
			{		
				if (tok[0]/16 == tnum) ++( _dicts[tnum][tok] ); //Simpler but slightly slower on the laptop
				
				//This version distributes words more effectively among threads but
				//has more computation per word
//				c = tok[0];
/*				if (c<47) slot = 0;
				else if (c < 58) slot = 1;
				else if (c < 65) slot = 2;
				else if (c < 97) slot = 3;
				else if (c > 118) slot = 15;
				else slot = (c - 97) / 2 + 4;

//if ((ct++ % 100000) == 0) 
	std::cerr << slot << ", " << tnum << std::endl;
				if (slot/tscale == tnum)
				{
					++( _dicts.at(tnum)[tok] ); // #### The Linux version is skipping words ?!
					//###Probably b/c file endings?
					//If the word isn't present in the dictionary, its count is initialized with
					//int() = 0; then its count is incremented.
				}
				*/
				if (tnum==0) tweet_words.insert(tok);
			}
			tok = strtok(NULL, whitespace);		
		}
		
		//Insert the count of unique words
		if (tnum==0) _counts.push_back( tweet_words.size() ); 
	}

	fclose(fp);
}

/*
				switch (tok[0])
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
					case 16:
					case 17:
					case 18:
					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					case 24:
					case 25:
					case 26:
					case 27:
					case 28:
					case 29:
					case 30:
					case 31:
					case 32:
					case 33:
					case 34:
					case 35:
					case 36:
					case 37:
					case 38:
					case 39:
					case 40:
					case 41:
					case 42:
					case 43:
					case 44:
					case 45:
					case 46:
						slot = 0;
						break;
					case 47:
					case 48:
					case 49:
					case 50:
					case 51:
					case 52:
					case 53:
					case 54:
					case 55:
					case 56:
					case 57:
						slot = 1;
						break;
					case 58:
					case 59:
					case 60:
					case 61:
					case 62:
					case 63:
					case 64:
						slot = 2;
						break;
					case 91:
					case 92:
					case 93:
					case 94:
					case 95:
					case 96:
						slot = 3;
						break;
					case 97:
					case 98:
						slot = 4;
						break;
					case 99:
					case 100:
						slot = 5;
						break;
					case 101:
					case 102:
						slot = 6;
						break;
					case 103:
					case 104:
						slot = 7;
						break;
					case 105:
					case 106:
						slot = 8;
						break;
					case 107:
					case 108:
						slot = 9;
						break;
					case 109:
					case 110:
						slot = 10;
						break;
					case 111:
					case 112:
						slot = 11;
						break;
					case 113:
					case 114:
						slot = 12;
						break;
					case 115:
					case 116:
						slot = 13;
						break;
					case 117:
					case 118:
						slot = 14;
						break;
					case 119:
					case 120:
					case 121:
					case 122:
					case 123:
					case 124:
					case 125:
					case 126:
					case 127:
						slot = 15;
						break;
				}
*/
