// Parallel Opimization hw1_matrix id:312611101
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <functional>

template <typename T>
class Row_Major_Matrix;

template <typename T>
class Column_Major_Matrix;

template <typename T>
class Column_Major_Matrix {
private:
    std::vector<std::vector<T>> all_column;
    size_t rows;
    size_t cols;

public:
    Column_Major_Matrix(size_t num_rows, size_t num_cols) : rows(num_rows), cols(num_cols) {

        all_column.resize(num_cols); // Initialize the row size into the number of columns
        
        for (auto& col : all_column) {
            col.resize(num_rows); // Initialize the column size into the number of rows
        }
        
        // Fill with random values
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100); // Random values between 1 and 100
        
        for (size_t j = 0; j < num_cols; ++j) {
            for (size_t i = 0; i < num_rows; ++i) {
                all_column[j][i] = static_cast<T>(dis(gen));
            }
        }
    }
    
    Column_Major_Matrix(const Column_Major_Matrix& other) : 
        all_column(other.all_column), rows(other.rows), cols(other.cols) {
        std::cout << "Column_Major_Matrix copy constructor called" << std::endl;
    }
    
    Column_Major_Matrix(Column_Major_Matrix&& other) noexcept : 
        all_column(std::move(other.all_column)), rows(other.rows), cols(other.cols) {
        std::cout << "Column_Major_Matrix move constructor called" << std::endl;
        other.rows = 0;
        other.cols = 0;
    }
    
    // Copy assignment operator
    Column_Major_Matrix& operator=(const Column_Major_Matrix& other) {
        std::cout << "Column_Major_Matrix copy assignment operator called" << std::endl;
        if (this != &other) {
            all_column = other.all_column;
            rows = other.rows;
            cols = other.cols;
        }
        return *this;
    }
    
    // Move assignment operator
    Column_Major_Matrix& operator=(Column_Major_Matrix&& other) noexcept {
        std::cout << "Column_Major_Matrix move assignment operator called" << std::endl;
        if (this != &other) {
            all_column = std::move(other.all_column);
            rows = other.rows;
            cols = other.cols;
            other.rows = 0;
            other.cols = 0;
        }
        return *this;
    }
    
    // Getter for column
    std::vector<T>& getColumn(size_t col_idx) {
        if (col_idx >= cols) {
            throw std::out_of_range("Column index out of range");
        }
        return all_column[col_idx];
    }
    
    // Getter for row (need to construct from columns)
    std::vector<T> getRow(size_t row_idx) const {
        if (row_idx >= rows) {
            throw std::out_of_range("Row index out of range");
        }
        
        std::vector<T> row_data(cols);
        for (size_t j = 0; j < cols; ++j) {
            row_data[j] = all_column[j][row_idx];
        }
        return row_data;
    }
    
    // Setter for column
    void setColumn(size_t col_idx, const std::vector<T>& column_data) {
        if (col_idx >= cols || column_data.size() != rows) {
            throw std::out_of_range("Column index out of range or invalid column size");
        }
        all_column[col_idx] = column_data;
    }
    
    // Setter for row (need to update multiple columns)
    void setRow(size_t row_idx, const std::vector<T>& row_data) {
        if (row_idx >= rows || row_data.size() != cols) {
            throw std::out_of_range("Row index out of range or invalid row size");
        }
        
        for (size_t j = 0; j < cols; ++j) {
            all_column[j][row_idx] = row_data[j];
        }
    }
    
    // Get element at position
    T& at(size_t row_idx, size_t col_idx) {
        if (row_idx >= rows || col_idx >= cols) {
            throw std::out_of_range("Index out of range");
        }
        return all_column[col_idx][row_idx];
    }
    
    // Const version of at
    const T& at(size_t row_idx, size_t col_idx) const {
        if (row_idx >= rows || col_idx >= cols) {
            throw std::out_of_range("Index out of range");
        }
        return all_column[col_idx][row_idx];
    }
    
    // Get dimensions
    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
    
    // Matrix multiplication with Row_Major_Matrix
    Column_Major_Matrix<T> operator*(const Row_Major_Matrix<T>& rhs) const {
        if (cols != rhs.getRows()) {
            throw std::invalid_argument("Matrix dimensions do not match for multiplication");
        }
        
        Column_Major_Matrix<T> result(rows, rhs.getCols());
        
        for (size_t j = 0; j < rhs.getCols(); ++j) {
            for (size_t i = 0; i < rows; ++i) {
                T sum = 0;
                for (size_t k = 0; k < cols; ++k) {
                    sum += this->at(i, k) * rhs.at(k, j);
                }
                result.at(i, j) = sum;
            }
        }
        
        return result;
    }
    
    // Multithreaded matrix multiplication with Row_Major_Matrix (operator%)
    Column_Major_Matrix<T> operator%(const Row_Major_Matrix<T>& rhs) const {
        if (cols != rhs.getRows()) {
            throw std::invalid_argument("Matrix dimensions do not match for multiplication");
        }
        
        Column_Major_Matrix<T> result(rows, rhs.getCols());
        
        // Number of threads to use
        const size_t num_threads = 10;
        
        // Lambda function for matrix multiplication of a subset of rows 
        auto multiply_subset = [&](size_t start_row, size_t end_row) {
            for (size_t j = 0; j < rhs.getCols(); ++j) {
                for (size_t i = start_row; i < end_row; ++i) {
                    T sum = 0;
                    for (size_t k = 0; k < cols; ++k) {
                        sum += this->at(i, k) * rhs.at(k, j);
                    }
                    result.at(i, j) = sum;
                }
            }
        };
        
        // Create and start threads
        std::vector<std::thread> threads;
        const size_t rows_per_thread = rows / num_threads;
        
        for (size_t t = 0; t < num_threads; ++t) {
            size_t start_row = t * rows_per_thread;
            size_t end_row = (t == num_threads - 1) ? rows : (t + 1) * rows_per_thread;
            threads.emplace_back(multiply_subset, start_row, end_row);
        }
        
        // Join all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        return result;
    }
    
    // Conversion operator to Row_Major_Matrix
    operator Row_Major_Matrix<T>() const {
        Row_Major_Matrix<T> result(rows, cols);
        
        for (size_t i = 0; i < rows; ++i) {
            result.setRow(i, this->getRow(i)); // The setRow is defined in Row_Major_Matrix class
        }
        
        return result;
    }
    
    // Print matrix (for testing)
    void print(std::ostream& os, int max_rows = 5, int max_cols = 5) const {
        os << "Column Major Matrix " << rows << "x" << cols << ":" << std::endl;
        int r_limit = std::min(max_rows, static_cast<int>(rows));
        int c_limit = std::min(max_cols, static_cast<int>(cols));
        
        for (int i = 0; i < r_limit; ++i) {
            for (int j = 0; j < c_limit; ++j) {
                os << at(i, j) << " ";
            }
            os << (c_limit < cols ? "..." : "") << std::endl;
        }
        if (r_limit < rows) {
            os << "..." << std::endl;
        }
    }
};

// Row Major Matrix Class
template <typename T>
class Row_Major_Matrix {
private:
    std::vector<std::vector<T>> all_row;
    size_t rows;
    size_t cols;

public:
    Row_Major_Matrix(size_t num_rows, size_t num_cols) : rows(num_rows), cols(num_cols) {
        // Initialize column with num_rows elements
        all_row.resize(num_rows);
        
        // Each row has num_cols elements
        for (auto& row : all_row) {
            row.resize(num_cols);
        }
        
        // Fill with random values
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100); // Random values between 1 and 100
        
        for (size_t i = 0; i < num_rows; ++i) {
            for (size_t j = 0; j < num_cols; ++j) {
                all_row[i][j] = static_cast<T>(dis(gen));
            }
        }
    }
    
    Row_Major_Matrix(const Row_Major_Matrix& other) : 
        all_row(other.all_row), rows(other.rows), cols(other.cols) {
        std::cout << "Row_Major_Matrix copy constructor called" << std::endl;
    }
    
    Row_Major_Matrix(Row_Major_Matrix&& other) noexcept : 
        all_row(std::move(other.all_row)), rows(other.rows), cols(other.cols) {
        std::cout << "Row_Major_Matrix move constructor called" << std::endl;
        other.rows = 0;
        other.cols = 0;
    }
    
    // Copy assignment operator
    Row_Major_Matrix& operator=(const Row_Major_Matrix& other) {
        std::cout << "Row_Major_Matrix copy assignment operator called" << std::endl;
        if (this != &other) {
            all_row = other.all_row;
            rows = other.rows;
            cols = other.cols;
        }
        return *this;
    }
    
    // Move assignment operator
    Row_Major_Matrix& operator=(Row_Major_Matrix&& other) noexcept {
        std::cout << "Row_Major_Matrix move assignment operator called" << std::endl;
        if (this != &other) {
            all_row = std::move(other.all_row);
            rows = other.rows;
            cols = other.cols;
            other.rows = 0;
            other.cols = 0;
        }
        return *this;
    }
    
    // Getter for row
    std::vector<T>& getRow(size_t row_idx) {
        if (row_idx >= rows) {
            throw std::out_of_range("Row index out of range");
        }
        return all_row[row_idx];
    }
    
    // Getter for column (need to construct from rows)
    std::vector<T> getColumn(size_t col_idx) const {
        if (col_idx >= cols) {
            throw std::out_of_range("Column index out of range");
        }
        
        std::vector<T> col_data(rows);
        for (size_t i = 0; i < rows; ++i) {
            col_data[i] = all_row[i][col_idx];
        }
        return col_data;
    }
    
    // Setter for row
    void setRow(size_t row_idx, const std::vector<T>& row_data) {
        if (row_idx >= rows || row_data.size() != cols) {
            throw std::out_of_range("Row index out of range or invalid row size");
        }
        all_row[row_idx] = row_data;
    }
    
    // Setter for column (need to update multiple rows)
    void setColumn(size_t col_idx, const std::vector<T>& column_data) {
        if (col_idx >= cols || column_data.size() != rows) {
            throw std::out_of_range("Column index out of range or invalid column size");
        }
        
        for (size_t i = 0; i < rows; ++i) {
            all_row[i][col_idx] = column_data[i];
        }
    }
    
    // Get element at position
    T& at(size_t row_idx, size_t col_idx) {
        if (row_idx >= rows || col_idx >= cols) {
            throw std::out_of_range("Index out of range");
        }
        return all_row[row_idx][col_idx];
    }
    
    // Const version of at
    const T& at(size_t row_idx, size_t col_idx) const {
        if (row_idx >= rows || col_idx >= cols) {
            throw std::out_of_range("Index out of range");
        }
        return all_row[row_idx][col_idx];
    }
    
    // Get dimensions
    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
    
    // Matrix multiplication with Column_Major_Matrix
    Row_Major_Matrix<T> operator*(const Column_Major_Matrix<T>& rhs) const {
        if (cols != rhs.getRows()) {
            throw std::invalid_argument("Matrix dimensions do not match for multiplication");
        }
        
        Row_Major_Matrix<T> result(rows, rhs.getCols());
        
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < rhs.getCols(); ++j) {
                T sum = 0;
                for (size_t k = 0; k < cols; ++k) {
                    sum += this->at(i, k) * rhs.at(k, j);
                }
                result.at(i, j) = sum;
            }
        }
        
        return result;
    }
    
    // Multithreaded matrix multiplication with Column_Major_Matrix (operator%)
    Row_Major_Matrix<T> operator%(const Column_Major_Matrix<T>& rhs) const {
        if (cols != rhs.getRows()) {
            throw std::invalid_argument("Matrix dimensions do not match for multiplication");
        }
        
        Row_Major_Matrix<T> result(rows, rhs.getCols());
        
        // Number of threads to use
        const size_t num_threads = 10;
        
        // Lambda function for matrix multiplication of a subset of rows
        auto multiply_subset = [&](size_t start_row, size_t end_row) {
            for (size_t i = start_row; i < end_row; ++i) {
                for (size_t j = 0; j < rhs.getCols(); ++j) {
                    T sum = 0;
                    for (size_t k = 0; k < cols; ++k) {
                        sum += this->at(i, k) * rhs.at(k, j);
                    }
                    result.at(i, j) = sum;
                }
            }
        };
        
        // Create and start threads
        std::vector<std::thread> threads;
        const size_t rows_per_thread = rows / num_threads;
        
        for (size_t t = 0; t < num_threads; ++t) {
            size_t start_row = t * rows_per_thread;
            size_t end_row = (t == num_threads - 1) ? rows : (t + 1) * rows_per_thread;
            threads.emplace_back(multiply_subset, start_row, end_row);
        }
        
        // Join all threads
        for (auto& thread : threads) {
            thread.join();
        }
        
        return result;
    }
    
    // Conversion operator to Column_Major_Matrix
    operator Column_Major_Matrix<T>() const {
        Column_Major_Matrix<T> result(rows, cols);
        
        for (size_t j = 0; j < cols; ++j) {
            result.setColumn(j, this->getColumn(j));
        }
        
        return result;
    }
    
    // Print matrix (for testing)
    void print(std::ostream& os, int max_rows = 5, int max_cols = 5) const {
        os << "Row Major Matrix " << rows << "x" << cols << ":" << std::endl;
        int r_limit = std::min(max_rows, static_cast<int>(rows));
        int c_limit = std::min(max_cols, static_cast<int>(cols));
        
        for (int i = 0; i < r_limit; ++i) {
            for (int j = 0; j < c_limit; ++j) {
                os << at(i, j) << " ";
            }
            os << (c_limit < cols ? "..." : "") << std::endl;
        }
        if (r_limit < rows) {
            os << "..." << std::endl;
        }
    }
};

// Matrix multiplication test function
template <typename T>
void test_matrix_multiplication() {
    std::cout << "Testing matrix multiplication..." << std::endl;
    
    // Create matrices
    Column_Major_Matrix<T> cm1(500, 500);
    Row_Major_Matrix<T> rm1(500, 500);
    
    // Test regular multiplication
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto result_cm = cm1 * rm1;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "Column_Major * Row_Major (regular): " << duration << " ms" << std::endl;
    }
    
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto result_rm = rm1 * cm1;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "Row_Major * Column_Major (regular): " << duration << " ms" << std::endl;
    }
    
    // Test multithreaded multiplication
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto result_cm = cm1 % rm1;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "Column_Major % Row_Major (multithreaded): " << duration << " ms" << std::endl;
    }
    
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto result_rm = rm1 % cm1;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "Row_Major % Column_Major (multithreaded): " << duration << " ms" << std::endl;
    }
}

// Test implicit conversion operators
template <typename T>
void test_conversion_operators() {
    std::cout << "Testing conversion operators..." << std::endl;
    
    Column_Major_Matrix<T> cc(55, 1000);
    Row_Major_Matrix<T> rr(1000, 66);
    
    std::cout << "Initial matrices:" << std::endl;
    cc.print(std::cout, 3, 3);
    rr.print(std::cout, 3, 3);
    
    // Test implicit conversion with matrix multiplication
    std::cout << "Testing: Row_Major_Matrix<T> result = cc * rr;" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    Row_Major_Matrix<T> result = cc * rr;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Conversion and multiplication completed in " << duration << " ms" << std::endl;
    result.print(std::cout, 3, 3);
}

int main() {
    std::cout << "===== Part I: Matrix =====" << std::endl;
    
    // e. Test
    std::cout << "\ne. Testing copy/move constructors and assignment operators:" << std::endl;
    Column_Major_Matrix<int> cc1(10, 10);
    Row_Major_Matrix<int> rr1(10, 10);
    Column_Major_Matrix<int> cc2(cc1);
    Row_Major_Matrix<int> rr2 = rr1;
    Column_Major_Matrix<int> cc3 = std::move(cc2);
    Row_Major_Matrix<int> rr3 = std::move(rr2);
    
    // h. Test implicit conversion operators
    std::cout << "\nTesting conversion operators:" << std::endl;
    test_conversion_operators<int>();
    
    // i. Test matrix multiplication with and without multithreading
    std::cout << "\nTesting matrix multiplication with and without multithreading:" << std::endl;
    test_matrix_multiplication<int>();
    
    return 0;
}