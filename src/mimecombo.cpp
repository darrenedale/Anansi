/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of EquitWebServer.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EquitWebServer. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file mimecombo.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the MimeCombo class for EquitWebServer
///
/// \dep
/// - <regex>
/// - <iostream>
/// - <QValidator>
/// - mimeicons.h
/// - configuration.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mimecombo.h"
#include "connectionpolicycombo.h"

#include <regex>
#include <iostream>

#include <QValidator>

#include "mimeicons.h"
#include "configuration.h"


namespace EquitWebServer {


	static constexpr const int MimeTypeRole = Qt::UserRole + 9814;

// this is a complicated regex with repetition. breaking it down into components
// makes it easier to maintain. using the preprocessor concatenates the strings
// in the pattern at compile time
#define RFC_2045_TOKEN_CHAR "[^[:^ascii:][:cntrl:] ()<>@,;:\\\"/\\[\\]?=]"
#define RFC_2045_TOKEN RFC_2045_TOKEN_CHAR "+"
#define RFC_822_QUOTED_STRING "\"(?:\\\\[[:ascii:]]|[^[:^ascii:]\"\\\\\\n])*\""
	static constexpr const char * MimeTypePattern = "^(?:|(?:[a-z]+|x-" RFC_2045_TOKEN ")/(?:(" RFC_2045_TOKEN ")( *; *" RFC_2045_TOKEN " *= *(?:" RFC_2045_TOKEN "|" RFC_822_QUOTED_STRING "))*))$";
#undef RFC_2045_TOKEN_CHAR
#undef RFC_2045_TOKEN
#undef RFC_822_QUOTED_STRING


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
			return Acceptable;
		}

		if(match.hasPartialMatch()) {
			return Intermediate;
		}

		return Invalid;
	}


	MimeCombo::MimeCombo(QWidget * parent)
	: MimeCombo(false, parent) {
	}


	MimeCombo::MimeCombo(bool allowCustom, QWidget * parent)
	: QComboBox(parent) {
		setDuplicatesEnabled(false);
		setCustomMimeTypesAllowed(allowCustom);
		setValidator(new MimeTypeValidator(this));
		setInsertPolicy(InsertAlphabetically);

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT currentMimeTypeChanged(currentMimeType());
		});

		connect(this, &QComboBox::currentTextChanged, this, &MimeCombo::currentMimeTypeChanged);
	}


	std::vector<QString> MimeCombo::availableMimeTypes() const {
		std::vector<QString> ret;
		int n = count();

		for(int i = 0; i < n; ++i) {
			ret.push_back(itemData(i).value<QString>());
		}

		return ret;
	}


	QString MimeCombo::currentMimeType() const {
		if(customMimeTypesAllowed()) {
			return currentText();
		}

		return currentData().value<QString>();
	}


	bool MimeCombo::hasMimeType(const QString & mime) const {
		return -1 != findData(mime, MimeTypeRole);
	}


	bool MimeCombo::addMimeType(const QString & mime) {
		if(hasMimeType(mime)) {
			return true;
		}

		if(!isValidMimeType(mime)) {
			return false;
		}

		QComboBox::addItem(mimeIcon(mime), mime, mime);
		Q_EMIT mimeTypeAdded(mime);
		return true;
	}


	void MimeCombo::removeMimeType(const QString & mime) {
		auto idx = findData(mime, MimeTypeRole);

		if(-1 == idx) {
			return;
		}

		QComboBox::removeItem(idx);
		Q_EMIT mimeTypeRemoved(mime);
	}


	void MimeCombo::setCurrentMimeType(const QString & type) {
		setCurrentText(type);
	}


}  // namespace EquitWebServer
