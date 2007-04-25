/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#define STRIGI_IMPORT_API
#include <strigi/streamthroughanalyzer.h>
#include <strigi/analyzerplugin.h>
#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>

#include "difflineanalyzer.h"

#include <QRegExp>
#include <QString>

using namespace std;
using namespace Strigi;

void DiffLineAnalyzerFactory::registerFields(FieldRegister& reg) {
    nbFilesField = reg.registerField("files" , FieldRegister::integerType, 1, 0);
    firstFileField = reg.registerField("first_file" , FieldRegister::stringType, 1, 0);    
    formatField = reg.registerField("format" , FieldRegister::stringType, 1, 0);
    diffProgramField = reg.registerField("diff program" , FieldRegister::stringType, 1, 0);
    hunksField = reg.registerField("hunks" , FieldRegister::integerType, 1, 0);
    insertFilesField = reg.registerField("insert_files" , FieldRegister::integerType, 1, 0);
    modifyFilesField = reg.registerField("modify_files" , FieldRegister::integerType, 1, 0);
    deleteFilesField = reg.registerField("delete_files" , FieldRegister::integerType, 1, 0); 
}

void DiffLineAnalyzer::startAnalysis(AnalysisResult* i) {
    analysisResult = i;
    ready = false;
    diffFormat = DiffLineAnalyzer::Unknown;
    nbFiles = 0;
    hunks = 0;
    insertFiles = 0;
    modifyFiles = 0;
    deleteFiles = 0;
}

void DiffLineAnalyzer::handleLine(const char* data, uint32_t length) {
    QString line(data);
    if(diffFormat == DiffLineAnalyzer::Unknown) //search format
    {
        if ( QRegExp( "^[0-9]+[0-9,]*[acd][0-9]+[0-9,]*$" ).exactMatch( line ) )
        {
            diffFormat = DiffLineAnalyzer::Normal;
        }
        else if ( line.contains( QRegExp( "^--- " ) ) )
        {
            // unified has first a '^--- ' line, then a '^+++ ' line
            diffFormat = DiffLineAnalyzer::Unified;
        }
        else if ( line.contains( QRegExp( "^\\*\\*\\* [^\\t]+\\t" ) ) )
        {
            // context has first a '^*** ' line, then a '^--- ' line
            diffFormat = DiffLineAnalyzer::Context;
        }
        else if ( line.contains( QRegExp( "^[acd][0-9]+ [0-9]+" ) ) )
        {
            diffFormat =  DiffLineAnalyzer::RCS;
        }
        else if ( line.contains( QRegExp( "^[0-9]+[0-9,]*[acd]" ) ) )
        {
            diffFormat = DiffLineAnalyzer::Ed;
        }
    }
    else //analyse files
    {
    }
}

void DiffLineAnalyzer::endAnalysis(){
    //don't add info if we didn't know diff format
    if(diffFormat != DiffLineAnalyzer::Unknown)
    {
    }
    ready = true;
}

bool DiffLineAnalyzer::isReadyWithStream() {
    return ready;
}

class Factory : public AnalyzerFactoryFactory {
public:
    list<StreamLineAnalyzerFactory*>
    streamLineAnalyzerFactories() const {
        list<StreamLineAnalyzerFactory*> af;
        af.push_back(new DiffLineAnalyzerFactory());
        return af;
    }
};

STRIGI_ANALYZER_FACTORY(Factory)

