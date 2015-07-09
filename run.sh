#!/usr/bin/env bash

INPUT_FN="tweet_input/tweets.txt"
MAX_THREADS=1
#Surprisingly multiple threads does not lead to improvement on my machine.

if [[ $OSTYPE == "linux-gnu" ]]; then
	rm -f tweet_digest;
	g++ -std=c++11 -pthread \
			src/main.cpp src/tweet.cpp src/runningmedian.cpp src/tweetwords.cpp \
			-o tweet_digest
	./tweet_digest $INPUT_FN $MAX_THREADS
else
	rm -f tweet_digest.exe;
	g++ -std=c++11 src/main.cpp src/tweet.cpp src/runningmedian.cpp src/tweetwords.cpp -o tweet_digest
	./tweet_digest.exe $INPUT_FN $MAX_THREADS
fi
#Note, the -O3 compiler option does not measurably improve speed, tested on Windows

