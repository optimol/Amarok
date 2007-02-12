// Maintainer: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright:  See COPYING file that comes with this distribution
//
// Description: a popupmenu to control various features of Amarok
//              also provides Amarok's helpMenu

#ifndef AMAROK_ACTIONCLASSES_H
#define AMAROK_ACTIONCLASSES_H

#include "engineobserver.h"
#include "prettypopupmenu.h"
#include "sliderwidget.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kselectaction.h>
#include <QPointer>
//Added by qt3to4:
#include <QResizeEvent>
#include <QMouseEvent>
#include <QContextMenuEvent>

class KActionCollection;
class KHelpMenu;


namespace Amarok
{
    class Menu : public PrettyPopupMenu
    {
        Q_OBJECT
        public:
            static Menu *instance();
            static KMenu *helpMenu( QWidget *parent = 0 );

            enum MenuIds {
                ID_CONF_DECODER,
                ID_SHOW_VIS_SELECTOR,
                ID_SHOW_COVER_MANAGER,
                ID_CONFIGURE_EQUALIZER,
                ID_RESCAN_COLLECTION
            };

        public slots:
            void slotActivated( int index );

        private slots:
            void slotAboutToShow();

        private:
            Menu();

            static KHelpMenu  *s_helpMenu;
    };


    class MenuAction : public KAction
    {
        public:
            MenuAction( KActionCollection* );
            virtual int plug( QWidget*, int index = -1 );
    };


    class PlayPauseAction : public KToggleAction, public EngineObserver
    {
        public:
            PlayPauseAction( KActionCollection* );
            virtual void engineStateChanged( Engine::State, Engine::State = Engine::Empty );
    };
    class AnalyzerContainer : public QWidget
    {
        public:
            AnalyzerContainer( QWidget *parent );
        protected:
            virtual void resizeEvent( QResizeEvent* );
            virtual void mousePressEvent( QMouseEvent* );
            virtual void contextMenuEvent( QContextMenuEvent* );
        private:
            void changeAnalyzer();
            QWidget *m_child;
    };

    class AnalyzerAction : public KAction
    {
        public:
            AnalyzerAction( KActionCollection* );
            virtual QWidget* createWidget( QWidget *);
    };

    class VolumeAction : public KAction, public EngineObserver
    {
        public:
            VolumeAction( KActionCollection* );
            virtual QWidget* createWidget( QWidget * );
        private:
            void engineVolumeChanged( int value );
            QPointer<Amarok::VolumeSlider> m_slider;
    };


    class ToggleAction : public KToggleAction
    {
        public:
            ToggleAction( const QString &text, void ( *f ) ( bool ), KActionCollection* const ac, const char *name );

            virtual void setChecked( bool b );

            virtual void setEnabled( bool b );

        private:
            void ( *m_function ) ( bool );
    };

    class SelectAction : public KSelectAction
    {
        public:
            SelectAction( const QString &text, void ( *f ) ( int ), KActionCollection* const ac, const char *name );

            virtual void setCurrentItem( int n );

            virtual void setEnabled( bool b );

            virtual void setIcons( QStringList icons );

            virtual QString currentText() const;

            QStringList icons() const;

            QString currentIcon() const;

        private:
            void ( *m_function ) ( int );
            QStringList m_icons;
    };


    class RandomAction : public SelectAction
    {
        public:
            RandomAction( KActionCollection *ac );
            virtual void setCurrentItem( int n );
    };

    class FavorAction : public SelectAction
    {
        public:
            FavorAction( KActionCollection *ac );
    };

    class RepeatAction : public SelectAction
    {
        public:
            RepeatAction( KActionCollection *ac );
    };

    class BurnMenu : public KMenu
    {
            Q_OBJECT

        public:
            enum MenuIds {
                CURRENT_PLAYLIST,
                SELECTED_TRACKS
            };

            static KMenu *instance();

        private slots:
            void slotAboutToShow();
            void slotActivated( int index );

        private:
            BurnMenu();
    };


    class BurnMenuAction : public KAction
    {
        public:
            BurnMenuAction( KActionCollection* );
            virtual int plug( QWidget*, int index = -1 );
    };

    class StopMenu : public KMenu
    {
            Q_OBJECT

        public:
            enum MenuIds {
                NOW,
                AFTER_TRACK,
                AFTER_QUEUE
            };

            static KMenu *instance();

        private slots:
            void slotAboutToShow();
            void slotActivated( int index );

        private:
            StopMenu();
    };


    class StopAction : public KAction
    {
        public:
            StopAction( KActionCollection* );
            virtual int plug( QWidget*, int index = -1 );
    };

} /* namespace Amarok */


#endif /* AMAROK_ACTIONCLASSES_H */

