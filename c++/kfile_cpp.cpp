/* This file is part of the KDE project
 * Copyright (C) 2002 Rolf Magnus <ramagnus@kde.org>
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

#include "kfile_cpp.h"

#include <kurl.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <qfile.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QTextStream>

typedef KGenericFactory<KCppPlugin> CppFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_cpp, CppFactory("kfile_cpp"))

KCppPlugin::KCppPlugin(QObject *parent, 
                       const QStringList &args)
    : KFilePlugin(parent, args)
{
    kDebug(7034) << "c++ plugin\n";
    makeMimeTypeInfo("text/x-c++src");
    makeMimeTypeInfo("text/x-chdr");
}

void KCppPlugin::makeMimeTypeInfo(const QString& mimetype)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( mimetype );

    KFileMimeTypeInfo::GroupInfo* group =
                  addGroupInfo(info, "General", i18n("General"));

    KFileMimeTypeInfo::ItemInfo* item;
    item = addItemInfo(group, "Lines", i18n("Lines"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "Code", i18n("Code"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "Comment", i18n("Comment"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "Blank", i18n("Blank"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "Strings", i18n("Strings"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "i18n Strings", i18n("i18n Strings"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    item = addItemInfo(group, "Included Files", i18n("Included Files"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
}

bool KCppPlugin::readInfo( KFileMetaInfo& info, uint )
{
    QFile f(info.path());
    if (!f.open(QIODevice::ReadOnly))
        return false;

    int codeLines     = 0;
    int commentLines  = 0;
    int totalLines    = 0;
    int emptyLines    = 0;
    int Strings       = 0;
    int Stringsi18n   = 0;
    int Includes      = 0;

    bool inComment = false;

    QString line;

    QTextStream stream( &f );
    while (!stream.atEnd())
    {
        line = stream.readLine();
        totalLines++;

        if (line.trimmed().isEmpty())
        {
            emptyLines++;
            continue;
        }

        if (line.contains("/*")) inComment = true;

        if (!inComment)
        {
            codeLines++;
            if (line.contains(QRegExp("^\\s*#\\s*include"))) Includes++;

            int pos = line.indexOf("//");
            if (pos>=0) commentLines++;
            // truncate the comment - we don't want to count strings in it
            line.truncate(pos);

            Strings+=line.count(QRegExp("\".*\""));
			Stringsi18n+=line.count(QRegExp("(?:i18n|I18N_NOOP)\\s*\\("));
        }
        else
            commentLines++;

        if (line.contains("*/")) inComment = false;
    }

    KFileMetaInfoGroup group = appendGroup(info, "General");

    appendItem(group, "Lines",          int(totalLines));
    appendItem(group, "Code",           int(codeLines));
    appendItem(group, "Comment",        int(commentLines));
    appendItem(group, "Blank",          int(emptyLines));
    appendItem(group, "Strings",        int(Strings));
    appendItem(group, "i18n Strings",   int(Stringsi18n));
    appendItem(group, "Included Files", int(Includes));
    return true;
}

#include "kfile_cpp.moc"
