/**************************************************************************
**                             kfile_diff.h
**                              -------------------
**      begin                   : Sun Jan 20 23:25:29 2002
**      copyright               : (C) 2002 by Otto Bruggeman
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

#ifndef __KFILE_PDF_H__
#define __KFILE_PDF_H__

#include <kfilemetainfo.h>

class QStringList;

class KDiffPlugin: public KFilePlugin
{
  Q_OBJECT

public:
	KDiffPlugin( QObject *parent, const char *name,
	             const QStringList& preferredItems );


	virtual bool readInfo( KFileMetaInfo& info, uint what );

public:
	enum Format      { Context, Ed, Normal, RCS, Unified, Empty, SideBySide, Unknown };
	enum DiffProgram { CVSDiff, Diff, Diff3, Perforce, SubVersion, Undeterminable }; // cant use Unknown again :(

private:
	enum Format      determineDiffFormat   ( const QStringList lines ) const;
	enum DiffProgram determineDiffProgram  ( const QStringList lines ) const;
	const QString    determineI18nedFormat ( enum KDiffPlugin::Format diffFormat ) const;
	const QString    determineI18nedProgram( enum KDiffPlugin::DiffProgram diffProgram ) const;
	// yes ugly, it's better to use a struct or classmembers to pass these parameters around
	void             determineDiffInfo     ( const QStringList lines,
	                                         enum KDiffPlugin::Format diffFormat, int* numberOfFiles,
	                                         int* numberOfHunks, int* numberOfAdditions,
                                             int* numberOfChanges, int* numberOfDeletions );
};

#endif
