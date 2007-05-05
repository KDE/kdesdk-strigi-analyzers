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

#include <klocale.h>

using namespace std;
using namespace Strigi;

void DiffLineAnalyzerFactory::registerFields(FieldRegister& reg) {
    nbFilesField = reg.registerField("diff.stats.modify_file_count" , FieldRegister::integerType, 1, 0);
    firstFileField = reg.registerField("diff.first_modify_file" , FieldRegister::stringType, 1, 0);    
    formatField = reg.registerField("content.format_subtype" , FieldRegister::stringType, 1, 0);
    diffProgramField = reg.registerField("content.generator" , FieldRegister::stringType, 1, 0);
    hunksField = reg.registerField("diff.stats.hunk_count" , FieldRegister::integerType, 1, 0);
    insertFilesField = reg.registerField("diff.stats.insert_line_count" , FieldRegister::integerType, 1, 0);
    modifyFilesField = reg.registerField("diff.stats.modify_line_count" , FieldRegister::integerType, 1, 0);
    deleteFilesField = reg.registerField("diff.stats.delete_line_count" , FieldRegister::integerType, 1, 0); 
}

void DiffLineAnalyzer::startAnalysis(AnalysisResult* i) {
    analysisResult = i;
    ready = false;
    indexFound = false;
    diffFormat = DiffLineAnalyzer::Unknown;
    diffProgram = DiffLineAnalyzer::Undeterminable;
    numberOfFiles = 0;
    numberOfHunks = 0;
    numberOfAdditions = 0;
    numberOfChanges = 0;
    numberOfDeletions = 0;

}

void DiffLineAnalyzer::handleLine(const char* data, uint32_t length) {
    QString line(data);

    QRegExp diffRE( "^diff .*" );
    QRegExp p4sRE("^==== ");
    QRegExp firstFile( "^Index: (.*)" );
    if(line.startsWith( "Index:" ) && !indexFound)
    {
	if(firstFile.exactMatch( line))
	{
        	QString filename = firstFile.cap(1);
        	analysisResult->addValue(factory->firstFileField, (const char*)filename.toUtf8());
	}
	indexFound = true;
    }
    else if ( line.startsWith( "retrieving revision") )
	diffProgram = DiffLineAnalyzer::CVSDiff;
    else if ( diffRE.exactMatch( line ) )
        diffProgram = DiffLineAnalyzer::Diff;
    else if ( p4sRE.exactMatch( line ) )
        diffProgram = DiffLineAnalyzer::Perforce;
    
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
	QRegExp edAdd( "([0-9]+)(|,([0-9]+))a" );
	QRegExp edDel( "([0-9]+)(|,([0-9]+))d" );
	QRegExp edMod( "([0-9]+)(|,([0-9]+))c" );

	QRegExp normalAdd( "[0-9]+a([0-9]+)(|,([0-9]+))" );
	QRegExp normalDel( "([0-9]+)(|,([0-9]+))d(|[0-9]+)" );
	QRegExp normalMod( "([0-9]+)(|,([0-9]+))c([0-9]+)(|,([0-9]+))" );

	QRegExp rcsAdd( "a[0-9]+ ([0-9]+)" );
	QRegExp rcsDel( "d[0-9]+ ([0-9]+)" );


	switch( diffFormat )
	{
	case DiffLineAnalyzer::Context:
		if ( line.startsWith("***************") )
		{
			numberOfHunks++;
			//kDebug(7034) << "Context Hunk      : " << line << endl;
		}
		else if ( line.startsWith("***") )
		{
			numberOfFiles++;
			//kDebug(7034) << "Context File      : " << line << endl;
		}
		else if ( line.startsWith("---") ) {} // ignore
		else if ( line.startsWith("+") )
		{
			numberOfAdditions++;
//			kDebug(7034) << "Context Insertion : " << line << endl;
		}
		else if ( line.startsWith("-") )
		{
			numberOfDeletions++;
//			kDebug(7034) << "Context Deletion  : " << line << endl;
		}
		else if ( line.startsWith("!") )
		{
			numberOfChanges++;
//			kDebug(7034) << "Context Modified  : " << line << endl;
		}
		else if ( line.startsWith(" ") )
		{
//				kDebug(7034) << "Context Context   : " << line << endl;
		}
		else
		{
//				kDebug(7034) << "Context Unknown   : " << line << endl;
		}
#if 0
		(*numberOfChanges) /= 2; // changes are in both parts of the hunks
		(*numberOfFiles) -= (*numberOfHunks); // it counts old parts of a hunk as files :(
#endif
		break;
	case DiffLineAnalyzer::Ed:
		if ( line.startsWith( "diff" ) )
		{
			numberOfFiles++;
//			kDebug(7034) << "Ed File         : " << line << endl;
		}
		else if ( edAdd.exactMatch( line ) )
		{
//				kDebug(7034) << "Ed Insertion    : " << line << endl;
			numberOfHunks++;
#if 0			
				while( it != lines.end() && !(*it).startsWith(".") )
				{
					(*numberOfAdditions)++;
//					kDebug(7034) << "Ed Insertion    : " << (*it) << endl;
					++it;
				}
#endif
		}
		else if ( edDel.exactMatch( line ) )
		{
//				kDebug(7034) << "Ed Deletion     : " << line << endl;
			numberOfHunks++;
			numberOfDeletions += (edDel.cap(3).isEmpty() ? 1 : edDel.cap(3).toInt() - edDel.cap(1).toInt() + 1);
//			kDebug(7034) << "Ed noOfLines    : " << (edDel.cap(3).isEmpty() ? 1 : edDel.cap(3).toInt() - edDel.cap(1).toInt() + 1) << endl;
		}
		else if ( edMod.exactMatch( line ) )
		{
//				kDebug(7034) << "Ed Modification : " << line << endl;
			if ( edMod.cap(3).isEmpty() )
				numberOfDeletions++;
			else
				numberOfDeletions += edMod.cap(3).toInt() - edMod.cap(1).toInt() + 1;
			numberOfHunks++;
#if 0
				++it;
				while( it != lines.end() && !(*it).startsWith(".") )
				{
					(*numberOfAdditions)++;
//					kDebug(7034) << "Ed Modification : " << (*it) << endl;
					++it;
				}
#endif
		}
		else
		{
//				kDebug(7034) << "Ed Unknown      : " << (*it) << endl;
		}

		break;
	case DiffLineAnalyzer::Normal:
		if ( line.startsWith( "diff" ) )
		{
			numberOfFiles++;
//			kDebug(7034) << "Normal File         : " << line << endl;
		}
		else if ( normalAdd.exactMatch( line ) )
		{
//				kDebug(7034) << "Normal Insertion    : " << line << endl;
			numberOfHunks++;
			if ( normalAdd.cap(3).isEmpty() )
			{
				numberOfAdditions++;
//				kDebug(7034) << "Normal Addition : " << 1 << endl;
			}
			else
			{
				numberOfAdditions += normalAdd.cap(3).toInt() - normalAdd.cap(1).toInt() + 1;
//					kDebug(7034) << "Normal Addition : " << normalAdd.cap(3).toInt() - normalAdd.cap(1).toInt() + 1 << endl;
			}
		}
		else if ( normalDel.exactMatch(line) )
			{
//				kDebug(7034) << "Normal Deletion     : " << line << endl;
			numberOfHunks++;
			if ( normalDel.cap(3).isEmpty() )
			{
				numberOfDeletions++;
//					kDebug(7034) << "Normal Deletion : " << 1 << endl;
			}
			else
			{
				numberOfDeletions += normalDel.cap(3).toInt() - normalDel.cap(1).toInt() + 1;
//					kDebug(7034) << "Normal Deletion : " << normalDel.cap(3).toInt() - normalDel.cap(1).toInt() + 1 << endl;
			}
		}
		else if ( normalMod.exactMatch( line ) )
			{
//				kDebug(7034) << "Normal Modification : " << line << endl;
			numberOfHunks++;
			if ( normalMod.cap(3).isEmpty() )
			{
				numberOfDeletions++;
//					kDebug(7034) << "Normal Deletion : " << 1 << endl;
			}
			else
			{
				numberOfDeletions += normalMod.cap(3).toInt() - normalMod.cap(1).toInt() + 1;
//					kDebug(7034) << "Normal Deletion : " << normalMod.cap(3).toInt() - normalMod.cap(1).toInt() + 1 << endl;
			}
			if ( normalMod.cap(6).isEmpty() )
			{
				numberOfAdditions++;
//					kDebug(7034) << "Normal Addition : " << 1 << endl;
			}
			else
			{
				numberOfAdditions += normalMod.cap(6).toInt() - normalMod.cap(4).toInt() + 1;
//					kDebug(7034) << "Normal Addition : " << normalMod.cap(6).toInt() - normalMod.cap(4).toInt() + 1 << endl;
			}
		}
		else if ( line.startsWith(">") )
		{
//				numberOfAdditions++;
//				kDebug(7034) << "Normal Insertion    : " << line << endl;
		}
		else if ( line.startsWith("<") )
		{
//				numberOfDeletions++;
//				kDebug(7034) << "Normal Deletion     : " << line << endl;
		}
		else
		{
//			kDebug(7034) << "Normal Unknown      : " << line << endl;
		}
		break;
	case DiffLineAnalyzer::RCS:
		if ( line.startsWith( "diff" ) ) // works for cvs diff, have to test for normal diff
		{
//				kDebug(7034) << "RCS File      : " << line << endl;
			numberOfFiles++;
		}
		else if ( rcsAdd.exactMatch( line ) )
		{
//				kDebug(7034) << "RCS Insertion : " << line << endl;
			numberOfHunks++;
			numberOfAdditions += rcsAdd.cap(1).toInt();
//				kDebug(7034) << "RCS noOfLines : " << rcsAdd.cap(1).toInt() << endl;
		}
		else if ( rcsDel.exactMatch( line ) )
		{
//				kDebug(7034) << "RCS Deletion  : " << line << endl;
			numberOfHunks++;
			numberOfDeletions += rcsDel.cap(1).toInt();
//				kDebug(7034) << "RCS noOfLines : " << rcsDel.cap(1).toInt() << endl;
		}
		else
		{
//				kDebug(7034) << "RCS Unknown   : " << line << endl;
		}
		break;
	case DiffLineAnalyzer::Unified:
		if ( line.startsWith("@@ ") )
		{
			numberOfHunks++;
			//kDebug(7034) << "Unified Hunk      : " << line << endl;
		}
		else if ( line.startsWith("---") )
		{
			numberOfFiles++;
			//kDebug(7034) << "Unified File      : " << line << endl;
		}
		else if ( line.startsWith("+++") ) {} // ignore (don't count as insertion)
		else if ( line.startsWith("+") )
		{
			numberOfAdditions++;
			//kDebug(7034) << "Unified Insertion : " << line << endl;
		}
		else if ( line.startsWith("-") )
		{
			numberOfDeletions++;
			//kDebug(7034) << "Unified Deletion  : " << line << endl;
		}
		else if ( line.startsWith(" ") )
		{
			//kDebug(7034) << "Unified Context   : " << line << endl;
		}
		else
		{
			//kDebug(7034) << "Unified Unknown   : " << line << endl;
		}
		break;
	case DiffLineAnalyzer::Empty:
	case DiffLineAnalyzer::Unknown:
	case DiffLineAnalyzer::SideBySide:
		break;
	}	    
    }
}

void DiffLineAnalyzer::endAnalysis(){
    //don't add info if we didn't know diff format
    if(diffFormat != DiffLineAnalyzer::Unknown)
    {
       analysisResult->addValue(factory->formatField, (const char*)determineI18nedFormat(diffFormat).toUtf8());
       if ( indexFound && diffProgram ==DiffLineAnalyzer::Undeterminable) // but no "retrieving revision" found like only cvs diff adds.
          diffProgram = DiffLineAnalyzer::SubVersion;
       analysisResult->addValue(factory->diffProgramField, (const char*)determineI18nedProgram(diffProgram).toUtf8());
       analysisResult->addValue(factory->nbFilesField, numberOfFiles);
       analysisResult->addValue(factory->insertFilesField, numberOfAdditions);
       analysisResult->addValue(factory->modifyFilesField, numberOfChanges);
       analysisResult->addValue(factory->deleteFilesField, numberOfDeletions);
       analysisResult->addValue(factory->hunksField, numberOfHunks);
    }
    ready = true;
}

const QString DiffLineAnalyzer::determineI18nedProgram( DiffLineAnalyzer::DiffProgram diffProgram ) const
{
    QString program;

    switch( diffProgram )
    {
    case DiffLineAnalyzer::CVSDiff:
        program = i18n( "CVSDiff" );
        break;
    case DiffLineAnalyzer::Diff:
        program = i18n( "Diff" );
        break;
    case DiffLineAnalyzer::Diff3:
        program = i18n( "Diff3" );
        break;
    case DiffLineAnalyzer::Perforce:
        program = i18n( "Perforce" );
        break;
    case DiffLineAnalyzer::SubVersion:
        program = i18n( "SubVersion" );
        break;
    case DiffLineAnalyzer::Undeterminable:
        program = i18n( "Unknown" );
        break;
    }
    return program;
}


const QString DiffLineAnalyzer::determineI18nedFormat( DiffLineAnalyzer::Format diffFormat ) const
{
    QString format;
    switch( diffFormat )
    {
    case DiffLineAnalyzer::Context:
        format = i18n( "Context" );
        break;
    case DiffLineAnalyzer::Ed:
        format = i18n( "Ed" );
        break;
    case DiffLineAnalyzer::Normal:
        format = i18n( "Normal" );
        break;
    case DiffLineAnalyzer::RCS:
        format = i18n( "RCS" );
        break;
    case DiffLineAnalyzer::Unified:
        format = i18n( "Unified" );
        break;
    case DiffLineAnalyzer::Empty:
        format = i18n( "Not Available (file empty)" );
        break;
    case DiffLineAnalyzer::Unknown:
        format = i18n( "Unknown" );
        break;
    case DiffLineAnalyzer::SideBySide:
        format = i18n( "Side by Side" );
    }
    return format;
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

