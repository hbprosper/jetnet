#ifndef NETWORK_H
#define NETWORK_H
///////////////////////////////////////////////////////////////
// File: network.h
// Purpose: Provide some feed forward neural network utilities
// Created: 17-Sep-2000 Harrison B. Prosper
// Updated: 04-Jul-2005 HBP Add mean and sigmas
///////////////////////////////////////////////////////////////

#include <string>
#include <vector>

// extract name from pathname

std::string nameonly(std::string pathname);

// Read contents of weight file

int   nnload(std::string filename, 
	     std::vector<int>&         nodes, 
	     std::vector<double>&      weight,
	     std::vector<std::string>& var,
	     std::vector<float>&       mean,
	     std::vector<float>&       sigma,
	     int &outputType);

// Run neural network and return results
///////////////////////////////////////////////////////////
int   nncompute(std::vector<int>&    nodes,
		std::vector<double>& weight, 
		std::vector<float>&  inp, 
		std::vector<float>&  out, int outputType);

// Write out C++ function

int   nnsaveCPP(std::string title1, 
		std::string title2,
		std::string sigmoid,
		std::string sigmoidout,
		std::string filename, 
		std::vector<int>&         nodes, 
		std::vector<double>&      weight, 
		std::vector<std::string>& var, 
		std::vector<float>&       mean,
		std::vector<float>&       sigma,
		int outputType);

float nnpower(std::vector<int>& s, std::vector<int>& b);

void  nnefficiencies(std::vector<int>& v, std::vector<float>& e);

float nnarea(std::vector<float>& ex, std::vector<float>& ey);

float nndivergence(std::vector<int>& s, std::vector<int>& b);

#endif















