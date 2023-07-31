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

#include <array>
#include <type_traits>

#include "utility.h"

namespace matrices {
    template<typename T, typename U>
    concept is_multiplicable = utility::is_matrix<T> && utility::is_matrix<U> && (T::rows_count == U::columns_count);

    template<typename T>
    concept is_square = utility::is_matrix<T> && (T::columns_count == T::rows_count);

    template<typename T, typename U>
    concept is_same_dimensions = utility::is_matrix<T> && utility::is_matrix<U> && (T::rows_count == U::rows_count && T::columns_count == U::columns_count);

    template<typename T, typename ...Args>
    concept is_same_size_init_list = utility::is_matrix<T> && ((T::rows_count * T::columns_count) == sizeof...(Args));

    template<typename T, std::uint32_t SubRows, std::uint32_t SubColumns, std::uint32_t StartRow, std::uint32_t StartColumn>
    concept is_valid_taking_submatrix = utility::is_matrix<T> && (SubRows > 0 && (StartRow + SubRows) <= T::rows_count && SubColumns > 0 && (StartColumn + SubColumns) <= T::columns_count);

    template<utility::Scalar T, std::uint32_t Rows, std::uint32_t Columns = Rows>
    requires (Rows > 0 && Columns > 0)
    class matrix_f final {
    public:
        using internal_type = T;
        using index_type = std::uint32_t;

        static constexpr index_type size = Rows * Columns;
        static constexpr index_type rows_count = Rows;
        static constexpr index_type columns_count = Columns;

        std::array<internal_type, size> data{};
    private:
        void make_identity() {
            auto min_dim = std::min(rows_count, columns_count);
            for (index_type ri = 0; ri < min_dim; ++ri) {
                operator()(ri, ri) = { 1 };
            }
        }

        template<utility::is_matrix U>
        [[nodiscard]] matrix_f<std::common_type_t<internal_type, typename U::internal_type>, U::rows_count - 1, U::columns_count - 1> get_minor(const U& matrix, const index_type& row, const index_type& col) const {
            using result_type = std::common_type_t<internal_type, typename U::internal_type>;
            matrix_f<result_type, U::rows_count - 1, U::columns_count - 1> result{};

            for (index_type ri = 0; ri < U::rows_count; ++ri) {
                for (index_type ci = 0; ci < U::columns_count; ++ci) {
                    if (ri != row && ci != col) {
                        index_type m_row = (ri < row) ? ri : ri - 1;
                        index_type m_col = (ci < col) ? ci : ci - 1;

                        result(m_row, m_col) = matrix(ri, ci);
                    }
                }
            }

            return result;
        }

        template<is_square U>
        [[nodiscard]] double determinant(const U& matrix) const {
            if constexpr (U::rows_count == 1) {
                return matrix(0, 0);
            }
            else {
                double result = 0;
                int sign = 1;

                for (index_type ci = 0; ci < U::columns_count; ++ci) {
                    auto minor = get_minor(matrix, 0, ci);
                    result += sign * matrix(0, ci) * determinant(minor);
                    sign *= -1;
                }

                return result;
            }
        }
    public:
        matrix_f() {
            make_identity();
        }

        matrix_f(std::vector<internal_type>&& input_data) {
            if (data.size() == input_data) {
                std::move(begin(input_data), end(input_data), begin(data));
            }
        }

        template<utility::Scalar ...Args> requires is_same_size_init_list<matrix_f, Args...>
        matrix_f(Args&&... args) {
            std::initializer_list input_data = { static_cast<internal_type> (args)... };

            index_type index{ 0 };
            for (const auto& value : input_data) {
                data.at(index++) = static_cast<internal_type>(value);
            }
        }

        matrix_f(matrix_f<T, Rows, Columns>& other) = default;
        matrix_f(matrix_f<T, Rows, Columns>&& other) = default;
        matrix_f<T, Rows, Columns>& operator=(const matrix_f<T, Rows, Columns>& other) = default;
        matrix_f<T, Rows, Columns>& operator=(matrix_f<T, Rows, Columns>&& other) = default;
        ~matrix_f() = default;

        index_type get_rows_count() const {
            return rows_count;
        }

        index_type get_columns_count() const {
            return columns_count;
        }

        [[nodiscard]] internal_type& operator()(const index_type& row, const index_type& col)
        {
            if (row >= rows_count || col >= columns_count) {
                std::out_of_range("Invalid row/column index");
            }
            return data.at(row * Columns + col);
        }

        [[nodiscard]] internal_type operator()(const index_type& row, const index_type& col) const
        {
            if (row >= rows_count || col >= columns_count) {
                std::out_of_range("Invalid row/column index");
            }
            return data.at(row * Columns + col);
        }

        [[nodiscard]] internal_type& operator[](const index_type& index)
        {
            return data.at(index);
        }

        template<typename U> requires is_multiplicable<matrix_f, U>
        [[nodiscard]] matrix_f<std::common_type_t<internal_type, typename U::internal_type>, rows_count, U::columns_count> operator*(const U& other) const {
            using result_type = std::common_type_t<internal_type, typename U::internal_type>;
            matrix_f<result_type, rows_count, U::columns_count> result{};

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < U::columns_count; ++ci) {
                    result_type dot = utility::multiply(operator()(ri, 0), other(0, ci));
                    for (size_t k = 1; k < Columns; ++k) {
                        dot = utility::add(dot, utility::multiply(operator()(ri, k), other(k, ci)));
                    }
                    result(ri, ci) = dot;
                }
            }

            return result;
        }

        template<typename U> requires is_multiplicable<matrix_f, U>
        [[nodiscard]] matrix_f<std::common_type_t<internal_type, typename U::internal_type>, rows_count, U::columns_count> multiply_with_threads(const U& other) const {
            using result_type = std::common_type_t<internal_type, typename U::internal_type>;
            matrix_f<result_type, rows_count, U::columns_count> result{};

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

        template<typename U> requires is_same_dimensions<matrix_f, U>
        [[nodiscard]] matrix_f<std::common_type_t<internal_type, typename U::internal_type>, rows_count, columns_count> operator+(const U& other) const {
            using result_type = std::common_type_t<internal_type, typename U::internal_type>;
            matrix_f<result_type, rows_count, columns_count> result{};

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::add(operator()(ri, ci), other(ri, ci));
                }
            }

            return result;
        }

        template<typename U> requires is_same_dimensions<matrix_f, U>
        [[nodiscard]] matrix_f<std::common_type_t<internal_type, typename U::internal_type>, rows_count, columns_count> operator-(const U& other) const {
            using result_type = std::common_type_t<internal_type, typename U::internal_type>;
            matrix_f<result_type, rows_count, columns_count> result{};

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::subtract(operator()(ri, ci), other(ri, ci));
                }
            }

            return result;
        }

        template <utility::Scalar U>
        [[nodiscard]] matrix_f<std::common_type_t<internal_type, U>, rows_count, columns_count> operator+(const U& value) const {
            using result_type = std::common_type_t<internal_type, U>;
            matrix_f<T, rows_count, columns_count> result{};

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::add(operator()(ri, ci), value);
                }
            }

            return result;
        }

        template <utility::Scalar U>
        [[nodiscard]] matrix_f<std::common_type_t<internal_type, U>, rows_count, columns_count> operator-(const U& value) const {
            using result_type = std::common_type_t<internal_type, U>;
            matrix_f<T, rows_count, columns_count> result{};

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::subtract(operator()(ri, ci), value);
                }
            }

            return result;
        }

        template <utility::Scalar U>
        [[nodiscard]] matrix_f<std::common_type_t<internal_type, U>, rows_count, columns_count> operator*(const U& value) const {
            using result_type = std::common_type_t<internal_type, U>;
            matrix_f<T, rows_count, columns_count> result{};

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = utility::multiply(operator()(ri, ci), value);
                }
            }

            return result;
        }

        [[nodiscard]] matrix_f<double, rows_count, columns_count> inverse_2() const requires is_square<matrix_f>{
            matrix_f<double, rows_count, columns_count * 2> augmented_matrix{};

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

                for (index_type ci = 0; ci < augmented_matrix.columns_count; ++ci) {
                    augmented_matrix(ri, ci) = augmented_matrix(ri, ci) / pivot;
                }

                for (index_type k = 0; k < rows_count; ++k) {
                    if (k != ri) {
                        double factor = augmented_matrix(k, ri);

                        for (index_type ci = 0; ci < augmented_matrix.columns_count; ++ci) {
                            augmented_matrix(k, ci) = augmented_matrix(k, ci) - factor * augmented_matrix(ri, ci);
                        }
                    }
                }
            }

            matrix_f<double, rows_count, columns_count> result;

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ri, ci) = augmented_matrix(ri, ci + Columns);
                }
            }

            return result;
        }

        [[nodiscard]] matrix_f<double, rows_count, columns_count> inverse_1() const requires is_square<matrix_f> {
            matrix_f<double, rows_count, columns_count> result{};

            double det = determinant(*this);

            if (det == 0) {
                throw std::invalid_argument("Inverse matrix calculation error");
            }

            if constexpr (rows_count == 1) {
                result(0, 0) = 1.0 / static_cast<double>(det);
            }
            else {
                for (index_type ri = 0; ri < rows_count; ++ri) {
                    for (index_type ci = 0; ci < columns_count; ++ci) {
                        auto minor = get_minor(*this, ri, ci);
                        auto tmp = (determinant(minor) / static_cast<double>(det)) * (((ri + ci) % 2) ? -1 : 1);
                        result(ci, ri) = tmp;
                    }
                }
            }

            return result;
        }

        template<index_type SubRows, index_type SubColumns, index_type StartRow, index_type StartColumn> requires is_valid_taking_submatrix<matrix_f, SubRows, SubColumns, StartRow, StartColumn>
        [[nodiscard]] matrix_f<T, SubRows, SubColumns> submatrix() const {
            matrix_f<T, SubRows, SubColumns> result;

            for (index_type ri = 0; ri < SubRows; ++ri) {
                for (index_type ci = 0; ci < SubColumns; ++ci) {
                    result(ri, ci) = operator()(StartRow + ri, StartColumn + ci);
                }
            }
            return result;
        }

        [[nodiscard]] matrix_f<T, columns_count, rows_count> transpose() const {
            matrix_f<T, columns_count, rows_count> result{};

            for (index_type ri = 0; ri < rows_count; ++ri) {
                for (index_type ci = 0; ci < columns_count; ++ci) {
                    result(ci, ri) = operator()(ri, ci);
                }
            }

            return result;
        }
    };
}