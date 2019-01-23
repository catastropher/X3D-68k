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

#include <cctype>

#include "Init.hpp"
#include "error/Error.hpp"
#include "error/Log.hpp"

bool ConfigurationFile::load(const char* fileName, const char* platform_)
{
    // File API has not been initialized at this point, so use c standard library
    FILE* file = fopen(fileName, "r");
    if(!file)
    {
        return false;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    // Need an extra byte for possible null terminator
    fileContents = (char *)malloc(size + 1);

    fread(fileContents, 1, size, file);
    fclose(file);

    platform = platform_;
    currentLocation = fileContents;
    fileEnd = currentLocation + size;

    while(parseLine()) ;

    setSectionParents();

    x_log("Loaded config file: %s", fileName);

    return true;
}

void ConfigurationFile::setSectionParents()
{
    sections[0].setParent(nullptr);

    for(int i = 0; i < totalSections; ++i)
    {
        if(sections[i].getPlatform() == nullptr)
        {
            sections[i].setParent(nullptr);
        }
        else
        {
            sections[i].setParent(getSectionForPlatform(sections[i].getName(), nullptr));
        }
    }
}

bool ConfigurationFile::parseLine()
{
    skipWhitespace();

    if(eof())
        return false;

    if(getCh() == '#')
    {
        // Skip comment line
        parseRestOfLine();
    }
    else if(getCh() == '[')
    {
        parseSectionDecl();
    }
    else
    {
        parseAssignment();
    }

    return true;
}

void ConfigurationFile::parseSectionDecl()
{
    expect('[');

    char* sectionNameStart = getCurrentLocation();
    char* sectionNameEnd = parseIdentifier();

    if(sectionNameStart == sectionNameEnd)
    {
        error("Invalid section name");
    }

    skipWhitespace();

    char* platformNameStart = nullptr;
    char* platformNameEnd = nullptr;

    if(getCh() == ':')
    {
        nextCh();
        skipWhitespace();
        platformNameStart = getCurrentLocation();
        platformNameEnd = parseIdentifier();

        if(platformNameStart == platformNameEnd)
        {
            error("Expected platform name");
        }
    }

    expect(']');

    terminateXString(sectionNameEnd);

    if(platformNameEnd)
    {
        terminateXString(platformNameEnd);
    }

    currentSection = createOrGetSection(sectionNameStart, platformNameStart);
}

void ConfigurationFile::parseAssignment()
{
    char* variableNameStart = getCurrentLocation();
    char* variableNameEnd = parseIdentifier();

    if(variableNameStart == variableNameEnd)
    {
        error("Invalid variable name");
    }

    skipWhitespace();
    expect('=');
    skipWhitespace();

    char* valueStart = getCurrentLocation();
    char* valueEnd = parseRestOfLine();

    terminateXString(variableNameEnd);
    terminateXString(valueEnd);

    if(currentSection == nullptr)
    {
        x_system_error("Var '%s' not in section", variableNameStart);
    }
    else
    {
        currentSection->addVar(variableNameStart, valueStart);
    }

    if(stringIsWhitespace(valueStart))
    {
        error("Missing value");
    }
}

char* ConfigurationFile::parseIdentifier()
{
    do
    {
        char c = getCh();

        if(!isalpha(c) && !isdigit(c) && c != '.' && c != '_' && c != '-')
        {
            return getCurrentLocation();
        }

        nextCh();
    } while(true);
}

char* ConfigurationFile::parseRestOfLine()
{
    while(!eof() && getCh() != '\n')
    {
        nextCh();
    }

    return getCurrentLocation();
}

char ConfigurationFile::getCh()
{
    if(eof())
    {
        return '\0';
    }
    else
    {
        if(*currentLocation == '\n')
            wasNewline = true;

        return *currentLocation;
    }
}

void ConfigurationFile::nextCh()
{
    if(getCh() == '\n' || wasNewline)
    {
        colNumber = 1;
        ++lineNumber;
    }
    else
    {
        ++colNumber;
    }

    ++currentLocation;
    wasNewline = false;
}

void ConfigurationFile::terminateXString(char* strEnd)
{
    *strEnd = '\0';
}

void ConfigurationFile::skipWhitespace()
{
    while(!eof())
    {
        char c = getCh();

        if(c != ' ' && c != '\t' && c != '\n' && c != '\0')
        {
            break;
        }

        nextCh();
    }
}

void ConfigurationFile::expect(char c)
{
    if(getCh() != c)
    {
        error("Unexpected character");
    }

    nextCh();
}

void ConfigurationFile::error(const char* msg)
{
    x_system_error("Error in loading config on line %d, col %d: %s", lineNumber, colNumber, msg);
}

bool ConfigurationFile::eof()
{
    return currentLocation == fileEnd;
}

ConfigurationFile::~ConfigurationFile()
{
    if(fileContents)
    {
        free(fileContents);
    }
}

static int nullstrcmp(const char* a, const char* b)
{
    if(a == nullptr && b == nullptr)
    {
        return 0;
    }

    if(!a || !b)
    {
        return -1;
    }

    return strcmp(a, b);
}

ConfigurationFileSection* ConfigurationFile::createOrGetSection(const char* name, const char* platform)
{
    if(totalSections == MAX_SECTIONS)
    {
        x_system_error("Config file has too many sections");
    }

    for(int i = 1; i < totalSections; ++i)
    {
        if(strcmp(name, sections[i].getName()) == 0 && nullstrcmp(platform, sections[i].getPlatform()) == 0)
        {
            return sections + i;
        }
    }

    sections[totalSections].setName(name);
    sections[totalSections].setPlatform(platform);

    return sections + totalSections++;
}

ConfigurationFileVariable* ConfigurationFileSection::getByName(const char* varName, bool required)
{
    for(int i = 0; i < totalVars; ++i)
    {
        if(strcmp(varName, vars[i].name) == 0)
        {
            return vars + i;
        }
    }

    if(parent != nullptr)
    {
        return parent->getByName(varName, required);
    }

    if(required)
    {
        x_system_error("No value for required config variable '%s' in section '%s'", varName, name);
    }

    return nullptr;
}

void ConfigurationFileSection::addVar(const char* varName, const char* value)
{
    if(totalVars == MAX_VARS)
    {
        x_system_error("Config section '%s' has too many vars", name);
    }

    vars[totalVars].name = varName;
    vars[totalVars].value = value;

    ++totalVars;
}

int ConfigurationFileSection::getInt(const char* varName, bool required, int defaultValue)
{
    auto var = getByName(varName, required);

    if(!var)
    {
        return defaultValue;
    }

    return atoi(var->value);
}

int ConfigurationFileSection::getPositiveInt(const char* varName, bool required, int defaultValue)
{
    int val = getInt(varName, required, defaultValue);

    if(val <= 0)
    {
        x_system_error("Expected positive value for %s", varName);
    }

    return val;
}

ConfigurationFileSection* ConfigurationFile::getSection(const char* name)
{
    return getSectionForPlatform(name, platform);
}

ConfigurationFileSection* ConfigurationFile::getSectionForPlatform(const char* name, const char* platformName)
{
    ConfigurationFileSection* matchingSection = nullptr;

    for(int i = 1; i < totalSections; ++i)
    {
        bool nameMatches = strcmp(name, sections[i].getName()) == 0;
        bool platformMatches = nullstrcmp(platformName, sections[i].getPlatform()) == 0;

        if(nameMatches)
        {
            if(platformMatches)
            {
                return sections + i;
            }
            else if(sections[i].getPlatform() == nullptr)
            {
                matchingSection = sections + i;
            }
        }
    }

    if(matchingSection)
    {
        return matchingSection;
    }

    // Section is missing, so return the default section with the expected name
    sections[0].setName(name);

    return sections + 0;
}

