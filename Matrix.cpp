#pragma once
#include "Matrix.h"

#include <iostream>     // For std::cerr and std::cout
#include <fstream>      // For std::ofstream

void Matrix::createMatrix()
{
	matrix = new double* [getRows()]; // Allocate an array of pointers (rows)
	for (int i = 0; i < getRows(); i++) 
	{
		matrix[i] = new double[getColumns()]; // Allocate each row
		for (int j = 0; j < getColumns(); j++) 
		{
			matrix[i][j] = 0.0; // Initialize each element to 0.0
		}
	}
};

int Matrix::getColumns() const
{
	return columns;
}

int Matrix::getRows() const 
{
	return rows;
}
int Matrix::saveMatrix(const std::string& filename)
{
	std::ofstream outFile(filename);
	if (!outFile) {
		std::cerr << "Error: Could not open the file " << filename << " for writing." << std::endl;
		return 1;
	}

	for (int i = 0; i < getRows(); ++i) {
		for (int j = 0; j < getColumns(); ++j) {
			outFile << matrix[i][j];
			if (j < getColumns() - 1) {
				outFile << " "; // Separate values with space
			}
		}
		outFile << "\n"; // New line after each row
	}

	outFile.close();
	std::cout << "Matrix saved to " << filename << std::endl;
	return 0;
}

double Matrix::getValue(int row, int col) const 
{
	if (row >= 0 && row < rows && col >= 0 && col < columns) 
	{
		return matrix[row][col];
	}
	else {
		std::cerr << "Error: Index out of bounds" << std::endl;
		return 0;
	}
}
void Matrix::setValue(int row, int col, double value) 
{
	if (row >= 0 && row < getRows() && col >= 0 && col < getColumns())
	{
		
		matrix[row][col]=value;
	}
	else {
		std::cerr << "Error: Index out of bounds" << std::endl;
	
	}
}

Matrix::~Matrix() 
{
	for (int i = 0; i < rows; i++) {
		delete[] matrix[i];
	}
	delete[] matrix;
}
