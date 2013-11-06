/**
Copyright (c) 2008 by Matjaz Kovac

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

\file RegExp.h
\author Matjaž Kovač
*/

#ifndef _REGEX_H_
#define _REGEX_H_

#ifdef __HAIKU__
#define __BEOS__
#endif

#include <regex.h>
#include <string.h>
#ifdef __BEOS__
#include <String.h>
#endif 

/**
	POSIX RegEx Engine Interface.
*/
class RegExp
{
public:

	/**
		Compiles a reqular expression pattern.
		\param pattern POSIX-compliant regular expression pattern
		\param flags REG_EXTENDED, REG_ICASE, REG_NOSUB, REG_NEWLINE
		\param max_substrings Maximum number of match substrings
	*/
	RegExp(const char *pattern, int flags = REG_EXTENDED, int max_substrings = 10):
		nsub(max_substrings),
		source(NULL)
	{
		match = new regmatch_t[nsub];
		status = regcomp(&re, pattern, flags);
	};


	~RegExp()
	{
		regfree(&re);
		delete [] match;
	}

	
	inline int Status()
	{
		return status;
	}

	/**
		Test the compiled pattern on a source string.
		Just a dry-run, state is not stored.
		\param s string to test
		\param flags REG_NOTBOL, REG_NOTEOL
	*/
	int Test(const char *s, int flags = 0)
	{
		return regexec(&re, s, 0, NULL, flags);
	}


	/**
		Matches a string against the complied pattern.
		Gets the first match and saves the source pointer.
		If 'src' is NULL the stored pointer is used to obtain 
		the next possible match, if there was one before.
		This makes it possible to use string constants
		but do not then call Match(NULL) out-of-scope!
		The expression must be compiled *without* REG_NOSUB.
		\param src string to match
		\param flags REG_NOTBOL, REG_NOTEOL
	*/
	int Match(const char *src, int flags = 0)
	{
		if (src)
			// first match
			source = src;
		else if (!status)
			// next match, if possible
			source += match[0].rm_eo;
		else
			return status;
        status = regexec(&re, source, nsub, match, flags);
		return status;
	}


	/**
		Copies a matched substring.
		Only works after a successful Match().
		Index 0 is the entire matched regex,
		1 and above are parenthesized subexpressions.
        The expression must be compiled *without* REG_NOSUB.
	*/
	const char* Sub(unsigned int index, char *dest) const
	{
		if (status || index > re.re_nsub + 1  || index >= nsub)
			return NULL;
		int n = match[index].rm_eo - match[index].rm_so;
		dest = strncpy(dest, source + match[index].rm_so, n);
		return dest;
	}


#ifdef __BEOS__
	/**
		Copies a matched substring into a BString.
		Only works after a successful Match().
		Index 0 is the entire matched regex,
		1 and above are parenthesized subexpressions.
        The expression must be compiled *without* REG_NOSUB.
	*/
	const char* Sub(unsigned int index, BString &s) const
	{
		if (status || index > re.re_nsub + 1  || index >= nsub)
			return NULL;
		int n = match[index].rm_eo - match[index].rm_so;
		char *dest = s.LockBuffer(n+1);
		strncpy(dest, source + match[index].rm_so, n);
		dest[n] = 0;
		s.UnlockBuffer();
		return dest;
	}
#endif

private:

	int status;
	unsigned nsub;
	const char *source;
	regex_t re;
	regmatch_t* match;
};

#endif

