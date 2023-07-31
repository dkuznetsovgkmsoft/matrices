/****************************************************************************************
* Copyright © 2023 Dmitry Kuznetsov.                                                    *
*                                                                                       *
* All rights reserved. No part of this software may be reproduced, distributed,         *
* or transmitted in any form or by any means, including photocopying, recording,        *
* or other electronic or mechanical methods, without the prior written permissin        *
* of the copyright owner.                                                               *
* Any unauthorized use, reproduction, or distribution of this software is strictly      *
* prohibited and may # result in severe civil and criminal penalties.                   *
*                                                                                       *
****************************************************************************************/

#pragma once

#include <vector>
#include <ranges>

#include "utility.h"

namespace matrices {
    template<typename T> requires std::is_arithmetic_v<T>
    class matrix_d final {
        using internal_type = T;
        using index_type = std::uint32_t;

        index_type rows_count{ 0 };
        index_type columns_count{ 0 };
        std::vector<internal_type> data{};

        void requires_same_size_for_matrices(const matrix_d<T>& other) const {
            if (rows_count != other.rows_count || columns_count != other.columns_count) {
                throw std::runtime_error("The dimensions of the matrices are not equal");
            }
        }

        void requires_square_matrix() const {
            if (rows_count != columns_count) {
                throw std::runtime_error("The matrix is not square");
            }
        }

        void requires_take_submatrix(const index_type& sub_rows, const index_type& sub_cols, const index_type& start_row, const index_type& start_col) const{
            if ((sub_rows + start_row) > rows_count ||
                (sub_cols + start_col) > columns_count) {
                throw std::runtime_error("The conditions for taking the submatrix are not met");
            }
        }

        void make_identity() {
            auto min_dim = std::min(rows_count, columns_count);
            for (index_type ri = 0; ri < min_dim; ++ri) {
                operator()(ri, ri) = { 1 };
            }
        }
    public:
        matrix_d(index_type rows, index_type cols) : rows_count(rows), columns_count(cols) {
            try {
                data.resize(utility::multiply(rows, cols));
            }
            catch (std::exception) {
                rows = 0;
                cols = 0;

                data.resize(0);
            }

            make_identity();
        }

        matrix_d() {
            make_identity();
        }

        matrix_d(index_type num_rows, index_type num_cols, std::vector<internal_type>&& input_data)
            : rows_count(num_rows), columns_count(num_cols) {

            try {
                data.resize(utility::multiply(num_rows, num_cols));
            }
            catch (std::exception) {
                rows_count = 0;
                columns_count = 0;

                data.resize(0);
            }

            if (input_data.size() > data.size()) {
                input_data.resize(data.size());
            }

            std::move(begin(input_data), end(input_data), begin(data));
        }

        template<utility::Scalar ...Args>
        matrix_d(index_type num_rows, index_type num_cols, Args&&... args)
            : rows_count(num_rows), columns_count(num_cols) {

            try {
                data.resize(utility::multiply(num_rows, num_cols));
            }
            catch (std::exception) {
                rows_count = 0;
                columns_count = 0;

                data.resize(0);
            }

            auto size_of_list = sizeof...(args);

            std::initializer_list input_data{ args... };
            for (index_type index{ 0 }; const auto & value : input_data) {
                if (index >= data.size())
                    break;

                data.at(index++) = static_cast<internal_type>(value);
            }
        }

        matrix_d(matrix_d<T>& other) = default;
        matrix_d(matrix_d<T>&& other) = default;
        matrix_d<T>& operator=(const matrix_d<T>& other) = default;
        matrix_d<T>& operator=(matrix_d<T>&& other) = default;
        ~matrix_d() = default;

        void add_row(index_type start_row, std::vector<internal_type>&& row_data) {
            auto start_position = std::next(std::begin(data), start_row * columns_count);

            if (row_data.size() > columns_count) {
                row_data.resize(columns_count);
            }

            std::move(std::begin(row_data), std::end(row_data), start_position);
        }

        index_type get_rows_count() const {
            return rows_count;
        }

        index_type get_columns_count() const {
            return columns_count;
        }

        [[nodiscard]] internal_type& operator()(const index_type& row, const index_type& col)
        {
            if (row >= rows_count || col >= columns_count) {
                std::out_of_range("Invalid row or column index");
            }
            return data[row * columns_count + col];
        }

        [[nodiscard]] internal_type operator()(const index_type& row, const index_type& col) const
        {
            if (row >= rows_count || col >= columns_count) {
                std::out_of_range("Invalid row or column index");
            }
            return data[row * columns_count + col];
        }

        [[nodiscard]] internal_type& operator[](const index_type& index)
        {
            return data.at(index);
        }

        [[nodiscard]] matrix_d<T> operator*(const matrix_d<T>& other) const {
            if (rows_count != other.columns_count) {
                throw std::runtime_error("Multiply operation: The conditions of the operation are not met");
            }

            matrix_d<T> result(rows_count, other.columns_count);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < other.columns_count; ++ci) {
                    T dot = static_cast<T>(operator()(ri, 0) * other(0, ci));
                    for (size_t k = 1; k < columns_count; ++k) {
                        dot = utility::add(dot, utility::multiply(operator()(ri, k), other(k, ci)));
                    }
                    result(ri, ci) = dot;
                }
            }

            return result;
        }

        [[nodiscard]] matrix_d<T> multiply_with_threads(const matrix_d<T>& other) const {
            if (rows_count != other.columns_count) {
                throw std::runtime_error("Multiply operation: The conditions of the operation are not met");
            }

            matrix_d<T> result(rows_count, other.columns_count);

            auto multiply_row_adn_col = [&](int row, int col) {
                T sum = 0;
                for (index_type k = 0; k < columns_count; ++k) {
                    sum += operator()(row, k) * other(k, col);
                }
                result(row, col) = sum;
            };

            std::vector<std::thread> threads;

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < other.columns_count; ++ci) {
                    threads.emplace_back(multiply_row_adn_col, ri, ci);
                }
            }

            for (auto& thread : threads) {
                thread.join();
            }

            return result;
        }
        

        [[nodiscard]] matrix_d<T> operator+(const matrix_d<T>& other) const {
            requires_same_size_for_matrices(other);

            matrix_d<T> result(rows_count, columns_count);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::add(operator()(ri, ci), other(ri, ci));
                }
            }

            return result;
        }

        [[nodiscard]] matrix_d<T> operator-(const matrix_d<T>& other) const {
            requires_same_size_for_matrices(other);

            matrix_d<T> result(rows_count, columns_count);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::subtract(operator()(ri, ci), other(ri, ci));
                }
            }

            return result;
        }

        [[nodiscard]] matrix_d<T> operator+(const T& value) const {
            matrix_d<T> result(rows_count, columns_count);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::add(operator()(ri, ci), value);
                }
            }

            return result;
        }

        [[nodiscard]] matrix_d<T> operator-(const T& value) const {
            matrix_d<T> result(rows_count, columns_count);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::subtract(operator()(ri, ci), value);
                }
            }

            return result;
        }

        [[nodiscard]] matrix_d<T> operator*(const T& value) const {
            matrix_d<T> result(rows_count, columns_count);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::multiply(operator()(ri, ci), value);
                }
            }

            return result;
        }

        [[nodiscard]] matrix_d<double> inverse() const  {
            requires_square_matrix();

            matrix_d<double> augmented_matrix(rows_count, columns_count * 2);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    augmented_matrix(ri, ci) = operator()(ri, ci);
                }
                augmented_matrix(ri, ri + rows_count) = 1.0;
            }

            for (index_type ri = 0; ri < rows_count; ++ri) {
                if (augmented_matrix(ri, ri) == 0.0) {
                    throw std::runtime_error("Inverse matrix operation: Invertible matrix");
                }

                double pivot = augmented_matrix(ri, ri);
                if (pivot == 0.0) {
                    throw std::runtime_error("Inverse matrix operation: can't calculate matrix");
                }

                for (index_type ci = 0; ci < augmented_matrix.get_columns_count(); ++ci) {
                    augmented_matrix(ri, ci) = augmented_matrix(ri, ci) / pivot;
                }

                for (index_type k = 0; k < rows_count; ++k) {
                    if (k != ri) {
                        double factor = augmented_matrix(k, ri);

                        for (index_type ci = 0; ci < augmented_matrix.get_columns_count(); ++ci) {
                            augmented_matrix(k, ci) = augmented_matrix(k, ci) - factor * augmented_matrix(ri, ci);
                        }
                    }
                }
            }

            matrix_d<double> result(rows_count, columns_count);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = augmented_matrix(ri, ci + columns_count);
                }
            }

            return result;
        }

        [[nodiscard]] matrix_d<T> submatrix(const index_type& sub_rows, const index_type& sub_cols, const index_type& start_row, const index_type& start_col) const {
            requires_take_submatrix(sub_rows, sub_cols, start_row, start_col);

            matrix_d<T> result(sub_rows, sub_cols);

            for (index_type ri = 0; ri < sub_rows; ++ri) {
                for (index_type ci = 0; ci < sub_cols; ++ci) {
                    result(ri, ci) = operator()(start_row + ri, start_col + ci);
                }
            }
            return result;
        }

        [[nodiscard]] matrix_d<T> transpose() const {
            matrix_d<T> result(rows_count, columns_count);

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ci, ri) = operator()(ri, ci);
                }
            }

            return result;
        }
    };
}