#include "tweet.h"

#include "C:\Users\matt\Libraries\boost_1_55_0\boost\tokenizer.hpp"

Tweet::Tweet(const std::string&& text) : _rawText(text)
{
}

std::multiset<std::string> Tweet::Words(int& unique_ct) const
{
	std::multiset<std::string> words;
	unique_ct = 0;
	
	const std::string whitespace = " \t\v\r\f"; 
		//See http://en.cppreference.com/w/cpp/string/byte/isspace.  We can
		//ignore \n since that is stripped by std::getline.  \v and \f are
		//probably overkill but I don't know the twitter API guarantees.

	/*
		We simply take each word from the tweet and insert it into the set.
		The implementation of std::multiset takes care of eliminating duplicates.
	*/
    const size_t pos0 = _rawText.find_first_not_of(whitespace);
	if (pos0 == std::string::npos) return words; 
		//We don't allow empty words
    size_t pos = _rawText.find_first_of(whitespace, pos0);
	words.insert(_rawText.substr(pos0, pos-pos0));
    while (pos != std::string::npos)
	{
		const size_t last_pos = pos+1;
		pos = _rawText.find_first_of(whitespace, last_pos);
		const std::string word = _rawText.substr(last_pos, pos-last_pos);
		if (!word.empty()) words.insert(word);
			//The test before the insertion ensures that sequential whitespace
			//is ignored.
	}
	
	auto words_end = words.cend();
	for (auto it = words.cbegin(); it!=words_end; it = words.upper_bound(*it))
		++unique_ct;
	
	return words;
}

//Boost tokenizer may add more flexibility, but this is not faster than C++
std::multiset<std::string> Tweet::WordsBoost(int& unique_ct) const
{
	std::multiset<std::string> words;
	unique_ct = 0;
	
	boost::char_separator<char> sep(" \t\v\r\f");
	boost::tokenizer< boost::char_separator<char> > tok(_rawText, sep);
	for(auto beg=tok.begin(); beg!=tok.end(); ++beg)
		if ((*beg).length()>0)
		{
			words.insert(*beg);
			if (words.count(*beg)==1) ++unique_ct;
		}
		
	return words;
}


//Looks like this substantially faster than C++ version, even with the copy
std::multiset<std::string> Tweet::WordsStrtok(int& unique_ct) const
{
	std::multiset<std::string> words;
	unique_ct = 0;
	
	const char whitespace[] = " \t\v\r\f"; 
	char text[MAX_TWEET_LEN];
	strcpy(text, _rawText.c_str());
	
	char* tok = strtok(text, whitespace);
	while (tok)
	{
		if (strlen(tok)>0)
			words.insert(tok);

		tok = strtok(NULL, whitespace);		
	}
	
	auto words_end = words.cend();
	for (auto it = words.cbegin(); it!=words_end; it = words.upper_bound(*it))
		++unique_ct;
		//Testing for uniqueness in the loop dramatically increases runtime(!)
	
	return words;
}