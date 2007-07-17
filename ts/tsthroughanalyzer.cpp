/* This file is part of the KDE project
 * Copyright (C) 2007 Montel Laurent <montel@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#define STRIGI_IMPORT_API
#include <strigi/streamthroughanalyzer.h>
#include <strigi/analyzerplugin.h>
#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>

//kde include
#include <KUrl>

//qt include
#include <QFile>
#include <QTextStream>


using namespace std;
using namespace Strigi;

class TsThroughAnalyzerFactory;
class TsThroughAnalyzer : public StreamThroughAnalyzer {
    private:
        const TsThroughAnalyzerFactory* factory;
        AnalysisResult* idx;
        const char* name() const {
	   return "TsThroughAnalyzer";
	} 
        void setIndexable( AnalysisResult *i ) {
            idx = i;
        }
        InputStream* connectInputStream( InputStream *in );
        bool isReadyWithStream() { return true; }
    public:
        TsThroughAnalyzer( const TsThroughAnalyzerFactory* f ) : factory( f ) {}
};

class TsThroughAnalyzerFactory : public StreamThroughAnalyzerFactory {
private:
    const char* name() const {
        return "TsThroughAnalyzer";
    }
    StreamThroughAnalyzer* newInstance() const {
        return new TsThroughAnalyzer(this);
    }
    void registerFields( FieldRegister& );

    static const std::string messageFieldName;
    static const std::string translatedFieldName;
    static const std::string untranslatedFieldName;
    static const std::string obsoleteFieldName;
public:
    const RegisteredField* messageField;
    const RegisteredField* translatedField;
    const RegisteredField* untranslatedField;
    const RegisteredField* obsoleteField;
};

const std::string TsThroughAnalyzerFactory::messageFieldName( "message" );
const std::string TsThroughAnalyzerFactory::translatedFieldName( "translated");
const std::string TsThroughAnalyzerFactory::untranslatedFieldName( "untranslated");
const std::string TsThroughAnalyzerFactory::obsoleteFieldName("obsolete");

void TsThroughAnalyzerFactory::registerFields( FieldRegister& reg ) {
	messageField = reg.registerField( messageFieldName, FieldRegister::integerType, 1, 0 );
	translatedField = reg.registerField( translatedFieldName, FieldRegister::integerType, 1, 0 );
	untranslatedField = reg.registerField(untranslatedFieldName, FieldRegister::integerType, 1, 0 );
	obsoleteField = reg.registerField(obsoleteFieldName, FieldRegister::stringType, 1, 0 );
}

InputStream* TsThroughAnalyzer::connectInputStream( InputStream* in ) {
    const string& path = idx->path();
    QFile f(path.c_str());
    if (!f.open(IO_ReadOnly))
        return in;

    int messages      = 0;
    int untranslated  = 0;
    int obsolete      = 0;

    QTextStream stream( &f );
    QString line = stream.readLine();

    // is it really a linguist file?
    if (!line.contains("<!DOCTYPE TS>", false))
        return in;

    do 
    {
        line = stream.readLine();

        if (line.contains("type=\"obsolete\"")) obsolete++;

        if (line.contains("<source>")) messages++;

        if (line.contains("type=\"unfinished\"")) untranslated++;

    }
    while (!line.isNull());

    idx->addValue( factory->messageField,messages);
    idx->addValue( factory->translatedField,(messages-untranslated-obsolete));
    idx->addValue( factory->untranslatedField,untranslated);
    idx->addValue( factory->obsoleteField,obsolete);
    return in;
}

class Factory : public AnalyzerFactoryFactory {
public:
    std::list<StreamThroughAnalyzerFactory*>
    streamThroughAnalyzerFactories() const {
        std::list<StreamThroughAnalyzerFactory*> af;
        af.push_back(new TsThroughAnalyzerFactory());
        return af;
    }
};

STRIGI_ANALYZER_FACTORY(Factory) 

