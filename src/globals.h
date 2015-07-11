#ifndef GLOBALS_H
#define GLOBALS_H

/*
	For convenience STL header files are included in a single global file
	instead of the individual files (tweet.h, tweetwords.h, runningmedian.h), 
	since it is	unlikely one will change without the others changing (e.g., 
	when a new version of the libraries are installed).
*/

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

typedef unsigned char					uchar;
typedef std::chrono::milliseconds		TimeT;

#define MAX_TWEET_LEN			140

const long int MAX_CHUNK_SIZE = INT_MAX;

#endif //GLOBALS_H