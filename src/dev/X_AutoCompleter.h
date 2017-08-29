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

#pragma once

typedef struct X_AutoCompleter
{
    char* strToMatch;
    int strToMatchLength;
    
    int totalMatches;
    int maxMatches;
    const char** matches;
    
    const char* minMatchStr;
    int minMatchLength;
} X_AutoCompleter;

void x_autocompleter_init(X_AutoCompleter* ac, char* strToMatch, int strToMatchLength, const char** matches, int maxMatches);
void x_autocompleter_add_match_candidate(X_AutoCompleter* ac, const char* candidate);
_Bool x_autocompleter_complete_partial_match(X_AutoCompleter* ac);
void x_autocompleter_sort_matches(X_AutoCompleter* ac);

static inline _Bool x_autocompleter_has_exact_match(const X_AutoCompleter* ac)
{
    return ac->minMatchLength == strlen(ac->minMatchStr);
}

