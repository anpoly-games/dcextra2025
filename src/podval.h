#pragma once

template<typename T>
struct PodVal
{
  T val = 0;

  operator T() { return val; }
  T& operator=(const T& n) { val = n; };
  T& operator+=(const T& rhs) { val += rhs; return val; };
  T& operator-=(const T& rhs) { val -= rhs; return val; };
};

template<typename T>
inline bool operator<(const PodVal<T>& lhs, const T& rhs) { return lhs.val < rhs; }
template<typename T>
inline bool operator<(const T& lhs, const PodVal<T>& rhs) { return lhs < rhs.val; }
template<typename T>
inline bool operator<(const PodVal<T>& lhs, const PodVal<T>& rhs) { return lhs.val < rhs.val; }
template<typename T>
inline bool operator>(const PodVal<T>& lhs, const PodVal<T>& rhs) { return lhs.val > rhs.val; }

template<typename T>
inline T operator+(const PodVal<T>& lhs, const T& rhs) { return lhs.val + rhs; }
template<typename T>
inline T operator+(const T& lhs, const PodVal<T>& rhs) { return lhs + rhs.val; }

using FVal = PodVal<float>;
using BVal = PodVal<bool>;
using IVal = PodVal<int>;


