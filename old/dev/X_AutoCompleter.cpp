// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include <string.h>

#include "X_AutoCompleter.h"
#include "util/X_util.h"

void x_autocompleter_init(X_AutoCompleter* ac, char* strToMatch, int strToMatchLength, const char** matches, int maxMatches)
{
    ac->strToMatch = strToMatch;
    ac->strToMatchLength = strToMatchLength;
    ac->matches = matches;
    ac->maxMatches = maxMatches;
    
    ac->totalMatches = 0;
    ac->minMatchStr = strToMatch;
    ac->minMatchLength = 0x7FFFFFFF;
}

void x_autocompleter_add_match_candidate(X_AutoCompleter* ac, const char* candidate)
{
    int matchLength = x_count_prefix_match_length(ac->minMatchStr, candidate);
    if(matchLength < ac->strToMatchLength)
        return;
    
    ac->matches[ac->totalMatches++] = candidate;
    
    if(ac->minMatchStr == ac->strToMatch)
    {
        ac->minMatchStr = candidate;
        ac->minMatchLength = strlen(candidate);
        return;
    }
    
    if(matchLength >= ac->minMatchLength)
        return;
    
    ac->minMatchLength = matchLength;
    ac->minMatchStr = candidate;
}

bool x_autocompleter_complete_partial_match(X_AutoCompleter* ac)
{    
    if(ac->minMatchStr == ac->strToMatch)
        return 0;
    
    x_strncpy(ac->strToMatch, ac->minMatchStr, ac->minMatchLength);
    return 1;
}

static int compare_matches(const void* a, const void* b)
{
    const char* strA = *((const char **)a);
    const char* strB = *((const char **)b);
    
    return strcmp(strA, strB);
}

void x_autocompleter_sort_matches(X_AutoCompleter* ac)
{
    qsort(ac->matches, ac->totalMatches, sizeof(char**), compare_matches);
}

