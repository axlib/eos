#ifndef __EOS_DESKTOP_H__
#define __EOS_DESKTOP_H__

#include "axLib/axLib.h"
#include "axLib/axButton.h"
#include "axLib/axToggle.h"

#include "eosAppLoader.h"
#include "eosStatusBar.h"
#include "eosFrame.h"
#include "eosDock.h"

#include "eosNotification.h"

#include "eosAlert.h"

#include "eosCoreSystem.h"

namespace eos
{
    class Desktop : public axPanel
    {
    public:
        
        Desktop(eos::Core::System* system,
                const ax::Rect& rect);
        
        inline eos::Core::Manager* GetWindowManager()
        {
            return _system->GetManager();
        }
        
        inline void AddFrame(eos::Frame* frame)
        {
            _system->GetManager()->AddFrame(frame);
        }
        
        inline void BringToFront(eos::Frame* frame)
        {
            _system->GetManager()->BringToFront(frame);
        }
        
        inline std::vector<eos::Frame*> GetFrameVector()
        {
            return _system->GetManager()->GetFrameVector();
        }
        
        void ShowView();
        
//        void ShowNotification();
//        
//        void ShowTerminal();
//        
//        void ShowTrace();
        
        enum DesktopApps
        {
            DSKT_APP_TERMINAL,
            DSKT_APP_TRACE,
            DSKT_APP_NOTIFY,
            DSKT_APP_COUNT
        };
        
        void ToggleDesktopApp(const DesktopApps& app);
        
    private:
        bool _desktop_apps_status[DSKT_APP_COUNT];
        axWindow* _desktop_apps[DSKT_APP_COUNT];
        
        bool _showView;
//        bool _terminalMode, _traceMode, _notifiMode;
        
        eos::Core::System* _system;
        
        axWindow* _terminal;
        AppLoader* _terminal_app;
        
//        eos::WindowManager* _eosManager;
        ax::Image* _bg_img;
        eos::Notification* _notification;
        
        eos::Alert* _trace_viewer;
        
        void ReposAllDesktopBuiltInApps();
        
        // Events.
        virtual void OnMouseMotion(const ax::Point& mouse);
        virtual void OnMouseLeftDown(const ax::Point& mouse);
        virtual void OnMouseLeftUp(const ax::Point& mouse);
        
        void OnPaint();
        void PaintView(ax::GC& gc);
        
    };
}

#endif //__EOS_DESKTOP_H__
