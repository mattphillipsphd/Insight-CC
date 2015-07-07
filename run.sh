#!/usr/bin/env bash

g++ -std=c++11 src/main.cpp src/tweet.cpp src/summary.cpp -o tweet_digest

./tweet_digest
