#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <QObject>
#include "ledcontrol.h"

#define GREEN 1
#define RED 0

class LedControl : public QObject
{
    typedef uint HANDLER;

    Q_OBJECT
public:
    explicit LedControl(QObject *parent = nullptr);
    bool connectLedControl(char *ip, int port);
    void disconnectLedControl();
    bool ping();
    bool setText(QString str,char color, uint fontsize);
signals:

public slots:

private:
};

#endif // LEDCONTROL_H
