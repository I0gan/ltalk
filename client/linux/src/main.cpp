#include "center.h"

#include <QApplication>
#include "http.h"
#include <unistd.h>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Ltalk for linux");
    app.setApplicationVersion("0.1");
    app.setOrganizationName("LYXF");
    app.setApplicationDisplayName("Ltalk");
    Center center;
    //center.test();
    center.init();
    center.changeTheme("default");
    center.start();
    return app.exec();
}
