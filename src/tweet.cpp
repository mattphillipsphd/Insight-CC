#include "tweet.h"

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
	++unique_ct;
    while (pos != std::string::npos)
	{
		const size_t last_pos = pos+1;
		pos = _rawText.find_first_of(whitespace, last_pos);
		const std::string word = _rawText.substr(last_pos, pos-last_pos);
		if (!word.empty()) 
		{
			words.insert(word);
			if (words.count(word)==1) ++unique_ct;
		}
			//The test before the insertion ensures that sequential whitespace
			//is ignored.
	}
	
	return words;
}