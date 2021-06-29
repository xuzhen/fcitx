/*
 * Copyright (C) 2011~2020 by CSSlayer
 * wengxt@gmail.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above Copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above Copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the authors nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */
#ifndef __FCITX_INPUT_CONTEXT_H_
#define __FCITX_INPUT_CONTEXT_H_

#include "config.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDir>
#include <QInputContext>
#include <QList>
#include <QWeakPointer>

#include "fcitx-config/hotkey.h"
#include "fcitx/ime.h"
#include "fcitxinputcontextproxy.h"
#include "fcitxwatcher.h"
#include <xkbcommon/xkbcommon-compose.h>

#include "fcitx/frontend.h"
#include <X11/Xlib.h>

struct FcitxQtICData {
    FcitxQtICData(FcitxWatcher *watcher)
        : capacity(0), proxy(new FcitxInputContextProxy(watcher, watcher)),
          surroundingAnchor(-1), surroundingCursor(-1) {}
    ~FcitxQtICData() { delete proxy; }
    QFlags<FcitxCapacityFlags> capacity;
    FcitxInputContextProxy *proxy;
    QRect rect;
    QString surroundingText;
    int surroundingAnchor;
    int surroundingCursor;
};

class FcitxQtConnection;

class ProcessKeyWatcher : public QDBusPendingCallWatcher {
    Q_OBJECT
public:
    ProcessKeyWatcher(XEvent *e, KeySym s, const QDBusPendingCall &call,
                      QObject *parent = 0)
        : QDBusPendingCallWatcher(call, parent) {
        event = static_cast<XEvent *>(malloc(sizeof(XEvent)));
        *event = *e;
        sym = s;
    }

    virtual ~ProcessKeyWatcher() { free(event); }

public slots:
    void processEvent() {
        qApp->x11ProcessEvent(event);
        deleteLater();
    }

public:
    XEvent *event;
    KeySym sym;
};

#define FCITX_IDENTIFIER_NAME "fcitx"

struct XkbContextDeleter {
    static inline void cleanup(struct xkb_context *pointer) {
        if (pointer)
            xkb_context_unref(pointer);
    }
};

struct XkbComposeTableDeleter {
    static inline void cleanup(struct xkb_compose_table *pointer) {
        if (pointer)
            xkb_compose_table_unref(pointer);
    }
};

struct XkbComposeStateDeleter {
    static inline void cleanup(struct xkb_compose_state *pointer) {
        if (pointer)
            xkb_compose_state_unref(pointer);
    }
};

class QFcitxInputContext : public QInputContext {
    Q_OBJECT
public:
    QFcitxInputContext();
    ~QFcitxInputContext();

    virtual QString identifierName();
    virtual QString language();
    virtual void reset();
    virtual bool isComposing() const;
    virtual void update();
    virtual void setFocusWidget(QWidget *w);

    virtual void widgetDestroyed(QWidget *w);

    virtual bool x11FilterEvent(QWidget *keywidget, XEvent *event);
    virtual void mouseHandler(int x, QMouseEvent *event);

private Q_SLOTS:
    void createInputContextFinished();
    void cleanUp();
    void commitString(const QString &str);
    void updateFormattedPreedit(const FcitxFormattedPreeditList &preeditList,
                                int cursorPos);
    void forwardKey(uint keyval, uint state, bool isRelease);
    void deleteSurroundingText(int offset, uint nchar);
    void updateCursor();
    void x11ProcessKeyEventCallback(QDBusPendingCallWatcher *watcher);

private:
    QWidget *validFocusWidget();
    bool processCompose(uint keyval, uint state, FcitxKeyEventType event);
    bool x11FilterEventFallback(XEvent *event, KeySym sym);
    XEvent *createXEvent(Display *dpy, WId wid, uint keyval, uint state,
                         bool isRelease);
    bool isValid();
    FcitxInputContextProxy *validIC();
    FcitxInputContextProxy *validICByWidget(QWidget *w);

    void addCapacity(FcitxQtICData *data, QFlags<FcitxCapacityFlags> capacity,
                     bool forceUpdate = false) {
        QFlags<FcitxCapacityFlags> newcaps = data->capacity | capacity;
        if (data->capacity != newcaps || forceUpdate) {
            data->capacity = newcaps;
            updateCapacity(data);
        }
    }

    void removeCapacity(FcitxQtICData *data,
                        QFlags<FcitxCapacityFlags> capacity,
                        bool forceUpdate = false) {
        QFlags<FcitxCapacityFlags> newcaps = data->capacity & (~capacity);
        if (data->capacity != newcaps || forceUpdate) {
            data->capacity = newcaps;
            updateCapacity(data);
        }
    }

    void updateCapacity(FcitxQtICData *data);
    void commitPreedit();
    void createICData(QWidget *w);

    QString m_preedit;
    QString m_commitPreedit;
    FcitxFormattedPreeditList m_preeditList;
    int m_cursorPos;
    bool m_useSurroundingText;
    bool m_syncMode;
    FcitxWatcher *m_watcher;
    QHash<WId, FcitxQtICData *> m_icMap;
    QScopedPointer<struct xkb_context, XkbContextDeleter> m_xkbContext;
    QScopedPointer<struct xkb_compose_table, XkbComposeTableDeleter>
        m_xkbComposeTable;
    QScopedPointer<struct xkb_compose_state, XkbComposeStateDeleter>
        m_xkbComposeState;
};

#endif //__FCITX_INPUT_CONTEXT_H_

// kate: indent-mode cstyle; space-indent on; indent-width 0;
