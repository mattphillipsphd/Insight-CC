#include "tweet.h"

Tweet::Tweet(const std::string&& text) : _rawText(text)
{
}

//This version is a bit faster than a C++ version using std::find_first_of and much
//faster than Boost::tokenizer
std::multiset<std::string> Tweet::Words(int& unique_ct) const
{
	std::multiset<std::string> words;
	unique_ct = 0;
	
	const char whitespace[] = " \t\v\r\f"; 
		//See http://en.cppreference.com/w/cpp/string/byte/isspace.  We can
		//ignore \n since that is stripped by std::getline.  \v and \f are
		//probably overkill but I don't know the twitter API guarantees.
	char text[MAX_TWEET_LEN];
	strcpy(text, _rawText.c_str());

	// We simply take each word from the tweet and insert it into the set.
	// The implementation of std::multiset lets us easily account for duplicates.
	char* tok = strtok(text, whitespace);
	while (tok)
	{
		if (strlen(tok)>0) words.insert(tok);
		tok = strtok(NULL, whitespace);		
	}
	
	auto words_end = words.cend();
	for (auto it = words.cbegin(); it!=words_end; it = words.upper_bound(*it))
		++unique_ct;
		//Testing for uniqueness here is faster than in the loop
	
	return words;
}
