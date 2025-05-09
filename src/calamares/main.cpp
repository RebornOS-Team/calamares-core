/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2014 Teo Mrnjavac <teo@kde.org>
 *   SPDX-FileCopyrightText: 2017-2020 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "CalamaresApplication.h"

#include "Settings.h"
#include "utils/Dirs.h"
#include "utils/Logger.h"
#include "utils/Retranslator.h"

// From 3rdparty/
#include "kdsingleapplication.h"

#include <KAboutData>
#ifdef BUILD_CRASH_REPORTING
#include <KCrash>
#endif

#include <QCommandLineParser>
#include <QDebug>
#include <QDir>

#include <memory>

/** @brief Gets debug-level from -D command-line-option
 *
 * If unset, use LOGERROR (corresponding to -D1), although
 * effectively -D2 is the lowest level you can set for
 * logging-to-the-console, and everything always gets
 * logged to the session file).
 */
static unsigned int
debug_level( QCommandLineParser& parser, QCommandLineOption& levelOption )
{
    if ( !parser.isSet( levelOption ) )
    {
        return Logger::LOGERROR;
    }

    bool ok = true;
    int l = parser.value( levelOption ).toInt( &ok );
    if ( !ok || ( l < 0 ) )
    {
        return Logger::LOGVERBOSE;
    }
    else
    {
        return static_cast< unsigned int >( l );  // l >= 0
    }
}

/** @brief Handles the command-line arguments
 *
 * Sets up internals for Calamares based on command-line arguments like `-D`,
 * `-d`, etc. Returns @c true if this is a *debug* run, i.e. if the `-d`
 * command-line flag is given, @c false otherwise.
 */
static bool
handle_args( CalamaresApplication& a )
{
    QCommandLineOption debugOption( QStringList { "d", "debug" },
                                    "Also look in current directory for configuration. Implies -D8." );
    QCommandLineOption debugLevelOption(
        QStringLiteral( "D" ), "Verbose output for debugging purposes (0-8).", "level" );
    QCommandLineOption debugTxOption( QStringList { "T", "debug-translation" },
                                      "Also look in the current directory for translation." );

    QCommandLineOption configOption(
        QStringList { "c", "config" }, "Configuration directory to use, for testing purposes.", "config" );
    QCommandLineOption xdgOption( QStringList { "X", "xdg-config" }, "Use XDG_{CONFIG,DATA}_DIRS as well." );

    QCommandLineParser parser;
    parser.setApplicationDescription( "Distribution-independent installer framework" );
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption( debugOption );
    parser.addOption( debugLevelOption );
    parser.addOption( configOption );
    parser.addOption( xdgOption );
    parser.addOption( debugTxOption );

    parser.process( a );

    Logger::setupLogLevel( parser.isSet( debugOption ) ? Logger::LOGVERBOSE : debug_level( parser, debugLevelOption ) );
    if ( parser.isSet( configOption ) )
    {
        Calamares::setAppDataDir( QDir( parser.value( configOption ) ) );
    }
    if ( parser.isSet( xdgOption ) )
    {
        Calamares::setXdgDirs();
    }
    Calamares::setAllowLocalTranslation( parser.isSet( debugOption ) || parser.isSet( debugTxOption ) );

    return parser.isSet( debugOption );
}

int
main( int argc, char* argv[] )
{
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    // Not needed in Qt6
    QApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
#endif
    CalamaresApplication a( argc, argv );

    KAboutData aboutData( "calamares",
                          "Calamares",
                          a.applicationVersion(),
                          "The universal system installer",
                          KAboutLicense::GPL_V3,
                          QString(),
                          QString(),
                          "https://calamares.io",
                          "https://codeberg.org/Calamares/calamares/issues" );
    KAboutData::setApplicationData( aboutData );
    a.setApplicationDisplayName( QString() );  // To avoid putting an extra "Calamares/" into the log-file

#ifdef BUILD_CRASH_REPORTING
    KCrash::initialize();
    // KCrash::setCrashHandler();
    KCrash::setDrKonqiEnabled( true );
    KCrash::setFlags( KCrash::SaferDialog | KCrash::AlwaysDirectly );
#endif

    std::unique_ptr< KDSingleApplication > possiblyUnique;
    const bool is_debug = handle_args( a );
    if ( !is_debug )
    {
        possiblyUnique = std::make_unique< KDSingleApplication >();
        if ( !possiblyUnique->isPrimaryInstance() )
        {
            qCritical() << "Calamares is already running.";
            return 87;  // EUSERS on Linux
        }
    }

    Calamares::Settings::init( is_debug );
    if ( !Calamares::Settings::instance() || !Calamares::Settings::instance()->isValid() )
    {
        qCritical() << "Calamares has invalid settings, shutting down.";
        return 78;  // EX_CONFIG on FreeBSD
    }
    a.init();
    return a.exec();
}
