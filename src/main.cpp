#include "MainWindow.hpp"

#include <QApplication>

#include <KLocalizedString>
#include <KAboutData>
#include <KCrash>
#include <KDBusService>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("KHashCat");

    KAboutData about_data {
        QStringLiteral("KHashCat"),
        i18n("KHashCat"),
        QStringLiteral("0.1"),
        i18n("Graphical interface for Hashcat"),
        KAboutLicense::GPL_V3,
        i18n("© 2025 Mohammadreza Hajianpour")
    };

    about_data.addAuthor(
        i18n("Mohammadreza Hajianpour"),
        i18n("Developer"),
        QStringLiteral("hajianpour.mr@gmail.com")
    );

    KAboutData::setApplicationData(about_data);

    // Crash handling
    KCrash::initialize();

    // Ensure single instance
    KDBusService service(KDBusService::Unique);

    MainWindow window;
    window.show();

    return app.exec();
}
