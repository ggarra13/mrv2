/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvRectangle.h
 * @author gga
 * @date   Mon Oct 23 13:36:30 2006
 * 
 * @brief  A simple rectangle containing double coordinates
 * 
 * 
 */

#ifndef mrvRectangle_h
#define mrvRectangle_h

#include <iostream>


namespace mrv
{
  template< typename T >
  class Rectangle
  {
    T x_, y_, w_, h_;

  public:
    inline Rectangle() : x_(0), y_(0), w_(0), h_(0) {}

    inline Rectangle( T x, T y, T w, T h ) :
      x_(x), y_(y), w_(w), h_(h) {}

    inline Rectangle( T w, T h ) :
      x_(0), y_(0), w_(w), h_(h) {}

    inline Rectangle( const Rectangle< T >& r ) :
      x_(r.x()), y_(r.y()), w_(r.w()), h_(r.h()) {}

      inline void x( T x ) { x_ = x; }
      inline void y( T x ) { y_ = x; }
      inline void w( T x ) { w_ = x; }
      inline void h( T x ) { h_ = x; }

    inline T x() const { return x_; }
    inline T y() const { return y_; }

    inline T w() const { return w_; }
    inline T h() const { return h_; }

    inline T t() const { return y_; }
    inline T l() const { return x_; }

    inline T r() const { return x_ + w_; }
    inline T b() const { return y_ + h_; }

  /*! Add \a d to x() without changing r() (it reduces w() by \a d). */
  void move_x(T d) {x_ += d; w_ -= d;}
  /*! Add \a d to y() without changing b() (it reduces h() by \a d). */
  void move_y(T d) {y_ += d; h_ -= d;}
  /*! Add \a d to r() and w(). */
  void move_r(T d) {w_ += d;}
  /*! Add \a d to b() and h(). */
  void move_b(T d) {h_ += d;}
      
    /*! Change x() without changing r(), by changing the width. */
    void set_x(T v) {w_ -= v-x_; x_ = v;}
    /*! Change y() without changing b(), by changing the height. */
    void set_y(T v) {h_ -= v-y_; y_ = v;}
    /*! Change r() without changing x(), by changine the width. */
    void set_r(T v) {w_ = v-x_;}
    /*! Change b() without changing y(), by changine the height. */
    void set_b(T v) {h_ = v-y_;}

  /*! Integer center position. Rounded to the left if w() is odd. */
      inline T center_x() const {return x_+ w_ / 2;}
      /*! Integer center position. Rounded to lower y if h() is odd. */
      inline T center_y() const {return y_+ h_/ 2;}
      
      bool empty() const { return ( h_ == 0 && w_ == 0 ); }
      
    inline void merge( const Rectangle< T >& R )
    {
        if (R.w() == 0) return;
        if (w() == 0) { *this = R; return; }

        if (R.x() < x()) set_x(R.x());
        if (R.r() > r()) set_r(R.r());
        if (R.y() < y()) set_y(R.y());
        if (R.b() > b()) set_b(R.b());
    }
      
      void intersect(const Rectangle< T >& R) {
          if (R.x() > x()) set_x(R.x());
          if (R.r() < r()) set_r(R.r());
          if (R.y() > y()) set_y(R.y());
          if (R.b() < b()) set_b(R.b());
      }

      inline bool operator==( const Rectangle< T >& b ) const
      {
          return ( b.x_ == x_ && b.y_ == y_ && b.w_ == w_ && b.h_ == h_ );
      }

      inline bool operator!=( const Rectangle< T >& b ) const
      {
          return ( b.x_ != x_ || b.y_ != y_ || b.w_ != w_ || b.h_ != h_ );
      }

    inline
    friend std::ostream& operator<<( std::ostream& o, const Rectangle< T >& r )
    {
      return o << '(' << r.l() << ',' << r.t() << " - "
	       << r.r() << ',' << r.b() << ") [" << r.w() << "-" << r.h()
               << ']';
    }

  };


  typedef Rectangle< double > Rectd;
  typedef Rectangle< float  > Rectf;
  typedef Rectangle< int    > Recti;


} // namespace mrv

#endif // mrvRectangle_h
