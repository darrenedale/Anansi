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

#ifndef ANANSI_IDENTITYCONTENTENCODER_H
#define ANANSI_IDENTITYCONTENTENCODER_H

#include "contentencoder.h"

namespace Anansi {

	class IdentityContentEncoder : public ContentEncoder {
	public:
		IdentityContentEncoder() = default;

		virtual bool encodeTo(QIODevice &, const QByteArray & data) override;
	};

}  // namespace Anansi

#endif  // ANANSI_IDENTITYCONTENTENCODER_H
