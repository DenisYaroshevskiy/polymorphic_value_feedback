/*
 * Copyright 2019, Denis Yaroshevskiy
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// Implementation of a wg21.link/p0201 (roughly) using std::function, which turns out to be quite simple.
// We can probably do better - like collapse virtual dispatch into the virtual dispatch of the class.

#ifndef POLYMORPHIC_VALUE_SIMPLE_H
#define POLYMORPHIC_VALUE_SIMPLE_H

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

// _ => private

namespace simple {
namespace stdx {

struct _default_copier {
  template <typename T>
  T* operator()(const T& x) const {
    return new T(x);
  }
};

template <typename A, typename T, typename C, typename D>
struct _user_pointer_impl : C {
  std::unique_ptr<T, D> ptr_;

  _user_pointer_impl(std::unique_ptr<T, D> ptr, C c)
      : C(std::move(c)), ptr_(std::move(ptr)) {}

  _user_pointer_impl(const _user_pointer_impl& x)
      : C{x}, ptr_{x.C::operator()(*x.ptr_), x.ptr_.get_deleter()}  {}
  _user_pointer_impl(_user_pointer_impl&&) = default;

  _user_pointer_impl& operator=(const _user_pointer_impl& x) {
    auto tmp = x;
    return *this = std::move(x);
  }
  _user_pointer_impl& operator=(_user_pointer_impl&&) = default;

  ~_user_pointer_impl() = default;

  A* operator()() const {
    return static_cast<A*>(ptr_.get());
  }
};

template <typename A>
class polymorphic_value {
  template <typename UA>
  friend class polymorphic_value;

  using store_t = std::function<A*()>;
  store_t store_;

  explicit polymorphic_value(const store_t& store) : store_(store) {}
  explicit polymorphic_value(store_t&& store) : store_(std::move(store)) {}

  template <typename T, typename C, typename D>
  polymorphic_value(std::unique_ptr<T, D> ptr, C c) : store_{_user_pointer_impl<A, T, C, D>{std::move(ptr), std::move(c)}} {}

  A* get() { return store_(); }
  const A* get() const { return store_(); }

  template <typename T>
  using enable_conversion_t = std::enable_if_t<std::is_convertible<T*, A*>::value>;

  template <typename UA>
  using enable_value_conversion_t =
      std::enable_if_t<!std::is_same<UA*, A*>::value &&
                       std::is_convertible<UA*, A*>::value>;

 public:
  polymorphic_value() = default;

  template <typename UA, typename = enable_value_conversion_t<UA>>
  explicit polymorphic_value(const polymorphic_value<UA>& x)
      : store_{x.store_} {}

  template <typename UA, typename = enable_value_conversion_t<UA>>
  explicit polymorphic_value(polymorphic_value<UA>&& x)
      : store_(std::move(x.store_)) {}

  template <typename T, typename C, typename D, typename = enable_conversion_t<T>>
  explicit polymorphic_value(T* ptr, C c, D d)
      : polymorphic_value{std::unique_ptr<T, D>{ptr, std::move(d)}, std::move(c)} {}

  template <typename T, typename C, typename = enable_conversion_t<T>>
  explicit polymorphic_value(T* x, C c)
      : polymorphic_value{std::unique_ptr<T>{x}, c} {}

  template <typename T, typename = enable_conversion_t<T>>
  explicit polymorphic_value(T* x)
      : polymorphic_value{std::unique_ptr<T>{x}, _default_copier{}} {}

  template <typename T, typename = enable_conversion_t<std::decay_t<T>>>
  explicit polymorphic_value(T&& x) : polymorphic_value{_make<std::decay_t<T>>(std::forward<T>(x))} {}

  void swap(polymorphic_value& x) { std::swap(*this, x); }

  explicit operator bool() const { return static_cast<bool>(store_); }

  A* operator->() { return get(); }
  const A* operator->() const { return get(); }

  A& operator*() { return *get(); }
  const A& operator*() const { return *get(); }

  template <typename T, typename ...Args>
  static polymorphic_value _make(Args&&... args) {
      return polymorphic_value{[x = T{std::forward<Args>(args)...}]() mutable -> A* {
        return &x;
      }};
  }
};

template <typename T, typename ...Args>
polymorphic_value<T> make_polymorphic_value(Args&& ... args) {
    return polymorphic_value<T>::template _make<T>(std::forward<Args>(args)...);
}

// TODO sfinae.
template <typename A, typename T, typename ...Args>
polymorphic_value<A> make_polymorphic_value(Args&& ... args) {
    return polymorphic_value<A>::template _make<T>(std::forward<Args>(args)...);
}

}  // namespace stdx
}  // namespace simple

#endif  // POLYMORPHIC_VALUE_SIMPLE_H