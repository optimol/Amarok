/***************************************************************************
begin                : 2004/02/07
copyright            : (C) Mark Kretschmann
email                : markey@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AMAROKCONFIGDIALOG_H
#define AMAROKCONFIGDIALOG_H

#include <qmap.h>
#include <kconfigdialog.h>

class QComboBox;
class QVBox;

namespace amaroK {
    class PluginConfig;
}

class AmarokConfigDialog : public KConfigDialog
{
    Q_OBJECT

    public:
        AmarokConfigDialog( QWidget *parent, const char* name, KConfigSkeleton *config );
        void triggerChanged();

    protected slots:
        void updateSettings();
        void updateWidgets();
        void updateWidgetsDefault();

    private slots:        
        void aboutEngine();
            
    protected:
        bool hasChanged();
        bool isDefault();
    
    private:
        void soundSystemChanged();

        QComboBox* m_soundSystem;
        amaroK::PluginConfig* m_engineConfig;
        QVBox* m_enginePage;
        bool m_changedExternal;

        QMap<QString, QString> m_pluginName;
        QMap<QString, QString> m_pluginAmarokName;
};


#endif // AMAROKCONFIGDIALOG_H
