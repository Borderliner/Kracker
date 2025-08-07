#include "MainWindow.hpp"

#include <QApplication>

#include <KLocalizedString>
#include <KAboutData>
#include <KCrash>
#include <KDBusService>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("KDE");

    KAboutData about_data {
        QStringLiteral("kracker"),
        i18n("Kracker"),
        QStringLiteral("0.3"),
        i18n("Graphical interface for Hashcat & John the Ripper"),
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
