#pragma once

#include <string> 
class Matrix
{
private:
	int rows;
	int columns;
	double** matrix;
public:
	Matrix(int r, int c) : rows(r), columns(c), matrix(nullptr) {}

	~Matrix();

	void createMatrix();

	int saveMatrix(const std::string& filename);
	
	double getValue(int row, int col) const;

	void setValue(int row, int col, double value);

	int getColumns() const;
	int getRows() const;

};

