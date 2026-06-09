#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setOrganizationName("xyBased");
    app.setApplicationName("Classifiable_RPG");
    MainWindow window;
    window.show();
    return app.exec();
}
