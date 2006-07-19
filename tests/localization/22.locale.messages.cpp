/***************************************************************************
 *
 * messages.cpp - tests exercising the std::messages facet
 *
 * $Id: //stdlib/dev/tests/stdlib/locale/messages.cpp#40 $
 *
 ***************************************************************************
 *
 * Licensed to the Apache Software  Foundation (ASF) under one or more
 * contributor  license agreements.  See  the NOTICE  file distributed
 * with  this  work  for  additional information  regarding  copyright
 * ownership.   The ASF  licenses this  file to  you under  the Apache
 * License, Version  2.0 (the  "License"); you may  not use  this file
 * except in  compliance with the License.   You may obtain  a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the  License is distributed on an  "AS IS" BASIS,
 * WITHOUT  WARRANTIES OR CONDITIONS  OF ANY  KIND, either  express or
 * implied.   See  the License  for  the  specific language  governing
 * permissions and limitations under the License.
 *
 * Copyright 2001-2006 Rogue Wave Software.
 *
 **************************************************************************/

#include <locale>

#include <driver.h>      // for rw_test()
#include <environ.h>     // for rw_putenv()
#include <file.h>        // for rw_nextfd()
#include <system.h>      // for rw_system()
#include <rw_locale.h>   // for rw_locales()

#ifndef _MSC_VER
#  include <nl_types.h>  // for cat operations
#endif

#include <cstring>   // for strlen()
#include <cstdlib>   // for getcwd(), getenv()
#include <cstdio>    // for file operations
#include <clocale>   // for LC_ALL

#include <cwchar>    // for mbsinit()


#ifndef _RWSTD_NO_NEW_HEADER

#  include <stdio.h>         // for fileno
#  include <stdlib.h>        // for getcwd

#  if defined (_MSC_VER)
#    include <io.h>          // for _open
#    include <direct.h>
#  else
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <unistd.h>      // for getcwd
#  endif

#  include <fcntl.h>         // for mode flags for _open

#endif   // _RWSTD_NO_NEW_HEADER

#undef open
#undef close

#if defined (_MSC_VER)
#  define open(f,m) _open  (f, _##m)
#  define close(f)  _close (f)
#else
#  define open(f,m) open  (f, m)
#  define close(f)  close (f)
#endif // defined (_MSC_VER)

/***************************************************************************/

struct CodeCvt: std::codecvt<wchar_t, char, std::mbstate_t>
{
    typedef std::codecvt<wchar_t, char, std::mbstate_t> Base;

    static bool _used;
    static bool _partial;
    static bool _noconv;
    static bool _check_state;

    static bool _valid_state;

    CodeCvt (std::size_t ref = 1U)
        : Base (ref) {
        _used = _partial = _noconv = _check_state = _valid_state = false;
    }

protected:

    result
    do_in (state_type         &state,
           const extern_type  *from,
           const extern_type  *from_end,
           const extern_type *&from_next,
           intern_type        *to,
           intern_type        *to_end,
           intern_type       *&to_next) const {

        _used = true;

#ifndef _RWSTD_NO_MBSINIT

        if (_check_state)
            _valid_state = std::mbsinit (&state) != 0;

#else   // if (_RWSTD_NO_MBSINIT)

        if (_check_state)
            _valid_state = true;

#endif   // _RWSTD_NO_MBSINIT

        const result res =
            Base::do_in (state, from, from_end, from_next,
                         to, to_end, to_next);

        if (_partial)
            return std::codecvt_base::partial;

        if (_noconv)
            return std::codecvt_base::noconv;

        return res;
    }
};

bool CodeCvt::_used;
bool CodeCvt::_partial;
bool CodeCvt::_noconv;
bool CodeCvt::_check_state;
bool CodeCvt::_valid_state;

/***************************************************************************/

// finds the first named locale other than "C" or "POSIX"; returns
// a pointer to the name of the locale on success, 0 on failure
const char*
find_named_locale ()
{
    const char *name = rw_locales (LC_ALL, "");

    char namebuf [256];

    // get the full name of the "C" locale for comparison with aliases
    const char* const classic_name =
        std::strcpy (namebuf, std::setlocale (LC_ALL, "C"));

    do {
        if (std::strcmp (name, "C") && std::strcmp (name, "POSIX")) {

            // skip over any aliases of the "C" and "POSIX" locales
            const char* const fullname = std::setlocale (LC_ALL, name);
            if (std::strcmp (fullname, classic_name)) {

                // skip over the C and POSIX locales
                _TRY {
                    const std::locale loc (name);
                    _RWSTD_UNUSED (loc);

                    std::setlocale (LC_ALL, "C");
                    return name;
                }
                _CATCH (...) { }
            }
        }
        name += std::strlen (name) + 1;

    } while (*name);

    std::setlocale (LC_ALL, "C");

    return 0;
}

/***************************************************************************/

#ifndef _WIN32
#  define CAT_NAME "./rwstdmessages.cat"
#  define MSG_NAME "rwstdmessages.msg"
#else
#  define CAT_NAME "rwstdmessages.dll"
#  define MSG_NAME "rwstdmessages.rc"
#endif

#define NLS_CAT_NAME "rwstdmessages"

#define MAX_SETS 5
#define MAX_MESSAGES  5


int msg_id (int set, int id)
{
#ifdef _WIN32

    return (set - 1) * 5 + id;

#else

    _RWSTD_UNUSED (set);
    return id;

#endif
}

/***************************************************************************/

static const char* const
messages [MAX_SETS][MAX_MESSAGES] = {
    { "First set, first message",
      "First set, second message",
      "First set, third message",
      "First set, fourth message",
      "First set, fifth message"
    },
    { "Second set, first message",
      "Second set, second message",
      "Second set, third message",
      "Second set, fourth message",
      "Second set, fifth message"
    },
    { "Third set, first message",
      "Third set, second message",
      "Third set, third message",
      "Third set, fourth message",
      "Third set, fifth message"
    },
    { "Fourth set, first message",
      "Fourth set, second message",
      "Fourth set, third message",
      "Fourth set, fourth message",
      "Fourth set, fifth message"
    },
    { "Fifth set, first message",
      "Fifth set, second message",
      "Fifth set, third message",
      "Fifth set, fourth message",
      "Fifth set, fifth message"
    }
};


void generate_catalog (const char *msg_name,
                       const char* const text [MAX_SETS][MAX_MESSAGES])
{
    std::FILE* const f = std::fopen (msg_name, "w");

    if (!f)
        return;

#ifndef _WIN32

    for (int i = 0; i < MAX_SETS; ++i) {
        std::fprintf (f, "$set %d This is Set %d\n", i+1, i+1);
        for (int j = 0; j < MAX_MESSAGES; ++j) {
            std::fprintf (f, "%d %s\n", j + 1, text [i][j]);
        }
    }

#else   // if defined (_WIN32)

    std::fprintf (f, "STRINGTABLE\nBEGIN\n");
    for (int i = 0; i < MAX_SETS; ++i) {
        for (int j = 0; j < MAX_MESSAGES; ++j) {
            std::fprintf (f, "%d \"%s\"\n", msg_id(i+1, j+1), text[i][j]);
        }
    }

    std::fprintf (f, "END\n");

#endif   // _WIN32

    std::fclose (f);

    char *cat_name = new char [std::strlen (msg_name) + 1];
    const char *dot = std::strrchr (msg_name, '.');
    std::strncpy (cat_name, msg_name, dot - msg_name);
    *(cat_name + (dot - msg_name)) = '\0';

#ifndef _WIN32

    rw_system ("gencat %s.cat %s", cat_name, msg_name);

#else   // if defined (_WIN32)

    char cpp_name [128];

    std::sprintf (cpp_name, "%s.cpp", cat_name);

    std::FILE* const cpp_file = std::fopen (cpp_name, "w");
    std::fprintf (cpp_file, "void foo () { }");
    std::fclose (cpp_file);

    rw_system (   "rc -r %s.rc "
               "&& cl -nologo -c %s"
               "&& link -nologo /DLL /OUT:%s.dll %s.obj %s.res",
               cat_name,
               cpp_name,
               cat_name, cat_name, cat_name);

    rw_system (SHELL_RM_F "%s %s.rc %s.res %s.obj",
               cpp_name, cat_name, cat_name, cat_name);

#endif   // _WIN32

    delete[] cat_name;

    std::remove (msg_name);
}

/***************************************************************************/


template <class charT>
std::basic_string<charT> widen (std::string message)
{
    std::basic_string<charT> result;
    for (std::size_t i = 0; i < message.length(); i++) {
        result.push_back ((charT)message[i]);
    }

    return result;
}

/***************************************************************************/

// Test that the message facet exists in the locale and that
// we can get a reference to it.
template <class charT>
void test_has_facet (const char *loc_name, const char *cname)
{
    const std::locale loc (loc_name);

    typedef std::messages<charT> Messages;

    const bool facet_exists = std::has_facet<Messages> (loc);

    rw_assert (facet_exists, 0, __LINE__,
               "has_facet<messages<%s> >(locale(%#s)) == true",
               cname, loc_name);

    try {
        std::use_facet<Messages>(loc);
    }

#ifndef _RWSTD_NO_EXCEPTIONS

    catch (std::exception &ex) {
        rw_assert (!facet_exists, 0, __LINE__,
                   "use_facet<messages<%s>> unexpectedly threw "
                   "exception(%#s)", cname, ex.what ());
    }

#endif   // _RWSTD_NO_EXCEPTIONS

}

/***************************************************************************/

template <class charT>
void test_open_close (const char *loc_name, const char *cname)
{
    int fdcount [2];
    int next_fd [2];

    next_fd [0] = rw_nextfd (fdcount + 0);

    rw_info (0, 0, __LINE__,
             "std::messages<%s>::open () and close() in locale (\"%s\")",
             cname, loc_name);

    const std::locale loc (loc_name);

    const std::messages<charT>& msgs =
        std::use_facet<std::messages<charT> >(loc);

    std::messages_base::catalog cat = -1;

    try {
        cat = (msgs.open)(CAT_NAME, loc);
    }

#ifndef _RWSTD_NO_EXCEPTIONS

    catch (std::exception &ex) {
        rw_assert (cat < 0, 0, __LINE__,
                   "messages<%s>::open(%#s, locale(%#s)) unexpectedly threw "
                   "exception(%#s)", cname, CAT_NAME, loc.name ().c_str (),
                   ex.what ());
    }

#endif   // _RWSTD_NO_EXCEPTIONS

    rw_assert (0 <= cat, 0, __LINE__,
               "messages<%s>::open(%#s, locale(%#s)) >= -1, got %d",
               cname, CAT_NAME, loc_name, cat);

    try {
        (msgs.close)(cat);
    }

#ifndef _RWSTD_NO_EXCEPTIONS

    catch (std::exception &ex) {
        rw_assert (cat < 0, 0, __LINE__,
                   "messages<%s>::close(%d) unexpectedly threw "
                   "exception(%#s)", cname, cat, ex.what ());
    }

#endif   // _RWSTD_NO_EXCEPTIONS

    try {
        // an already closed cat should throw an exception
        (msgs.close)(cat);

        rw_assert (0, 0, __LINE__,
                   "std::messages<%s>::get(-1, ...) failed "
                   "to throw an exception", cname);
    }

#ifndef _RWSTD_NO_EXCEPTIONS

    catch (std::exception &ex) {
        rw_assert (true, 0, __LINE__,
                   "messages<%s>::close(%d) threw exception(%#s)",
                   cname, cat, ex.what ());
    }

#endif   // _RWSTD_NO_EXCEPTIONS

    // verify that no file descriptor has leaked
    next_fd [1] = rw_nextfd (fdcount + 1);

    rw_assert (next_fd [1] == next_fd [0] && fdcount [0] == fdcount [1],
               0, __LINE__,
               "std::messages<%s>::close() leaked %d file descriptor(s) "
               "(or descriptor mismatch)",
               fdcount [1] - fdcount [0]);
}

/***************************************************************************/

template <class charT>
void test_get (const char *loc_name,
               const char* const text[5][5],
               const char *cname)
{
    const std::locale loc (loc_name);

    const std::messages<charT>& msgs =
        std::use_facet<std::messages<charT> > (loc);

    const charT def[] = {
        'N', 'o', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e', '.', '\0'
    };

    std::messages_base::catalog cat = -1;

    try {
        cat = (msgs.open)(CAT_NAME, loc);
    }

#ifndef _RWSTD_NO_EXCEPTIONS

    catch (std::exception &ex) {
        rw_assert (cat < 0, 0, __LINE__,
                   "messages<%s>::open(%#s, locale(%#s)) unexpectedly threw "
                   "exception(%#s)", cname, CAT_NAME, loc.name ().c_str (),
                   ex.what ());
    }

#endif   // _RWSTD_NO_EXCEPTIONS

    rw_assert (-1 <= cat, 0, __LINE__,
               "messages<%s>::open(%#s, locale(%#s)) <= -1, got %d",
               cname, CAT_NAME, loc_name, cat);

    if (cat == -1)
        return;

    typedef std::char_traits<charT>                     Traits;
    typedef std::allocator<charT>                       Allocator;
    typedef std::basic_string<charT, Traits, Allocator> String;

    for (int setId = 1; setId < MAX_SETS; ++setId) {

        for (int msgId = 1; msgId < MAX_MESSAGES; ++msgId) {

            const String got =
                msgs.get (cat, setId, msg_id (setId, msgId), def);

            rw_assert (got == widen<charT>(text [setId - 1][msgId - 1]),
                       0, __LINE__,
                       "messages<%s>::get(%d, %d, %d, %{*Ac}) == %#s, "
                       "got %{#*S}",
                       cname, cat, setId, msg_id (setId, msgId),
                       int (sizeof *def), def,
                       text [setId - 1][msgId - 1],
                       int (sizeof (charT)), &got);
        }
    }

    // bad catalog id
    const std::messages_base::catalog bad_cat = 777;

    try {
        // get() will either throw or return `def'
        // when passed a bad catalog id
        const String got = msgs.get (bad_cat, 1, 1, def);

        rw_assert (def == got, 0, __LINE__,
                   "messages<%s>::get(%d, 1, 1, %{*Ac}) == %{*Ac}, got %{#*S}",
                   cname, bad_cat, int (sizeof *def), def,
                   int (sizeof *def), def, int (sizeof (charT)), &got);
    }
#ifndef _RWSTD_NO_EXCEPTIONS

    catch (std::exception &ex) {
        rw_assert (true, 0, __LINE__,
                   "messages<%s>::get(%d, 1, 1, %{*Ac}) unexpectedly threw "
                   "exception(%#s)", cname, bad_cat, int (sizeof *def), def,
                   ex.what ());
    }

#endif   // _RWSTD_NO_EXCEPTIONS

    // Bad set id
#if !defined (_WIN32) && !defined (_WIN64)
    // When we use resource files for the message catalogs
    // the set ids are ignored.
    rw_assert (msgs.get (cat, 777, 1, def) == def, 0, __LINE__,
               "messages<%s>::get(%d, 777, 1, %{#*Ac}) == %{#*Ac}",
               cname, cat, int (sizeof *def), def, int (sizeof *def), def);
#endif   // !defined (_WIN32) && !defined (_WIN64)

    // Bad message id
    rw_assert (msgs.get (cat, 1, 777, def) == def, 0, __LINE__,
               "messages<%s>::get(%d, 777, 1) == \"\"", cname);

    (msgs.close)(cat);
}

/***************************************************************************/

template <class charT>
void test_use_codecvt (const char *cname)
{
    CodeCvt cvt (1);

    const std::locale loc (std::locale::classic (), &cvt);

    const charT def[] = {
        'N', 'o', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e', '.', '\0'
    };

    const std::messages<charT>& msgs =
        std::use_facet <std::messages<charT> >(loc);

    cvt._used = false;

    std::messages_base::catalog cat = -1;

    try {
        cat = (msgs.open)(CAT_NAME, loc);
    }

#ifndef _RWSTD_NO_EXCEPTIONS

    catch (std::exception &ex) {
        rw_assert (cat < 0, 0, __LINE__,
                   "messages<%s>::open(%#s, locale(%#s)) unexpectedly threw "
                   "exception(%#s)", cname, CAT_NAME, loc.name ().c_str (),
                   ex.what ());
    }

#endif   // _RWSTD_NO_EXCEPTIONS

    if (!rw_error (-1 < cat, 0, __LINE__,
                   "messages<%s>::open(%#s, locale(%#s)) >= -1, got %d",
                   CAT_NAME, loc.name ().c_str (), cat))
        return;

    cvt._check_state = true;

    std::basic_string<charT> got = msgs.get (cat, 1, msg_id(1,1), def);
    rw_assert (cvt._used, 0, __LINE__,
               "messages<%s>::get() uses codecvt<%1$s, char>", cname);

    rw_assert (cvt._valid_state, 0, __LINE__,
               "messages<%s>::get() initializes mbstate_t argument", cname);

    cvt._check_state = false;
    cvt._partial     = true;

    got = msgs.get (cat, 1, msg_id (1, 1), def);
    rw_assert (got == def, 0, __LINE__, "messages<%s>::get() == %{*Ac}",
               cname, int (sizeof *def), def);

    cvt._partial = false;
    cvt._noconv  = true;

    got = msgs.get (cat, 1, msg_id(1,1), def);
    rw_assert (got == widen<charT>(std::string(messages[0][0])), 0, __LINE__,
               "messages<%s>::get() == %#s, got %{#*S}",
               cname, messages [0][0], int (sizeof (charT)), &got);

    cvt._noconv = false;

    (msgs.close)(cat);

    cvt._used = false;
}

/***************************************************************************/


template <class charT>
void test_use_nls_path (const char *cname)
{
    // get working directory
    char cwd[2048];
    getcwd(cwd, 2048);
    char *nlspath = new char[std::strlen(cwd) + 512];

    // create NLSPATH string
    std::sprintf (nlspath, "NLSPATH=%s/%%N.cat", cwd);
    rw_putenv (nlspath);

    const std::locale loc;

    const std::messages<charT>& msgs =
        std::use_facet <std::messages<charT> >(loc);

    const std::messages_base::catalog cat = (msgs.open)(NLS_CAT_NAME, loc);

    if (-1 == cat) {

        const char* const envvar = std::getenv ("NLSPATH");

        rw_assert (0, 0, __LINE__,
                   "messages<%s>::open(%#s, locale ()) != -1; "
                   "NLSPATH=%s", cname, NLS_CAT_NAME, envvar);
    }
    else {
        (msgs.close)(cat);
    }

    delete[] nlspath;
}

/***************************************************************************/


template <class charT>
void stress_test (const char *cname)
{
    char catalog_names [24][24];
    std::messages_base::catalog cats[24];

    const int fd1 = open (__FILE__, O_RDONLY);

    const std::locale loc = std::locale::classic ();

    const std::messages<charT>& msgs =
        std::use_facet<std::messages<charT> > (loc);

    char msg_name[24];

    int i;

    for (i = 0; i < 24; i++) {
#ifndef _WIN32
        std::sprintf (msg_name, "rwstdmessages_%d.msg", i);
#else
        std::sprintf (msg_name, "rwstdmessages_%d.rc", i);
#endif

        generate_catalog (msg_name, messages);
        char *dot = std::strrchr (msg_name, '.');
        std::strncpy (catalog_names[i], msg_name, dot - msg_name);
        *(catalog_names[i] + (dot - msg_name)) = '\0';

#ifndef _WIN32
        std::strcat (catalog_names[i], ".cat");
#else
        std::strcat (catalog_names[i], ".dll");
#endif

        cats[i] = (msgs.open)(catalog_names [i], loc);
        rw_assert (cats [i] != -1, 0, __LINE__,
                   "messages<%s>::open(%#s) != -1, got %d",
                   cname, catalog_names [i], cats [i]);
    }

    // close smallest first and check for descriptor leaks
    for (i = 0; i < 24; i++) {
        if (-1 != cats [i])
            (msgs.close)(cats [i]);
    }

    int fd2 = open (__FILE__, O_RDONLY);
    rw_assert (fd2 - fd1 == 1, 0, __LINE__,
               "messages<%s>::close():  Look for file descriptor leaks");

    //open again, close largest first and check for descriptor leaks
    for (i = 0; i < 24; i++) {
        cats [i] = (msgs.open)(catalog_names[i], loc);
        rw_assert (cats [i] != -1, 0, __LINE__,
                   "messages<%s>::open(%#s) != -1, got %d",
                   cname, catalog_names[i], cats [i]);
    }

    for (i = 23; i >=0 ; i--)
        if (-1 != cats[i])
            (msgs.close)(cats [i]);

    // close again fd2
    close (fd2);

    fd2 = open (__FILE__, O_RDONLY);
    rw_assert (fd2 - fd1 == 1, 0, __LINE__,
               "messages<%s>::close():  Look for file descriptor leaks");

    close (fd1);
    close (fd2);

    for (i = 0; i < 24; i++) {

#ifndef _WIN32
        std::sprintf (msg_name, "rwstdmessages_%d.msg", i);
#else
        std::sprintf (msg_name, "rwstdmessages_%d.dll", i);

        std::remove (msg_name);

        std::sprintf (msg_name, "rwstdmessages_%d.rc", i);
#endif

        std::remove (msg_name);

        std::sprintf (msg_name, "rwstdmessages_%d.cat", i);

        std::remove (msg_name);
    }
}

/**************************************************************************/

int opt_has_facet;
int opt_open_close;
int opt_get;
int opt_codecvt;
int opt_nlspath;
int opt_stress_test;


template <class charT>
void test_messages (charT, const char *cname, const char *locname)
{
    if (rw_note (0 <= opt_has_facet, 0, __LINE__,
                 "has_facet<messages<%s> > tests disabled", cname)) {
        test_has_facet<charT>("C", cname);
        test_has_facet<charT>(locname, cname);
    }

    if (rw_note (0 <= opt_open_close, 0, __LINE__,
                 "messages<%s>::open() and close() tests disabled", cname)) {
        test_open_close<charT>("C", cname);
        test_open_close<charT>(locname, cname);
    }

    if (rw_note (0 <= opt_has_facet, 0, __LINE__,
                 "messages<%s>::get() tests disabled", cname)) {
        test_get<charT>("C", messages, cname);
        test_get<charT>(locname, messages, cname);
    }

    if (rw_note (0 <= opt_has_facet, 0, __LINE__,
                 "messages<%s> codecvt tests disabled", cname))
        test_use_codecvt<charT>(cname);

    if (rw_note (0 <= opt_nlspath, 0, __LINE__,
                 "messages<%s> NLSPATH tests disabled", cname))
        test_use_nls_path<charT>(cname);

    if (rw_note (0 <= opt_has_facet, 0, __LINE__,
                 "messages<%s> codecvt tests disabled", cname))
        stress_test<charT>(cname);
}

/**************************************************************************/

static int
run_test (int, char*[])
{
    const char* const locname = find_named_locale ();

    generate_catalog (MSG_NAME, messages);

    test_messages (char (), "char", locname);

#ifndef _RWSTD_NO_WCHAR_T

    test_messages (wchar_t (), "wchar_t", locname);

#endif   // _RWSTD_NO_WCHAR_T

    std::remove (CAT_NAME);

    return 0;
}


/**************************************************************************/

int main (int argc, char *argv[])
{
    return rw_test (argc, argv, __FILE__,
                    "lib.category.messages",
                    0 /* no comment */,
                    run_test,
                    "|-has_facet~ "
                    "|-open_close~ "
                    "|-get~ "
                    "|-codecvt~ "
                    "|-nlspath~ "
                    "|-stress-test~ ",
                    &opt_has_facet,
                    &opt_open_close,
                    &opt_get,
                    &opt_codecvt,
                    &opt_nlspath,
                    &opt_stress_test,
                    (void*)0   /* sentinel */);
}