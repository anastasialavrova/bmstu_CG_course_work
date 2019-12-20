#include "window361.h"
#include "client.h"
#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);   // because it's a Qt application
    Window361 window;               // make and show the window--size is already correct
    window.show();
    Drawable *sheet = window.getDrawable();

    // Handle command line arguments:
    QStringList args = app.arguments();
    Client client(sheet);           // the client gets a (Drawable *)
    window.setPageTurner(&client);  // the window must be given a (PageTurner *)

    return app.exec();
}

