#include <string>
#include <QtGui/QApplication>
#include <mainwindow.h>

using namespace std;

int main(int argc, char **argv)
{
    if (argc > 1) {
        for (size_t ii = 1; ii < (argc -1); ii++) {
            const string argument = string(argv[ii]);
            const string following_argument = string(argv[++ii]);
            if (argument == "--config") {
//                Config::setSoftwareConfigFilename(following_argument);
            }
        }
    }
    QApplication a(argc, argv);

    MainWindow w(0);
    w.show();

    int retVal = a.exec();
    return retVal;
}
