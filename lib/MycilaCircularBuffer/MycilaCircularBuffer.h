// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include <stddef.h>

#include <functional>
#include <limits>

namespace Mycila {
  template <typename T, size_t N>
  class CircularBuffer {
    public:
      CircularBuffer() { reset(); }

      T const& operator[](size_t index) const { return _buffer[(_index + index) % N]; }

      size_t count() const { return _count; };
      T avg() const { return _count == 0 ? 0 : _sum / _count; }
      T first() const { return _buffer[_index]; }
      T last() const { return _buffer[_index == 0 ? N - 1 : _index - 1]; }
      T sum() const { return _sum; }
      T max() const { return _max; }
      T min() const { return _min; }
      T rate() const {
        T diff = last() - first();
        return diff == 0 ? 0 : _count / diff;
      }

      T add(T value) {
        T current = _buffer[_index];
        _buffer[_index++] = value;
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

      void copy(T* dest) const { // NOLINT (build/include_what_you_use)
        const size_t start = _index;
        for (size_t i = 0; i < N; i++)
          dest[i] = _buffer[(start + i) % N];
      }

      void reset() {
        _sum = 0;
        _min = std::numeric_limits<T>::max();
        _max = std::numeric_limits<T>::min();
        _index = 0;
        _count = 0;
        for (int i = 0; i < N; i++)
          _buffer[i] = 0;
      }

      void dump(Print& printer) {
        printer.print(F("CircularBuffer("));
        printer.print(N);
        printer.print(F(")={"));
        for (size_t i = 0; i < N; i++) {
          printer.print(_buffer[i]);
          if (i < N - 1)
            printer.print(F(","));
        }
        printer.print(F("}"));
      }

    private:
      T _buffer[N];
      T _sum;
      T _min;
      T _max;
      size_t _index = 0;
      size_t _count = 0;
  };

} // namespace Mycila
