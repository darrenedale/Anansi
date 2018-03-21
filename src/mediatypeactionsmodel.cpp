/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Anansi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Anansi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file mediatypeactionsmodel.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MediaTypeActionsModel class.
///
/// \dep
/// - mediatypeactionsmodel.h
/// - <cstddef>
/// - <algorithm>
/// - <iostream>
/// - assert.h
/// - types.h
/// - qtmetatypes.h
/// - numerics.h
/// - display_strings.h
/// - mediatypeicons.h
/// - server.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mediatypeactionsmodel.h"

#include <cstddef>
#include <algorithm>
#include <iostream>

#include "assert.h"
#include "types.h"
#include "qtmetatypes.h"
#include "numerics.h"
#include "display_strings.h"
#include "mediatypeicons.h"
#include "server.h"


namespace Anansi {


	using Equit::max;


	template<int ColumnIndex>
	QModelIndex MediaTypeActionsModel::findHelper(const QString & mediaType) const {
		const auto mediaTypes = m_server->configuration().registeredMediaTypes();
		const auto & begin = mediaTypes.cbegin();
		const auto & end = mediaTypes.cend();
		const auto mediaTypeIt = std::find(begin, end, mediaType);

		if(end == mediaTypeIt) {
			return {};
		}

		return createIndex(static_cast<int>(std::distance(begin, mediaTypeIt)), ColumnIndex);
	}


	MediaTypeActionsModel::MediaTypeActionsModel(Server * server, QObject * parent)
	: QAbstractItemModel(parent),
	  m_server(server) {
		eqAssert(m_server, "server to observe must not be null");
	}


	QModelIndex MediaTypeActionsModel::findMediaType(const QString & mediaType) const {
		return findHelper<MediaTypeColumnIndex>(mediaType);
	}


	QModelIndex MediaTypeActionsModel::findMediaTypeAction(const QString & mediaType) const {
		return findHelper<ActionColumnIndex>(mediaType);
	}


	QModelIndex MediaTypeActionsModel::findMediaTypeCgi(const QString & mediaType) const {
		return findHelper<CgiColumnIndex>(mediaType);
	}


	int MediaTypeActionsModel::rowCount(const QModelIndex &) const {
		return m_server->configuration().registeredMediaTypeCount();
	}


	int MediaTypeActionsModel::columnCount(const QModelIndex &) const {
		return 1 + max<int, MediaTypeColumnIndex, ActionColumnIndex, CgiColumnIndex>();
	}


	QVariant MediaTypeActionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if(Qt::DisplayRole != role) {
			return QAbstractItemModel::headerData(section, orientation, role);
		}

		switch(section) {
			case MediaTypeColumnIndex:
				return tr("Media type");

			case ActionColumnIndex:
				return tr("Action");

			case CgiColumnIndex:
				return tr("CGI executable");
		}

		return {};
	}


	QModelIndex MediaTypeActionsModel::index(int row, int column, const QModelIndex &) const {
		if(0 > column || max<int, MediaTypeColumnIndex, ActionColumnIndex, CgiColumnIndex>() < column) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: invalid column (" << column << ")\n";
			return {};
		}

		if(0 > row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: invalid row (" << row << ")\n";
			return {};
		}

		// for anything else, return a top-level extension item index
		if(rowCount() <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: row (" << row << ") for item index is out of bounds\n";
			return {};
		}

		return createIndex(row, column);
	}


	QModelIndex MediaTypeActionsModel::parent(const QModelIndex &) const {
		return {};
	}


	QVariant MediaTypeActionsModel::data(const QModelIndex & idx, int role) const {
		if(Qt::DisplayRole != role && Qt::EditRole != role && Qt::DecorationRole != role) {
			return {};
		}

		if(!idx.isValid()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		auto row = idx.row();

		if(0 > row || rowCount() <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: index is not valid\n";
			return {};
		}

		const auto & config = m_server->configuration();
		const auto mediaTypes = config.registeredMediaTypes();
		const auto & mediaType = mediaTypes[static_cast<std::size_t>(row)];

		switch(idx.column()) {
			case MediaTypeColumnIndex:
				if(Qt::DecorationRole == role) {
					return mediaTypeIcon(mediaType);
				}

				return mediaType;
				break;

			case ActionColumnIndex:
				switch(role) {
					case Qt::DecorationRole:
						switch(config.mediaTypeAction(mediaType)) {
							case WebServerAction::Ignore:
								return QIcon::fromTheme(QStringLiteral("trash-empty"), QIcon(QStringLiteral(":/icons/webserveractions/ignore")));

							case WebServerAction::Serve:
								return QIcon::fromTheme(QStringLiteral("dialog-ok"), QIcon(QStringLiteral(":/icons/webserveractions/serve")));

							case WebServerAction::CGI:
								return QIcon::fromTheme(QStringLiteral("system-run"), QIcon(QStringLiteral(":/icons/webserveractions/cgi")));

							case WebServerAction::Forbid:
								return QIcon::fromTheme(QStringLiteral("error"), QIcon(QStringLiteral(":/icons/webserveractions/forbid")));
						}
						break;

					case Qt::DisplayRole:
						return displayString(config.mediaTypeAction(mediaType));

					case Qt::EditRole:
						return QVariant::fromValue(config.mediaTypeAction(mediaType));
				}
				break;

			case CgiColumnIndex: {
				if(WebServerAction::CGI == config.mediaTypeAction(mediaType)) {
					return config.mediaTypeCgi(mediaType);
				}
				break;
			}
		}

		return {};
	}


	Qt::ItemFlags MediaTypeActionsModel::flags(const QModelIndex & idx) const {
		auto ret = QAbstractItemModel::flags(idx);

		if(!idx.isValid()) {
			return ret;
		}

		ret |= Qt::ItemNeverHasChildren;

		switch(idx.column()) {
			case ActionColumnIndex:
				ret |= Qt::ItemIsEditable;
				break;

			case CgiColumnIndex: {
				const auto & config = m_server->configuration();
				const auto mediaTypes = config.registeredMediaTypes();
				const auto row = idx.row();

				if(0 <= row && static_cast<int>(mediaTypes.size()) > row && WebServerAction::CGI == config.mediaTypeAction(mediaTypes[static_cast<std::size_t>(row)])) {
					ret |= Qt::ItemIsEditable;
				}

				break;
			}
		}

		return ret;
	}


	bool MediaTypeActionsModel::setData(const QModelIndex & idx, const QVariant & value, int role) {
		if(!idx.isValid()) {
			return false;
		}

		if(Qt::EditRole != role) {
			return QAbstractItemModel::setData(idx, value, role);
		}

		auto row = idx.row();

		if(0 > row || rowCount() <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __FILE__ << "]: invalid index - row " << row << " does not exist\n";
			return false;
		}

		auto & config = m_server->configuration();

		switch(idx.column()) {
			case MediaTypeColumnIndex:
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __FILE__ << "]: can't change the media type for an action\n";
				return false;

			case ActionColumnIndex: {
				const auto mediaType = config.registeredMediaTypes()[static_cast<std::size_t>(row)];
				const auto oldAction = config.mediaTypeAction(mediaType);
				const auto newAction = value.value<WebServerAction>();

				if(newAction == oldAction) {
					// no change
					return true;
				}

				if(!config.setMediaTypeAction(mediaType, newAction)) {
					std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to set action for \"" << qPrintable(mediaType) << "\"\n";
					return false;
				}

				Q_EMIT actionChanged(mediaType, newAction);
				return true;
			}

			case CgiColumnIndex: {
				const auto mediaType = config.registeredMediaTypes()[static_cast<std::size_t>(row)];
				const auto oldCgi = config.mediaTypeCgi(mediaType);
				const auto newCgi = value.value<QString>();

				if(oldCgi == newCgi) {
					// no change
					return true;
				}

				config.setMediaTypeCgi(mediaType, newCgi);
				Q_EMIT cgiChanged(mediaType, newCgi);
				return true;
			}
		}

		return QAbstractItemModel::setData(idx, value, role);
	}


	QModelIndex MediaTypeActionsModel::addMediaType(QString mediaType, WebServerAction action, const QString & cgi) {
		auto & config = m_server->configuration();

		if(mediaType.isEmpty()) {
			mediaType = QStringLiteral("application/x-subtype");

			if(config.mediaTypeIsRegistered(mediaType)) {
				int idx = 1;

				do {
					++idx;
					mediaType = QStringLiteral("application/x-subtype-%1").arg(idx);
				} while(config.mediaTypeIsRegistered(mediaType));
			}
		}
		else if(config.mediaTypeIsRegistered(mediaType)) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: media type \"" << qPrintable(mediaType) << "\" already exists\n";
			return {};
		}

		if(!config.setMediaTypeAction(mediaType, action)) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to set action " << enumeratorString<std::string>(action) << " for media type \"" << qPrintable(mediaType) << "\"\n";
			return {};
		}

		if(WebServerAction::CGI == action) {
			config.setMediaTypeCgi(mediaType, cgi);
		}
		else if(!cgi.isEmpty()) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: received CGI \"" << qPrintable(cgi) << "\" for media type \"" << qPrintable(mediaType) << "\" but its action was not WebServerAction::CGI\n";
		}

		beginResetModel();
		endResetModel();
		return findMediaTypeAction(mediaType);
	}


	bool MediaTypeActionsModel::removeRows(int row, int count, const QModelIndex & parent) {
		if(1 > count) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: count of items to remove must be > 0\n";
			return false;
		}

		auto & config = m_server->configuration();
		const int mediaTypeCount = config.registeredMediaTypeCount();

		if(0 > row || mediaTypeCount <= row) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: first row to remove out of bounds: " << row << "\n";
			return false;
		}

		int endRow = row + count - 1;

		if(mediaTypeCount <= endRow) {
			std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: last row to remove out of bounds: " << endRow << "\n";
			return false;
		}

		beginRemoveRows(parent, row, endRow);
		const auto mediaTypes = config.registeredMediaTypes();
		auto begin = mediaTypes.cbegin() + row;

		std::for_each(begin, begin + count, [&config](const auto & mediaType) {
			config.unsetMediaTypeAction(mediaType);
		});

		endRemoveRows();
		return true;
	}


	void MediaTypeActionsModel::clear() {
		beginResetModel();
		m_server->configuration().clearAllMediaTypeActions();
		endResetModel();
	}


}  // namespace Anansi
