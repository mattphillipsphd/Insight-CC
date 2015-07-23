#ifndef GLOBALS_H
#define GLOBALS_H

/*
	For convenience STL header files are included in a single global file
	instead of the individual files (tweetwords.h, runningmedian.h), 
	since it is	unlikely one will change without the others changing (e.g., 
	when a new version of the libraries are installed).
*/

//Testing indicated that on Windows under Cygwin threads actually slowed down
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#define USE_THREADS
#endif
#define MAX_NUM_THREADS					16

//To use the single-threaded implementation on any platform, uncomment this line
//#undef USE_THREADS

#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#ifdef USE_THREADS
#include <thread>
#endif
#include <unordered_set>
#include <vector>

typedef unsigned char					uchar;
typedef std::chrono::milliseconds		TimeT;

#define MAX_TWEET_LEN					140

const long int MAX_CHUNK_SIZE = INT_MAX;

#endif //GLOBALS_H