#ifndef TWEET_H
#define TWEET_H

#include "globals.h"

/*
	class Tweet
	This class encapsulates a single tweet.  It does on-demand calculation
	of the unique words, which eliminates redundant storage and copying.
	However if the program evolved to make multiple calls to UniqueWords,
	the # unique words could be calculated on construction and stored in the class,
	and UniqueWords could become a simple 'getter' function.
*/
class Tweet
{
	public:
		/*
			Tweet(): Our constructor requires an xvalue, since the only purpose for	
			reading a line from tweets.txt is to convert it immediately into 
			a Tweet object. 
		*/
		explicit Tweet(const std::string&& text);
		
		/*
			UniqueWords(): Returns a set of the words in the tweet, which by 
			definition of std::set, will alphabetize them and include only 
			unique elements.
				Note that UniqueWords *ignores* sequential whitespace.  It will 
			not return a set with empty words.
		*/
		std::multiset<std::string> Words(int& unique_ct) const;
		
	private:
	
		//_rawText:  The raw text of the tweet as it is read from the file.
		const std::string _rawText;
};

#endif //TWEET_H