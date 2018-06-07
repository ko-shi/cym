#ifndef CYM_ERROR_MESSAGE_HPP
#define CYM_ERROR_MESSAGE_HPP

#include<cym/CymBase.hpp>
#include<cym/utils/string/CymStringConverter.hpp>

namespace cym {

	struct ErrorMessage {
		enum Kind {
			UNKNOWN_VARIABLE,
			CONCECTIVE_INFIXES,
			CONCECTIVE_NONINFIXES,
			ENDED_WITH_INFIX,
			BEGUN_WITH_INFIX
		};
		const Kind kind;
		const Size line;
		const Size pos;
		const StrView point;
		const Str message;
		ErrorMessage(Kind k,Size l,Size n, StrView s,const Str &m = Str(u"")) : kind(k),line(l),pos(n),point(s),message(m) {

		}
		Str getKind()const {
			const Str table[] = {
				u"Unknown variable",
				u"Concective infixes",
				u"Concective non-infixes",
				u"Ended with infix",
				u"Begun with infix"
			};
			return table[kind];
		}
		Str str()const {
			return Str(u"kind = ") + getKind() +
				Str(u"\nline = ") + toU16String(line) +
				Str(u"\npos = ") + toU16String(pos) +
				Str(u"\npoint = ") + Str(point) +
				Str(u"\nmessage = ") + (message.empty() ? u"null" : message);
		}
	};
}


#endif