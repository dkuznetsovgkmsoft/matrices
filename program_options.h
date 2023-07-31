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

#include <locale>
#include <iostream>
#include <algorithm>

#include "serializer.h"

#include "boost/program_options.hpp"
namespace matrices::program_options {
    enum class Operation : short {
        Unknown,
        Add,
        Subtract,
        Multiply,
        Traspose,
        Invert,
        Submatrix,
        At
    };

    bool matrix_with_scalar(const std::filesystem::path& result_path, const std::filesystem::path& first_matrix_path, const Operation& operation, const double& scalar) {
        try {
            auto first_matrix = matrices::serialize::from_csv(first_matrix_path);
            matrices::matrix_d<double> result_matrix;
            switch (operation)
            {
            case Operation::Add: {
                result_matrix = first_matrix + scalar;
                break;
            }
            case Operation::Subtract: {
                result_matrix = first_matrix - scalar;
                break;
            }
            case Operation::Multiply: {
                result_matrix = first_matrix * scalar;
                break;
            }
            case Operation::At: {
                result_matrix = matrices::matrix_d<double>(1, 1, { first_matrix[scalar] });
                break;
            }
            default:
                return false;
                break;
            }
            matrices::serialize::to_csv(result_path, result_matrix, ',');
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool submatrix(const std::filesystem::path& result_path, const std::filesystem::path& first_matrix_path,
        const std::pair<std::uint32_t, std::uint32_t>& counts, const std::pair<std::uint32_t, std::uint32_t>& starts) {
        try {
            auto first_matrix = matrices::serialize::from_csv(first_matrix_path);
            matrices::matrix_d<double> result_matrix = first_matrix.submatrix(std::get<0>(counts), std::get<1>(counts), std::get<0>(starts), std::get<1>(starts));
            matrices::serialize::to_csv(result_path, result_matrix, ',');
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool single_matrix(const std::filesystem::path& result_path, const std::filesystem::path& first_matrix_path, const Operation& operation) {
        try {
            auto first_matrix = matrices::serialize::from_csv(first_matrix_path);
            matrices::matrix_d<double> result_matrix;
            switch (operation)
            {
            case Operation::Invert: {
                result_matrix = first_matrix.inverse();
                break;
            }
            case Operation::Traspose: {
                result_matrix = first_matrix.transpose();
                break;
            }
            default:
                return false;
                break;
            }
            matrices::serialize::to_csv(result_path, result_matrix, ',');
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool matrix_with_matrix(const std::filesystem::path& result_path, const std::filesystem::path& first_matrix_path, const std::filesystem::path& second_matrix_path, const Operation& operation) {
        try {
            auto first_matrix = matrices::serialize::from_csv(first_matrix_path);
            auto second_matrix = matrices::serialize::from_csv(second_matrix_path);
            matrices::matrix_d<double> result_matrix;
            switch (operation)
            {
            case Operation::Add: {
                result_matrix = first_matrix + second_matrix;
                break;
            }
            case Operation::Subtract: {
                result_matrix = first_matrix - second_matrix;
                break;
            }
            case Operation::Multiply: {
                result_matrix = first_matrix * second_matrix;
                break;
            }
            default:
                return false;
                break;
            }
            matrices::serialize::to_csv(result_path, result_matrix, ',');
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool process_arguments(const boost::program_options::variables_map& v_maps) {
        std::filesystem::path first_matrix_path, second_matrix_path, result_path;

        first_matrix_path = v_maps["input-matrix"].as<std::string>();
        result_path = v_maps["result-file"].as<std::string>();

        auto operation = v_maps["operation"].as<std::string>();
        std::transform(std::begin(operation), std::end(operation), std::begin(operation), [](unsigned char c) { return std::tolower(c); });

        std::map<std::string, Operation> available_operations{
            {"+", Operation::Add} , {"-", Operation::Subtract},
            {"*", Operation::Multiply} , {"invert", Operation::Invert},
            {"transpose", Operation::Traspose},
            {"submatrix", Operation::Submatrix}, {"at", Operation::At} };

        if (!available_operations.contains(operation)) {
            std::cout << "Matrix with matrix: unknown operation for this type" << std::endl;
            return false;
        }

        if (v_maps.contains("operand-matrix")) {
            second_matrix_path = v_maps["operand-matrix"].as<std::string>();
        }

        double scalar_value{ 0.0 };
        if (v_maps.contains("scalar-value")) {
            scalar_value = v_maps["scalar-value"].as<double>();
        }

        auto operation_v = available_operations[operation];
        if (second_matrix_path.empty()) {
            switch (operation_v) {
            case Operation::Submatrix: {
                auto counts = std::make_pair(v_maps["row"].as<std::uint32_t>(), v_maps["column"].as<std::uint32_t>());
                auto starts = std::make_pair(v_maps["start-row"].as<std::uint32_t>(), v_maps["start-column"].as<std::uint32_t>());
                return submatrix(result_path, first_matrix_path, counts, starts);
            }
            case Operation::Invert:
            case Operation::Traspose: {
                return single_matrix(result_path, first_matrix_path, operation_v);
            }
            default:
            case Operation::At: {
                auto [row_index, column_index] = std::make_pair(v_maps["row"].as<std::uint32_t>(), v_maps["column"].as<std::uint32_t>());
                return submatrix(result_path, first_matrix_path, { 1, 1 }, { row_index , column_index });
            }
            }
        }
        else {
            return matrix_with_matrix(result_path, first_matrix_path, second_matrix_path, operation_v);
        }

        return false;
    }

    void print_help(const boost::program_options::options_description& options)
    {
        std::cout << "Matrices Task :)\n";
        std::cout << "=================================================\n";
        std::cout << "\nUsage:\n\n";
        options.print(std::cout);

        std::cout << "\nSupported operations:\n";
        std::cout << "\tMatrix with matrix:\n";
        std::cout << "\t\t Multiplication (operation command: *)\n";
        std::cout << "\t\t Addition (operation command: /)\n";
        std::cout << "\t\t Subtraction (operation command: -)\n";
        std::cout << "\tMatrix with Scalar:\n";
        std::cout << "\t\t Multiplication (operation command: *)\n";
        std::cout << "\t\t Addition (operation command: /)\n";
        std::cout << "\t\t Subtraction (operation command: -)\n";
        std::cout << "\tSingle matrix: \n";
        std::cout << "\t\tTranspose\t(operation command: transpose)\n";
        std::cout << "\t\tInvert\t(operation command: invert)\n";
        std::cout << "\t\tTaking an element by index.\t(operation command: at)\n";
    }

    bool parse_command_line(int argc, char** argv, boost::program_options::options_description& options, boost::program_options::variables_map& v_maps) {
        setlocale(LC_ALL, ".65001");
        std::string task_type;
        options.add_options()
            ("help", "produce help message")
            ("input-matrix,I", boost::program_options::value<std::string>()->required(), "Input file name for the first matrix")
            ("operand-matrix,M", boost::program_options::value<std::string>(), "Input file name for the second matrix")
            ("operation,O", boost::program_options::value < std::string>()->required(), "operation which we should call")
            ("scalar-value,S", boost::program_options::value<double>()->default_value({ 1.0 }), "scalar for the operaiton")
            ("result-file,R", boost::program_options::value<std::string>()->default_value({ "result.csv" }), "output file path for result");

        boost::program_options::options_description take_submatrix("\"Submatrix take\" and \"Taking an element by index\" arguments");
        take_submatrix.add_options()
            ("row", boost::program_options::value<std::uint32_t>()->default_value({ 1 }), "count of rows (submatrix) or index")
            ("column", boost::program_options::value<std::uint32_t>()->default_value({ 1 }), "count of columns (submatrix) or index")
            ("start-row", boost::program_options::value<std::uint32_t>()->default_value({ 0 }), "start row position")
            ("start-column", boost::program_options::value<std::uint32_t>()->default_value({ 0 }), "start column position");

        options.add(take_submatrix);
        try {
            boost::program_options::command_line_parser parser{ argc, argv };
            parser.options(options);
            auto parsed = parser.allow_unregistered().run();

            boost::program_options::store(parsed, v_maps);
            boost::program_options::notify(v_maps);

            boost::program_options::store(boost::program_options::parse_command_line(argc, argv, options), v_maps);
            return process_arguments(v_maps);

        }
        catch (std::exception& ex) {
            print_help(options);
        }
        return false;
    }
}
