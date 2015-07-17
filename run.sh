#!/usr/bin/env bash

INPUT_FN="tweet_input/tweets.txt"
MAX_THREADS=16

if [[ $OSTYPE == "linux-gnu" ]]; then
	TARGET=tweet_digest
	if [ ! -e $TARGET ]; then
		g++ -std=c++11 -pthread \
				src/main.cpp src/runningmedian.cpp src/tweetwords.cpp \
				-o $TARGET
	fi
else
	TARGET=tweet_digest.exe
	if [ ! -e $TARGET ]; then
		g++ -std=c++11 src/main.cpp src/runningmedian.cpp src/tweetwords.cpp \
				-o $TARGET
	fi
fi
#Note, the -O3 compiler option does not measurably improve speed, tested on Windows

./$TARGET $INPUT_FN $MAX_THREADS
