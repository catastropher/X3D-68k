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

#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;

void replace(string original, string replacement)
{
    char cmd[4096];
    sprintf(
        cmd,
        "cd .. && git grep -l '%s' | xargs sed -i 's/%s/%s/g'",
        original.c_str(),
        original.c_str(),
        replacement.c_str());

    system(cmd);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout << "Usage: " << argv[0] << " [original] [new]" << endl;
    }

    string original = argv[1];
    string replacement = argv[2];

    replace(original, replacement);
    replace(replacement + ".h", original + ".h");
    replace(replacement + ".cpp", original + ".cpp");
    replace(replacement + ".hpp", original + ".hpp");
}

