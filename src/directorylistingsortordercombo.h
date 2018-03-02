/// \file directorysortordercombo.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the DirectorySortOrderCombo class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_DIRECTORYSORTORDERCOMBO_H
#define EQUITWEBSERVER_DIRECTORYSORTORDERCOMBO_H

#include <QComboBox>

#include "configuration.h"

namespace EquitWebServer {

	class DirectoryListingSortOrderCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit DirectoryListingSortOrderCombo(QWidget * parent = nullptr);
		virtual ~DirectoryListingSortOrderCombo() = default;

		void addItem() = delete;

		DirectoryListingSortOrder sortOrder();

	public Q_SLOTS:
		void setSortOrder(DirectoryListingSortOrder order);

	Q_SIGNALS:
		void sortOrderChanged(DirectoryListingSortOrder);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_DIRECTORYSORTORDERCOMBO_H
