#pragma once
#include <iostream>

extern int g_cntBrowseSource;
extern int g_cntBrowsePathMatcher;
extern int g_cntBrowseLegacy;

static inline void browseSource()
{
    ++g_cntBrowseSource;
}

static inline void browsePathMatcher()
{
    ++g_cntBrowsePathMatcher;
}

static inline void browseLegacy()
{
    ++g_cntBrowseLegacy;
}

static inline void resetCounters()
{
    g_cntBrowseLegacy=0;
    g_cntBrowsePathMatcher=0;
    g_cntBrowseSource=0;
}

static inline void printCounters()
{
    std::cout << "browse (PathMatcher): " << g_cntBrowsePathMatcher << "\n";
    std::cout << "browse (Source): " << g_cntBrowseSource << "\n";
    std::cout << "browse (Legacy): " << g_cntBrowseLegacy << "\n";
}
