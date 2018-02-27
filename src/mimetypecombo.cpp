#include "mimetypecombo.h"
#include "connectionpolicycombo.h"

#include <regex>
#include <iostream>

#include <QValidator>

#include "configuration.h"


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy)


namespace EquitWebServer {


	static constexpr const int MimeTypeRole = Qt::UserRole + 9814;
#define RFC_2045_TOKEN_CHAR "[^[:^ascii:][:cntrl:] ()<>@,;:\\\"/\\[\\]?=]"
#define RFC_2045_TOKEN RFC_2045_TOKEN_CHAR "+"
#define RFC_822_QUOTED_STRING "\"(?:\\\\[[:ascii:]]|[^[:^ascii:]\"\\\\\\n])*\""
	static constexpr const char * MimeTypePattern = "^(?:|(?:[a-z]+|x-" RFC_2045_TOKEN ")/(?:(" RFC_2045_TOKEN ")( *; *" RFC_2045_TOKEN " *= *(?:" RFC_2045_TOKEN "|" RFC_822_QUOTED_STRING "))*))$";

	template<class T, class CharT = typename T::value_type>
	static const auto MimeTypeRegex = std::basic_regex<CharT>(MimeTypePattern);

	template<class T>
	static constexpr bool isValidMimeType(const T & mime) {
		return std::regex_match(mime.cbegin(), mime.cend(), MimeTypeRegex<T>);
	}


	template<>
	bool isValidMimeType<QString>(const QString & mime) {
		static const QRegularExpression rx(MimeTypePattern);
		return rx.match(mime).hasMatch();
	}


	struct MimeTypeValidator : public QValidator {
		explicit MimeTypeValidator(QObject * parent = nullptr)
		: QValidator(parent) {
		}

		virtual State validate(QString &, int &) const override;
	};


	QValidator::State MimeTypeValidator::validate(QString & input, int &) const {
		static const QRegularExpression rx(MimeTypePattern);
		const auto match = rx.match(input, 0, QRegularExpression::PartialPreferCompleteMatch);

		if(match.hasMatch()) {
			std::cout << "mime type content acceptable\n"
						 << std::flush;
			return Acceptable;
		}

		if(match.hasPartialMatch()) {
			std::cout << "mime type content POSSIBLY acceptable\n"
						 << std::flush;
			return Intermediate;
		}

		std::cout << "mime type content invalid\n"
					 << std::flush;
		return Invalid;
	}


	MimeTypeCombo::MimeTypeCombo(QWidget * parent)
	: MimeTypeCombo(false, parent) {
	}


	MimeTypeCombo::MimeTypeCombo(bool allowCustom, QWidget * parent)
	: QComboBox(parent) {
		setDuplicatesEnabled(false);
		setCustomMimeTypesAllowed(allowCustom);
		setValidator(new MimeTypeValidator(this));
		setInsertPolicy(InsertAlphabetically);

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: emitting currentMimeTypeChanged(\"" << qPrintable(currentMimeType()) << "\")\n"
						 << std::flush;
			Q_EMIT currentMimeTypeChanged(currentMimeType());
		});

		connect(this, &QComboBox::currentTextChanged, this, &MimeTypeCombo::currentMimeTypeChanged);
	}


	std::vector<QString> MimeTypeCombo::availableMimeTypes() const {
		std::vector<QString> ret;
		int n = count();

		for(int i = 0; i < n; ++i) {
			ret.push_back(itemData(i).value<QString>());
		}

		return ret;
	}


	QString MimeTypeCombo::currentMimeType() const {
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
		setCurrentText(type);
	}


}  // namespace EquitWebServer
