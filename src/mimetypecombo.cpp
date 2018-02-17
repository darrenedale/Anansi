#include "mimetypecombo.h"
#include "connectionpolicycombo.h"

#include <regex>

#include "configuration.h"


Q_DECLARE_METATYPE(EquitWebServer::Configuration::ConnectionPolicy)


namespace EquitWebServer {


	static constexpr const int MimeTypeRole = Qt::UserRole + 9814;
#define RFC_2045_TOKEN "[^[:^ascii:][:cntrl:] ()<>@,;:\\\"/\\[\\]?=]+"
#define RFC_822_QUOTED_STRING "\"(?:\\\\[[:ascii:]]|[^[:^ascii:]\"\\\\\\n])*\""
	static constexpr const char * MimeTypePattern = "^([a-z]+|x-" RFC_2045_TOKEN ")/(?:(" RFC_2045_TOKEN ")( *; *" RFC_2045_TOKEN " *= *(?:" RFC_2045_TOKEN "|" RFC_822_QUOTED_STRING "))*)$";

	template<class T, class CharT = typename T::value_type>
	static const auto MimeTypeRegex = std::basic_regex<CharT>(MimeTypePattern);

	template<class T>
	static constexpr bool isValidMimeType(const T & mime) {
		return std::regex_match(mime.cbegin(), mime.cend(), MimeTypeRegex<T>);
	}


	template<>
	bool isValidMimeType<QString>(const QString & mime) {
		return QRegularExpression(MimeTypePattern).match(mime).hasMatch();
	}


	MimeTypeCombo::MimeTypeCombo(QWidget * parent)
	: MimeTypeCombo(false, parent) {
	}


	MimeTypeCombo::MimeTypeCombo(bool allowCustom, QWidget * parent)
	: QComboBox(parent) {
		setCustomMimeTypesAllowed(allowCustom);

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT currentMimeTypeChanged(currentMimeType());
		});

		connect(this, &QComboBox::editTextChanged, this, &MimeTypeCombo::currentMimeTypeChanged);
	}


	QString MimeTypeCombo::currentMimeType() {
		if(customMimeTypesAllowed()) {
			return currentText();
		}

		return currentData().value<QString>();
	}


	bool MimeTypeCombo::hasMimeType(const QString & mime) const {
		return -1 != findData(mime, MimeTypeRole);
	}


	bool MimeTypeCombo::addMimeType(const QString & mime) {
		if(hasMimeType(mime)) {
			return true;
		}

		if(!isValidMimeType(mime)) {
			return false;
		}

		auto iconName = mime;
		iconName.replace('/', '-');
		QComboBox::addItem(QIcon::fromTheme(iconName), mime, mime);
		Q_EMIT mimeTypeAdded(mime);
		return true;
	}


	void MimeTypeCombo::removeMimeType(const QString & mime) {
		auto idx = findData(mime, MimeTypeRole);

		if(-1 == idx) {
			return;
		}

		QComboBox::removeItem(idx);
		Q_EMIT mimeTypeRemoved(mime);
	}


	void MimeTypeCombo::setCurrentMimeType(const QString & type) {
		if(customMimeTypesAllowed()) {
			setCurrentText(type);
		}

		setCurrentIndex(findData(QVariant::fromValue(type)));
	}


}  // namespace EquitWebServer
