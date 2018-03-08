#include "display_strings.h"

#include <QApplication>


namespace Anansi {


	QString connectionPolicyDisplayString(Anansi::ConnectionPolicy policy) {
		switch(policy) {
			case ConnectionPolicy::None:
				return QApplication::tr("No Policy");

			case ConnectionPolicy::Accept:
				return QApplication::tr("Accept Connection");

			case ConnectionPolicy::Reject:
				return QApplication::tr("Reject Connection");
		}

		return {};
	}


}  // namespace Anansi
