/// \file directorysortordercombo.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the DirectorySortOrderCombo class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_DIRECTORYSORTORDERCOMBO_H
#define ANANSI_DIRECTORYSORTORDERCOMBO_H

#include <QComboBox>

#include "configuration.h"

namespace Anansi {

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

}  // namespace Anansi

#endif  // ANANSI_DIRECTORYSORTORDERCOMBO_H
