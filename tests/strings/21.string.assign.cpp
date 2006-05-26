/***************************************************************************
 *
 * 21.string.assign.cpp - test exercising [lib.string.assign]
 *
 * $Id$
 *
 ***************************************************************************
 *
 * Copyright 2006 The Apache Software Foundation or its licensors,
 * as applicable.
 *
 * Copyright 2006 Rogue Wave Software.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************/

#include <string>         // for string
#include <cstddef>        // for size_t
#include <stdexcept>      // for out_of_range, length_error

#include <21.strings.h>   // for StringMembers
#include <alg_test.h>     // for InputIter
#include <driver.h>       // for rw_test()
#include <rw_allocator.h> // for UserAlloc
#include <rw_char.h>      // for rw_expand()
#include <rw_new.h>       // for bad_alloc, replacement operator new

/**************************************************************************/

// for convenience and brevity
#define Assign(sig) StringIds::assign_ ## sig

static const char* const exceptions[] = {
    "unknown exception", "out_of_range", "length_error",
    "bad_alloc", "exception"
};

/**************************************************************************/

// used to exercise:
// assign (const value_type*)
static const StringTestCase
cptr_test_cases [] = {

#undef TEST
#define TEST(str, arg, res, bthrow) {                           \
        __LINE__, -1, -1, -1, -1, -1,                           \
        str, sizeof str - 1,                                    \
        arg, sizeof arg - 1, res, sizeof res - 1, bthrow        \
    }

    //    +----------------------------------------- controlled sequence
    //    |             +--------------------------- sequence to be assigned
    //    |             |             +------------- expected result sequence
    //    |             |             |        +---- exception info
    //    |             |             |        |      0 - no exception
    //    |             |             |        |      1 - out_of_range
    //    |             |             |        |      2 - length_error
    //    |             |             |        |     -1 - exc. safety
    //    |             |             |        |
    //    |             |             |        +-------+
    //    V             V             V                V
    TEST ("ab",         "c",          "c",             0),

    TEST ("",           "",           "",              0),
    TEST ("",           "\0",         "",              0),
    TEST ("",           "abc",        "abc",           0),
    TEST ("abc",        "",           "",              0),

    TEST ("\0",         "",           "",              0),
    TEST ("\0",         "a",          "a",             0),
    TEST ("\0",         "\0\0",       "",              0),

    TEST ("\0\0ab",     "cdefghij",   "cdefghij",      0),
    TEST ("a\0\0b",     "cdefghij",   "cdefghij",      0),
    TEST ("ab\0\0",     "cdefghij",   "cdefghij",      0),
    TEST ("a\0b\0\0c",  "e\0",        "e",             0),
    TEST ("\0ab\0\0c",  "e\0",        "e",             0),

    TEST ("",           "x@4096",     "x@4096",        0),
    TEST ("x@4096",     "",           "",              0),
    TEST ("a@4096",     "b@4096",     "b@4096",        0),

    TEST ("",           0,            "",              0),
    TEST ("abc",        0,            "abc",           0),
    TEST ("a\0\0bc",    0,            "a",             0),
    TEST ("\0\0abc",    0,            "",              0),
    TEST ("abc\0\0",    0,            "abc",           0),
    TEST ("x@4096",     0,            "x@4096",        0),

    TEST ("last",       "test",       "test",          0)
};

/**************************************************************************/

// used to exercise:
// assign (const basic_string&)
static const StringTestCase
cstr_test_cases [] = {

#undef TEST
#define TEST(s, arg, res, bthrow) {                             \
        __LINE__, -1, -1, -1, -1, -1,                           \
        s, sizeof s - 1,                                        \
        arg, sizeof arg - 1, res, sizeof res - 1, bthrow        \
    }

    //    +----------------------------------------- controlled sequence
    //    |             +--------------------------- sequence to be assigned
    //    |             |             +------------- expected result sequence
    //    |             |             |        +---- exception info:
    //    |             |             |        |      0 - no exception
    //    |             |             |        |      1 - out_of_range
    //    |             |             |        |      2 - length_error
    //    |             |             |        |     -1 - exc. safety
    //    |             |             |        |
    //    |             |             |        +-----+
    //    |             |             |              |
    //    V             V             V              V
    TEST ("ab",         "c",          "c",           0),

    TEST ("",           "",           "",            0),
    TEST ("",           "\0",         "\0",          0),
    TEST ("",           "abc",        "abc",         0),
    TEST ("abc",        "",           "",            0),

    TEST ("\0",         "",           "",            0),
    TEST ("\0",         "a",          "a",           0),
    TEST ("\0",         "\0\0",       "\0\0",        0),

    TEST ("ab",         "c\0e",       "c\0e",        0),

    TEST ("\0\0ab",     "cdefghij",   "cdefghij",    0),
    TEST ("a\0\0b",     "cdefghij",   "cdefghij",    0),
    TEST ("ab\0\0",     "cdefghij",   "cdefghij",    0),
    TEST ("a\0b\0\0c",  "e\0",        "e\0",         0),
    TEST ("\0ab\0\0c",  "e\0",        "e\0",         0),
    TEST ("ab\0\0c\0",  "\0e",        "\0e",         0),

    TEST ("e\0",        "a\0b\0\0c",  "a\0b\0\0c",   0),
    TEST ("e\0\0",      "\0ab\0\0c",  "\0ab\0\0c",   0),
    TEST ("\0e",        "ab\0\0c\0",  "ab\0\0c\0",   0),

    TEST ("",           "x@4096",     "x@4096",      0),
    TEST ("x@4096",     "",           "",            0),
    TEST ("a@4096",     "b@4096",     "b@4096",      0),

    TEST ("",           0,            "",            0),
    TEST ("abc",        0,            "abc",         0),
    TEST ("a\0\0bc",    0,            "a\0\0bc",     0),
    TEST ("\0\0abc",    0,            "\0\0abc",     0),
    TEST ("abc\0\0",    0,            "abc\0\0",     0),
    TEST ("x@4096",     0,            "x@4096",      0),

    TEST ("last",       "test",       "test",        0)
};

/**************************************************************************/

// used to exercise:
// assign (const value_type*, size_type)
static const StringTestCase
cptr_size_test_cases [] = {

#undef TEST
#define TEST(str, arg, size, res, bthrow) {                     \
        __LINE__, -1, size, -1, -1, -1,                         \
        str, sizeof str - 1,                                    \
        arg, sizeof arg - 1, res, sizeof res - 1, bthrow        \
    }

    //    +----------------------------------------- controlled sequence
    //    |            +---------------------------- sequence to be assigned
    //    |            |            +--------------- assign() n argument
    //    |            |            |  +------------ expected result sequence
    //    |            |            |  |     +------ exception info
    //    |            |            |  |     |        0 - no exception
    //    |            |            |  |     |        1 - out_of_range
    //    |            |            |  |     |        2 - length_error
    //    |            |            |  |     |       -1 - exc. safety
    //    |            |            |  |     |
    //    |            |            |  |     +--------+
    //    V            V            V  V              V
    TEST ("ab",        "c",         1, "c",           0),

    TEST ("",          "",          0,  "",           0),
    TEST ("",          "abc",       1,  "a",          0),
    TEST ("",          "\0",        1,  "\0",         0),

    TEST ("\0",        "",          0,  "",           0),
    TEST ("\0",        "a",         0,  "",           0),
    TEST ("\0",        "a",         1,  "a",          0),
    TEST ("\0",        "\0\0",      1,  "\0",         0),
    TEST ("\0",        "\0\0",      2,  "\0\0",       0),

    TEST ("cde",       "ab",        2,  "ab",         0),
    TEST ("cde",       "ab",        1,  "a",          0),

    TEST ("\0e\0",     "a\0b\0\0c", 0,  "",           0),
    TEST ("\0e\0",     "\0ab\0\0c", 3,  "\0ab",       0),
    TEST ("\0e\0",     "a\0b\0\0c", 6,  "a\0b\0\0c",  0),

    TEST ("a\0b\0\0c", "\0e\0",     3,  "\0e\0",      0),
    TEST ("a\0b\0\0c", "\0\0e\0",   2,  "\0\0",       0),
    TEST ("\0ab\0\0c", "\0e\0",     1,  "\0",         0),
    TEST ("a\0bc\0\0", "\0e",       2,  "\0e",        0),

    TEST ("",          0,           0,  "",           0),
    TEST ("abc",       0,           0,  "",           0),
    TEST ("abc",       0,           1,  "a",          0),
    TEST ("abc",       0,           2,  "ab",         0),
    TEST ("a\0bc",     0,           2,  "a\0",        0),
    TEST ("\0abc\0\0", 0,           1,  "\0",         0),
    TEST ("a\0bc\0\0", 0,           3,  "a\0b",       0),
    TEST ("a\0bc\0\0", 0,           6,  "a\0bc\0\0",  0),
    TEST ("x@4096",    0,        2048,  "x@2048",     0),

    TEST ("",          "x@4096", 4096,  "x@4096",     0),
    TEST ("abcd",      "x@4096",    0,  "",           0),
    TEST ("x@4096",    "x@4096",    0,  "",           0),
    TEST ("x@4096",    "x@4096",    1,  "x",          0),
    TEST ("x@4096",    "",          0,  "",           0),
    TEST ("a@4096",    "b@4096", 2048,  "b@2048",     0),

    TEST ("",          "",         -1,  "",           2),

    TEST ("last",      "test",      4, "test",        0)
};

/**************************************************************************/

// used to exercise:
// assign (const basic_string&, size_type, size_type)
// assign (InputIterator, InputIterator)
static const StringTestCase
range_test_cases [] = {

// range_test_cases serves a double duty
#define cstr_size_size_test_cases range_test_cases

#undef TEST
#define TEST(str, arg, off, size, res, bthrow) {                \
        __LINE__, off, size, -1, -1, -1,                        \
        str, sizeof str - 1,                                    \
        arg, sizeof arg - 1, res, sizeof res - 1, bthrow        \
    }

    //    +----------------------------------------- controlled sequence
    //    |            +---------------------------- sequence to be inserted
    //    |            |            +--------------- assign() pos argument
    //    |            |            |  +------------ assign() n argument
    //    |            |            |  |  +--------- expected result sequence
    //    |            |            |  |  |  +------ exception info
    //    |            |            |  |  |  |        0 - no exception
    //    |            |            |  |  |  |        1 - out_of_range
    //    |            |            |  |  |  |        2 - length_error
    //    |            |            |  |  |  |       -1 - exc. safety
    //    |            |            |  |  |  |
    //    |            |            |  |  |  +-----------+
    //    V            V            V  V  V              V
    TEST ("ab",        "c",         0, 1, "c",           0),

    TEST ("",          "",          0, 0,  "",           0),
    TEST ("",          "abc",       1, 1,  "b",          0),
    TEST ("",          "\0",        0, 1,  "\0",         0),

    TEST ("\0",        "",          0, 0,  "",           0),

    TEST ("abc",       "",          0, 0,  "",           0),

    TEST ("\0",        "a",         0, 1,  "a",          0),
    TEST ("\0",        "\0\0",      1, 1,  "\0",         0),
    TEST ("\0",        "\0\0",      0, 2,  "\0\0",       0),
    TEST ("\0",        "\0\0",      1, 5,  "\0",         0),

    TEST ("cde",       "ab",        0, 2,  "ab",         0),
    TEST ("cde",       "ab",        0, 1,  "a",          0),
    TEST ("cde",       "ab",        1, 5,  "b",          0),

    TEST ("ab",        "c\0e",      0, 3,  "c\0e",       0),
    TEST ("ab",        "c\0e",      1, 2,  "\0e",        0),
    TEST ("ab",        "c\0e",      0, 2,  "c\0",        0),

    TEST ("\0e\0",     "\0ab\0\0c", 0, 9,  "\0ab\0\0c",  0),
    TEST ("\0e\0",     "\0ab\0\0c", 0, 3,  "\0ab",       0),
    TEST ("a\0b\0\0c", "\0e\0",     0, 3,  "\0e\0",      0),
    TEST ("a\0b\0\0c", "\0\0e\0",   0, 2,  "\0\0",       0),
    TEST ("\0ab\0\0c", "\0e\0",     2, 1,  "\0",         0),
    TEST ("\0ab\0\0c", "\0e\0",     2, 9,  "\0",         0),
    TEST ("a\0bc\0\0", "\0e",       0, 2,  "\0e",        0),

    TEST ("",          0,           0, 0,  "",           0),
    TEST ("abc",       0,           1, 0,  "",           0),
    TEST ("abc",       0,           1, 1,  "b",          0),
    TEST ("abc",       0,           0, 2,  "ab",         0),
    TEST ("a\0bc\0\0", 0,           4, 2,  "\0\0",       0),
    TEST ("a\0bc\0\0", 0,           1, 3,  "\0bc",       0),
    TEST ("a\0bc\0\0", 0,           0, 9,  "a\0bc\0\0",  0),
    TEST ("abcdef",    0,           1, 2,  "bc",         0),
    TEST ("x@4096",    0,           1, 2,  "xx",         0),

    TEST ("x@4096",    "",          0, 0,  "",           0),
    TEST ("",          "x@4096",    9, 2,  "xx",         0),
    TEST ("",          "x@4096",    9, 0,  "",           0),
    TEST ("abc",       "x@4096",    2, 1,  "x",          0),
    TEST ("x@4096",    "x@4096",    2, 3,  "xxx",        0),
    TEST ("",          "x@4096",    0, 4096, "x@4096",   0),
    TEST ("",          "x@4096",  100, 2000, "x@2000",   0),

    TEST ("",          "\0",        2, 0,  "",           1),
    TEST ("",          "a",         2, 0,  "",           1),
    TEST ("",          "x@4096", 4106, 0,  "",           1),

    TEST ("last",      "test",      0, 4, "test",        0)
};

/**************************************************************************/

// used to exercise:
// assign (size_type, value_type)
static const StringTestCase
size_val_test_cases [] = {

#undef TEST
#define TEST(str, size, val, res, bthrow) {     \
        __LINE__, -1, size, -1, -1, val,        \
        str, sizeof str - 1,                    \
        0, 0, res, sizeof res - 1, bthrow       \
    }

    //    +----------------------------------------- controlled sequence
    //    |            +---------------------------- assign() count argument
    //    |            |   +------------------------ character to be assigned
    //    |            |   |   +-------------------- expected result sequence
    //    |            |   |   |       +------------ exception info
    //    |            |   |   |       |              0 - no exception
    //    |            |   |   |       |              1 - out_of_range
    //    |            |   |   |       |              2 - length_error
    //    |            |   |   |       |             -1 - exc. safety
    //    |            |   |   |       |
    //    |            |   |   |       +--------+
    //    V            V   V   V                V
    TEST ("ab",        1, 'c', "c",             0),

    TEST ("",          0, ' ',  "",             0),
    TEST ("",          1, 'b',  "b",            0),
    TEST ("",          3, 'b',  "bbb",          0),

    TEST ("\0",        0, ' ',  "",             0),
    TEST ("",          2, '\0', "\0\0",         0),

    TEST ("\0",        1, 'a',  "a",            0),
    TEST ("\0",        1, '\0', "\0",           0),
    TEST ("\0",        2, '\0', "\0\0",         0),
    TEST ("\0",        0, '\0', "",             0),

    TEST ("cde",       3, 'a',  "aaa",          0),
    TEST ("ab",        2, '\0', "\0\0",         0),
    TEST ("ab",        1, '\0', "\0",           0),

    TEST ("a\0b\0\0c", 2, '\0', "\0\0",         0),
    TEST ("a\0b\0\0c", 1, '\0', "\0",           0),
    TEST ("\0ab\0\0c", 3, '\0', "\0\0\0",       0),
    TEST ("a\0bc\0\0", 2, 'a',  "aa",           0),

    TEST ("",       4096, 'x',  "x@4096",       0),
    TEST ("x@4096",    0, 'x',  "",             0),

    TEST ("",         -1, 'x',  "",             2),

    TEST ("last",      4, 't',  "tttt",         0)
};

/**************************************************************************/

template <class charT, class Traits, class Allocator, class Iterator>
void test_assign_range (const charT    *wstr,
                        std::size_t     wstr_len,
                        const charT    *warg,
                        std::size_t     warg_len,
                        std::size_t     res_len,
                        Traits*,
                        Allocator*, 
                        const Iterator &it,
                        const StringTestCase &tcase)
{
    typedef std::basic_string <charT, Traits, Allocator> String;
    typedef typename String::iterator                    StringIter;

    const char* const itname =
        tcase.arg ? type_name (it, (charT*)0) : "basic_string::iterator";

    // compute the size of the controlled sequence and the size
    // of the sequence denoted by the argument keeping in mind
    // that the latter may refer to the former
    const std::size_t size1 = wstr_len;
    const std::size_t size2 = tcase.arg ? warg_len : size1;

    // construct the string object to be modified
    String str (wstr, size1);

    // compute the offset and the extent (the number of elements)
    // of the second range into the argument of the function call
    const std::size_t off =
        std::size_t (tcase.off) < size2 ? std::size_t (tcase.off) : size2;
    const std::size_t ext =
        off + tcase.size < size2 ? std::size_t (tcase.size) : size2 - off;

    if (tcase.arg) {
        const charT* const beg = warg + off;
        const charT* const end = beg + ext;

        const Iterator first (beg, beg, end);
        const Iterator last  (end, beg, end);

        str.assign (first, last);
    }
    else {
        // self-referential modification (appending a range
        // of elements with a subrange of its own elements)
        const StringIter first (str.begin () + off);
        const StringIter last (first + ext);

        str.assign (first, last);
    }

    // detrmine whether the produced sequence matches the exepceted result
    const std::size_t match = rw_match (tcase.res, str.data (), tcase.nres);

    rw_assert (match == res_len, 0, tcase.line,
               "line %d. %{$FUNCALL} expected %{#*s}, got %{/*.*Gs}, "
               "difference at off %zu for %s",
               __LINE__, int (tcase.nres), tcase.res,
               int (sizeof (charT)), int (str.size ()), str.c_str (),
               match, itname);
}

/**************************************************************************/

template <class charT, class Traits, class Allocator>
void test_assign_range (const charT    *wstr,
                        std::size_t     wstr_len,
                        const charT    *warg,
                        std::size_t     warg_len,
                        std::size_t     res_len,
                        Traits*,
                        Allocator*, 
                        const StringTestCase &tcase)
{
    if (tcase.bthrow)  // this method doesn't throw
        return;

    test_assign_range (wstr, wstr_len, warg, warg_len, res_len, (Traits*)0,
                       (Allocator*)0, InputIter<charT>(0, 0, 0), tcase);

    // there is no need to call test_assign_range
    // for other iterators in this case
    if (0 == tcase.arg)
        return;

    test_assign_range (wstr, wstr_len, warg, warg_len, res_len, (Traits*)0,
                       (Allocator*)0, ConstFwdIter<charT>(0, 0, 0), tcase);

    test_assign_range (wstr, wstr_len, warg, warg_len, res_len, (Traits*)0,
                       (Allocator*)0, ConstBidirIter<charT>(0, 0, 0), tcase);

    test_assign_range (wstr, wstr_len, warg, warg_len, res_len, 
                      (Traits*)0, (Allocator*)0, 
                       ConstRandomAccessIter<charT>(0, 0, 0), tcase);
}

/**************************************************************************/

template <class charT, class Traits, class Allocator>
void test_assign (charT, Traits*, Allocator*,
                  const StringFunc     &func,
                  const StringTestCase &tcase)
{
    typedef std::basic_string <charT, Traits, Allocator> String;
    typedef typename String::iterator                    StringIter;
    typedef typename UserTraits<charT>::MemFun           UTMemFun;

    static const std::size_t BUFSIZE = 256;

    static charT wstr_buf [BUFSIZE];
    static charT warg_buf [BUFSIZE];

    std::size_t str_len = sizeof wstr_buf / sizeof *wstr_buf;
    std::size_t arg_len = sizeof warg_buf / sizeof *warg_buf;

    charT* wstr = rw_expand (wstr_buf, tcase.str, tcase.str_len, &str_len);
    charT* warg = rw_expand (warg_buf, tcase.arg, tcase.arg_len, &arg_len);

    static charT wres_buf [BUFSIZE];
    std::size_t res_len = sizeof wres_buf / sizeof *wres_buf;
    charT* wres = rw_expand (wres_buf, tcase.res, tcase.nres, &res_len);

    // special processing for assign_range to exercise all iterators
    if (Assign (range) == func.which_) {
        test_assign_range (wstr, str_len, warg, arg_len, 
                           res_len, (Traits*)0, (Allocator*)0, tcase);

        if (wstr != wstr_buf)
            delete[] wstr;

        if (warg != warg_buf)
            delete[] warg;

        if (wres != wres_buf)
            delete[] wres;

        return;
    }

    // construct the string object to be modified
    // and the (possibly unused) argument string
    /* const */ String str (wstr, str_len);
    const       String arg (warg, arg_len);

    if (wstr != wstr_buf)
        delete[] wstr;

    if (warg != warg_buf)
        delete[] warg;

    wstr = 0;
    warg = 0;

    std::size_t size = tcase.size >= 0 ? tcase.size : str.max_size () + 1;

    // save the state of the string object before the call
    // to detect wxception safety violations (changes to
    // the state of the object after an exception)
    const StringState str_state (rw_get_string_state (str));

    // first function argument
    const charT* const arg_ptr = tcase.arg ? arg.c_str () : str.c_str ();
    const String&      arg_str = tcase.arg ? arg : str;

    std::size_t total_length_calls = 0;
    std::size_t n_length_calls = 0;
    std::size_t* const rg_calls = rw_get_call_counters ((Traits*)0, (charT*)0);

    if (rg_calls)
        total_length_calls = rg_calls [UTMemFun::length];

    rwt_free_store* const pst = rwt_get_free_store (0);
    SharedAlloc*    const pal = SharedAlloc::instance ();

    // iterate for`throw_after' starting at the next call to operator new,
    // forcing each call to throw an exception, until the function finally
    // succeeds (i.e, no exception is thrown)
    std::size_t throw_count;
    for (throw_count = 0; ; ++throw_count) {

        // (name of) expected and caught exception
        const char* expected = 0;
        const char* caught   = 0;

#ifndef _RWSTD_NO_EXCEPTIONS

        if (1 == tcase.bthrow && Assign (cstr_size_size) == func.which_)
            expected = exceptions [1];   // out_of_range
        else if (2 == tcase.bthrow)
            expected = exceptions [2];   // length_error
        else if (0 == tcase.bthrow) {
            // by default excercise the exception safety of the function
            // by iteratively inducing an exception at each call to operator
            // new or Allocator::allocate() until the call succeeds
            expected = exceptions [3];      // bad_alloc
            *pst->throw_at_calls_ [0] = pst->new_calls_ [0] + throw_count + 1;
            pal->throw_at_calls_ [pal->m_allocate] =
                pal->throw_at_calls_ [pal->m_allocate] + throw_count + 1;
        }
        else {
            // exceptions disabled for this test case
        }

#else   // if defined (_RWSTD_NO_EXCEPTIONS)

        if (tcase.bthrow) {
            if (wres != wres_buf)
                delete[] wres;

            return;
        }

#endif   // _RWSTD_NO_EXCEPTIONS

        // pointer to the returned reference
        const String* ret_ptr = 0;

        // start checking for memory leaks
        rw_check_leaks (str.get_allocator ());

        try {
            switch (func.which_) {

            case Assign (cptr):
                ret_ptr = &str.assign (arg_ptr);
                if (rg_calls)
                    n_length_calls = rg_calls [UTMemFun::length];
                break;

            case Assign (cstr):
                ret_ptr = &str.assign (arg_str);
                break;

            case Assign (cptr_size):
                ret_ptr = &str.assign (arg_ptr, size);
                break;

            case Assign (cstr_size_size):
                ret_ptr = &str.assign (arg_str, tcase.off, size);
                break;

            case Assign (size_val): {
                const charT val = make_char (char (tcase.val), (charT*)0);
                ret_ptr = &str.assign (size, val);
                break;
            }

            default:
                RW_ASSERT (!"test logic error: unknown assign overload");
            }

            // verify that the reference returned from the function
            // refers to the modified string object (i.e., *this
            // within the function)
            const std::ptrdiff_t ret_off = ret_ptr - &str;

            // verify the returned value
            rw_assert (0 == ret_off, 0, tcase.line,
                       "line %d. %{$FUNCALL} returned invalid reference, "
                       "offset is %td", __LINE__, ret_off);

            // verfiy that strings length are equal
            rw_assert (res_len == str.size (), 0, tcase.line,
                       "line %d. %{$FUNCALL}: expected %{#*s} with length "
                       "%zu, got %{/*.*Gs} with length %zu", __LINE__, 
                       int (tcase.nres), tcase.res, res_len, 
                       int (sizeof (charT)), int (str.size ()), 
                       str.c_str (), str.size ());

            if (res_len == str.size ()) {
                // if the result length matches the expected length
                // (and only then), also verify that the modified
                // string matches the expected result
                const std::size_t match =
                    rw_match (tcase.res, str.c_str(), tcase.nres);

                rw_assert (match == res_len, 0, tcase.line,
                           "line %d. %{$FUNCALL}: expected %{#*s}, "
                           "got %{/*.*Gs}, difference at off %zu",
                           __LINE__, int (tcase.nres), tcase.res,
                           int (sizeof (charT)), int (str.size ()), 
                           str.c_str (), match);
            }

            // verify that Traits::length was used
            if (Assign (cptr) == func.which_ && rg_calls) {
                rw_assert (n_length_calls - total_length_calls > 0, 
                           0, tcase.line, "line %d. %{$FUNCALL} doesn't "
                           "use traits::length()", __LINE__);
            }
        }

#ifndef _RWSTD_NO_EXCEPTIONS

        catch (const std::out_of_range &ex) {
            caught = exceptions [1];
            rw_assert (caught == expected, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught std::%s(%#s)",
                       __LINE__, 0 != expected, expected, caught, ex.what ());
        }
        catch (const std::length_error &ex) {
            caught = exceptions [2];
            rw_assert (caught == expected, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught std::%s(%#s)",
                       __LINE__, 0 != expected, expected, caught, ex.what ());
        }
        catch (const std::bad_alloc &ex) {
            caught = exceptions [3];
            rw_assert (0 == tcase.bthrow, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught std::%s(%#s)",
                       __LINE__, 0 != expected, expected, caught, ex.what ());
        }
        catch (const std::exception &ex) {
            caught = exceptions [4];
            rw_assert (0, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught std::%s(%#s)",
                       __LINE__, 0 != expected, expected, caught, ex.what ());
        }
        catch (...) {
            caught = exceptions [0];
            rw_assert (0, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s,%{:}"
                       "unexpectedly%{;} caught %s",
                       __LINE__, 0 != expected, expected, caught);
        }

#endif   // _RWSTD_NO_EXCEPTIONS

        // FIXME: verify the number of blocks the function call
        // is expected to allocate and detect any memory leaks
        rw_check_leaks (str.get_allocator (), tcase.line,
                        std::size_t (-1), std::size_t (-1));

        if (caught) {
            // verify that an exception thrown during allocation
            // didn't cause a change in the state of the object
            str_state.assert_equal (rw_get_string_state (str),
                                    __LINE__, tcase.line, caught);

            if (0 == tcase.bthrow) {
                // allow this call to operator new to succeed and try
                // to make the next one to fail during the next call
                // to the same function again
                continue;
            }
        }
        else if (0 < tcase.bthrow) {
            rw_assert (caught == expected, 0, tcase.line,
                       "line %d. %{$FUNCALL} %{?}expected %s, caught %s"
                       "%{:}unexpectedly caught %s%{;}",
                       __LINE__, 0 != expected, expected, caught, caught);
        }

        break;
    }

    std::size_t expect_throws = 0;

#ifndef _RWSTD_NO_STRING_REF_COUNT
    if (Assign (cstr) != func.which_) {
#endif

#ifndef _RWSTD_NO_REPLACEABLE_NEW_DELETE

    // verify that if exceptions are enabled and when capacity changes
    // at least one exception is thrown
    expect_throws = str_state.capacity_ < str.capacity ();

#else   // if defined (_RWSTD_NO_REPLACEABLE_NEW_DELETE)

    expect_throws = (StringIds::UserAlloc == func.alloc_id_) 
        ? str_state.capacity_ < str.capacity (): 0;

#endif   // _RWSTD_NO_REPLACEABLE_NEW_DELETE

#ifndef _RWSTD_NO_STRING_REF_COUNT
    }
#endif

    rw_assert (expect_throws == throw_count, 0, tcase.line,
               "line %d: %{$FUNCALL}: expected exactly 1 %s exception "
               "while changing capacity from %zu to %zu, got %zu",
               __LINE__, exceptions [3],
               str_state.capacity_, str.capacity (), throw_count);

    // disable bad_alloc exceptions
    *pst->throw_at_calls_ [0] = 0;
    pal->throw_at_calls_ [pal->m_allocate] = 0;

    if (wres != wres_buf)
        delete[] wres;

}

/**************************************************************************/

DEFINE_STRING_TEST_DISPATCH (test_assign);

int main (int argc, char** argv)
{
    static const StringTest
    tests [] = {

#undef TEST
#define TEST(sig) {                                             \
        Assign (sig), sig ## _test_cases,                       \
        sizeof sig ## _test_cases / sizeof *sig ## _test_cases  \
    }

        TEST (cptr),
        TEST (cstr),
        TEST (cptr_size),
        TEST (cstr_size_size),
        TEST (size_val),
        TEST (range)
    };

    const std::size_t test_count = sizeof tests / sizeof *tests;

    return rw_run_string_test (argc, argv, __FILE__,
                               "lib.string.assign",
                               test_assign, tests, test_count);
}
