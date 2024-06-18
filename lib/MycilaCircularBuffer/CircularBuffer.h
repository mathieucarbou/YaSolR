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
      CircularBuffer() { reset(); }

      size_t count() const { return _count; };
      T avg() const { return _count == 0 ? 0 : _sum / _count; }
      T last() const { return _last; }
      T sum() const { return _sum; }

      T add(T value) {
        T current = _buffer[_index];
        _buffer[_index++] = value;
        _last = value;
        _sum += value;
        _sum -= current;
        if (_index == N)
          _index = 0;
        if (_count < N)
          _count++;
        return current;
      };

      void reset() {
        _count = 0;
        _index = 0;
        _last = 0;
        _sum = 0;
        for (int i = 0; i < N; i++)
          _buffer[i] = 0;
      }

    private:
      T _buffer[N];
      T _sum;
      T _last;
      size_t _index = 0;
      size_t _count = 0;
  };

} // namespace Mycila
