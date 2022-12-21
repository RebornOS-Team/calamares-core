/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2017 Kyle Robbertze <kyle@aims.ac.za>
 *   SPDX-FileCopyrightText: 2017-2018 2020, Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "PackageModel.h"

#include "utils/Logger.h"
#include "utils/Variant.h"
#include "utils/Yaml.h"

/// Recursive helper for setSelections()
void PackageModel::internalSetSelections( const QStringList& selectNames, PackageTreeItem* item )
{
    // cDebug()<<">> In PackageModel::internalSetSelections("<<selectNames<<", "<<item<<")";
    for ( int i = 0; i < item->childCount(); i++ )
    {
        auto* child = item->child( i );
        if ( !child->isGroup() && !child->packageName().isEmpty() )
        {
            // cDebug() << Logger::SubEntry << Logger::SubEntry << "Running this->updatePackageSelectionStates(" <<child->packageName()<<" ,"<<Qt::CheckState::Checked<<")";
            this->updatePackageSelectionStates( child->packageName(), Qt::CheckState::Checked );
        }
        else
        {
            // cDebug() << Logger::SubEntry << Logger::SubEntry << "Running this->internalSetSelections(" <<selectNames<<" ,"<<child<<")";
            this->internalSetSelections( selectNames, child );        
        }
    }
    if ( item->isGroup() && selectNames.contains( item->name() ) )
    {     
        cDebug()<<"*****************************************************************************************************************************************";
        item->setSelected( Qt::CheckState::Checked );
    }
}

/** @brief Propagates selection state @p selectState over @p item
 *
 * Propagates @p selectState throughout the tree under @p item
 * and all the duplicates of its packages
 */
void
PackageModel::propagatePackageSelectionStates( const Qt::CheckState& selectState, PackageTreeItem* item )
{
    if ( selectState != Qt::PartiallyChecked && item->isGroup() )
    {
        for ( int i = 0; i < item->childCount(); i++ )
        {
            auto* child = item->child( i );
            if ( !child->isGroup() )
            {
                this->updatePackageSelectionStates( child->packageName(), selectState );
            }
            else
            {
                propagatePackageSelectionStates(selectState, child);
            }
        }
    }
}

/** @brief Copies selection state @p selectState
 * for packages with name @p selectName over @p item
 *
 * Sets state to @p selectState for all copies of "packageName" @p selectName
 * throughout the tree under @p item 
 */
static void
updatePackageSelectionStates( const QString& selectName, const Qt::CheckState& selectState, PackageTreeItem* item )
{
    if ( item->isGroup() )
    {
        for ( int i = 0; i < item->childCount(); i++ )
        {
            auto* child = item->child( i );
            // cDebug()<<">> In updatePackageSelectionStates("<<selectName<<", "<<selectState<<", "<<item<<")";
            // cDebug() << Logger::SubEntry << Logger::SubEntry << "Running updatePackageSelectionStates(" <<selectName<<" ,"<<selectState<<", "<<child<<")";
            updatePackageSelectionStates( selectName, selectState, child );
        }
    }
    else if ( QString::compare(selectName, item->packageName(), Qt::CaseInsensitive) == 0 && item->isSelected() != selectState )
    {
        cDebug()<<">> In updatePackageSelectionStates("<<selectName<<", "<<selectState<<", "<<item<<")";
        cDebug() << Logger::SubEntry << "Running item->setSelected("<<selectState<<")"<<" for "<<"item->name(): "<<item->name()<<", "<<"item->packageName(): "<<item->packageName();
        item->setSelected( selectState );
    }
}

/** @brief Collects all the "source" values from @p groupList
 *
 * Iterates over @p groupList and returns all nonempty "source"
 * values from the maps.
 *
 */
static QStringList
collectSources( const QVariantList& groupList )
{
    QStringList sources;
    for ( const QVariant& group : groupList )
    {
        QVariantMap groupMap = group.toMap();
        if ( !groupMap[ "source" ].toString().isEmpty() )
        {
            sources.append( groupMap[ "source" ].toString() );
        }
    }

    return sources;
}

PackageModel::PackageModel( QObject* parent )
    : QAbstractItemModel( parent )
{
}

PackageModel::~PackageModel()
{
    delete m_rootItem;
}

QModelIndex
PackageModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !m_rootItem || !hasIndex( row, column, parent ) )
    {
        return QModelIndex();
    }

    PackageTreeItem* parentItem;

    if ( !parent.isValid() )
    {
        parentItem = m_rootItem;
    }
    else
    {
        parentItem = static_cast< PackageTreeItem* >( parent.internalPointer() );
    }

    PackageTreeItem* childItem = parentItem->child( row );
    if ( childItem )
    {
        return createIndex( row, column, childItem );
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex
PackageModel::parent( const QModelIndex& index ) const
{
    if ( !m_rootItem || !index.isValid() )
    {
        return QModelIndex();
    }

    PackageTreeItem* child = static_cast< PackageTreeItem* >( index.internalPointer() );
    PackageTreeItem* parent = child->parentItem();

    if ( parent == m_rootItem )
    {
        return QModelIndex();
    }
    return createIndex( parent->row(), 0, parent );
}

int
PackageModel::rowCount( const QModelIndex& parent ) const
{
    if ( !m_rootItem || ( parent.column() > 0 ) )
    {
        return 0;
    }

    PackageTreeItem* parentItem;
    if ( !parent.isValid() )
    {
        parentItem = m_rootItem;
    }
    else
    {
        parentItem = static_cast< PackageTreeItem* >( parent.internalPointer() );
    }

    return parentItem->childCount();
}

int
PackageModel::columnCount( const QModelIndex& ) const
{
    return 2;
}

QVariant
PackageModel::data( const QModelIndex& index, int role ) const
{
    if ( !m_rootItem || !index.isValid() )
    {
        return QVariant();
    }

    PackageTreeItem* item = static_cast< PackageTreeItem* >( index.internalPointer() );
    switch ( role )
    {
    case Qt::CheckStateRole:
        return index.column() == NameColumn ? ( item->isImmutable() ? QVariant() : item->isSelected() ) : QVariant();
    case Qt::DisplayRole:
        return item->isHidden() ? QVariant() : item->data( index.column() );
    case MetaExpandRole:
        return item->isHidden() ? false : item->expandOnStart();
    default:
        return QVariant();
    }
}

bool
PackageModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    cDebug()<<">> In PackageModel::setData("<<index<<", "<<value<<", "<<role<<")";
    if ( !m_rootItem )
    {
        return false;
    }

    if ( role == Qt::CheckStateRole && index.isValid() )
    {
        PackageTreeItem* item = static_cast< PackageTreeItem* >( index.internalPointer() );
        item->setSelected( static_cast< Qt::CheckState >( value.toInt() ) );
        cDebug() << Logger::SubEntry << " item->isGroup()" << item->isGroup();
        cDebug() << Logger::SubEntry << " item->name()" << item->name();
        cDebug() << Logger::SubEntry << " item->packageName()" << item->packageName();

        if( !item->isGroup() )
        {
            cDebug() << Logger::SubEntry << "Running this->updatePackageSelectionStates(" <<item->packageName()<<" ,"<<item->isSelected()<<")";
            this->updatePackageSelectionStates(item->packageName(), item->isSelected());
        } else {
            propagatePackageSelectionStates( item->isSelected(), item );
        }

        emit dataChanged( this->index( 0, 0 ),
                          index.sibling( index.column(), index.row() + 1 ),
                          QVector< int >( Qt::CheckStateRole ) );
    }
    return true;
}

Qt::ItemFlags
PackageModel::flags( const QModelIndex& index ) const
{
    if ( !m_rootItem || !index.isValid() )
    {
        return Qt::ItemFlags();
    }
    if ( index.column() == NameColumn )
    {
        PackageTreeItem* item = static_cast< PackageTreeItem* >( index.internalPointer() );
        if ( item->isImmutable() || item->isNoncheckable() )
        {
            return QAbstractItemModel::flags( index );  //Qt::NoItemFlags;
        } 
        // else if ( item->isNoncheckable() )
        // {
        //     return static_cast<Qt::ItemFlags>(~ Qt::ItemIsEnabled & QAbstractItemModel::flags( index ));
        // }
        return Qt::ItemIsUserCheckable | QAbstractItemModel::flags( index );
    }
    return QAbstractItemModel::flags( index );
}

QVariant
PackageModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        return ( section == NameColumn ) ? tr( "Name" ) : tr( "Description" );
    }
    return QVariant();
}

void
PackageModel::setSelections( const QStringList& selectNames )
{
    cDebug()<<">> In PackageModel::setSelections("<<selectNames<<")";
    if ( m_rootItem )
    {
        cDebug() << Logger::SubEntry << "Running this->internalSetSelections(" <<selectNames<<" ,"<<m_rootItem<<")";
        this->internalSetSelections( selectNames, m_rootItem );

        emit dataChanged( this->index( 0, 0 ),
                          this->index( 1000, 1000 ),
                          QVector< int >( Qt::CheckStateRole ) );        
    }
}

void
PackageModel::updatePackageSelectionStates( const QString& selectName, const Qt::CheckState& selectState )
{
    if ( m_rootItem && !selectName.isEmpty() )
    {
        ::updatePackageSelectionStates( selectName, selectState, m_rootItem );

        emit dataChanged( this->index( 0, 0 ),
                          this->index( 1000, 1000 ),
                          QVector< int >( Qt::CheckStateRole ) );
    }
}

PackageTreeItem::List
PackageModel::getPackages() const
{
    if ( !m_rootItem )
    {
        return PackageTreeItem::List();
    }

    auto items = getItemPackages( m_rootItem );
    for ( auto package : m_hiddenItems )
    {
        if ( package->hiddenSelected() )
        {
            items.append( getItemPackages( package ) );
        }
    }
    return items;
}

PackageTreeItem::List
PackageModel::getItemPackages( PackageTreeItem* item ) const
{
    PackageTreeItem::List selectedPackages;
    for ( int i = 0; i < item->childCount(); i++ )
    {
        auto* child = item->child( i );
        if ( child->isSelected() == Qt::Unchecked )
        {
            continue;
        }

        if ( child->isPackage() )  // package
        {
            selectedPackages.append( child );
        }
        else
        {
            selectedPackages.append( getItemPackages( child ) );
        }
    }
    return selectedPackages;
}

void
PackageModel::setupModelData( const QVariantList& groupList, PackageTreeItem* parent )
{
    for ( const auto& group : groupList )
    {
        QVariantMap groupMap = group.toMap();
        if ( groupMap.isEmpty() )
        {
            continue;
        }

        PackageTreeItem* item = new PackageTreeItem( groupMap, PackageTreeItem::GroupTag { parent } );
        if ( groupMap.contains( "selected" ) )
        {
            item->setSelected( CalamaresUtils::getBool( groupMap, "selected", false ) ? Qt::Checked : Qt::Unchecked );
        }
        if ( groupMap.contains( "packages" ) )
        {
            for ( const auto& packageName : groupMap.value( "packages" ).toList() )
            {
                if ( packageName.type() == QVariant::String )
                {
                    item->appendChild( new PackageTreeItem( packageName.toString(), item ) );
                }
                else
                {
                    QVariantMap m = packageName.toMap();
                    if ( !m.isEmpty() )
                    {
                        item->appendChild( new PackageTreeItem( m, PackageTreeItem::PackageTag { item } ) );
                    }
                }
            }
            if ( !item->childCount() )
            {
                cWarning() << "*packages* under" << item->name() << "is empty.";
            }
        }
        if ( groupMap.contains( "subgroups" ) )
        {
            bool haveWarned = false;
            const auto& subgroupValue = groupMap.value( "subgroups" );
            if ( !subgroupValue.canConvert( QVariant::List ) )
            {
                cWarning() << "*subgroups* under" << item->name() << "is not a list.";
                haveWarned = true;
            }

            QVariantList subgroups = groupMap.value( "subgroups" ).toList();
            if ( !subgroups.isEmpty() )
            {
                setupModelData( subgroups, item );
                // The children might be checked while the parent isn't (yet).
                // Children are added to their parent (below) without affecting
                // the checked-state -- do it manually. Items with subgroups
                // but no children have only hidden children -- those get
                // handled specially.
                if ( item->childCount() > 0 )
                {
                    item->updateSelected();
                }
            }
            else
            {
                if ( !haveWarned )
                {
                    cWarning() << "*subgroups* list under" << item->name() << "is empty.";
                }
            }
        }
        if ( item->isHidden() )
        {
            m_hiddenItems.append( item );
            if ( !item->isSelected() )
            {
                cWarning() << "Item" << ( item->parentItem() ? item->parentItem()->name() : QString() ) << '.'
                           << item->name() << "is hidden, but not selected.";
            }
        }
        else
        {
            item->setCheckable( true );
            parent->appendChild( item );
        }
    }
}

void
PackageModel::setupModelData( const QVariantList& l )
{
    beginResetModel();
    delete m_rootItem;
    m_rootItem = new PackageTreeItem();
    setupModelData( l, m_rootItem );
    endResetModel();
}

void
PackageModel::appendModelData( const QVariantList& groupList )
{
    if ( m_rootItem )
    {
        beginResetModel();

        const QStringList sources = collectSources( groupList );

        if ( !sources.isEmpty() )
        {
            // Prune any existing data from the same source
            QList< int > removeList;
            for ( int i = 0; i < m_rootItem->childCount(); i++ )
            {
                PackageTreeItem* child = m_rootItem->child( i );
                if ( sources.contains( child->source() ) )
                {
                    removeList.insert( 0, i );
                }
            }
            for ( const int& item : qAsConst( removeList ) )
            {
                m_rootItem->removeChild( item );
            }
        }

        // Add the new data to the model
        setupModelData( groupList, m_rootItem );

        endResetModel();
    }
}
