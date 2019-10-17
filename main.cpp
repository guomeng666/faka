#include <QApplication>
#include "mainwindow.h"
#include "qglobal.h"
#include "ccrashstack.h"
#include <QSharedMemory>
#include <QMessageBox>
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QDebug>
#include "logindialog.h"

long __stdcall   callback(_EXCEPTION_POINTERS*   excp)
{
    CCrashStack crashStack(excp);
    QString sCrashInfo = crashStack.GetExceptionInfo();
    QString sFileName = "crash.log";

    QFile file(sFileName);
    if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        file.write(sCrashInfo.toUtf8());
        file.close();
    }
    return   EXCEPTION_EXECUTE_HANDLER;
}

bool isDirExist(QString fullPath)
{
    QDir dir(fullPath);
    if(dir.exists())
    {
      return true;
    }
    else
    {
        bool ok = dir.mkdir(fullPath);
       return ok;
    }
}

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

    static QMutex mutex;

    mutex.lock();

    QString text;

    switch(type)
    {

    case QtDebugMsg:

        text = QString("Debug:");

        break;

    case QtWarningMsg:

        text = QString("Warning:");

        break;

    case QtCriticalMsg:

        text = QString("Critical:");

        break;

    case QtFatalMsg:

        text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);

    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");

    QString current_date = QString("(%1)").arg(current_date_time);

    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

    isDirExist(QDir::current().path()+"/loginfo");

    QFile file("./loginfo/"+QDateTime::currentDateTime().toString("yy-MM-dd")+".txt");

    file.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream text_stream(&file);

    text_stream << message << "\r\n";

    file.flush();

    file.close();

    mutex.unlock();
}

int main(int argc, char *argv[])
{
#ifdef QT_NO_DEBUG
    qInstallMessageHandler(outputMessage);
#else

#endif
    SetUnhandledExceptionFilter(callback);
    QApplication a(argc, argv);

    QSharedMemory shared_memory;
    shared_memory.setKey(QString("main_window"));
    if(shared_memory.attach())
    {
        QMessageBox::warning(NULL,"启动提示","软件已经启动","确定");
        return 0;
    }
    if(shared_memory.create(1))
    {
        MainWindow w;

        loginDialog* loginForm = new loginDialog;
        loginForm->setWebApiAddress(w.getWebServiceIp());
        if(loginForm->exec() == QDialog::Accepted) //登录界面
        {
            loginForm->hide();
            qDebug()<<"显示主界面";
            w.setLoginDialog(loginForm);
            w.showMaximized();
            a.exec();
        }
    }
    return 0;
}
