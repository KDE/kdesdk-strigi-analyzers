/**************************************************************************
**                             kfile_diff.cpp
**                              -------------------
**      begin                   : Sun Jan 20 23:25:44 2002
**      copyright               : (C) 2002-2003 by Otto Bruggeman
**      email                   : otto.bruggeman@home.nl
**
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

/*
** Patch by Volker Augustin for empty diff files. Februari 8, 2002
**
** Patched to work with CVS from after February 26, 2002 Otto
**
** Patched to work with CVS from after March 24, 2002 Otto
**
** Added support for Perforce diffs, April 26, 2003 Otto Bruggeman
**
** Added support for Subversion diffs, September 11, 2003 Otto Bruggeman
*/

#include <qcstring.h>
#include <qdatetime.h>
#include <qdict.h>
#include <qfile.h>
#include <qregexp.h>
#include <qvalidator.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kprocess.h>
#include <kurl.h>

#include "kfile_diff.h"

K_EXPORT_COMPONENT_FACTORY(kfile_diff, KGenericFactory<KDiffPlugin>("kfile_diff"))

KDiffPlugin::KDiffPlugin(QObject *parent, const char *name,
                         const QStringList &preferredItems)
	: KFilePlugin(parent, name, preferredItems)
{
	kdDebug(7034) << "diff plugin" << endl;

	KFileMimeTypeInfo* info = addMimeTypeInfo( "text/x-diff" );

	KFileMimeTypeInfo::GroupInfo* group;
	group = addGroupInfo( info, "General", i18n( "General" ) );
	addItemInfo( group, "Files", i18n( "Files" ), QVariant::UInt );
	addItemInfo( group, "First", i18n( "First File" ), QVariant::String );
	addItemInfo( group, "Format", i18n( "Format" ), QVariant::String );
	addItemInfo( group, "DiffProgram", i18n( "Diff Program" ), QVariant::String );
	addItemInfo( group, "Hunks", i18n( "Hunks" ), QVariant::UInt );
	group = addGroupInfo( info, "Statistics", i18n( "Statistics" ) );
	addItemInfo( group, "Insert", i18n( "Insertions" ), QVariant::UInt );
	addItemInfo( group, "Modify", i18n( "Changes" ),    QVariant::UInt );
	addItemInfo( group, "Delete", i18n( "Deletions" ),  QVariant::UInt );
}

bool KDiffPlugin::readInfo( KFileMetaInfo& info, uint what )
{
	// This is a hack to avoid using the what stuff, since it is not yet implemented
	what = 0;

	// Used to determine if false or true should be returned
	bool dataSet = false;

	KFileMetaInfoGroup group;

	QFile file( info.path() );
	QStringList lines;

	if( file.open( IO_ReadOnly ) )
	{
		QTextStream stream( &file );
		while (!stream.eof())
		{
			lines.append( stream.readLine() );
		}
		file.close();
	}

	QString format;
	QString program;

	enum KDiffPlugin::Format       diffFormat;
	enum KDiffPlugin::DiffProgram  diffProgram;

	diffFormat  = determineDiffFormat ( lines );

	format = determineI18nedFormat( diffFormat );

	diffProgram = determineDiffProgram( lines );

	program = determineI18nedProgram( diffProgram );

	int numberOfAdditions = 0;
	int numberOfDeletions = 0;
	int numberOfChanges = 0;
	int numberOfHunks = 0;
	int numberOfFiles = 0;


	if ( what != KFileMetaInfo::Fastest )
	{
		determineDiffInfo( lines, diffFormat, &numberOfFiles, &numberOfHunks, &numberOfAdditions, &numberOfChanges, &numberOfDeletions );
	}

	QString filename;
	QRegExp firstFile( "^Index: (.*)" );
	QStringList::ConstIterator it = lines.begin();

	it = lines.begin();
	while ( it != lines.end() )
	{
		if ( firstFile.exactMatch( (*it) ) )
		{
			filename = firstFile.cap(1);
			// only interested in the first filename
			break;
		}
		++it;
	}

	kdDebug(7034) << "Diff Format         : " << format << endl; // i18n-ed but that is not a problem unless i get i18n-ed debug output ah well, we'll figure something out when then happens

	if (what != KFileMetaInfo::Fastest )
	{
		// These dont get calculated in fastest mode...
		kdDebug(7034) << "Number of additions : " << numberOfAdditions << endl;
		kdDebug(7034) << "Number of deletions : " << numberOfDeletions << endl;
		kdDebug(7034) << "Number of changes   : " << numberOfChanges << endl;
		kdDebug(7034) << "Number of hunks     : " << numberOfHunks << endl;
	}

	group = appendGroup( info, "General" );

	if ( numberOfFiles != 0 && what != KFileMetaInfo::Fastest )
	{
		appendItem( group, "Files", numberOfFiles );
		dataSet = true;
	}

	if ( !filename.isEmpty() )
	{
		appendItem( group, "First", filename );
		dataSet = true;
	}

	if ( !format.isEmpty() )
	{
		appendItem( group, "Format", format );
		dataSet = true;
	}

	if ( !program.isEmpty() )
	{
		appendItem( group, "DiffProgram", program );
		dataSet = true;
	}

	if ( numberOfHunks != 0 && what != KFileMetaInfo::Fastest )
	{
		appendItem( group, "Hunks", numberOfHunks );
		dataSet = true;
	}

	group = appendGroup( info, "Statistics" );

	if ( numberOfAdditions != 0 && what != KFileMetaInfo::Fastest )
	{
		appendItem( group, "Insert", numberOfAdditions );
		dataSet = true;
	}

	if ( numberOfChanges != 0 && what != KFileMetaInfo::Fastest )
	{
		appendItem( group, "Modify", numberOfChanges );
		dataSet = true;
	}

	if ( numberOfDeletions != 0 && what != KFileMetaInfo::Fastest )
	{
		appendItem( group, "Delete", numberOfDeletions );
		dataSet = true;
	}

	return dataSet;
}

enum KDiffPlugin::Format KDiffPlugin::determineDiffFormat( const QStringList lines ) const
{
	QString line;

	if ( lines.count() == 0 )
	{
		return KDiffPlugin::Empty;
	}

	QStringList::ConstIterator it = lines.begin();

	while ( it != lines.end() )
	{
		line = (*it);
		if ( line.find( QRegExp( "^[0-9]+[0-9,]*[acd][0-9]+[0-9,]*$" ), 0 ) == 0 )
		{
			return KDiffPlugin::Normal;
		}
		else if ( line.find( QRegExp( "^--- " ), 0 ) == 0 )
		{
			// unified has first a '^--- ' line, then a '^+++ ' line
			return KDiffPlugin::Unified;
		}
		else if ( line.find( QRegExp( "^\\*\\*\\* [^\\t]+\\t" ), 0 ) == 0 )
		{
			// context has first a '^*** ' line, then a '^--- ' line
			return KDiffPlugin::Context;
		}
		else if ( line.find( QRegExp( "^[acd][0-9]+ [0-9]+" ), 0 ) == 0 )
		{
			return KDiffPlugin::RCS;
		}
		else if ( line.find( QRegExp( "^[0-9]+[0-9,]*[acd]" ), 0 ) == 0 )
		{
			return KDiffPlugin::Ed;
		}
		++it;
	}
	return KDiffPlugin::Unknown;
}

enum KDiffPlugin::DiffProgram KDiffPlugin::determineDiffProgram( const QStringList lines ) const
{
	if ( lines.count() == 0 )
	{
		return KDiffPlugin::Undeterminable;
	}

	QStringList::ConstIterator it = lines.begin();
	// very crude, might need some more refining
	QRegExp diffRE( "^diff .*" );
	QRegExp p4sRE("^==== ");

	bool indexFound = false;

	while ( it != lines.end() )
	{
		if ( (*it).startsWith( "Index:" ) )
			indexFound = true;
		else if ( (*it).startsWith( "retrieving revision") )
			return KDiffPlugin::CVSDiff;
		else if ( diffRE.exactMatch( *it ) )
			return KDiffPlugin::Diff;
		else if ( p4sRE.exactMatch( *it ) )
			return KDiffPlugin::Perforce;

		++it;
	}

	if ( indexFound ) // but no "retrieving revision" found like only cvs diff adds.
		return KDiffPlugin::SubVersion;

	return KDiffPlugin::Undeterminable;
}

const QString KDiffPlugin::determineI18nedFormat( enum KDiffPlugin::Format diffFormat ) const
{
	QString format;
	switch( diffFormat )
	{
	case KDiffPlugin::Context:
		format = i18n( "Context" );
		break;
	case KDiffPlugin::Ed:
		format = i18n( "Ed" );
		break;
	case KDiffPlugin::Normal:
		format = i18n( "Normal" );
		break;
	case KDiffPlugin::RCS:
		format = i18n( "RCS" );
		break;
	case KDiffPlugin::Unified:
		format = i18n( "Unified" );
		break;
	case KDiffPlugin::Empty:
		format = i18n( "Not Available (file empty)" );
		break;
	case KDiffPlugin::Unknown:
		format = i18n( "Unknown" );
		break;
	case KDiffPlugin::SideBySide:
		format = i18n( "Side by Side" );
	}
	return format;
}

const QString KDiffPlugin::determineI18nedProgram( enum KDiffPlugin::DiffProgram diffProgram ) const
{
	QString program;

	switch( diffProgram )
	{
	case KDiffPlugin::CVSDiff:
		program = i18n( "CVSDiff" );
		break;
	case KDiffPlugin::Diff:
		program = i18n( "Diff" );
		break;
	case KDiffPlugin::Diff3:
		program = i18n( "Diff3" );
		break;
	case KDiffPlugin::Perforce:
		program = i18n( "Perforce" );
		break;
	case KDiffPlugin::SubVersion:
		program = i18n( "SubVersion" );
		break;
	case KDiffPlugin::Undeterminable:
		program = i18n( "Unknown" );
		break;
	}
	return program;
}

void KDiffPlugin::determineDiffInfo( const QStringList lines,
                                     enum KDiffPlugin::Format diffFormat,
                                     int* numberOfFiles,
                                     int* numberOfHunks,
                                     int* numberOfAdditions,
                                     int* numberOfChanges,
                                     int* numberOfDeletions )
{
	QString line;

	QRegExp edAdd( "([0-9]+)(|,([0-9]+))a" );
	QRegExp edDel( "([0-9]+)(|,([0-9]+))d" );
	QRegExp edMod( "([0-9]+)(|,([0-9]+))c" );

	QRegExp normalAdd( "[0-9]+a([0-9]+)(|,([0-9]+))" );
	QRegExp normalDel( "([0-9]+)(|,([0-9]+))d(|[0-9]+)" );
	QRegExp normalMod( "([0-9]+)(|,([0-9]+))c([0-9]+)(|,([0-9]+))" );

	QRegExp rcsAdd( "a[0-9]+ ([0-9]+)" );
	QRegExp rcsDel( "d[0-9]+ ([0-9]+)" );

	QStringList::ConstIterator it = lines.begin();

	switch( diffFormat )
	{
	case KDiffPlugin::Context:
		while ( it != lines.end() )
		{
			if ( (*it).startsWith("***************") )
			{
				(*numberOfHunks)++;
//				kdDebug(7034) << "Context Hunk      : " << (*it) << endl;
			}
			else if ( (*it).startsWith("***") )
			{
				(*numberOfFiles)++;
//				kdDebug(7034) << "Context File      : " << (*it) << endl;
			}
			else if ( (*it).startsWith("---") ) {} // ignore
			else if ( (*it).startsWith("+") )
			{
				(*numberOfAdditions)++;
//				kdDebug(7034) << "Context Insertion : " << (*it) << endl;
			}
			else if ( (*it).startsWith("-") )
			{
				(*numberOfDeletions)++;
//				kdDebug(7034) << "Context Deletion  : " << (*it) << endl;
			}
			else if ( (*it).startsWith("!") )
			{
				(*numberOfChanges)++;
//				kdDebug(7034) << "Context Modified  : " << (*it) << endl;
			}
			else if ( (*it).startsWith(" ") )
			{
//				kdDebug(7034) << "Context Context   : " << (*it) << endl;
			}
			else
			{
//				kdDebug(7034) << "Context Unknown   : " << (*it) << endl;
			}

			++it;
		}
		(*numberOfChanges) /= 2; // changes are in both parts of the hunks
		(*numberOfFiles) -= (*numberOfHunks); // it counts old parts of a hunk as files :(
		break;
	case KDiffPlugin::Ed:
		while ( it != lines.end() )
		{
			if ( (*it).startsWith( "diff" ) )
			{
				(*numberOfFiles)++;
//				kdDebug(7034) << "Ed File         : " << (*it) << endl;
			}
			else if ( edAdd.exactMatch( (*it) ) )
			{
//				kdDebug(7034) << "Ed Insertion    : " << (*it) << endl;
				(*numberOfHunks)++;
				++it;
				while( it != lines.end() && !(*it).startsWith(".") )
				{
					(*numberOfAdditions)++;
//					kdDebug(7034) << "Ed Insertion    : " << (*it) << endl;
					++it;
				}
			}
			else if ( edDel.exactMatch( (*it) ) )
			{
//				kdDebug(7034) << "Ed Deletion     : " << (*it) << endl;
				(*numberOfHunks)++;
				(*numberOfDeletions) += (edDel.cap(3).isEmpty() ? 1 : edDel.cap(3).toInt() - edDel.cap(1).toInt() + 1);
//				kdDebug(7034) << "Ed noOfLines    : " << (edDel.cap(3).isEmpty() ? 1 : edDel.cap(3).toInt() - edDel.cap(1).toInt() + 1) << endl;
			}
			else if ( edMod.exactMatch( (*it) ) )
			{
//				kdDebug(7034) << "Ed Modification : " << (*it) << endl;
				if ( edMod.cap(3).isEmpty() )
					(*numberOfDeletions)++;
				else
					(*numberOfDeletions) += edMod.cap(3).toInt() - edMod.cap(1).toInt() + 1;
				(*numberOfHunks)++;
				++it;
				while( it != lines.end() && !(*it).startsWith(".") )
				{
					(*numberOfAdditions)++;
//					kdDebug(7034) << "Ed Modification : " << (*it) << endl;
					++it;
				}
			}
			else
			{
//				kdDebug(7034) << "Ed Unknown      : " << (*it) << endl;
			}

			++it;
		}
		break;
	case KDiffPlugin::Normal:
		while ( it != lines.end() )
		{
			if ( (*it).startsWith( "diff" ) )
			{
				(*numberOfFiles)++;
//				kdDebug(7034) << "Normal File         : " << (*it) << endl;
			}
			else if ( normalAdd.exactMatch( *it ) )
			{
//				kdDebug(7034) << "Normal Insertion    : " << (*it) << endl;
				(*numberOfHunks)++;
				if ( normalAdd.cap(3).isEmpty() )
				{
					(*numberOfAdditions)++;
//					kdDebug(7034) << "Normal Addition : " << 1 << endl;
				}
				else
				{
					(*numberOfAdditions) += normalAdd.cap(3).toInt() - normalAdd.cap(1).toInt() + 1;
//					kdDebug(7034) << "Normal Addition : " << normalAdd.cap(3).toInt() - normalAdd.cap(1).toInt() + 1 << endl;
				}
			}
			else if ( normalDel.exactMatch( *it ) )
			{
//				kdDebug(7034) << "Normal Deletion     : " << (*it) << endl;
				(*numberOfHunks)++;
				if ( normalDel.cap(3).isEmpty() )
				{
					(*numberOfDeletions)++;
//					kdDebug(7034) << "Normal Deletion : " << 1 << endl;
				}
				else
				{
					(*numberOfDeletions) += normalDel.cap(3).toInt() - normalDel.cap(1).toInt() + 1;
//					kdDebug(7034) << "Normal Deletion : " << normalDel.cap(3).toInt() - normalDel.cap(1).toInt() + 1 << endl;
				}
			}
			else if ( normalMod.exactMatch( *it ) )
			{
//				kdDebug(7034) << "Normal Modification : " << (*it) << endl;
				(*numberOfHunks)++;
				if ( normalMod.cap(3).isEmpty() )
				{
					(*numberOfDeletions)++;
//					kdDebug(7034) << "Normal Deletion : " << 1 << endl;
				}
				else
				{
					(*numberOfDeletions) += normalMod.cap(3).toInt() - normalMod.cap(1).toInt() + 1;
//					kdDebug(7034) << "Normal Deletion : " << normalMod.cap(3).toInt() - normalMod.cap(1).toInt() + 1 << endl;
				}
				if ( normalMod.cap(6).isEmpty() )
				{
					(*numberOfAdditions)++;
//					kdDebug(7034) << "Normal Addition : " << 1 << endl;
				}
				else
				{
					(*numberOfAdditions) += normalMod.cap(6).toInt() - normalMod.cap(4).toInt() + 1;
//					kdDebug(7034) << "Normal Addition : " << normalMod.cap(6).toInt() - normalMod.cap(4).toInt() + 1 << endl;
				}
			}
			else if ( (*it).startsWith(">") )
			{
//				numberOfAdditions++;
//				kdDebug(7034) << "Normal Insertion    : " << (*it) << endl;
			}
			else if ( (*it).startsWith("<") )
			{
//				numberOfDeletions++;
//				kdDebug(7034) << "Normal Deletion     : " << (*it) << endl;
			}
			else
			{
//				kdDebug(7034) << "Normal Unknown      : " << (*it) << endl;
			}

			++it;
		}
		break;
	case KDiffPlugin::RCS:
		while ( it != lines.end() )
		{
			if ( (*it).startsWith( "diff" ) ) // works for cvs diff, have to test for normal diff
			{
//				kdDebug(7034) << "RCS File      : " << (*it) << endl;
				(*numberOfFiles)++;
			}
			else if ( rcsAdd.exactMatch( *it ) )
			{
//				kdDebug(7034) << "RCS Insertion : " << (*it) << endl;
				(*numberOfHunks)++;
				(*numberOfAdditions) += rcsAdd.cap(1).toInt();
//				kdDebug(7034) << "RCS noOfLines : " << rcsAdd.cap(1).toInt() << endl;
			}
			else if ( rcsDel.exactMatch( *it ) )
			{
//				kdDebug(7034) << "RCS Deletion  : " << (*it) << endl;
				(*numberOfHunks)++;
				(*numberOfDeletions) += rcsDel.cap(1).toInt();
//				kdDebug(7034) << "RCS noOfLines : " << rcsDel.cap(1).toInt() << endl;
			}
			else
			{
//				kdDebug(7034) << "RCS Unknown   : " << (*it) << endl;
			}

			++it;
		}
		break;
	case KDiffPlugin::Unified:
		while ( it != lines.end() )
		{
			if ( (*it).startsWith("@@ ") )
			{
				(*numberOfHunks)++;
//				kdDebug(7034) << "Unified Hunk      : " << (*it) << endl;
			}
			else if ( (*it).startsWith("---") )
			{
				(*numberOfFiles)++;
//				kdDebug(7034) << "Unified File      : " << (*it) << endl;
			}
			else if ( (*it).startsWith("+++") ) {} // ignore (dont count as insertion)
			else if ( (*it).startsWith("+") )
			{
				(*numberOfAdditions)++;
//				kdDebug(7034) << "Unified Insertion : " << (*it) << endl;
			}
			else if ( (*it).startsWith("-") )
			{
				(*numberOfDeletions)++;
//				kdDebug(7034) << "Unified Deletion  : " << (*it) << endl;
			}
			else if ( (*it).startsWith(" ") )
			{
//				kdDebug(7034) << "Unified Context   : " << (*it) << endl;
			}
			else
			{
//				kdDebug(7034) << "Unified Unknown   : " << (*it) << endl;
			}

			++it;
		}
		break;
	case KDiffPlugin::Empty:
	case KDiffPlugin::Unknown:
	case KDiffPlugin::SideBySide:
		break;
	}
}

#include "kfile_diff.moc"

/* vim: set ts=4 sw=4 noet: */

