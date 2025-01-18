#pragma once
#include "Queue.h"
#include <string>
#include <stdexcept>

#define SIZE 10

class SequenceQueue : public Queue<std::pair<int, std::string>> {
private:
	std::pair<int, std::string> data[SIZE]; // Array to hold the pairs
	int front, rear;

public:
	Seq
};