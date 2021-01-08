#pragma once
#ifndef _CELLTimestamp_
#define _CELLTimestamp_

#include <chrono>
#include <iostream>
using namespace std::chrono;


class  CELLTimestamp
{
public:
	CELLTimestamp() {};
	~CELLTimestamp() {};

	void update() {
		begin = high_resolution_clock::now();
	}

	double getElapsedTimeSec() {
		return this->getElapsedTimeInMicroSec() * 0.000001;
	}

	double getElapsedTimeInMilliSec() {
		return this->getElapsedTimeInMicroSec() * 0.001;
	}

	long long getElapsedTimeInMicroSec() {
		return duration_cast<microseconds>(high_resolution_clock::now() - begin).count();
	}


private:
	time_point<high_resolution_clock> begin;
};


#endif // !_CELLTimestamp_
