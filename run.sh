
#If running this on a platform (Windows) without pthread support, comment out the compiler 
#command below and uncomment the one beneath it.
g++ -std=c++11 -pthread src/main.cpp src/runningmedian.cpp src/tweetwords.cpp -o tweet_digest
#g++ -std=c++11 src/main.cpp src/runningmedian.cpp src/tweetwords.cpp -o tweet_digest

#Note, the -O2, -O3 compiler options do not measurably improve speed on either Windows or Linux

./tweet_digest
