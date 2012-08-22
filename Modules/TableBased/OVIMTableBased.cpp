//
// OVIMTableBased.cpp
//
// Copyright (c) 2004-2012 Lukhnos Liu (lukhnos at openvanilla dot org)
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#include "OVIMTableBased.h"
#include "OVIMTableBasedContext.h"

using namespace OpenVanilla;

OVIMTableBased::OVIMTableBased(const string& tablePath)
    : m_tablePath(tablePath)
    , m_table(0)
    , m_configClearReadingBufferAtCompositionError(false)
    , m_configComposeWhileTyping(false)
    , m_configMaximumRadicalLength(5)
    , m_configSendFirstCandidateWithSpaceWithOnePageList(true)
    , m_configShouldCommitAtMaximumRadicalLength(true)
    , m_configUseSpaceAsFirstCandidateSelectionKey(false)
    , m_configMatchOneChar('?')
    , m_configMatchZeroOrMoreChar('*')
{
}

OVIMTableBased::~OVIMTableBased()
{
    if (m_table) {
        delete m_table;
    }
}

OVEventHandlingContext* OVIMTableBased::createContext()
{
    return new OVIMTableBasedContext(this);
}

const string OVIMTableBased::identifier() const
{
    string filename = OVPathHelper::FilenameWithoutPath(m_tablePath);
    string basename = OVPathHelper::FilenameWithoutExtension(filename);
    if (!basename.length()) {
        basename = "undefined";
    }

    return string("org.openvanilla.OVIMTableBased.") + basename;
}

const string OVIMTableBased::localizedName(const string& locale)
{
    bool find;
    string value;

    if (locale == "zh-Hant" || locale == "zh-TW" || locale == "zh_TW" || locale == "zh-HK" || locale == "zh_HK") {
        find = fetchTableProperty("tcname", value);
        if (find) {
            return value;
        }
    }

    if (locale.find("zh") == 0) {
        find = fetchTableProperty("scname", value);
        if (find) {
            return value;
        }

        find = fetchTableProperty("tcname", value);
        if (find) {
            return value;
        }


        find = fetchTableProperty("cname", value);
        if (find) {
            return value;
        }
    }

    find = fetchTableProperty("ename", value);
    if (find) {
        return value;
    }

    find = fetchTableProperty("name", value);
    if (find) {
        return value;
    }

    return identifier();
}


bool OVIMTableBased::initialize(OVPathInfo* pathInfo, OVLoaderService* loaderService)
{
    if (!OVPathHelper::PathExists(m_tablePath)) {
        return false;
    }

    m_tableTimestamp = OVPathHelper::TimestampForPath(m_tablePath);
    m_preloadedTableProperties = OVCINDataTableParser::QuickParseProperty(m_tablePath);
    if (!m_preloadedTableProperties.size()) {
        return false;
    }

    return true;
}

void OVIMTableBased::loadConfig(OVKeyValueMap* moduleConfig, OVLoaderService* loaderService)
{
    if (moduleConfig->hasKey("ClearReadingBufferAtCompositionError")) {
        m_configClearReadingBufferAtCompositionError = moduleConfig->isKeyTrue("ClearReadingBufferAtCompositionError");
    }

    if (moduleConfig->hasKey("ComposeWhileTyping")) {
        m_configComposeWhileTyping = moduleConfig->isKeyTrue("ComposeWhileTyping");
    }

    if (moduleConfig->hasKey("MatchOneChar")) {
        string ch = moduleConfig->stringValueForKey("MatchOneChar");
        if (ch.length()) {
            m_configMatchOneChar = ch[0];
        }
    }

    if (moduleConfig->hasKey("MatchZeroOrMoreChar")) {
        string ch = moduleConfig->stringValueForKey("MatchZeroOrMoreChar");
        if (ch.length()) {
            m_configMatchZeroOrMoreChar = ch[0];
        }
    }

    if (moduleConfig->hasKey("MaximumRadicalLength")) {
        size_t length = (size_t)moduleConfig->intValueForKey("MaximumRadicalLength");
        m_configMaximumRadicalLength = length ? length : m_configMaximumRadicalLength;
    }

    if (moduleConfig->hasKey("SendFirstCandidateWithSpaceWithOnePageList")) {
        m_configSendFirstCandidateWithSpaceWithOnePageList = moduleConfig->isKeyTrue("SendFirstCandidateWithSpaceWithOnePageList");
    }

    if (moduleConfig->hasKey("ShouldCommitAtMaximumRadicalLength")) {
        m_configShouldCommitAtMaximumRadicalLength = moduleConfig->isKeyTrue("ShouldCommitAtMaximumRadicalLength");
    }

    if (moduleConfig->hasKey("UseSpaceAsFirstCandidateSelectionKey")) {
        m_configUseSpaceAsFirstCandidateSelectionKey = moduleConfig->isKeyTrue("UseSpaceAsFirstCandidateSelectionKey");
    }
}

void OVIMTableBased::saveConfig(OVKeyValueMap* moduleConfig, OVLoaderService* loaderService)
{
    moduleConfig->setKeyBoolValue("ClearReadingBufferAtCompositionError", m_configClearReadingBufferAtCompositionError);
    moduleConfig->setKeyBoolValue("ComposeWhileTyping", m_configComposeWhileTyping);
    moduleConfig->setKeyStringValue("MatchOneChar", string(1, m_configMatchOneChar));
    moduleConfig->setKeyStringValue("MatchZeroOrMoreChar", string(1, m_configMatchZeroOrMoreChar));
    moduleConfig->setKeyIntValue("MaximumRadicalLength", (int)m_configMaximumRadicalLength);
    moduleConfig->setKeyBoolValue("SendFirstCandidateWithSpaceWithOnePageList", m_configSendFirstCandidateWithSpaceWithOnePageList);
    moduleConfig->setKeyBoolValue("ShouldCommitAtMaximumRadicalLength", m_configShouldCommitAtMaximumRadicalLength);
    moduleConfig->setKeyBoolValue("UseSpaceAsFirstCandidateSelectionKey", m_configUseSpaceAsFirstCandidateSelectionKey);
}

bool OVIMTableBased::fetchTableProperty(const string& key, string& outValue)
{
    if (m_table) {
        string value = m_table->findProperty(key);
        if (value.length()) {
            outValue = value;
            return true;
        }
        return false;
    }

    map<string, string>::iterator f = m_preloadedTableProperties.find(key);
    if (f == m_preloadedTableProperties.end()) {
        return false;
    }

    outValue = (*f).second;
    return true;
}

void OVIMTableBased::checkTable()
{
    if (!OVPathHelper::PathExists(m_tablePath)) {
        return;
    }

    OVFileTimestamp timestamp = OVPathHelper::TimestampForPath(m_tablePath);
    if (timestamp > m_tableTimestamp && m_table) {
        delete m_table;
        m_table = 0;
        m_tableTimestamp = timestamp;
    }

    OVCINDataTableParser parser;
    m_table = parser.CINDataTableFromFileName(m_tablePath);
}