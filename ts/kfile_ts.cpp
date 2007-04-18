/* This file is part of the KDE project
 * Copyright (C) 2002 Carsten Niehaus <cniehaus@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "kfile_ts.h"

#include <kgenericfactory.h>
#include <kdebug.h>

#include <qfile.h>
#include <qstringlist.h>

typedef KGenericFactory<KTsPlugin> TsFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_ts, TsFactory("kfile_ts"))

KTsPlugin::KTsPlugin(QObject *parent, const char *name,
        const QStringList &args) : KFilePlugin(parent, name, args)
{
    makeMimeTypeInfo( "application/x-linguist" );
}

void KTsPlugin::makeMimeTypeInfo(const QString& mimeType)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo(mimeType);

    KFileMimeTypeInfo::GroupInfo* group =
            addGroupInfo(info, "General", i18n("General"));

    KFileMimeTypeInfo::ItemInfo* item;
    item = addItemInfo(group, "Messages", i18n("Messages"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "Translated", i18n("Translated"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "Untranslated", i18n("Untranslated"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "Obsolete", i18n("Obsolete"), QVariant::Int);
}

bool KTsPlugin::readInfo(KFileMetaInfo& info, uint)
{
    QFile f(info.path());
    if (!f.open(IO_ReadOnly))
        return false;

    int messages      = 0;
    int untranslated  = 0;
    int obsolete      = 0;

    QTextStream stream( &f );
    QString line = stream.readLine();

    // is it really a linguist file?
    if (!line.contains("<!DOCTYPE TS>", false))
        return false;

    while (!stream.eof())
    {
        line = stream.readLine();

        if (line.contains("type=\"obsolete\"")) obsolete++;

        if (line.contains("<source>")) messages++;
        
        if (line.contains("type=\"unfinished\"")) untranslated++;
        
    }

    KFileMetaInfoGroup group = appendGroup(info, "General");
    appendItem(group, "Messages", messages);
    appendItem(group, "Translated", messages-untranslated-obsolete);
    appendItem(group, "Untranslated", untranslated);
    appendItem(group, "Obsolete", obsolete);

    return true;
}

#include "kfile_ts.moc"
