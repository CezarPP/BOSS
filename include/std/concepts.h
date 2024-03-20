/*
 * concepts.h
 *
 *  Created on: 3/20/24.
 *      Author: Cezar PP
 */
#pragma once

namespace Std {
    template<typename T, typename U>
    inline constexpr bool IsSame = false;

    template<typename T>
    inline constexpr bool IsSame<T, T> = true;

    template<typename T, typename U>
    concept SameAs = IsSame<T, U>;
}