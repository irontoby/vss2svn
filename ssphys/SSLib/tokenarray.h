// $Id$
// Tokenize a string into an array of strings. The delimeter can be either
// a set of chars (as a C string) or a Boost regular expression.
// Provides a read-only random access iterator. The array is not
// sortable, or otherwise writeable.
//
// Each token in the input string is only extracted when it is asked
// for. On construction, the postions of the token delimeters are
// found and stored in a std::vector.
//
// Written by C. Ashley Sanders, a.sanders@man.ac.uk, University of
// Manchester, 16/7/2004.

# ifndef TOKENARRAY_HH_
# define TOKENARRAY_HH_

# include <iterator>
# include <vector>
# ifndef ASH_NO_REGEX
# include <boost/regex.hpp>
# endif

namespace ash {
  template<class CharT = char>
  class tokenarray {
  public:
  	typedef tokenarray<CharT> _Myt;
    typedef typename std::basic_string<CharT> String;
# ifndef ASH_NO_REGEX
    typedef typename boost::basic_regex<CharT> RegExp;
# endif
    typedef typename String::size_type string_size_type;
    typedef std::vector<std::pair<string_size_type, string_size_type> > tarray;
    typedef typename tarray::size_type size_type;

    class const_iterator {
    public:
      // This class could derive from std::iterator rather than have all
      // the typedefs below. However, neither of the two versions of the
      // STL I have seem to cut the mustard when it comes to std::iterator.
      typedef std::random_access_iterator_tag iterator_category;
      typedef String value_type;
      typedef typename tarray::size_type difference_type;
      typedef String* pointer;
      typedef String& reference;

    private:
      const tokenarray<CharT> *pvs;
      difference_type i;
    public:
      const_iterator () : pvs(0), i(0) {}
      const_iterator (const const_iterator &s)
	: pvs(s.pvs), i(s.i) {};
      const_iterator (const tokenarray<CharT> *ps, difference_type d)
	: pvs(ps), i(d) {};
      const_iterator& operator= (const const_iterator &s) {
	if (this != &s)
	  {
	    pvs = s.pvs;
	    i = s.i;
	  }
	return *this;
      }
      const_iterator& operator++ () { *this += 1; return *this; };
      const const_iterator operator++ (int) {
	const_iterator s (*this);
	++*this;
	return s;
      };
      const_iterator& operator+= (difference_type d) { i += d; return *this; }
      const_iterator operator+ (difference_type d) const {
	const_iterator s (*this);
	return s += d;
      }
      const_iterator& operator-- () { *this -= 1; return *this; };
      const const_iterator operator-- (int) {
	const_iterator s (*this);
	--*this;
	return s;
      };
      const_iterator& operator-= (difference_type d) { i -= d; return *this; }
      const_iterator operator- (difference_type d) const {
	const_iterator s (*this);
	return s -= d;
      }
      difference_type operator- (const const_iterator &s) const {
	return i - s.i;
      }
      bool operator== (const const_iterator &s) const {
	return i == s.i;
      }
      bool operator< (const const_iterator &s) const {
	return i < s.i;
      }
      bool operator<= (const const_iterator &s) const {
	return i <= s.i;
      }
      bool operator> (const const_iterator &s) const {
	return i > s.i;
      }
      bool operator>= (const const_iterator &s) const {
	return i >= s.i;
      }
      bool operator!= (const const_iterator &s) const {
	return i != s.i;
      }
      const value_type operator[] (difference_type d) const {
	return (*pvs)[i+d];
      }
      const value_type operator* () const {
	return (*pvs)[i];
      };
    };

  private:
    tarray vsz;
    String st;


  public:
    tokenarray () {};
    tokenarray (const String &s, const CharT *pc) : st(s) {
      string_size_type pos;
      for (pos = 0; (pos = st.find_first_of (pc, pos)) != String::npos; ++pos)
	vsz.push_back (std::make_pair (pos, pos+1));
    }
# ifndef ASH_NO_REGEX
    tokenarray (const String &s, const RegExp &re) : st(s) {
      boost::match_results<typename String::const_iterator> match;
      typename String::const_iterator pc = st.begin (), pcEnd = st.end ();
      string_size_type szOffset = 0;
      while (boost::regex_search (pc, pcEnd, match, re))
	{
	  vsz.push_back (std::make_pair (szOffset + match.position (),
					 szOffset + match.position () + match.length ()));
	  pc = match[0].second;
	  szOffset += match.position() + match.length();
	}
    }
# endif // ASH_NO_REGEX
    tokenarray (const tokenarray& s) : vsz(s.vsz), st(s.st) {};
    tokenarray& operator= (const tokenarray &s) {
      tokenarray tmp (s);
      swap (tmp);
      return *this;
    }
    void swap (tokenarray &s) {
      vsz.swap (s.vsz);
      st.swap (s.st);
    }
    String data () const { return st; }
    void data (const String &s, const CharT *pc) {
      tokenarray tmp (s, pc);
      swap (tmp);
    }
    size_type size () const {
      return st.empty () ? 0 : vsz.size () + 1;
    };
    const String operator[] (size_type i) const {
      // Note: no bounds checking on vsz.
      string_size_type start = (i) ? vsz[i-1].second : 0;
      string_size_type len = (i < size ()-1) ? vsz[i].first - start : String::npos;
      return st.substr (start, len);
    }
    const_iterator begin () const {
      return const_iterator (this, 0);
    }
    const_iterator end () const {
      return const_iterator (this, size ());
    }
  };

} // namespace ash

# endif // TOKENARRAY_HH_
