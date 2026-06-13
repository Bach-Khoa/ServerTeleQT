#include <QApplication>
#include <QStyleFactory>
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("GDTTelemetryTLDK35");
    app.setOrganizationName("GDT");
    app.setApplicationVersion("1.0.0");
    app.setStyle(QStyleFactory::create("Fusion"));

    GDT::MainWindow win;
    win.showMaximized();
    return app.exec();
}
