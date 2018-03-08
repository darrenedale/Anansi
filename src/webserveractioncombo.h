/// \file webserveractioncombo.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the WebServerActionCombo class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_WEBSERVERACTIONCOMBO_H
#define ANANSI_WEBSERVERACTIONCOMBO_H

#include <QComboBox>

#include "configuration.h"

namespace Anansi {

	class WebServerActionCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit WebServerActionCombo(QWidget * parent = nullptr);
		virtual ~WebServerActionCombo() = default;

		void addItem() = delete;

		WebServerAction webServerAction();

	public Q_SLOTS:
		void setWebServerAction(WebServerAction action);

	Q_SIGNALS:
		void webServerActionChanged(WebServerAction);
	};

}  // namespace Anansi

#endif  // ANANSI_WEBSERVERACTIONCOMBO_H
