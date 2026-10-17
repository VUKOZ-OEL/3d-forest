#pragma once
#include "core.h"
template <class T> class PortablePoint {
  T x_ = 0, y_ = 0;

public:
  PortablePoint() = default;
  PortablePoint(T x, T y) : x_(x), y_(y) {}
  template <class U>
  PortablePoint(const PortablePoint<U> &p)
      : x_(static_cast<T>(p.x())), y_(static_cast<T>(p.y())) {}
  T x() const { return x_; }
  T y() const { return y_; }
  T &rx() { return x_; }
  T &ry() { return y_; }
  void setX(T x) { x_ = x; }
  void setY(T y) { y_ = y; }
  bool isNull() const { return x_ == 0 && y_ == 0; }
  T manhattanLength() const { return std::abs(x_) + std::abs(y_); }
  PortablePoint &operator+=(const PortablePoint &p) {
    x_ += p.x_;
    y_ += p.y_;
    return *this;
  }
  PortablePoint &operator-=(const PortablePoint &p) {
    x_ -= p.x_;
    y_ -= p.y_;
    return *this;
  }
  PortablePoint &operator*=(double d) {
    x_ = static_cast<T>(x_ * d);
    y_ = static_cast<T>(y_ * d);
    return *this;
  }
  PortablePoint &operator/=(double d) { return *this *= 1 / d; }
  friend PortablePoint operator+(PortablePoint a, const PortablePoint &b) {
    return a += b;
  }
  friend PortablePoint operator-(PortablePoint a, const PortablePoint &b) {
    return a -= b;
  }
  friend PortablePoint operator-(PortablePoint a) { return {-a.x_, -a.y_}; }
  friend PortablePoint operator*(PortablePoint a, double d) { return a *= d; }
  friend PortablePoint operator*(double d, PortablePoint a) { return a *= d; }
  friend PortablePoint operator/(PortablePoint a, double d) { return a /= d; }
  friend bool operator==(const PortablePoint &a, const PortablePoint &b) {
    return a.x_ == b.x_ && a.y_ == b.y_;
  }
  friend bool operator<(const PortablePoint &a, const PortablePoint &b) {
    return std::tie(a.x_, a.y_) < std::tie(b.x_, b.y_);
  }
  friend bool operator!=(const PortablePoint &a, const PortablePoint &b) {
    return !(a == b);
  }
  static double dotProduct(const PortablePoint &a, const PortablePoint &b) {
    return a.x_ * b.x_ + a.y_ * b.y_;
  }
};
using QPoint = PortablePoint<int>;
using QPointF = PortablePoint<double>;
template <class T>
std::ostream &operator<<(std::ostream &s, const PortablePoint<T> &p) {
  return s << '(' << p.x() << ',' << p.y() << ')';
}
template <class T> class PortableSize {
  T w_ = 0, h_ = 0;

public:
  PortableSize() = default;
  PortableSize(T w, T h) : w_(w), h_(h) {}
  T width() const { return w_; }
  T height() const { return h_; }
};
using QSize = PortableSize<int>;
using QSizeF = PortableSize<double>;
template <class T> class PortableRect {
  T x_ = 0, y_ = 0, w_ = 0, h_ = 0;
  static constexpr T inc = std::is_integral_v<T> ? 1 : 0;

public:
  PortableRect() = default;
  PortableRect(T x, T y, T w, T h) : x_(x), y_(y), w_(w), h_(h) {}
  PortableRect(PortablePoint<T> a, PortablePoint<T> b)
      : PortableRect(a.x(), a.y(), b.x() - a.x() + inc, b.y() - a.y() + inc) {}
  PortableRect(PortablePoint<T> p, PortableSize<T> s)
      : PortableRect(p.x(), p.y(), s.width(), s.height()) {}
  template <class U>
  PortableRect(const PortableRect<U> &r)
      : PortableRect(T(r.x()), T(r.y()), T(r.width()), T(r.height())) {}
  T x() const { return x_; }
  T y() const { return y_; }
  T left() const { return x_; }
  T top() const { return y_; }
  T right() const { return x_ + w_ - inc; }
  T bottom() const { return y_ + h_ - inc; }
  T width() const { return w_; }
  T height() const { return h_; }
  PortablePoint<T> topLeft() const { return {left(), top()}; }
  PortablePoint<T> topRight() const { return {right(), top()}; }
  PortablePoint<T> bottomLeft() const { return {left(), bottom()}; }
  PortablePoint<T> bottomRight() const { return {right(), bottom()}; }
  PortablePoint<T> center() const { return {x_ + w_ / 2, y_ + h_ / 2}; }
  PortableSize<T> size() const { return {w_, h_}; }
  bool isNull() const { return w_ == 0 && h_ == 0; }
  bool isEmpty() const { return w_ <= 0 || h_ <= 0; }
  bool isValid() const { return w_ > 0 && h_ > 0; }
  void setWidth(T w) { w_ = w; }
  void setHeight(T h) { h_ = h; }
  void setLeft(T x) {
    w_ += x_ - x;
    x_ = x;
  }
  void setTop(T y) {
    h_ += y_ - y;
    y_ = y;
  }
  void setRight(T x) { w_ = x - x_ + inc; }
  void setBottom(T y) { h_ = y - y_ + inc; }
  void setTopLeft(PortablePoint<T> p) {
    setLeft(p.x());
    setTop(p.y());
  }
  void setBottomRight(PortablePoint<T> p) {
    setRight(p.x());
    setBottom(p.y());
  }
  void setBottomLeft(PortablePoint<T> p) {
    setLeft(p.x());
    setBottom(p.y());
  }
  void setTopRight(PortablePoint<T> p) {
    setRight(p.x());
    setTop(p.y());
  }
  void moveTo(T x, T y) {
    x_ = x;
    y_ = y;
  }
  void moveTo(PortablePoint<T> p) { moveTo(p.x(), p.y()); }
  void moveCenter(PortablePoint<T> p) {
    x_ = p.x() - w_ / 2;
    y_ = p.y() - h_ / 2;
  }
  void translate(T x, T y) {
    x_ += x;
    y_ += y;
  }
  void translate(PortablePoint<T> p) { translate(p.x(), p.y()); }
  PortableRect translated(PortablePoint<T> p) const {
    return translated(p.x(), p.y());
  }
  void setCoords(T l, T t, T r, T b) {
    x_ = l;
    y_ = t;
    w_ = r - l + inc;
    h_ = b - t + inc;
  }
  PortableRect &operator&=(const PortableRect &r) {
    *this = intersected(r);
    return *this;
  }
  PortableRect translated(T x, T y) const {
    auto r = *this;
    r.translate(x, y);
    return r;
  }
  void adjust(T l, T t, T r, T b) {
    x_ += l;
    y_ += t;
    w_ += r - l;
    h_ += b - t;
  }
  PortableRect adjusted(T l, T t, T r, T b) const {
    auto v = *this;
    v.adjust(l, t, r, b);
    return v;
  }
  bool contains(PortablePoint<T> p) const {
    return p.x() >= left() && p.x() <= right() && p.y() >= top() &&
           p.y() <= bottom();
  }
  bool contains(T x, T y) const { return contains({x, y}); }
  bool contains(const PortableRect &r) const {
    return contains(r.topLeft()) && contains(r.bottomRight());
  }
  bool intersects(const PortableRect &r) const {
    return std::max(left(), r.left()) <= std::min(right(), r.right()) &&
           std::max(top(), r.top()) <= std::min(bottom(), r.bottom());
  }
  PortableRect intersected(const PortableRect &r) const {
    if (!intersects(r))
      return {};
    return PortableRect(
        PortablePoint<T>(std::max(left(), r.left()), std::max(top(), r.top())),
        PortablePoint<T>(std::min(right(), r.right()),
                         std::min(bottom(), r.bottom())));
  }
  PortableRect united(const PortableRect &r) const {
    if (isEmpty())
      return r;
    if (r.isEmpty())
      return *this;
    return PortableRect(
        PortablePoint<T>(std::min(left(), r.left()), std::min(top(), r.top())),
        PortablePoint<T>(std::max(right(), r.right()),
                         std::max(bottom(), r.bottom())));
  }
  PortableRect normalized() const {
    auto r = *this;
    if (w_ < 0) {
      r.x_ += w_;
      r.w_ = -w_;
    }
    if (h_ < 0) {
      r.y_ += h_;
      r.h_ = -h_;
    }
    return r;
  }
  PortableRect<int> toRect() const {
    return {qRound(x_), qRound(y_), qRound(w_), qRound(h_)};
  }
  friend bool operator==(const PortableRect &a, const PortableRect &b) {
    return a.x_ == b.x_ && a.y_ == b.y_ && a.w_ == b.w_ && a.h_ == b.h_;
  }
  friend bool operator!=(const PortableRect &a, const PortableRect &b) {
    return !(a == b);
  }
  friend PortableRect operator&(const PortableRect &a, const PortableRect &b) {
    return a.intersected(b);
  }
  friend PortableRect operator|(const PortableRect &a, const PortableRect &b) {
    return a.united(b);
  }
};
using QRect = PortableRect<int>;
using QRectF = PortableRect<double>;
template <class T>
std::ostream &operator<<(std::ostream &s, const PortableRect<T> &r) {
  return s << '(' << r.x() << ',' << r.y() << ',' << r.width() << ','
           << r.height() << ')';
}
