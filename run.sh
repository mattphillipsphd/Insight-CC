#!/usr/bin/env bash

INPUT_FN="tweet_input/tweets.txt"
MAX_THREADS=1
#It turns out that multiple threads do not lead to consistent improvement, at least on my machines.

if [[ $OSTYPE == "linux-gnu" ]]; then
	TARGET="tweet_digest"
	if [ ! -e $TARGET ]; then
		g++ -std=c++11 -pthread \
				src/main.cpp src/tweet.cpp src/runningmedian.cpp src/tweetwords.cpp \
				-o $TARGET
	fi
	./tweet_digest $INPUT_FN $MAX_THREADS
else
	TARGET="tweet_digest.exe"
	if [ ! -e $TARGET ]; then
		g++ -std=c++11 src/main.cpp src/tweet.cpp src/runningmedian.cpp src/tweetwords.cpp -o $TARGET
	fi
	./$TARGET $INPUT_FN $MAX_THREADS
fi
#Note, the -O3 compiler option does not measurably improve speed, tested on Windows

