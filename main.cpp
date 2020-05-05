#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 防止qDebug中文乱码
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    // 设置皮肤
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(15, 15, 15));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    qApp->setPalette(palette);

    MainWindow w;
    w.show();

    return a.exec();
}
