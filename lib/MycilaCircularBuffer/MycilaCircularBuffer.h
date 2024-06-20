// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <stddef.h>

namespace Mycila {
  template <typename T, int N>
  class CircularBuffer {
    public:
      CircularBuffer(T typeMin, T typeMax) { reset(typeMin, typeMax); }

      size_t count() const { return _count; };
      T avg() const { return _count == 0 ? 0 : _sum / _count; }
      T last() const { return _last; }
      T sum() const { return _sum; }
      T max() const { return _max; }
      T min() const { return _min; }

      T add(T value) {
        T current = _buffer[_index];
        _buffer[_index++] = value;
        _last = value;
        if (value > _max)
          _max = value;
        if (value < _min)
          _min = value;
        _sum += value;
        _sum -= current;
        if (_index == N)
          _index = 0;
        if (_count < N)
          _count++;
        return current;
      };

      void reset(T typeMin, T typeMax) {
        _sum = 0;
        _last = 0;
        _min = typeMax;
        _max = typeMin;
        _index = 0;
        _count = 0;
        for (int i = 0; i < N; i++)
          _buffer[i] = 0;
      }

    private:
      T _buffer[N];
      T _sum;
      T _last;
      T _min;
      T _max;
      size_t _index = 0;
      size_t _count = 0;
  };

} // namespace Mycila
