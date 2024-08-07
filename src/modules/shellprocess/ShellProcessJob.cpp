/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2018 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "ShellProcessJob.h"

#include "CalamaresVersion.h"
#include "GlobalStorage.h"
#include "JobQueue.h"

#include "utils/CommandList.h"
#include "utils/Logger.h"
#include "utils/Variant.h"

#include <QDateTime>
#include <QThread>

ShellProcessJob::ShellProcessJob( QObject* parent )
    : Calamares::CppJob( parent )
    , m_commands( nullptr )
{
}

ShellProcessJob::~ShellProcessJob() {}

QString
ShellProcessJob::prettyName() const
{
    if ( m_name )
    {
        return m_name->get();
    }
    return tr( "Running shell processes…", "@status" );
}

Calamares::JobResult
ShellProcessJob::exec()
{

    if ( !m_commands || m_commands->isEmpty() )
    {
        cWarning() << "No commands to execute" << moduleInstanceKey();
        return Calamares::JobResult::ok();
    }

    return m_commands->run();
}

void
ShellProcessJob::setConfigurationMap( const QVariantMap& configurationMap )
{
    bool dontChroot = Calamares::getBool( configurationMap, "dontChroot", false );
    qint64 timeout = Calamares::getInteger( configurationMap, "timeout", 30 );
    if ( timeout < 1 )
    {
        timeout = 30;
    }
    bool verbose = Calamares::getBool( configurationMap, "verbose", false );

    if ( configurationMap.contains( "script" ) )
    {
        m_commands = std::make_unique< Calamares::CommandList >(
            configurationMap.value( "script" ), !dontChroot, std::chrono::seconds( timeout ) );
        if ( m_commands->isEmpty() )
        {
            cDebug() << "ShellProcessJob: \"script\" contains no commands for" << moduleInstanceKey();
        }
        m_commands->updateVerbose( verbose );
    }
    else
    {
        cWarning() << "No script given for ShellProcessJob" << moduleInstanceKey();
    }

    bool labels_ok = false;
    auto labels = Calamares::getSubMap( configurationMap, "i18n", labels_ok );
    if ( labels_ok )
    {
        if ( labels.contains( "name" ) )
        {
            m_name = std::make_unique< Calamares::Locale::TranslatedString >( labels, "name" );
        }
    }
}

CALAMARES_PLUGIN_FACTORY_DEFINITION( ShellProcessJobFactory, registerPlugin< ShellProcessJob >(); )
