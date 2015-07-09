#!/usr/bin/env bash

rm -f tweet_digest*;

if [[ $OSTYPE == "linux-gnu" ]]; then
	g++ -std=c++11 src/main.cpp src/tweet.cpp src/runningmedian.cpp src/tweetwords.cpp -lpthread -o tweet_digest
else
	g++ -std=c++11 src/main.cpp src/tweet.cpp src/runningmedian.cpp src/tweetwords.cpp -o tweet_digest
fi
#Note, the -O3 compiler option does not measurably improve speed, tested on Windows

./tweet_digest
