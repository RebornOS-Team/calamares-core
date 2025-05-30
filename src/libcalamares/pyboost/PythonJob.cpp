/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2014-2016 Teo Mrnjavac <teo@kde.org>
 *   SPDX-FileCopyrightText: 2018-2020 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */
#include "PythonJob.h"

#include "PythonHelper.h"
#include "PythonJobApi.h"
#include "PythonTypes.h"

#include "CalamaresVersion.h"
#include "GlobalStorage.h"
#include "JobQueue.h"
#include "python/Api.h"
#include "utils/Logger.h"

#include <QDir>

#ifdef WITH_PYBIND11
#error Source only for Boost::Python
#else
#endif

static const char* s_preScript = nullptr;

namespace bp = boost::python;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG( "-Wdisabled-macro-expansion" )

BOOST_PYTHON_FUNCTION_OVERLOADS( mount_overloads, Calamares::Python::mount, 2, 4 );
BOOST_PYTHON_FUNCTION_OVERLOADS( target_env_call_str_overloads, CalamaresPython::target_env_call, 1, 3 );
BOOST_PYTHON_FUNCTION_OVERLOADS( target_env_call_list_overloads, CalamaresPython::target_env_call, 1, 3 );
BOOST_PYTHON_FUNCTION_OVERLOADS( check_target_env_call_str_overloads, CalamaresPython::check_target_env_call, 1, 3 );
BOOST_PYTHON_FUNCTION_OVERLOADS( check_target_env_call_list_overloads, CalamaresPython::check_target_env_call, 1, 3 );
BOOST_PYTHON_FUNCTION_OVERLOADS( check_target_env_output_str_overloads,
                                 CalamaresPython::check_target_env_output,
                                 1,
                                 3 );
BOOST_PYTHON_FUNCTION_OVERLOADS( check_target_env_output_list_overloads,
                                 CalamaresPython::check_target_env_output,
                                 1,
                                 3 );
BOOST_PYTHON_FUNCTION_OVERLOADS( target_env_process_output_overloads,
                                 CalamaresPython::target_env_process_output,
                                 1,
                                 4 );
BOOST_PYTHON_FUNCTION_OVERLOADS( host_env_process_output_overloads, CalamaresPython::host_env_process_output, 1, 4 );

QT_WARNING_POP

BOOST_PYTHON_MODULE( libcalamares )
{
    bp::object package = bp::scope();
    package.attr( "__path__" ) = "libcalamares";

    bp::scope().attr( "ORGANIZATION_NAME" ) = CALAMARES_ORGANIZATION_NAME;
    bp::scope().attr( "ORGANIZATION_DOMAIN" ) = CALAMARES_ORGANIZATION_DOMAIN;
    bp::scope().attr( "APPLICATION_NAME" ) = CALAMARES_APPLICATION_NAME;
    bp::scope().attr( "VERSION" ) = CALAMARES_VERSION;
    bp::scope().attr( "VERSION_SHORT" ) = CALAMARES_VERSION_SHORT;

    bp::class_< CalamaresPython::PythonJobInterface >( "Job", bp::init< Calamares::PythonJob* >() )
        .def_readonly( "module_name", &CalamaresPython::PythonJobInterface::moduleName )
        .def_readonly( "pretty_name", &CalamaresPython::PythonJobInterface::prettyName )
        .def_readonly( "working_path", &CalamaresPython::PythonJobInterface::workingPath )
        .def_readonly( "configuration", &CalamaresPython::PythonJobInterface::configuration )
        .def( "setprogress",
              &CalamaresPython::PythonJobInterface::setprogress,
              bp::args( "progress" ),
              "Reports the progress status of this job to Calamares, "
              "as a real number between 0 and 1." );

    bp::class_< CalamaresPython::GlobalStoragePythonWrapper >( "GlobalStorage",
                                                               bp::init< Calamares::GlobalStorage* >() )
        .def( "contains", &CalamaresPython::GlobalStoragePythonWrapper::contains )
        .def( "count", &CalamaresPython::GlobalStoragePythonWrapper::count )
        .def( "insert", &CalamaresPython::GlobalStoragePythonWrapper::insert )
        .def( "keys", &CalamaresPython::GlobalStoragePythonWrapper::keys )
        .def( "remove", &CalamaresPython::GlobalStoragePythonWrapper::remove )
        .def( "value", &CalamaresPython::GlobalStoragePythonWrapper::value );

    // libcalamares.utils submodule starts here
    bp::object utilsModule( bp::handle<>( bp::borrowed( PyImport_AddModule( "libcalamares.utils" ) ) ) );
    bp::scope().attr( "utils" ) = utilsModule;
    bp::scope utilsScope = utilsModule;
    Q_UNUSED( utilsScope )

    // .. Logging functions
    bp::def(
        "debug", &Calamares::Python::debug, bp::args( "s" ), "Writes the given string to the Calamares debug stream." );
    bp::def( "warning",
             &Calamares::Python::warning,
             bp::args( "s" ),
             "Writes the given string to the Calamares warning stream." );
    bp::def( "warn",
             &Calamares::Python::warning,
             bp::args( "s" ),
             "Writes the given string to the Calamares warning stream." );
    bp::def(
        "error", &Calamares::Python::error, bp::args( "s" ), "Writes the given string to the Calamares error stream." );


    // .. YAML functions
    bp::def( "load_yaml", &Calamares::Python::load_yaml, bp::args( "path" ), "Loads YAML from a file." );

    // .. Filesystem functions
    bp::def( "mount",
             &Calamares::Python::mount,
             mount_overloads( bp::args( "device_path", "mount_point", "filesystem_name", "options" ),
                              "Runs the mount utility with the specified parameters.\n"
                              "Returns the program's exit code, or:\n"
                              "-1 = QProcess crash\n"
                              "-2 = QProcess cannot start\n"
                              "-3 = bad arguments" ) );

    // .. Process functions
    bp::def(
        "target_env_call",
        static_cast< int ( * )( const std::string&, const std::string&, int ) >( &CalamaresPython::target_env_call ),
        target_env_call_str_overloads( bp::args( "command", "stdin", "timeout" ),
                                       "Runs the specified command in the chroot of the target system.\n"
                                       "Returns the program's exit code, or:\n"
                                       "-1 = QProcess crash\n"
                                       "-2 = QProcess cannot start\n"
                                       "-3 = bad arguments\n"
                                       "-4 = QProcess timeout" ) );
    bp::def( "target_env_call",
             static_cast< int ( * )( const bp::list&, const std::string&, int ) >( &CalamaresPython::target_env_call ),
             target_env_call_list_overloads( bp::args( "command_list", "stdin", "timeout" ),
                                             "Runs the specified command_list in the chroot of the target system.\n"
                                             "Returns the program's exit code, or:\n"
                                             "-1 = QProcess crash\n"
                                             "-2 = QProcess cannot start\n"
                                             "-3 = bad arguments\n"
                                             "-4 = QProcess timeout" ) );

    bp::def( "check_target_env_call",
             static_cast< int ( * )( const std::string&, const std::string&, int ) >(
                 &CalamaresPython::check_target_env_call ),
             check_target_env_call_str_overloads( bp::args( "command", "stdin", "timeout" ),
                                                  "Runs the specified command in the chroot of the target system.\n"
                                                  "Returns 0, which is program's exit code if the program exited "
                                                  "successfully, or raises a subprocess.CalledProcessError." ) );
    bp::def(
        "check_target_env_call",
        static_cast< int ( * )( const bp::list&, const std::string&, int ) >( &CalamaresPython::check_target_env_call ),
        check_target_env_call_list_overloads( bp::args( "args", "stdin", "timeout" ),
                                              "Runs the specified command in the chroot of the target system.\n"
                                              "Returns 0, which is program's exit code if the program exited "
                                              "successfully, or raises a subprocess.CalledProcessError." ) );

    bp::def( "check_target_env_output",
             static_cast< std::string ( * )( const std::string&, const std::string&, int ) >(
                 &CalamaresPython::check_target_env_output ),
             check_target_env_output_str_overloads( bp::args( "command", "stdin", "timeout" ),
                                                    "Runs the specified command in the chroot of the target system.\n"
                                                    "Returns the program's standard output, and raises a "
                                                    "subprocess.CalledProcessError if something went wrong." ) );
    bp::def( "check_target_env_output",
             static_cast< std::string ( * )( const bp::list&, const std::string&, int ) >(
                 &CalamaresPython::check_target_env_output ),
             check_target_env_output_list_overloads( bp::args( "args", "stdin", "timeout" ),
                                                     "Runs the specified command in the chroot of the target system.\n"
                                                     "Returns the program's standard output, and raises a "
                                                     "subprocess.CalledProcessError if something went wrong." ) );
    bp::def( "target_env_process_output",
             &CalamaresPython::target_env_process_output,
             target_env_process_output_overloads( bp::args( "command", "callback", "stdin", "timeout" ),
                                                  "Runs the specified @p command in the target system." ) );
    bp::def( "host_env_process_output",
             &CalamaresPython::host_env_process_output,
             host_env_process_output_overloads( bp::args( "command", "callback", "stdin", "timeout" ),
                                                "Runs the specified command in the host system." ) );

    // .. String functions
    bp::def( "obscure",
             &Calamares::Python::obscure,
             bp::args( "s" ),
             "Simple string obfuscation function based on KStringHandler::obscure.\n"
             "Returns a string, generated using a simple symmetric encryption.\n"
             "Applying the function to a string obscured by this function will result "
             "in the original string." );

    // .. Translation functions
    bp::def( "gettext_languages",
             &Calamares::Python::gettext_languages,
             "Returns list of languages (most to least-specific) for gettext." );

    bp::def( "gettext_path", &Calamares::Python::gettext_path, "Returns path for gettext search." );
}


namespace Calamares
{

struct PythonJob::Private
{
    bp::object m_prettyStatusMessage;
};

PythonJob::PythonJob( const QString& scriptFile,
                      const QString& workingPath,
                      const QVariantMap& moduleConfiguration,
                      QObject* parent )
    : Job( parent )
    , m_d( std::make_unique< Private >() )
    , m_scriptFile( scriptFile )
    , m_workingPath( workingPath )
    , m_configurationMap( moduleConfiguration )
{
}


PythonJob::~PythonJob() {}

QString
PythonJob::prettyName() const
{
    return QDir( m_workingPath ).dirName();
}


QString
PythonJob::prettyStatusMessage() const
{
    // The description is updated when progress is reported, see emitProgress()
    const auto s = getDescription();
    if ( s.isEmpty() )
    {
        return tr( "Running %1 operation…", "@status" ).arg( QDir( m_workingPath ).dirName() );
    }
    else
    {
        return s;
    }
}

static QString
pythonStringMethod( bp::dict& script, const char* funcName )
{
    bp::object func = script.get( funcName, bp::object() );
    if ( !func.is_none() )
    {
        bp::extract< std::string > result( func() );
        return result.check() ? QString::fromStdString( result() ).trimmed() : QString();
    }
    return QString();
}


JobResult
PythonJob::exec()
{
    // We assume m_scriptFile to be relative to m_workingPath.
    QDir workingDir( m_workingPath );
    if ( !workingDir.exists() || !workingDir.isReadable() )
    {
        return JobResult::error( tr( "Bad working directory path", "@error" ),
                                 tr( "Working directory %1 for python job %2 is not readable.", "@error" )
                                     .arg( m_workingPath )
                                     .arg( prettyName() ) );
    }

    QFileInfo scriptFI( workingDir.absoluteFilePath( m_scriptFile ) );
    if ( !scriptFI.exists() || !scriptFI.isFile() || !scriptFI.isReadable() )
    {
        return JobResult::error( tr( "Bad main script file", "@error" ),
                                 tr( "Main script file %1 for python job %2 is not readable.", "@error" )
                                     .arg( scriptFI.absoluteFilePath() )
                                     .arg( prettyName() ) );
    }

    try
    {
        bp::dict scriptNamespace = CalamaresPython::Helper::instance()->createCleanNamespace();

        bp::object calamaresModule = bp::import( "libcalamares" );
        bp::dict calamaresNamespace = bp::extract< bp::dict >( calamaresModule.attr( "__dict__" ) );

        calamaresNamespace[ "job" ] = CalamaresPython::PythonJobInterface( this );
        calamaresNamespace[ "globalstorage" ]
            = CalamaresPython::GlobalStoragePythonWrapper( JobQueue::instance()->globalStorage() );

        if ( s_preScript )
        {
            bp::exec( s_preScript, scriptNamespace, scriptNamespace );
        }

        cDebug() << "Job file" << scriptFI.absoluteFilePath();
        bp::object execResult
            = bp::exec_file( scriptFI.absoluteFilePath().toLocal8Bit().data(), scriptNamespace, scriptNamespace );
        bp::object entryPoint = scriptNamespace[ "run" ];

        m_d->m_prettyStatusMessage = scriptNamespace.get( "pretty_status_message", bp::object() );
        QString possibleDescription = pythonStringMethod( scriptNamespace, "pretty_name" );
        if ( possibleDescription.isEmpty() )
        {
            bp::extract< std::string > entryPoint_doc_attr( entryPoint.attr( "__doc__" ) );

            if ( entryPoint_doc_attr.check() )
            {
                possibleDescription= QString::fromStdString( entryPoint_doc_attr() ).trimmed();
                auto i_newline = possibleDescription.indexOf( '\n' );
                if ( i_newline > 0 )
                {
                    possibleDescription.truncate( i_newline );
                }
                cDebug() << Logger::SubEntry << "Job description from __doc__" << prettyName() << '=' << possibleDescription;
            }
        }
        else
        {
            cDebug() << Logger::SubEntry << "Job description from pretty_name" << prettyName() << '=' << possibleDescription;
        }
        setDescription( possibleDescription);
        emit progress( 0 );

        bp::object runResult = entryPoint();

        if ( runResult.is_none() )
        {
            return JobResult::ok();
        }
        else  // Something happened in the Python job
        {
            bp::tuple resultTuple = bp::extract< bp::tuple >( runResult );
            QString message = QString::fromStdString( bp::extract< std::string >( resultTuple[ 0 ] ) );
            QString description = QString::fromStdString( bp::extract< std::string >( resultTuple[ 1 ] ) );
            return JobResult::error( message, description );
        }
    }
    catch ( bp::error_already_set& )
    {
        QString msg;
        if ( PyErr_Occurred() )
        {
            msg = CalamaresPython::Helper::instance()->handleLastError();
        }
        bp::handle_exception();
        PyErr_Clear();
        return JobResult::internalError( tr( "Boost.Python error in job \"%1\"", "@error" ).arg( prettyName() ),
                                         msg,
                                         JobResult::PythonUncaughtException );
    }
}


void
PythonJob::emitProgress( qreal progressValue )
{
    // This is called from the JobApi (and only from there) from the Job thread,
    // so it is safe to call into the Python interpreter. Update the description
    // as needed (don't call this from prettyStatusMessage(), which can be
    // called from other threads as well).
    if ( m_d && !m_d->m_prettyStatusMessage.is_none() )
    {
        QString r;
        bp::extract< std::string > result( m_d->m_prettyStatusMessage() );
        r = result.check() ? QString::fromStdString( result() ).trimmed() : QString();
        if ( !r.isEmpty() )
        {
            setDescription(r);
        }
    }
    emit progress( progressValue );
}

void
PythonJob::setInjectedPreScript( const char* preScript )
{
    s_preScript = preScript;
    cDebug() << "Python pre-script set to string" << Logger::Pointer( preScript ) << "length"
             << ( preScript ? strlen( preScript ) : 0 );
}

QString PythonJob::getDescription() const
{
    QMutexLocker l(&m_descriptionMutex);
    return m_description;
}

void PythonJob::setDescription(const QString & s)
{
    QMutexLocker l(&m_descriptionMutex);
    m_description = s;
}

}  // namespace Calamares
