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

#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <format>

#include "matrices.h"

namespace matrices::serialize {
    template<typename Matrix>
    concept is_matrix = requires(Matrix value) {
        value.get_columns_count();
        value.get_rows_count();
        value(0, 0);
    };

    template<is_matrix Matrix>
    void to_csv(const std::filesystem::path& input_file, const Matrix& matrix, const char delim = ',') {
        std::ofstream file(input_file);

        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file for writting");
        }

        for (std::uint32_t ri = 0; ri < matrix.get_rows_count(); ++ri) {
            for (std::uint32_t ci = 0; ci < matrix.get_columns_count(); ++ci) {
                file << matrix(ri, ci);

                if (ci < matrix.get_columns_count() - 1)
                    file << delim;
            }
            file << "\n";
        }

        file.close();
        std::cout << std::format("Exported to {}\n", input_file.string());
    }

    auto from_csv(const std::filesystem::path& input_file) {
        if (!std::filesystem::exists(input_file) || !std::filesystem::is_regular_file(input_file)) {
            throw std::runtime_error("Unable to open file for reading");
        }

        std::ifstream in(input_file.c_str());
        if (!in.good() || !in.is_open()) {
            throw std::runtime_error("Unable to open file for reading");
        }

        std::uint32_t num_rows{ 0 }, num_columns{ 0 };
        std::vector<std::vector<double>> data;

        std::string line;
        while (std::getline(in, line)) {
            std::stringstream ss(line);

            double value{ 0 };
            std::vector<double> row_data;
            row_data.reserve(num_columns);

            std::uint32_t columns_counter{ 0 };
            for (std::string token{}; std::getline(ss, token, ','); ++columns_counter) {
                std::istringstream(token) >> value;
                row_data.push_back(value);
            }

            num_columns = std::max(num_columns, columns_counter);
            data.push_back(std::move(row_data));
        }

        num_rows = data.size();
        matrices::matrix_d<double> result(num_rows, num_columns);

        for (std::uint32_t row_index{ 0 }; auto & row : data) {
            result.add_row(row_index++, std::move(row));
        }

        std::cout << std::format("Loaded from {}\n", input_file.string());
        return result;
    }
}
