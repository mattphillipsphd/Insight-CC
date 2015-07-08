#!/usr/bin/env bash

rm -f tweet_digest*;

if [[ $OSTYPE == "linux-gnu" ]]; then
	g++ -std=c++11 -O3 src/main.cpp src/tweet.cpp src/runningmedian.cpp src/tweetwords.cpp -lpthread -o tweet_digest
else
	g++ -std=c++11 -O3 src/main.cpp src/tweet.cpp src/runningmedian.cpp src/tweetwords.cpp -o tweet_digest
fi

./tweet_digest
