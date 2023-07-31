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

namespace utility {
    template<typename T>
    concept Scalar = std::is_arithmetic_v<T>;

    template<typename T>
    concept is_matrix = requires (T & value) {
        typename T::internal_type;
        T::rows_count;
        T::columns_count;
        T::size;
    };

    template<typename T, typename U> requires (std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>)
        [[nodiscard]] inline std::common_type_t<T, U> multiply(const T& a, const U& b) {
        using result_type = std::common_type_t<T, U>;

        if constexpr (std::numeric_limits<result_type>::is_integer) {
            if (a == 0 || b == 0) {
                return { 0 };
            }

            std::int64_t result = static_cast<std::int64_t>(a) * static_cast<std::int64_t>(b);
            if (result > static_cast<std::int64_t>(std::numeric_limits<T>::max()) ||
                result < static_cast<std::int64_t>(std::numeric_limits<T>::min())) {
                throw std::overflow_error("Multiply operation: Unable to multiply values(overflow value)");
            }

            return static_cast<T>(result);
        }
        else {
            T result = a * b;

            /*if (std::isinf(result) || std::isnan(result)) {
                throw std::overflow_error("Multiply operation: Unable to add values(overflow value)");
            }*/

            return result;
        }
    }

    template<typename T, typename U> requires (std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>)
        [[nodiscard]] inline std::common_type_t<T, U> add(const T& a, const U& b) {
        using result_type = std::common_type_t<T, U>;

        if constexpr (std::numeric_limits<result_type>::is_integer) {
            std::int64_t result = static_cast<std::int64_t>(a) + static_cast<std::int64_t>(b);
            if (result > static_cast<std::int64_t>(std::numeric_limits<T>::max()) ||
                result < static_cast<std::int64_t>(std::numeric_limits<T>::min())) {
                throw std::overflow_error("Add operation: Unable to add values(overflow value)");
            }

            return static_cast<T>(result);
        }
        else {
            T result = a + b;

            /*if (std::isinf(result) || std::isnan(result)) {
                throw std::overflow_error("Add operation: Unable to add values(overflow value)");
            }*/

            return result;
        }
    }

    template<typename T, typename U> requires (std::is_arithmetic_v<T>&& std::is_arithmetic_v<U>)
        [[nodiscard]] inline std::common_type_t<T, U> subtract(const T& a, const U& b) {
        using result_type = std::common_type_t<T, U>;

        if constexpr (std::numeric_limits<result_type>::is_integer) {
            std::int64_t result = static_cast<std::int64_t>(a) - static_cast<std::int64_t>(b);
            if (result > static_cast<std::int64_t>(std::numeric_limits<T>::max()) ||
                result < static_cast<std::int64_t>(std::numeric_limits<T>::min())) {
                throw std::overflow_error("Subtract operation: Unable to subtract values(overflow value)");
            }

            return static_cast<T>(result);
        }
        else {
            T result = a - b;

            /*if (std::isinf(result) || std::isnan(result)) {
                throw std::overflow_error("Subtract operation: Unable to subtract values(overflow value)");
            }*/

            return result;
        }
    };
}