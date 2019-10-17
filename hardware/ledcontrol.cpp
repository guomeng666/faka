#include "ledcontrol.h"
#include <Header.h>
#include "QDebug"

LedControl::LedControl(QObject *parent) : QObject(parent)
{

}

HANDLER screen;
uint profile;

bool LedControl::connectLedControl(char *ip, int port)
{
    int error = 0;
    uint response = 0;
    //创建屏幕客户端
    Check_Error(Create_BxScreenClient(&screen));
    //创建屏幕连接
    Check_Error(BxScreenClient_connect(screen, ip, port, true));
    //取得当前屏幕规格。屏幕规格在连线成功后自动从控制器上获取，若控制器未加载屏参时回复NULL
    Check_Error(BxScreen_getProfile(&profile,screen));
    Check_Error(BxScreen_ping(&response, screen));
Clean:
    return error;
}

bool LedControl::ping()
{
    int error = 0;
    //发送ping命令
    uint response = 0;
    Check_Error(BxScreen_ping(&response, screen));
Clean:
    Check_Release(response);
    return false;
}

void LedControl::disconnectLedControl()
{
    BxController_disconnect(screen);
    BxScreen_disconnect(screen);
    qDebug()<<"LED关闭";
}


bool LedControl::setText(QString str,char color, uint fontsize)
{
    QByteArray gbk = str.toLocal8Bit();
    int error = 0;
    HANDLER font = 0;
    HANDLER factory = 0;
    HANDLER style = 0;
    HANDLER frontColor = 0;
    HANDLER backColor = 0;
    uint program;
    uint page =0;
    uint area = 0;
    int left = 0;
    int top = 0;
    int width = 64;
    int heigth = 64;
    qDebug()<<"节目profile:"<<profile<<"节目screen:"<<screen;
    //创建节目
    Check_Error(Create_ProgramBxFile2(&program,"P000",profile));
    //创建图文区
    Check_Error(Create_TextCaptionBxArea(&area,left,top,width,heigth,profile));
    //创建page页
    Check_Error(Create_FFont(&font, "宋体", fontsize, false));
    if(color == RED)
    {
        Check_Error(Create_FColor(&frontColor,(byte)0xff,(byte)0x00,(byte)0x00));
    }
    else
    {
        Check_Error(Create_FColor(&frontColor,(byte)0x00,(byte)0xff,(byte)0x00));
    }
    Check_Error(Create_FColor(&backColor, (byte)0x00,(byte)0x00,(byte)0x00));
    Check_Error(Create_TextBxPage4(&page,gbk.data(),font,frontColor,backColor));
    {
        //设置字体
//        Check_Error(Create_FFont(&font, "宋体", 14, false));
//        Check_Error(TextBxPage_setFont(page, font));

        // config the display style
        Check_Error(Create_DisplayStyleFactory(&factory));
        Check_Error(DisplayStyleFactory_getStyle(&style, factory, 1));
        Check_Error(FObject_dump(style));
        Check_Error(BxPage_setDisplayStyle(page, style));
    }
    //将图文区添加到节目中
    Check_Error(ProgramBxFile_addArea(program,area));
    //将page页添加到图文区中
    Check_Error(AbstractTextCaptionBxArea_addPage(area, page));
    //发送节目
    Check_Error(BxScreen_writeProgramQuickly(screen, program));
Clean:
    Check_Release(program);
    Check_Release(area);
    Check_Release(page);
    Check_Release(font);
    Check_Release(style);
    Check_Release(factory);
    Check_Release(frontColor);
    Check_Release(backColor);
    return error;
}

