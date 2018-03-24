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

/// \file mediatypecombo.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MediaTypeCombo class for Anansi.
///
/// \dep
/// - mediatypecombo.h
/// - <regex>
/// - <QValidator>
/// - <QRegularExpression>
/// - mediatypeicons.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mediatypecombo.h"

#include <regex>

#include <QValidator>
#include <QRegularExpression>

#include "mediatypeicons.h"


namespace Anansi {


	static constexpr const int MediaTypeRole = Qt::UserRole + 9814;

// this is a complicated regex with repetition. breaking it down into components
// makes it easier to maintain. using the preprocessor concatenates the strings
// in the pattern at compile time
#define RFC_2045_TOKEN_CHAR "[^[:^ascii:][:cntrl:] ()<>@,;:\\\"/\\[\\]?=]"
#define RFC_2045_TOKEN RFC_2045_TOKEN_CHAR "+"
#define RFC_822_QUOTED_STRING "\"(?:\\\\[[:ascii:]]|[^[:^ascii:]\"\\\\\\n])*\""
	static constexpr const char * MediaTypePattern = "^(?:|(?:[a-z]+|x-" RFC_2045_TOKEN ")/(?:(" RFC_2045_TOKEN ")( *; *" RFC_2045_TOKEN " *= *(?:" RFC_2045_TOKEN "|" RFC_822_QUOTED_STRING "))*))$";
#undef RFC_2045_TOKEN_CHAR
#undef RFC_2045_TOKEN
#undef RFC_822_QUOTED_STRING


	template<class T, class CharT = typename T::value_type>
	static const auto MediaTypeRegex = std::basic_regex<CharT>(MediaTypePattern);


	template<class T>
	static constexpr bool isValidMediaType(const T & mediaType) {
		return std::regex_match(mediaType.cbegin(), mediaType.cend(), MediaTypeRegex<T>);
	}


	template<>
	bool isValidMediaType<QString>(const QString & mediaType) {
		static const QRegularExpression mediaTypeRx(MediaTypePattern);
		return mediaTypeRx.match(mediaType).hasMatch();
	}


	struct MediaTypeValidator : public QValidator {
		explicit MediaTypeValidator(QObject * parent = nullptr)
		: QValidator(parent) {
		}

		virtual State validate(QString &, int &) const override;
	};


	QValidator::State MediaTypeValidator::validate(QString & input, int &) const {
		static const QRegularExpression rx(MediaTypePattern);
		const auto match = rx.match(input, 0, QRegularExpression::PartialPreferCompleteMatch);

		if(match.hasMatch()) {
			return Acceptable;
		}

		if(match.hasPartialMatch()) {
			return Intermediate;
		}

		return Invalid;
	}


	MediaTypeCombo::MediaTypeCombo(bool allowCustom, QWidget * parent)
	: QComboBox(parent) {
		setDuplicatesEnabled(false);
		setCustomMediaTypesAllowed(allowCustom);
		setValidator(new MediaTypeValidator(this));
		setInsertPolicy(InsertAlphabetically);

        // can't use qOverload() with MSVC because it doesn't implement SD-6 (feature
        // detection macros)
        connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT currentMediaTypeChanged(currentMediaType());
		});

		connect(this, &QComboBox::currentTextChanged, this, &MediaTypeCombo::currentMediaTypeChanged);
	}


	MediaTypeCombo::MediaTypeCombo(QWidget * parent)
	: MediaTypeCombo(false, parent) {
	}


	std::vector<QString> MediaTypeCombo::availableMediaTypes() const {
		std::vector<QString> ret;
		int n = count();

		for(int i = 0; i < n; ++i) {
			ret.push_back(itemData(i).value<QString>());
		}

		return ret;
	}


	QString MediaTypeCombo::currentMediaType() const {
		if(customMediaTypesAllowed()) {
			return currentText();
		}

		return currentData().value<QString>();
	}


	void MediaTypeCombo::setCurrentMediaType(const QString & mediaType) {
		setCurrentText(mediaType);
	}


	bool MediaTypeCombo::hasMediaType(const QString & mediaType) const {
		return -1 != findData(mediaType, MediaTypeRole);
	}


	bool MediaTypeCombo::addMediaType(const QString & mediaType) {
		if(hasMediaType(mediaType)) {
			return true;
		}

		if(!isValidMediaType(mediaType)) {
			return false;
		}

		QComboBox::addItem(mediaTypeIcon(mediaType), mediaType, mediaType);
		Q_EMIT mediaTypeAdded(mediaType);
		return true;
	}


	void MediaTypeCombo::removeMediaType(const QString & mediaType) {
		auto idx = findData(mediaType, MediaTypeRole);

		if(-1 == idx) {
			return;
		}

		QComboBox::removeItem(idx);
		Q_EMIT mediaTypeRemoved(mediaType);
	}


}  // namespace Anansi
