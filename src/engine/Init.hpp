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

#include "render/Screen.hpp"
#include "error/Error.hpp"
#include "util/Json.hpp"

struct ConfigurationFileVariable
{
    const char* name;
    const char* value;
};

class ConfigurationFileSection
{
public:
    ConfigurationFileSection() : totalVars(0)
    {
    }

    void setName(const char* newName)
    {
        name = newName;
    }

    const char* getName() const
    {
        return name;
    }

    void setPlatform(const char* newPlatform)
    {
        platform = newPlatform;
    }

    const char* getPlatform() const
    {
        return platform;
    }

    void setParent(ConfigurationFileSection* newParent)
    {
        parent = newParent;
    }

    void addVar(const char* varName, const char* value);
    int getInt(const char* varName, bool required = false, int defaultValue = 0);
    int getPositiveInt(const char* varName, bool requried = false, int defaultValue = 1);
    void getStr(const char* varName, char* dest, bool requied = false, const char* defaultValue = "");

private:
    ConfigurationFileVariable* getByName(const char* name, bool required);

    static const int MAX_VARS = 32;

    const char* name;
    const char* platform;
    ConfigurationFileVariable vars[MAX_VARS];
    int totalVars;
    ConfigurationFileSection* parent;
};

class ConfigurationFile
{
public:
    ConfigurationFile()
        : currentSection(nullptr),
        totalSections(1),
        lineNumber(1),
        colNumber(1),
        fileContents(nullptr),
        wasNewline(false)
    {

    }

    bool load(const char* fileName, const char* platform_);
    ConfigurationFileSection* getSection(const char* name);
    ConfigurationFileSection* getSectionForPlatform(const char* name, const char* platformName);

    ~ConfigurationFile();

private:
    char* getCurrentLocation()
    {
        return currentLocation;
    }

    static bool isValidIdentifier(const char* str)
    {
        int length = strlen(str);

        return length > 0 && length < MAX_SECTION_NAME_LENGTH;
    }

    static bool stringIsWhitespace(const char* str)
    {
        while(*str)
        {
            if(*str != ' ' || *str != '\n')
                return false;
        }

        return true;
    }

    void setSectionParents();

    bool loadLine();
    void skipWhitespace();
    bool parseLine();
    char getCh();
    void nextCh();
    void terminateXString(char* strEnd);
    bool eof();
    
    void parseSectionDecl();
    void parseAssignment();
    char* parseIdentifier();
    char* parseRestOfLine();

    void expect(char c);
    void error(const char* msg);
    ConfigurationFileSection* createOrGetSection(const char* name, const char* platform);

    static const int MAX_SECTIONS = 64;
    static const int MAX_LINE_LENGTH = 1024;
    static const int MAX_SECTION_NAME_LENGTH = 128;

    ConfigurationFileSection sections[MAX_SECTIONS];
    ConfigurationFileSection* currentSection;
    int totalSections;
    
    int lineNumber;
    int colNumber;

    char* fileContents;
    char* fileEnd;
    char* currentLocation;

    bool wasNewline;
    const char* platform;
};

struct ScreenConfig
{
    ScreenConfig() : fov(0)     // Because fp doesn't have a default constructor yet
    {
        screenHandlers.displayFrame = NULL;
        screenHandlers.isValidResolution = NULL;
        screenHandlers.restartVideo = NULL;
        screenHandlers.userData = NULL;

        fov = 0;
        w = 320;
        h = 240;
        fullscreen = false;
    }

    ScreenConfig& displayFrameCallback(void (*callback)(struct Screen* screen, void* userData))
    {
        screenHandlers.displayFrame = callback;

        return *this;
    }

    ScreenConfig& restartVideoCallback(void (*callback)(struct X_EngineContext* context, void* userData))
    {
        screenHandlers.restartVideo = callback;

        return *this;
    }

    ScreenConfig& isValidResolutionCallback(bool (*callback)(int w, int h))
    {
        screenHandlers.isValidResolution = callback;

        return *this;
    }

    ScreenConfig& cleanupVideoCallback(void (*callback)(struct X_EngineContext* context, void* userData))
    {
        screenHandlers.cleanupVideo = callback;

        return *this;
    }

    ScreenConfig& userData(void* data)
    {
        screenHandlers.userData = data;

        return *this;
    }

    ScreenConfig& resolution(int w, int h)
    {
        this->w = w;
        this->h = h;

        return *this;
    }

    ScreenConfig& fieldOfView(fp fov)
    {
        this->fov = fov;

        return *this;
    }

    ScreenConfig& enableFullscreen()
    {
        fullscreen = true;

        return *this;
    }

    ScreenConfig& colorPalette(const X_Palette* palette)
    {
        palette = palette;

        return *this;
    }

    ScreenConfig& useQuakeColorPalette()
    {
        palette = x_palette_get_quake_palette();

        return *this;
    }

    int w;
    int h;
    fp fov;
    bool fullscreen;
    const X_Palette* palette;

    ScreenEventHandlers screenHandlers;
};

struct SystemConfig
{
    const char* programPath = nullptr;
    const char* logFile = "engine.log";
    int hunkSize = 4 * 1024 * 1024;
    int zoneSize = 1024 * 1024;
    bool enableLogging = true;
};

struct X_Config
{
    X_Config()
    {
        path = nullptr;
        font = "font.xtex";
    }

    X_Config& screenConfig(ScreenConfig& config)
    {
        screen = &config;

        return *this;
    }

    X_Config& programPath(const char* path)
    {
        this->path = path;

        return *this;
    }

    X_Config& defaultFont(const char* fileName)
    {
        font = fileName;

        return *this;
    }

    X_Config& configurationFile(const char* fileName, const char* platformName)
    {
        if(!configFile.load(fileName, platformName))
        {
            x_system_error("Failed to load config %s\n", fileName);
        }

        return *this;
    }
    
    const char* path;
    const char* font;
    ScreenConfig* screen;
    ConfigurationFile configFile;
    SystemConfig systemConfig;
};

void x_config_init(X_Config* config);

