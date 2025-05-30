/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2020 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#ifndef PARTITION_CONFIG_H
#define PARTITION_CONFIG_H

#include "utils/NamedEnum.h"

#include <kpmcore/core/partition.h>
#include <kpmcore/fs/filesystem.h>

#include <QObject>
#include <QSet>

class Config : public QObject
{
    Q_OBJECT
    ///@brief The installation choice (Erase, Alongside, ...)
    Q_PROPERTY( InstallChoice installChoice READ installChoice WRITE setInstallChoice NOTIFY installChoiceChanged )

    ///@brief The swap choice (None, Small, Hibernate, ...) which only makes sense when Erase is chosen
    Q_PROPERTY( SwapChoice swapChoice READ swapChoice WRITE setSwapChoice NOTIFY swapChoiceChanged )

    ///@brief Name of the FS that will be used when erasing type disk (e.g. "default filesystem")
    Q_PROPERTY(
        QString eraseModeFilesystem READ eraseFsType WRITE setEraseFsTypeChoice NOTIFY eraseModeFilesystemChanged )

    Q_PROPERTY( QString replaceModeFilesystem READ replaceModeFilesystem WRITE setReplaceFilesystemChoice NOTIFY
                    replaceModeFilesystemChanged )

    Q_PROPERTY( bool allowManualPartitioning READ allowManualPartitioning CONSTANT FINAL )
    Q_PROPERTY( bool preCheckEncryption READ preCheckEncryption CONSTANT FINAL )
    Q_PROPERTY( bool showNotEncryptedBootMessage READ showNotEncryptedBootMessage CONSTANT FINAL )

    Q_PROPERTY( bool lvmEnabled READ isLVMEnabled CONSTANT FINAL )

    Q_PROPERTY( QStringList essentialMounts READ essentialMounts CONSTANT FINAL )

public:
    Config( QObject* parent );
    ~Config() override = default;

    enum InstallChoice
    {
        NoChoice,
        Alongside,
        Erase,
        Replace,
        Manual
    };
    Q_ENUM( InstallChoice )
    static const NamedEnumTable< InstallChoice >& installChoiceNames();

    /** @brief Choice of swap (size and type) */
    enum SwapChoice
    {
        NoSwap,  // don't create any swap, don't use any
        ReuseSwap,  // don't create, but do use existing
        SmallSwap,  // up to 8GiB of swap
        FullSwap,  // ensureSuspendToDisk -- at least RAM size
        SwapFile  // use a file (if supported)
    };
    Q_ENUM( SwapChoice )
    static const NamedEnumTable< SwapChoice >& swapChoiceNames();
    using SwapChoiceSet = QSet< SwapChoice >;

    using EraseFsTypesSet = QStringList;

    /** @brief Choice of LUKS disk encryption generation */
    enum class LuksGeneration
    {
        Luks1,  // First generation of LUKS
        Luks2,  // Second generation of LUKS, default since cryptsetup >= 2.1.0
    };
    Q_ENUM( LuksGeneration )
    static const NamedEnumTable< LuksGeneration >& luksGenerationNames();

    void setConfigurationMap( const QVariantMap& );
    /** @brief Set GS values where other modules configuration has priority
     *
     * Some "required" values are duplicated between modules; if some
     * othe module hasn't already set the GS value, take a value from
     * the partitioning configuration.
     *
     * Applicable GS keys:
     *  - requiredStorageGiB
     */
    void fillGSSecondaryConfiguration() const;

    /** @brief What kind of installation (partitioning) is requested **initially**?
     *
     * @return the partitioning choice (may be @c NoChoice)
     */
    InstallChoice initialInstallChoice() const { return m_initialInstallChoice; }

    /** @brief What kind of installation (partition) is requested **now**?
     *
     * This changes depending on what the user selects (unlike the initial choice,
     * which is fixed by the configuration).
     *
     * @return the partitioning choice (may be @c NoChoice)
     */
    InstallChoice installChoice() const { return m_installChoice; }

    /** @brief The set of swap choices enabled for this install
     *
     * Not all swap choices are supported by each distro, so they
     * can choose to enable or disable them. This method
     * returns a set (hopefully non-empty) of configured swap choices.
     */
    SwapChoiceSet swapChoices() const { return m_swapChoices; }

    /** @brief What kind of swap selection is requested **initially**?
     *
     * @return The swap choice (may be @c NoSwap )
     */
    SwapChoice initialSwapChoice() const { return m_initialSwapChoice; }

    /** @brief What kind of swap selection is requested **now**?
     *
     * A choice of swap only makes sense when install choice Erase is made.
     *
     * @return The swap choice (may be @c NoSwap).
     */
    SwapChoice swapChoice() const { return m_swapChoice; }

    /** @brief Get the list of configured FS types to use with *erase* mode
     *
     * This list is not empty.
     */
    EraseFsTypesSet eraseFsTypes() const { return m_eraseFsTypes; }

    /** @brief Currently-selected FS type for *erase* mode
     */
    QString eraseFsType() const { return m_eraseFsTypeChoice; }

    /// @brief Currently-selected FS type for *replace* mode
    QString replaceModeFilesystem() const { return m_replaceFileSystemChoice; }

    /** @brief Configured default FS type (for other modes than erase)
     *
     * This is not "Unknown" or "Unformatted"
     */
    FileSystem::Type defaultFsType() const { return m_defaultFsType; }

    /// @brief Is manual partitioning allowed (not explicitly disabled in the config file)?
    bool allowManualPartitioning() const { return m_allowManualPartitioning; }

    /** @brief Pre-check encryption checkbox.
     *
     * This is meaningful only if enableLuksAutomatedPartitioning is @c true.
     * Default value is @c false
     */
    bool preCheckEncryption() const { return m_preCheckEncryption; }

    /// @brief Show "Boot partition not encrypted" warning (not explicitly disabled in the config file)?
    bool showNotEncryptedBootMessage() const { return m_showNotEncryptedBootMessage; }

    /** @brief Will @p tableType be ok?
     *
     * If no required types are specified, it's ok, otherwise the
     * type must be named in the list of required types.
     */
    bool acceptPartitionTableType( PartitionTable::TableType tableType ) const;
    /// @brief Returns list of acceptable types. May be empty.
    QStringList partitionTableTypes() const { return m_requiredPartitionTableType; }

    /** @brief The configured LUKS generation (1 or 2)
     */
    LuksGeneration luksFileSystemType() const { return m_luksFileSystemType; }

    /// @brief If zfs encryption should be allowed
    bool allowZfsEncryption() const { return m_allowZfsEncryption; }

    bool isLVMEnabled() const { return m_isLVMEnabled; }

    /** @brief A list of names that can follow /dev/mapper/ that must not be closed
     *
     * These names (if any) are skipped by the ClearMountsJob.
     * The names may contain a trailing '*' which acts as a wildcard.
     * In any other position, '*' is interpreted literally.
     */
    QStringList essentialMounts() const { return m_essentialMounts; }

public Q_SLOTS:
    void setInstallChoice( int );  ///< Translates a button ID or so to InstallChoice
    void setInstallChoice( InstallChoice );
    void setSwapChoice( int );  ///< Translates a button ID or so to SwapChoice
    void setSwapChoice( SwapChoice );
    void setEraseFsTypeChoice( const QString& filesystemName );  ///< See property eraseModeFilesystem
    void setReplaceFilesystemChoice( const QString& filesystemName );

Q_SIGNALS:
    void installChoiceChanged( InstallChoice );
    void swapChoiceChanged( SwapChoice );
    void eraseModeFilesystemChanged( const QString& );
    void replaceModeFilesystemChanged( const QString& );

private:
    /** @brief Handle FS-type configuration, for erase and default */
    void fillConfigurationFSTypes( const QVariantMap& configurationMap );
    EraseFsTypesSet m_eraseFsTypes;
    QString m_eraseFsTypeChoice;
    QString m_replaceFileSystemChoice;
    FileSystem::Type m_defaultFsType;

    SwapChoiceSet m_swapChoices;
    SwapChoice m_initialSwapChoice = NoSwap;
    SwapChoice m_swapChoice = NoSwap;
    LuksGeneration m_luksFileSystemType = LuksGeneration::Luks1;
    InstallChoice m_initialInstallChoice = NoChoice;
    InstallChoice m_installChoice = NoChoice;
    qreal m_requiredStorageGiB = 0.0;  // May duplicate setting in the welcome module
    QStringList m_requiredPartitionTableType;
    bool m_allowZfsEncryption = true;
    bool m_allowManualPartitioning = true;
    bool m_preCheckEncryption = false;
    bool m_showNotEncryptedBootMessage = true;
    bool m_isLVMEnabled = true;
    QStringList m_essentialMounts;
};

/** @brief Given a set of swap choices, return a sensible value from it.
 *
 * "Sensible" here means: if there is one value, use it; otherwise, use
 * NoSwap if there are no choices, or if NoSwap is one of the choices, in the set.
 * If that's not possible, any value from the set.
 */
Config::SwapChoice pickOne( const Config::SwapChoiceSet& s );


#endif
